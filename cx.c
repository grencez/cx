/**
 * \file cx.c
 * C code transformation utility.
 **/
#include "syscx.h"
#include "associa.h"
#include "fileb.h"
#include "sxpn.h"
#include "table.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct AST AST;
typedef struct ASTree ASTree;
typedef struct CxCtx CxCtx;

typedef
enum SyntaxKind
{   Syntax_WS
    ,Syntax_Iden
    ,Syntax_LineComment
    ,Syntax_BlockComment
    ,Syntax_Directive
    ,Syntax_CharLit
    ,Syntax_StringLit
    ,Syntax_Parens
    ,Syntax_Braces
    ,Syntax_Brackets
    ,Syntax_Stmt

    ,Syntax_ForLoop
    ,Syntax_WhileLoop
    ,Syntax_If
    ,Syntax_Else

    /* TODO */
    ,Syntax_Struct
    ,Syntax_Typedef

    ,Beg_Syntax_LexWords
    ,Lexical_Struct = Beg_Syntax_LexWords
    ,Lexical_Enum
    ,Lexical_Typedef
    ,Lexical_Union

    ,Lexical_Sizeof
    ,Lexical_Offsetof
    ,Lexical_For
    ,Lexical_If
    ,Lexical_Else
    ,Lexical_While
    ,Lexical_Do
    ,Lexical_Continue
    ,Lexical_Switch
    ,Lexical_Break
    ,Lexical_Case
    ,Lexical_Default
    ,Lexical_Return

    ,Lexical_Extern
    ,Lexical_Goto

    ,Lexical_Auto
    ,Lexical_Restrict
    ,Lexical_Register

    ,Lexical_Const
    ,Lexical_Static
    ,Lexical_Volatile
    ,Lexical_Inline

    ,Lexical_Void
    ,Lexical_Unsigned
    ,Lexical_Signed
    ,Lexical_Char
    ,Lexical_Short
    ,Lexical_Int
    ,Lexical_Long
    ,Lexical_Float
    ,Lexical_Double
    ,End_Syntax_LexWords

    ,Beg_Syntax_LexOps = End_Syntax_LexWords
    ,Lexical_Add = Beg_Syntax_LexOps
    ,Lexical_Inc
    ,Lexical_Sub
    ,Lexical_Dec
    ,Lexical_Mul
    ,Lexical_Div
    ,Lexical_Mod
    ,Lexical_BitAnd
    ,Lexical_And
    ,Lexical_BitXor
    ,Lexical_BitOr
    ,Lexical_Or
    ,Lexical_BitNot
    ,Lexical_Not
    ,Lexical_Dot
    ,Lexical_Comma
    ,Lexical_Question
    ,Lexical_Colon
    ,Lexical_Semicolon
    ,Lexical_GT
    ,Lexical_PMemb
    ,Lexical_RShift
    ,Lexical_LT
    ,Lexical_LShift
    ,Lexical_Assign
    ,Lexical_AddAssign
    ,Lexical_SubAssign
    ,Lexical_MulAssign
    ,Lexical_DivAssign
    ,Lexical_ModAssign
    ,Lexical_BitAndAssign
    ,Lexical_BitXorAssign
    ,Lexical_BitOrAssign
    ,Lexical_NotEq
    ,Lexical_GTEq
    ,Lexical_RShiftAssign
    ,Lexical_LTEq
    ,Lexical_LShiftAssign
    ,Lexical_Eq
    ,End_Syntax_LexOps

    ,NSyntaxKinds = End_Syntax_LexOps
} SyntaxKind;

struct AST
{
    SyntaxKind kind;
    ujint line;
    AlphaTab txt;
    Cons* cons;
};

struct ASTree
{
    LgTable lgt;
    Cons* head;
    Sxpn sx;
};

/** Unused.**/
struct CxCtx
{
    ASTree ast;
    Associa type_lookup;
};


    AST
dflt_AST ()
{
    AST ast;
    ast.kind = NSyntaxKinds;
    ast.line = 0;
    ast.txt = dflt_AlphaTab ();
    ast.cons = 0;
    return ast;
}

    AST*
take_ASTree (ASTree* t)
{
    AST* ast = (AST*) take_LgTable (&t->lgt);
    *ast = dflt_AST ();
    ast->cons = take_Sxpn (&t->sx);
    /* InitDomMax( ast->cons->nrefs ); */
    ast->cons->car.kind = Cons_MemLoc;
    ast->cons->car.as.memloc = ast;
    return ast;
}

    AST*
take1_ASTree (ASTree* t, SyntaxKind kind)
{
    AST* ast = take_ASTree (t);
    ast->kind = kind;
    return ast;
}

    void
give_ASTree (ASTree* t, AST* ast)
{
    LoseTable( ast->txt );
    give_LgTable (&t->lgt, ast);
    /* ast->cons->nrefs = 0; */
    if (ast->cons->car.kind == Cons_Cons)
        ast->cons->car.as.cons = 0;
    ast->cons->cdr = 0;
    give_Sxpn (&t->sx, ast->cons);
}

    ASTree
cons_ASTree ()
{
    ASTree t;
    t.lgt = dflt1_LgTable (sizeof (AST));
    t.sx = dflt_Sxpn ();
    t.head = 0;
    return t;
}

    void
