
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
req_ASTree (ASTree* t)
{
    AST* ast = (AST*) req_LgTable (&t->lgt);
    *ast = dflt_AST ();
    ast->cons = req_Sxpn (&t->sx);
    /* InitDomMax( ast->cons->nrefs ); */
    ast->cons->car.kind = Cons_MemLoc;
    ast->cons->car.as.memloc = ast;
    return ast;
}

    AST*
req1_ASTree (ASTree* t, SyntaxKind kind)
{
    AST* ast = req_ASTree (t);
    ast->kind = kind;
    return ast;
}

    void
giv_ASTree (ASTree* t, AST* ast)
{
    LoseTable( ast->txt );
    giv_LgTable (&t->lgt, ast);
    /* ast->cons->nrefs = 0; */
    if (ast->cons->car.kind == Cons_Cons)
        ast->cons->car.as.cons = 0;
    ast->cons->cdr = 0;
    giv_Sxpn (&t->sx, ast->cons);
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

    AST*
cdar_of_AST (AST* ast, const ASTree* t)
{
    Cons* c = ast->cons;
    (void) t;

    Claim2( Cons_Cons ,==, c->car.kind );
    c = c->car.as.cons;
    Claim( c );
    c = c->cdr;
    if (!c)  return 0;
    return AST_of_Cons (c);
}

    AST*
cdr_of_AST (AST* ast, const ASTree* t)
{
    Cons* c = ast->cons->cdr;
    (void) t;

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
    DeclAssocia( AlphaTab, SyntaxKind, tmp_map,
                 (SwappedFn) swapped_AlphaTab );
    *map = *tmp_map;

    for (SyntaxKind kind = Beg_Syntax_LexWords;
         kind < End_Syntax_LexWords;
         kind = (SyntaxKind) (kind + 1))
    {
        AlphaTab key = dflt1_AlphaTab (cstr_SyntaxKind (kind));
        insert_Associa (map, &key, &kind);
    }
}

void
dump_AST (OFileB* of, AST* ast, const ASTree* t);

    void
dump1_AST (OFileB* of, AST* ast, const ASTree* t)
{
    if (!ast)  return;

    switch (ast->kind)
    {
    case Syntax_WS:
        dump_AlphaTab (of, &ast->txt);
        break;
    case Syntax_Iden:
        Claim2( ast->txt.sz ,>, 0 );
        dump_AlphaTab (of, &ast->txt);
        break;
    case Syntax_CharLit:
        dump_char_OFileB (of, '\'');
        Claim2( ast->txt.sz ,>, 0 );
        dump_AlphaTab (of, &ast->txt);
        dump_char_OFileB (of, '\'');
        break;
    case Syntax_StringLit:
        dump_char_OFileB (of, '"');
        Claim2( ast->txt.sz ,>, 0 );
        dump_AlphaTab (of, &ast->txt);
        dump_char_OFileB (of, '"');
        break;
    case Syntax_Parens:
        dump_char_OFileB (of, '(');
        dump_AST (of, cdar_of_AST (ast, t), t);
        dump_char_OFileB (of, ')');
        break;
    case Syntax_Braces:
        dump_char_OFileB (of, '{');
        dump_AST (of, cdar_of_AST (ast, t), t);
        dump_char_OFileB (of, '}');
        break;
    case Syntax_Brackets:
        dump_char_OFileB (of, '[');
        dump_AST (of, cdar_of_AST (ast, t), t);
        dump_char_OFileB (of, ']');
        break;
    case Syntax_Stmt:
        dump_AST (of, cdar_of_AST (ast, t), t);
        dump_char_OFileB (of, ';');
        break;
    case Syntax_ForLoop:
        dump_cstr_OFileB (of, "for");
        dump_AST (of, cdar_of_AST (ast, t), t);
        break;
    case Syntax_LineComment:
        dump_cstr_OFileB (of, "//");
        dump_AlphaTab (of, &ast->txt);
        dump_char_OFileB (of, '\n');
        break;
    case Syntax_BlockComment:
        dump_cstr_OFileB (of, "/*");
        dump_AlphaTab (of, &ast->txt);
        dump_cstr_OFileB (of, "*/");
        break;
    case Syntax_Directive:
        dump_char_OFileB (of, '#');
        dump_AlphaTab (of, &ast->txt);
        dump_char_OFileB (of, '\n');
        break;
    default:
        if ((ast->kind >= Beg_Syntax_LexWords &&
            ast->kind < End_Syntax_LexWords) ||
            (ast->kind >= Beg_Syntax_LexOps &&
             ast->kind < End_Syntax_LexOps))
        {
            dump_cstr_OFileB (of, cstr_SyntaxKind (ast->kind));
        }
        else
        {
            DBog1( "No Good! Enum value: %u", (uint) ast->kind );
        }
        break;
    }
}

    void
dump_AST (OFileB* of, AST* ast, const ASTree* t)
{
    while (ast)
    {
        dump1_AST (of, ast, t);
        ast = cdr_of_AST (ast, t);
    }
}

    void
dump_ASTree (OFileB* of, ASTree* t)
{
    dump_AST (of, AST_of_Cons (t->head), t);
}

    void
bevel_AST (AST* ast, ASTree* t)
{
    Cons* c = req_Sxpn (&t->sx);

    c->car.kind = Cons_MemLoc;
    c->car.as.memloc = ast;

    ast->cons->car.kind = Cons_Cons;
    ast->cons->car.as.cons = c;
}

    bool
parse_escaped (XFileB* xf, AlphaTab* t, char delim)
{
    char delims[2];

    delims[0] = delim;
    delims[1] = 0;

    for (char* s = nextds_XFileB (xf, 0, delims);
         s;
         s = nextds_XFileB (xf, 0, delims))
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
lex_AST (XFileB* xf, ASTree* t)
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
    (ast) = req_ASTree (t); \
    (ast)->line = line; \
    *p = (ast)->cons; \
    p = &(ast)->cons->cdr;

    init_lexwords (keyword_map);

    for (char* s = nextds_XFileB (xf, &match, delims);
         s;
         s = nextds_XFileB (xf, &match, delims))
    {
        off = IdxEltTable( xf->buf, s );

        if (s[0])
        {
            DecloStack( XFileB, olay );
            *olay = olay_XFileB (xf, off);

            while (olay->buf.sz > 0)
            {
                skipds_XFileB (olay, 0);
                if (olay->off > 0)
                {
                    AlphaTab ts = AlphaTab_XFileB (olay, 0);
                    InitLeaf( ast );
                    ast->kind = Syntax_WS;
                    cat_AlphaTab (&ast->txt, &ts);
                    line += count_newlines (cstr_AlphaTab (&ast->txt));
                }
                off += olay->off;
                *olay = olay_XFileB (xf, off);
                if (!olay->buf.s[0])  break;

                olay->off = IdxEltTable( olay->buf, tods_XFileB (olay, 0) );
                if (olay->off > 0)
                {
                    AlphaTab ts = AlphaTab_XFileB (olay, 0);
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
                *olay = olay_XFileB (xf, off);
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
            up = req2_Sxpn (&t->sx, dflt_Cons_ConsAtom (ast->cons), up);
            p = &ast->cons->car.as.cons->cdr;
            break;
        case '{':
            InitLeaf( ast );
            ast->kind = Syntax_Braces;
            bevel_AST (ast, t);
            up = req2_Sxpn (&t->sx, dflt_Cons_ConsAtom (ast->cons), up);
            p = &ast->cons->car.as.cons->cdr;
            break;
        case '[':
            InitLeaf( ast );
            ast->kind = Syntax_Brackets;
            bevel_AST (ast, t);
            up = req2_Sxpn (&t->sx, dflt_Cons_ConsAtom (ast->cons), up);
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
            cat_cstr_AlphaTab (&ast->txt, getlined_XFileB (xf, "\n"));
            ++ line;
            while (endc_ck_AlphaTab (&ast->txt, '\\'))
            {
                cat_cstr_AlphaTab (&ast->txt, "\n");
                cat_cstr_AlphaTab (&ast->txt, getlined_XFileB (xf, "\n"));
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
                cat_cstr_AlphaTab (&ast->txt, getlined_XFileB (xf, "*/"));
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
                cat_cstr_AlphaTab (&ast->txt, getlined_XFileB (xf, "\n"));
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
    for (ast = cdr_of_AST (ast, 0);
         ast;
         ast = cdr_of_AST (ast, 0))
    {
        if (ast->kind == Lexical_Semicolon)  return ast;
        if (ast->kind == Syntax_Braces)  return ast;
    }
    return 0;
}

    AST*
next_parens (AST* ast)
{
    for (ast = cdr_of_AST (ast, 0);
         ast;
         ast = cdr_of_AST (ast, 0))
    {
        if (ast->kind == Syntax_Parens)  return ast;
    }
    return 0;
}

void
build_stmts_AST (Cons** ast_p, ASTree* t);

    void
build_ForLoop_AST (AST* ast, ASTree* t)
{
    /* (... for (parens ...) ... ; ...)
     *  -->
     * (... (for (parens ..) (; ...)) ...)
     */
    AST* d_for;
    AST* d_parens;
    AST* d_semic;

    d_for = ast;
    Claim2( d_for->kind ,==, Lexical_For );
    d_for->kind = Syntax_ForLoop;

    d_parens = next_parens (d_for);
    if (!d_parens)
    {
        DBog1( "No parens for for-loop. Line: %u", (uint) ast->line);
        failout_sysCx ("");
    }

    d_semic = next_semicolon_or_braces_AST (d_parens);
    if (!d_semic)
    {
        DBog1( "No end of for for-loop. Line: %u", (uint) ast->line);
        failout_sysCx ("");
    }

    bevel_AST (d_for, t);
    d_for->cons->car.as.cons->cdr = d_for->cons->cdr;
    d_for->cons->cdr = d_semic->cons->cdr;
    d_semic->cons->cdr = 0;

    /* Only gets first two statements in for loop.*/
    build_stmts_AST (&d_parens->cons->car.as.cons->cdr, t);

    build_stmts_AST (&d_parens->cons->cdr, t);
}

    void
build_stmts_AST (Cons** ast_p, ASTree* t)
{
    AST* pending = 0;
    SyntaxKind pending_kind = NSyntaxKinds;
    Cons** pending_p = 0;
    AST* ast = AST_of_Cons (*ast_p);
    Cons** p = ast_p;

    for (; ast; ast = cdr_of_AST (ast, t))
    {
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
            build_ForLoop_AST (ast, t);
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

            d_parens = cdar_of_AST (d_for, 0);
            while (d_parens && d_parens->kind != Syntax_Parens)
                d_parens = cdr_of_AST (d_parens, 0);

            Claim( d_parens );
            d_stmt = cdar_of_AST (d_parens, 0);

            if (d_stmt)
            {
                AST* d_braces = req1_ASTree (t, Syntax_Braces);
                AST* d_stmt1 = req1_ASTree (t, Syntax_Stmt);

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
        ast = cdr_of_AST (ast, 0);
    }
}

    void
load_ASTree (XFileB* xf, ASTree* t)
{
    DeclAssocia( AlphaTab, uint, type_lookup,
                 (SwappedFn) swapped_AlphaTab );

    lex_AST (xf, t);
    build_stmts_AST (&t->head, t);
    xfrm_stmts_AST (&t->head, t);

    lose_Associa (type_lookup);
}

int main (int argc, char** argv)
{
    int argi =
        (init_sysCx (&argc, &argv),
         1);
    DecloStack1( ASTree, t, cons_ASTree () );
    FileB xfb = dflt_FileB ();
    XFileB* xf = 0;
    FileB ofb = dflt_FileB ();
    OFileB* of = 0;

    seto_FileB (&ofb, 1);

    while (argi < argc)
    {
        if (0 == strcmp (argv[argi], "-x"))
        {
            ++ argi;
            if (!open_FileB (&xfb, 0, argv[argi++]))
            {
                failout_sysCx ("Could not open file for reading.");
            }
            xf = &xfb.xo;
        }
        else if (0 == strcmp (argv[argi], "-o"))
        {
            ++ argi;
            if (!open_FileB (&ofb, 0, argv[argi++]))
            {
                failout_sysCx ("Could not open file for writing.");
            }
            of = &ofb.xo;
        }
        else
        {
            bool good = (0 == strcmp (argv[argi], "-h"));
            OFileB* of = (good ? stdout_OFileB () : stderr_OFileB ());
            printf_OFileB (of, "Usage: %s [-x IN] [-o OUT]\n", argv[0]);
            dump_cstr_OFileB (of, "  If -x is not specified, stdin is used.\n");
            dump_cstr_OFileB (of, "  If -o is not specified, stdout is used.\n");
            if (!good)  failout_sysCx ("Exiting in failure...");
            lose_sysCx ();
            return 0;
        }
    }

    if (!xf)  xf = stdin_XFileB ();
    if (!of)  of = stdout_OFileB ();

    load_ASTree (xf, t);
    close_XFileB (xf);
    lose_FileB (&xfb);

    dump_ASTree (of, t);
    close_OFileB (of);
    lose_FileB (&ofb);

    lose_ASTree (t);
    lose_sysCx ();
    return 0;
}