lose_ASTree (ASTree* t)
{
    for (ujint i = begidx_LgTable (&t->lgt);
         i < Max_ujint;
         i = nextidx_LgTable (&t->lgt, i))
    {
        AST* ast = (AST*) elt_LgTable (&t->lgt, i);
        lose_AlphaTab (&ast->txt);
    }
    lose_LgTable (&t->lgt);
    lose_Sxpn (&t->sx);
}

    AST*
AST_of_Cons (Cons* c)
{
    if (!c)  return 0;
    if (c->car.kind == Cons_Cons)
    {
        c = c->car.as.cons;
        Claim( c );
    }
    Claim2( c->car.kind ,==, Cons_MemLoc );
    return (AST*) c->car.as.memloc;
}

/** Get (car ast) as an AST, which is mostly useless,
 * as it holds a pointer to the AST.
 **/
    AST*
car_of_AST (AST* ast)
{
    Cons* c = ast->cons;
    Claim2( Cons_Cons ,==, c->car.kind );
    c = c->car.as.cons;
    if (!c)  return 0;
    return AST_of_Cons (c);
}

    AST*
cdar_of_AST (AST* ast)
{
    Cons* c = ast->cons;

    Claim2( Cons_Cons ,==, c->car.kind );
    c = c->car.as.cons;
    Claim( c );
    c = c->cdr;
    if (!c)  return 0;
    return AST_of_Cons (c);
}

    AST*
cdr_of_AST (AST* ast)
{
    Cons* c = ast->cons->cdr;

    if (!c)  return 0;
    return AST_of_Cons (c);
}

    const char*
cstr_SyntaxKind (SyntaxKind kind)
{
    switch (kind)
    {
    case Lexical_Struct   : return "struct"  ;
    case Lexical_Enum     : return "enum"    ;
    case Lexical_Typedef  : return "typedef" ;
    case Lexical_Union    : return "union"   ;
    case Lexical_Sizeof   : return "sizeof"  ;
    case Lexical_Offsetof : return "offsetof";
    case Lexical_For      : return "for"     ;
    case Lexical_If       : return "if"      ;
    case Lexical_Else     : return "else"    ;
    case Lexical_While    : return "while"   ;
    case Lexical_Do       : return "do"      ;
    case Lexical_Continue : return "continue";
    case Lexical_Switch   : return "switch"  ;
    case Lexical_Break    : return "break"   ;
    case Lexical_Case     : return "case"    ;
    case Lexical_Default  : return "default" ;
    case Lexical_Return   : return "return"  ;
    case Lexical_Extern   : return "extern"  ;
    case Lexical_Goto     : return "goto"    ;
    case Lexical_Auto     : return "auto"    ;
    case Lexical_Restrict : return "restrict";
    case Lexical_Register : return "register";
    case Lexical_Const    : return "const"   ;
    case Lexical_Static   : return "static"  ;
    case Lexical_Volatile : return "volatile";
    case Lexical_Inline   : return "inline"  ;
    case Lexical_Void     : return "void"    ;
    case Lexical_Unsigned : return "unsigned";
    case Lexical_Signed   : return "signed"  ;
    case Lexical_Char     : return "char"    ;
    case Lexical_Short    : return "short"   ;
    case Lexical_Int      : return "int"     ;
    case Lexical_Long     : return "long"    ;
    case Lexical_Float    : return "float"   ;
    case Lexical_Double   : return "double"  ;

    case Lexical_Add         : return "+"  ;
    case Lexical_Inc         : return "++" ;
    case Lexical_Sub         : return "-"  ;
    case Lexical_Dec         : return "--" ;
    case Lexical_Mul         : return "*"  ;
    case Lexical_Div         : return "/"  ;
    case Lexical_Mod         : return "%"  ;
    case Lexical_BitAnd      : return "&"  ;
    case Lexical_And         : return "&&" ;
    case Lexical_BitXor      : return "^"  ;
    case Lexical_BitOr       : return "|"  ;
    case Lexical_Or          : return "||" ;
    case Lexical_BitNot      : return "~"  ;
    case Lexical_Not         : return "!"  ;
    case Lexical_Dot         : return "."  ;
    case Lexical_Comma       : return ","  ;
    case Lexical_Question    : return "?"  ;
    case Lexical_Colon       : return ":"  ;
    case Lexical_Semicolon   : return ";"  ;
    case Lexical_GT          : return ">"  ;
    case Lexical_PMemb       : return "->" ;
    case Lexical_RShift      : return ">>" ;
    case Lexical_LT          : return "<"  ;
    case Lexical_LShift      : return "<<" ;
    case Lexical_Assign      : return "="  ;
    case Lexical_AddAssign   : return "+=" ;
    case Lexical_SubAssign   : return "-=" ;
    case Lexical_MulAssign   : return "*=" ;
    case Lexical_DivAssign   : return "/=" ;
    case Lexical_ModAssign   : return "%=" ;
    case Lexical_BitAndAssign: return "&=" ;
    case Lexical_BitXorAssign: return "^=" ;
    case Lexical_BitOrAssign : return "|=" ;
    case Lexical_NotEq       : return "!=" ;
    case Lexical_GTEq        : return ">=" ;
    case Lexical_RShiftAssign: return ">>=";
    case Lexical_LTEq        : return "<=" ;
    case Lexical_LShiftAssign: return "<<=";
    case Lexical_Eq          : return "==" ;
    default              : return 0;
    }
}

    void
init_lexwords (Associa* map)
{
    InitAssocia( AlphaTab, SyntaxKind, *map, swapped_AlphaTab );

    for (SyntaxKind kind = Beg_Syntax_LexWords;
         kind < End_Syntax_LexWords;
         kind = (SyntaxKind) (kind + 1))
    {
        AlphaTab key = dflt1_AlphaTab (cstr_SyntaxKind (kind));
        insert_Associa (map, &key, &kind);
    }
}

void
oput_AST (OFile* of, AST* ast, const ASTree* t);

    void
oput1_AST (OFile* of, AST* ast, const ASTree* t)
{
  if (!ast)  return;

  switch (ast->kind)
  {
  case Syntax_WS:
    oput_AlphaTab (of, &ast->txt);
    break;
  case Syntax_Iden:
    Claim2( ast->txt.sz ,>, 0 );
    oput_AlphaTab (of, &ast->txt);
    break;
  case Syntax_CharLit:
    oput_char_OFile (of, '\'');
    Claim2( ast->txt.sz ,>, 0 );
    oput_AlphaTab (of, &ast->txt);
    oput_char_OFile (of, '\'');
    break;
  case Syntax_StringLit:
    oput_char_OFile (of, '"');
    Claim2( ast->txt.sz ,>, 0 );
    oput_AlphaTab (of, &ast->txt);
    oput_char_OFile (of, '"');
    break;
  case Syntax_Parens:
    oput_char_OFile (of, '(');
    oput_AST (of, cdar_of_AST (ast), t);
    oput_char_OFile (of, ')');
    break;
  case Syntax_Braces:
    oput_char_OFile (of, '{');
    oput_AST (of, cdar_of_AST (ast), t);
    oput_char_OFile (of, '}');
    break;
  case Syntax_Brackets:
    oput_char_OFile (of, '[');
    oput_AST (of, cdar_of_AST (ast), t);
    oput_char_OFile (of, ']');
    break;
  case Syntax_Stmt:
    oput_AST (of, cdar_of_AST (ast), t);
    oput_char_OFile (of, ';');
    break;
  case Syntax_ForLoop:
    oput_cstr_OFile (of, "for");
    oput_AST (of, cdar_of_AST (ast), t);
    break;
  case Syntax_WhileLoop:
    oput_cstr_OFile (of, "while");
    oput_AST (of, cdar_of_AST (ast), t);
    break;
  case Syntax_If:
    oput_cstr_OFile (of, "if");
    oput_AST (of, cdar_of_AST (ast), t);
    break;
  case Syntax_Else:
    oput_cstr_OFile (of, "else");
    oput_AST (of, cdar_of_AST (ast), t);
    break;
  case Syntax_LineComment:
    oput_cstr_OFile (of, "//");
    oput_AlphaTab (of, &ast->txt);
    oput_char_OFile (of, '\n');
    break;
  case Syntax_BlockComment:
    oput_cstr_OFile (of, "/*");
    oput_AlphaTab (of, &ast->txt);
    oput_cstr_OFile (of, "*/");
    break;
  case Syntax_Directive:
    oput_char_OFile (of, '#');
    oput_AlphaTab (of, &ast->txt);
    oput_char_OFile (of, '\n');
    break;
  default:
    if ((ast->kind >= Beg_Syntax_LexWords &&
         ast->kind < End_Syntax_LexWords) ||
        (ast->kind >= Beg_Syntax_LexOps &&
         ast->kind < End_Syntax_LexOps))
    {
      oput_cstr_OFile (of, cstr_SyntaxKind (ast->kind));
    }
    else
    {
      DBog1( "No Good! Enum value: %u", (uint) ast->kind );
    }
    break;
  }
}

    void
oput_AST (OFile* of, AST* ast, const ASTree* t)
{
    while (ast)
    {
        oput1_AST (of, ast, t);
        ast = cdr_of_AST (ast);
    }
}

    void
oput_ASTree (OFile* of, ASTree* t)
{
    oput_AST (of, AST_of_Cons (t->head), t);
}

    void
oput_sxpn_AST (OFile* of, AST* ast)
{
    bool first = true;
    while (ast)
    {
        if (first)  first = false;
        else        oput_char_OFile (of, ' ');
        oput_uint_OFile (of, (uint) ast->kind);
        if (ast->cons->car.kind == Cons_Cons)
        {
            oput_char_OFile (of, ':');
            oput_char_OFile (of, '(');
            oput_sxpn_AST (of, cdar_of_AST (ast));
            oput_char_OFile (of, ')');
        }
        ast = cdr_of_AST (ast);
    }
}

    void
oput_sxpn_ASTree (OFile* of, ASTree* t)
{
    oput_sxpn_AST (of, AST_of_Cons (t->head));
    oput_char_OFile (of, '\n');
}

    void
bevel_AST (AST* ast, ASTree* t)
{
    Cons* c = take_Sxpn (&t->sx);

    c->car.kind = Cons_MemLoc;
    c->car.as.memloc = ast;

    ast->cons->car.kind = Cons_Cons;
    ast->cons->car.as.cons = c;
}

    bool
parse_escaped (XFile* xf, AlphaTab* t, char delim)
{
    char delims[2];

    delims[0] = delim;
    delims[1] = 0;

    for (char* s = nextds_XFile (xf, 0, delims);
         s;
         s = nextds_XFile (xf, 0, delims))
    {
        bool escaped = false;
        ujint off;

        cat_cstr_AlphaTab (t, s);
        off = t->sz-1;

        while (off > 0 && t->s[off-1] == '\\')
        {
            escaped = !escaped;
            -- off;
        }
        if (escaped)
            cat_cstr_AlphaTab (t, delims);
        else
            return true;
    }
    return false;
}

    ujint
count_newlines (const char* s)
{
    ujint n = 0;
    for (s = strchr (s, '\n'); s;  s = strchr (&s[1], '\n'))
        ++ n;
    return n;
}

/** Tokenize while dealing with
 * - line/block comment
 * - directive (perhaps this should happen later)
 * - char, string
 * - parentheses, braces, brackets
 * - statement ending with semicolon
 **/
    void
lex_AST (XFile* xf, ASTree* t)
{
    char match = 0;
    const char delims[] = "'\"(){}[];#+-*/%&^|~!.,?:><=";
    ujint off;
    ujint line = 0;
    DecloStack( Associa, keyword_map );
    Cons* up = 0;
    AST dummy_ast = dflt_AST ();
    AST* ast = &dummy_ast;
    Cons** p = &t->head;

#define InitLeaf(ast) \
    (ast) = take_ASTree (t); \
    (ast)->line = line; \
    *p = (ast)->cons; \
    p = &(ast)->cons->cdr;

    init_lexwords (keyword_map);

    for (char* s = nextds_XFile (xf, &match, delims);
         s;
         s = nextds_XFile (xf, &match, delims))
    {
        off = IdxEltTable( xf->buf, s );

        if (s[0])
        {
            XFile olay[1];
            olay_XFile (olay, xf, off);

            while (olay->buf.sz > 0)
            {
                skipds_XFile (olay, 0);
                if (olay->off > 0)
                {
                    AlphaTab ts = AlphaTab_XFile (olay, 0);
                    InitLeaf( ast );
                    ast->kind = Syntax_WS;
                    cat_AlphaTab (&ast->txt, &ts);
                    line += count_newlines (cstr_AlphaTab (&ast->txt));
                }
                off += olay->off;
                olay_XFile (olay, xf, off);
                if (!olay->buf.s[0])  break;

                olay->off = IdxEltTable( olay->buf, tods_XFile (olay, 0) );
                if (olay->off > 0)
                {
                    AlphaTab ts = AlphaTab_XFile (olay, 0);
                    Assoc* luk = lookup_Associa (keyword_map, &ts);

                    InitLeaf( ast );
                    ast->kind = Syntax_WS;

                    if (luk)
                    {
                        ast->kind = *(SyntaxKind*) val_of_Assoc (luk);
                    }
                    else
                    {
                        ast->kind = Syntax_Iden;
                        AffyTable( ast->txt, ts.sz+1 );
                        cat_AlphaTab (&ast->txt, &ts);
                    }
                }

                off += olay->off;
                olay_XFile (olay, xf, off);
            }
        }

        switch (match)
        {
        case '\0':
            break;
        case '\'':
            InitLeaf( ast );
            ast->kind = Syntax_CharLit;
            if (!parse_escaped (xf, &ast->txt, '\''))
                DBog1( "Gotta problem with single quotes! line:%u",
                       (uint) line );
            break;
        case '"':
            InitLeaf( ast );
            ast->kind = Syntax_StringLit;
            if (!parse_escaped (xf, &ast->txt, '"'))
                DBog1( "Gotta problem with double quotes! line:%u",
                       (uint) line );
            break;
        case '(':
            InitLeaf( ast );
            ast->kind = Syntax_Parens;
            bevel_AST (ast, t);
            up = take2_Sxpn (&t->sx, dflt_Cons_ConsAtom (ast->cons), up);
            p = &ast->cons->car.as.cons->cdr;
            break;
        case '{':
            InitLeaf( ast );
            ast->kind = Syntax_Braces;
            bevel_AST (ast, t);
            up = take2_Sxpn (&t->sx, dflt_Cons_ConsAtom (ast->cons), up);
            p = &ast->cons->car.as.cons->cdr;
            break;
        case '[':
            InitLeaf( ast );
            ast->kind = Syntax_Brackets;
            bevel_AST (ast, t);
            up = take2_Sxpn (&t->sx, dflt_Cons_ConsAtom (ast->cons), up);
            p = &ast->cons->car.as.cons->cdr;
            break;
        case ')':
        case '}':
        case ']':
            if (!up)
            {
                DBog2( "Unmatched closing '%c', line: %u.", match, line );
                return;
            }
            ast = AST_of_Cons (up->car.as.cons);
            up = pop_Sxpn (&t->sx, up);
            if ((match == ')' && ast->kind == Syntax_Parens) ||
                (match == '}' && ast->kind == Syntax_Braces) ||
                (match == ']' && ast->kind == Syntax_Brackets))
            {
                p = &ast->cons->cdr;
            }
            else
            {
                DBog2( "Mismatched closing '%c', line: %u.", match, line );
                return;
            }
            break;
        case '#':
            InitLeaf( ast );
            ast->kind = Syntax_Directive;
            cat_cstr_AlphaTab (&ast->txt, getlined_XFile (xf, "\n"));
            ++ line;
            while (endc_ck_AlphaTab (&ast->txt, '\\'))
            {
                cat_cstr_AlphaTab (&ast->txt, "\n");
                cat_cstr_AlphaTab (&ast->txt, getlined_XFile (xf, "\n"));
                ++ line;
            }
            break;

#define LexiCase( c, k )  case c: \
            InitLeaf( ast ); \
            ast->kind = k; \
            break;

#define Lex2Case( c, k1, k2 )  case c: \
            if (ast->kind == k1) \
            { \
                ast->kind = k2; \
            } \
            else \
            { \
                InitLeaf( ast ); \
                ast->kind = k1; \
            } \
            break;

            Lex2Case( '+', Lexical_Add, Lexical_Inc );
            Lex2Case( '-', Lexical_Sub, Lexical_Dec );
        case '*':
            if (ast->kind == Lexical_Div)
            {
                ast->kind = Syntax_BlockComment;
                cat_cstr_AlphaTab (&ast->txt, getlined_XFile (xf, "*/"));
                line += count_newlines (ast->txt.s);
            }
            else
            {
                InitLeaf( ast );
                ast->kind = Lexical_Mul;
            }
            break;
        case '/':
            if (ast->kind == Lexical_Div)
            {
                ast->kind = Syntax_LineComment;
                cat_cstr_AlphaTab (&ast->txt, getlined_XFile (xf, "\n"));
                ++ line;
            }
            else
            {
                InitLeaf( ast );
                ast->kind = Lexical_Div;
            }
            break;
            LexiCase( '%', Lexical_Mod );
            Lex2Case( '&', Lexical_BitAnd, Lexical_And );
            LexiCase( '^', Lexical_BitXor );
            Lex2Case( '|', Lexical_BitOr, Lexical_Or );
            LexiCase( '~', Lexical_BitNot );
            LexiCase( '!', Lexical_Not );
            LexiCase( '.', Lexical_Dot );
            LexiCase( ',', Lexical_Comma );
            LexiCase( '?', Lexical_Question );
            LexiCase( ':', Lexical_Colon );
            LexiCase( ';', Lexical_Semicolon );
        case '>':
            if (ast->kind == Lexical_GT)
            {
                ast->kind = Lexical_RShift;
            }
            else if (ast->kind == Lexical_Sub)
            {
                ast->kind = Lexical_PMemb;
            }
            else
            {
                InitLeaf( ast );
                ast->kind = Lexical_GT;
            }
            break;
            Lex2Case( '<', Lexical_LT, Lexical_LShift );
        case '=':
            if (ast->kind == Lexical_Add)
                ast->kind = Lexical_AddAssign;
            else if (ast->kind == Lexical_Sub)
                ast->kind = Lexical_SubAssign;
            else if (ast->kind == Lexical_Mul)
                ast->kind = Lexical_MulAssign;
            else if (ast->kind == Lexical_Div)
                ast->kind = Lexical_DivAssign;
            else if (ast->kind == Lexical_Mod)
                ast->kind = Lexical_ModAssign;
            else if (ast->kind == Lexical_BitAnd)
                ast->kind = Lexical_BitAndAssign;
            else if (ast->kind == Lexical_BitXor)
                ast->kind = Lexical_BitXorAssign;
            else if (ast->kind == Lexical_BitOr)
                ast->kind = Lexical_BitOrAssign;
            else if (ast->kind == Lexical_Not)
                ast->kind = Lexical_NotEq;
            else if (ast->kind == Lexical_GT)
                ast->kind = Lexical_GTEq;
            else if (ast->kind == Lexical_RShift)
                ast->kind = Lexical_RShiftAssign;
            else if (ast->kind == Lexical_LT)
                ast->kind = Lexical_LTEq;
            else if (ast->kind == Lexical_LShift)
                ast->kind = Lexical_LShiftAssign;
            else if (ast->kind == Lexical_Assign)
                ast->kind = Lexical_Eq;
            else
            {
                InitLeaf( ast );
                ast->kind = Lexical_Assign;
            }
            break;
#undef Lex2Case
#undef LexiCase
        }
    }

    while (up)
    {
        ast = AST_of_Cons (up->car.as.cons);
        up = pop_Sxpn (&t->sx, up);
        switch (ast->kind)
        {
        case Syntax_Parens:
            DBog1( "Unclosed '(', line %u", (uint) ast->line );
            break;
        case Syntax_Braces:
            DBog1( "Unclosed '{', line %u", (uint) ast->line );
            break;
        case Syntax_Brackets:
            DBog1( "Unclosed '[', line %u", (uint) ast->line );
            break;
        default:
            break;
        }
    }
    lose_Associa (keyword_map);
#undef InitLeaf
}

    AST*
next_semicolon_or_braces_AST (AST* ast)
{
    for (ast = cdr_of_AST (ast);
         ast;
         ast = cdr_of_AST (ast))
    {
        if (ast->kind == Lexical_Semicolon)  return ast;
        if (ast->kind == Syntax_Braces)  return ast;
    }
    return 0;
}

    AST*
next_parens (AST* ast)
{
    for (ast = cdr_of_AST (ast);
         ast;
         ast = cdr_of_AST (ast))
    {
        if (ast->kind == Syntax_Parens)  return ast;
    }
    return 0;
}

void
build_stmts_AST (Cons** ast_p, ASTree* t);

    AST*
ins1_cadr_AST (AST* a, ASTree* t, SyntaxKind kind)
{
    Cons* c;
    AST* b = take1_ASTree (t, kind);
    b->line = a->line;

    Claim2( a->cons->car.kind ,==, Cons_Cons );
    c = a->cons->car.as.cons->cdr;
    a->cons->car.as.cons->cdr = b->cons;
    b->cons->cdr = c;
    return b;
}

    AST*
ins1_cdr_AST (AST* a, ASTree* t, SyntaxKind kind)
{
    Cons* c;
    AST* b = take1_ASTree (t, kind);
    b->line = a->line;

    c = a->cons->cdr;
    a->cons->cdr = b->cons;
    b->cons->cdr = c;
    return b;
}

    bool
ws_ck_SyntaxKind (SyntaxKind kind)
{
    switch (kind)
    {
    case Syntax_WS:
    case Syntax_LineComment:
    case Syntax_BlockComment:
    case Syntax_Directive:
        return true;
    default:
        return false;
    }
}

    void
quickforloop_AST (Cons** ast_p, ASTree* t)
{
    AST* a = AST_of_Cons (*ast_p);
    AST* b;
    AST* c;
    AST* d;
    AST* iden = 0;

    if (!a || a->kind != Syntax_Stmt)  return;

    b = cdr_of_AST (a);
    if (!b || b->kind == Syntax_Stmt)  return;

    c = cdar_of_AST (a);
    while (c)
    {
        if (c->kind == Syntax_Iden)
        {
            if (iden)
            {
                DBog1( "Loop already has an identifier: %s",
                       cstr_AlphaTab (&iden->txt) );
                return;
            }
            iden = c;
        }
        else if (!ws_ck_SyntaxKind (c->kind))
        {
            DBog0( "Just name an identifier for the loop!" );
            return;
        }
        if (!c->cons->cdr)  break;
        c = cdr_of_AST (c);
    }

    /* (parens (; i) ...)
     * -->
     * (parens (; unsigned int i = 0) ...)
     */
    d = ins1_cadr_AST (a, t, Lexical_Unsigned);
    d = ins1_cdr_AST (d, t, Syntax_WS);
    copy_cstr_AlphaTab (&d->txt, " ");

    d = ins1_cdr_AST (d, t, Lexical_Int);
    d = ins1_cdr_AST (d, t, Syntax_WS);
    copy_cstr_AlphaTab (&d->txt, " ");

    d = ins1_cdr_AST (c, t, Lexical_Assign);
    d = ins1_cdr_AST (d, t, Syntax_Iden);
    copy_cstr_AlphaTab (&d->txt, "0");

    /* (parens (; unsigned int i = 0) n)
     * -->
     * (parens (; unsigned int i = 0) (; i < n) ++ i)
     */
    c = ins1_cdr_AST (a, t, Syntax_Stmt);
    bevel_AST (c, t);
    c->cons->cdr = 0;

    d = ins1_cadr_AST (c, t, Syntax_Parens);
    bevel_AST (d, t);
    d->cons->car.as.cons->cdr = b->cons;

    d = ins1_cadr_AST (c, t, Syntax_Iden);
    copy_AlphaTab (&d->txt, &iden->txt);

    d = ins1_cdr_AST (d, t, Lexical_LT);

    d = ins1_cdr_AST (c, t, Lexical_Inc);
    d = ins1_cdr_AST (d, t, Syntax_Iden);
    copy_AlphaTab (&d->txt, &iden->txt);
}

    void
build_ctrl_AST (AST* ast, ASTree* t)
{
    /* (... for (parens ...) ... ; ...)
     *  -->
     * (... (for (parens ..) (; ...)) ...)
     */
    AST* d_ctrl;
    AST* d_parens;
    AST* d_semic;
    const char* ctrl_name = "unknown";

    d_ctrl = ast;
    if (d_ctrl->kind == Lexical_For)
    {
        d_ctrl->kind = Syntax_ForLoop;
        ctrl_name = "for-loop";
    }
    else if (d_ctrl->kind == Lexical_While)
    {
        d_ctrl->kind = Syntax_WhileLoop;
        ctrl_name = "while-loop";
    }
    else if (d_ctrl->kind == Lexical_If)
    {
        d_ctrl->kind = Syntax_If;
        ctrl_name = "if-statement";
    }
    else if (d_ctrl->kind == Lexical_Else)
    {
        d_ctrl->kind = Syntax_Else;
        ctrl_name = "else-statement";
    }
    else
    {
        Claim( false );
    }

    d_parens = d_ctrl;
    if (d_ctrl->kind != Syntax_Else)
    {
        d_parens = next_parens (d_ctrl);
        if (!d_parens)
        {
            DBog2( "No parens for %s. Line: %u", ctrl_name, (uint) ast->line);
            failout_sysCx ("");
        }
    }

    d_semic = next_semicolon_or_braces_AST (d_parens);
    if (!d_semic)
    {
        DBog2( "No end of for %s. Line: %u", ctrl_name, (uint) ast->line);
        failout_sysCx ("");
    }

    bevel_AST (d_ctrl, t);
    d_ctrl->cons->car.as.cons->cdr = d_ctrl->cons->cdr;
    d_ctrl->cons->cdr = d_semic->cons->cdr;
    d_semic->cons->cdr = 0;

    if (d_ctrl->kind == Syntax_Else)
    {
        build_stmts_AST (&d_ctrl->cons->car.as.cons->cdr, t);
    }
    else
    {
        /* Only gets first two statements in for loop.*/
        build_stmts_AST (&d_parens->cons->car.as.cons->cdr, t);
        if (d_ctrl->kind == Syntax_ForLoop)
            quickforloop_AST (&d_parens->cons->car.as.cons->cdr, t);

        build_stmts_AST (&d_parens->cons->cdr, t);
    }
}

    void
prebrace_AST (Cons** ast_p, ASTree* t)
{
    AST* ast = AST_of_Cons (*ast_p);
    AST* d_colon;
    Cons** p_colon;
    AST* d_ctrl;
    Cons** p_ctrl;
    AST* d_body;
    Cons** p_body;

    p_colon = &ast->cons->car.as.cons->cdr;
    d_colon = cdar_of_AST (ast);
    while (d_colon && ws_ck_SyntaxKind (d_colon->kind))
    {
        p_colon = &d_colon->cons->cdr;
        d_colon = cdr_of_AST (d_colon);
    }
    if (!d_colon || d_colon->kind != Lexical_Colon)  return;

    p_ctrl = &d_colon->cons->cdr;
    d_ctrl = cdr_of_AST (d_colon);
    while (d_ctrl && ws_ck_SyntaxKind (d_ctrl->kind))
    {
        p_ctrl = &d_ctrl->cons->cdr;
        d_ctrl = cdr_of_AST (d_ctrl);
    }
    if (!d_ctrl)  return;

    p_body = &d_ctrl->cons->cdr;
    d_body = cdr_of_AST (d_ctrl);

    if (d_ctrl->kind == Lexical_Else)
    {
        while (d_body && ws_ck_SyntaxKind (d_body->kind))
        {
            p_body = &d_body->cons->cdr;
            d_body = cdr_of_AST (d_body);
        }
    }

    if (d_ctrl->kind == Lexical_For ||
        d_ctrl->kind == Lexical_While ||
        d_ctrl->kind == Lexical_Switch ||
        d_ctrl->kind == Lexical_If ||
        (d_ctrl->kind == Lexical_Else &&
         d_body &&
         d_body->kind == Lexical_If))
    {
        while (d_body && d_body->kind != Syntax_Parens)
        {
            p_body = &d_body->cons->cdr;
            d_body = cdr_of_AST (d_body);
        }
    }
    else if (d_ctrl->kind == Lexical_Else ||
             d_ctrl->kind == Lexical_Do)
    {
        p_body = p_ctrl;
        d_body = d_ctrl;
    }
    else
    {
        while (d_body && d_body->kind != Lexical_Semicolon)
        {
            p_body = &d_body->cons->cdr;
            d_body = cdr_of_AST (d_body);
        }
    }
    if (!d_body)  return;

    // Add control statement on top.
    *ast_p = d_ctrl->cons;

    // Snip control statement from braces.
    *p_ctrl = d_body->cons->cdr;

    // Snip colon from existence.
    *p_colon = d_colon->cons->cdr;
    // Free the colon;
    d_colon->cons->cdr = 0;
    give_ASTree (t, d_colon);

    if (d_body->kind == Lexical_Semicolon)
    {
        // Make braces start at body.
        *p_body = ast->cons;
        // Free the semicolon.
        d_body->cons->cdr = 0;
        give_ASTree (t, d_body);
    }
    else
    {
        d_body->cons->cdr = ast->cons;
    }
}


    void
build_stmts_AST (Cons** ast_p, ASTree* t)
{
    AST* pending = 0;
    SyntaxKind pending_kind = NSyntaxKinds;
    Cons** pending_p = 0;
    AST* ast = AST_of_Cons (*ast_p);
    Cons** p = ast_p;

    for (; ast; ast = cdr_of_AST (ast))
    {
        if (ast->kind == Syntax_Braces)
        {
            prebrace_AST (p, t);
            ast = AST_of_Cons (*p);
        }

        if (pending)
        {
            if (pending_kind == NSyntaxKinds)
                pending_kind = pending->kind;
            if (!pending_p)
                pending_p = p;
        }
        else
        {
            pending_kind = NSyntaxKinds;
            pending_p = 0;
        }

        switch  (ast->kind)
        {
        case Syntax_Directive:
        case Syntax_LineComment:
        case Syntax_BlockComment:
            break;
        case Lexical_For:
        case Lexical_While:
        case Lexical_If:
        case Lexical_Else:
            build_ctrl_AST (ast, t);
            pending = 0;
            break;
        case Syntax_Parens:
            if (!pending)
            {
                pending = ast;
                pending_kind = ast->kind;
                pending_p = p;
            }
            break;
        case Syntax_Braces:
            build_stmts_AST (&ast->cons->car.as.cons->cdr, t);
            if (pending_kind != Lexical_Do)
            {
                pending = 0;
            }
            break;
        case Lexical_Colon:
            if (pending_kind == Lexical_Case)
            {
            }
            else if (pending_kind == Lexical_Goto)
            {
            }
            else
            {
                
            }
            pending = 0;
            break;
        case Lexical_Semicolon:
            ast->kind = Syntax_Stmt;
            bevel_AST (ast, t);
            /* Empty statement.*/
            if (!pending)  break;

            
            *p = 0;
            *pending_p = ast->cons;
            ast->cons->car.as.cons->cdr = pending->cons;

            pending = 0;
            break;
        default:
            if (!pending)
            {
                pending = ast;
                pending_p = p;
                pending_kind = ast->kind;
            }
            break;
        }
        p = &ast->cons->cdr;
    }
}

    bool
declaration_ck (AST* a)
{
    uint n = 0;
    while (a && a->kind != Lexical_Assign)
    {
        if (a->kind != Syntax_WS)  ++n;
        a = cdr_of_AST (a);
    }

    return (n > 1);
}

    void
xfrm_stmts_AST (Cons** ast_p, ASTree* t)
{
    AST* ast = AST_of_Cons (*ast_p);
    Cons** p = ast_p;

    while (ast)
    {
        if (ast->cons->car.kind == Cons_Cons)
            xfrm_stmts_AST (&ast->cons->car.as.cons->cdr, t);
        if (ast->kind == Syntax_LineComment)
        {
            AlphaTab ts = dflt1_AlphaTab ("\n");
            ast->kind = Syntax_WS;
            copy_AlphaTab (&ast->txt, &ts);
        }
        else if (ast->kind == Syntax_ForLoop)
        {
            /* (... (for (parens (; a) ...) x) ...)
             * -->
             * (... (braces (for (parens (; a) ...) x)) ...)
             */
            AST* d_for = ast;
            AST* d_parens;
            AST* d_stmt;

            d_parens = cdar_of_AST (d_for);
            while (d_parens && d_parens->kind != Syntax_Parens)
                d_parens = cdr_of_AST (d_parens);

            Claim( d_parens );
            d_stmt = cdar_of_AST (d_parens);

            if (d_stmt && declaration_ck (cdar_of_AST (d_stmt)))
            {
                AST* d_braces = take1_ASTree (t, Syntax_Braces);
                AST* d_stmt1 = take1_ASTree (t, Syntax_Stmt);

                bevel_AST (d_stmt1, t);
                d_parens->cons->car.as.cons->cdr = d_stmt1->cons;
                d_stmt1->cons->cdr = d_stmt->cons->cdr;

                *p = d_braces->cons;
                bevel_AST (d_braces, t);
                d_braces->cons->cdr = d_for->cons->cdr;
                d_braces->cons->car.as.cons->cdr = d_stmt->cons;
                d_stmt->cons->cdr = d_for->cons;
                d_for->cons->cdr = 0;
                ast = d_braces;
            }
        }
        p = &ast->cons->cdr;
        ast = cdr_of_AST (ast);
    }
}

    void
xget_ASTree (XFile* xf, ASTree* t)
{
    Associa type_lookup;
    InitAssocia( AlphaTab, uint, type_lookup, swapped_AlphaTab );

    lex_AST (xf, t);
    build_stmts_AST (&t->head, t);
    //oput_sxpn_ASTree (stderr_OFile (), t);
    xfrm_stmts_AST (&t->head, t);

    lose_Associa (&type_lookup);
}

  int
main (int argc, char** argv)
{
  int argi =
    (init_sysCx (&argc, &argv),
     1);
  DecloStack1( ASTree, t, cons_ASTree () );
  XFileB xfb[1];
  XFile* xf = 0;
  OFileB ofb[1];
  OFile* of = 0;

  init_XFileB (xfb);
  init_OFileB (ofb);

  while (argi < argc)
  {
    if (0 == strcmp (argv[argi], "-x"))
    {
      ++ argi;
      if (!open_FileB (&xfb->fb, 0, argv[argi++]))
      {
        failout_sysCx ("Could not open file for reading.");
      }
      xf = &xfb->xf;
    }
    else if (0 == strcmp (argv[argi], "-o"))
    {
      ++ argi;
      if (!open_FileB (&ofb->fb, 0, argv[argi++]))
      {
        failout_sysCx ("Could not open file for writing.");
      }
      of = &ofb->of;
    }
    else
    {
      bool good = (0 == strcmp (argv[argi], "-h"));
      OFile* of = (good ? stdout_OFile () : stderr_OFile ());
      printf_OFile (of, "Usage: %s [-x IN] [-o OUT]\n", argv[0]);
      oput_cstr_OFile (of, "  If -x is not specified, stdin is used.\n");
      oput_cstr_OFile (of, "  If -o is not specified, stdout is used.\n");
      if (!good)  failout_sysCx ("Exiting in failure...");
      lose_sysCx ();
      return 0;
    }
  }

  if (!xf)  xf = stdin_XFile ();
  if (!of)  of = stdout_OFile ();

  xget_ASTree (xf, t);
  close_XFile (xf);
  lose_XFileB (xfb);

  oput_ASTree (of, t);
  close_OFile (of);
  lose_OFileB (ofb);

  lose_ASTree (t);
  lose_sysCx ();
  return 0;
}

