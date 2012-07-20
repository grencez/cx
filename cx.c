
#include "associa.h"
#include "fileb.h"
#include "bstree.h"
#include "rbtree.h"
#include "sys-cx.h"
#include "table.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct AST AST;
typedef struct ASTree ASTree;
typedef struct CxCtx CxCtx;

typedef
enum SyntaxKind
{   Syntax_Root
    ,Syntax_Cons
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

    /* TODO */
    ,Syntax_Struct
    ,Syntax_Typedef

    ,Syntax_Inc

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
    BSTNode bst;
    ujint line;
    TabStr txt;
};

struct ASTree
{
    BSTree bst;
    BSTNode sentinel;
    AST* root;
};

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
    ast.bst = dflt_BSTNode ();
    ast.line = 0;
    InitTable( ast.txt );
    return ast;
}

    AST*
make_AST ()
{
    AST* ast = AllocT( AST, 1 );
    *ast = dflt_AST ();
    return ast;
}

    AST*
split_of_AST (AST* ast, Bit side)
{
    BSTNode* bst = ast->bst.split[side];
    if (!bst)  return 0;
    return CastUp( AST, bst, bst );
}

    AST*
joint_of_AST (AST* ast)
{
    BSTNode* bst = ast->bst.joint;
    if (!bst->joint)  return 0;
    return CastUp( AST, bst, bst );
}

    void
join_AST (AST* y, AST* x, Bit side)
{
    join_BSTNode (&y->bst, (x ? &x->bst : 0), side);
}

    void
init_ASTree (ASTree* t)
{
    init_BSTree (&t->bst, &t->sentinel, NULL);
    t->root = make_AST ();
    t->root->kind = Syntax_Root;
    root_fo_BSTree (&t->bst, &t->root->bst);
}

static
    void
lose_AST (AST* ast)
{
    LoseTable( ast->txt );
    free (ast);
}

static
    void
lose_bst_AST (BSTNode* bst)
{
    lose_AST (CastUp( AST, bst, bst ));
}

    void
lose_ASTree (ASTree* ast)
{
    lose_BSTree (&ast->bst, lose_bst_AST);
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
    SyntaxKind kind;
    init3_Associa (map, sizeof(TabStr), sizeof(SyntaxKind),
                   (Trit (*) (const void*, const void*)) swapped_TabStr);

    for (kind = Beg_Syntax_LexWords;
         kind < End_Syntax_LexWords;
         kind = (SyntaxKind) (kind + 1))
    {
        TabStr key = dflt1_TabStr (cstr_SyntaxKind (kind));
        insert_Associa (map, &key, &kind);
    }
}

    void
dump_AST (OFileB* of, AST* ast)
{
    if (!ast)  return;
    switch (ast->kind)
    {
    case Syntax_Root:
    case Syntax_Cons:
        do
        {
            dump_TabStr_OFileB (of, &ast->txt);
            dump_AST (of, split_of_AST (ast, 0));
            ast = split_of_AST (ast, 1);
        } while (ast);
        break;
    case Syntax_Iden:
        Claim( ast->txt.sz > 0 );
        dump_TabStr_OFileB (of, &ast->txt);
        break;
    case Syntax_CharLit:
        dump_char_OFileB (of, '\'');
        Claim( ast->txt.sz > 0 );
        dump_TabStr_OFileB (of, &ast->txt);
        dump_char_OFileB (of, '\'');
        break;
    case Syntax_StringLit:
        dump_char_OFileB (of, '"');
        Claim( ast->txt.sz > 0 );
        dump_TabStr_OFileB (of, &ast->txt);
        dump_char_OFileB (of, '"');
        break;
    case Syntax_Parens:
        dump_char_OFileB (of, '(');
        dump_TabStr_OFileB (of, &ast->txt);
        dump_AST (of, split_of_AST (ast, 0));
        dump_AST (of, split_of_AST (ast, 1));
        dump_char_OFileB (of, ')');
        break;
    case Syntax_Braces:
        dump_char_OFileB (of, '{');
        dump_TabStr_OFileB (of, &ast->txt);
        dump_AST (of, split_of_AST (ast, 0));
        dump_AST (of, split_of_AST (ast, 1));
        dump_char_OFileB (of, '}');
        break;
    case Syntax_Brackets:
        dump_char_OFileB (of, '[');
        dump_TabStr_OFileB (of, &ast->txt);
        dump_AST (of, split_of_AST (ast, 0));
        dump_AST (of, split_of_AST (ast, 1));
        dump_char_OFileB (of, ']');
        break;
    case Syntax_Stmt:
        dump_AST (of, split_of_AST (ast, 0));
        dump_AST (of, split_of_AST (ast, 1));
        dump_TabStr_OFileB (of, &ast->txt);
        dump_char_OFileB (of, ';');
        break;
    case Syntax_LineComment:
        dump_cstr_OFileB (of, "//");
        dump_TabStr_OFileB (of, &ast->txt);
        dump_char_OFileB (of, '\n');
        break;
    case Syntax_BlockComment:
        dump_cstr_OFileB (of, "/*");
        dump_TabStr_OFileB (of, &ast->txt);
        dump_cstr_OFileB (of, "*/");
        break;
    case Syntax_Directive:
        dump_char_OFileB (of, '#');
        dump_TabStr_OFileB (of, &ast->txt);
        dump_char_OFileB (of, '\n');
        break;
    case Syntax_Inc:
        dump_AST (of, split_of_AST (ast, 0));
        dump_cstr_OFileB (of, "++");
        dump_AST (of, split_of_AST (ast, 1));
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
            DBog0( "No Good!" );
        }
        break;
    }
}

    void
dump_ASTree (OFileB* of, ASTree* t)
{
    dump_AST (of, t->root);
}

    AST*
app_AST (AST* ast)
{
    AST* b;
    if (ast->bst.split[0])
    {
        Claim( !ast->bst.split[1] );
        b = make_AST ();
        b->kind = Syntax_Cons;
        join_BSTNode (&ast->bst, &b->bst, 1);
        ast = b;
    }

    b = make_AST ();
    join_BSTNode (&ast->bst, &b->bst, 0);
    return b;
}

    AST*
wsnode_AST (AST* ast)
{
    if (ast->bst.split[0])
    {
        AST* b;
        Claim( !ast->bst.split[1] );
        b = make_AST ();
        b->kind = Syntax_Cons;
        join_BSTNode (&ast->bst, &b->bst, 1);
        ast = b;
    }
    return ast;
}

    bool
parse_escaped (XFileB* xf, TabStr* t, char delim)
{
    char delims[2];
    char* s;

    delims[0] = delim;
    delims[1] = 0;

    for (s = nextds_XFileB (xf, 0, delims);
         s;
         s = nextds_XFileB (xf, 0, delims))
    {
        bool escaped = false;
        ujint off;

        app_TabStr (t, s);
        off = t->sz-1;

        while (off > 0 && t->s[off-1] == '\\')
        {
            escaped = !escaped;
            -- off;
        }
        if (escaped)
            app_TabStr (t, delims);
        else
            return true;
    }
    return false;
}

    ujint
count_newlines (const char* s)
{
    ujint n = 0;
    for (s = strchr (s, '\n');  s;  s = strchr (&s[1], '\n'))
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
lex_AST (XFileB* xf, AST* ast)
{
    char match = 0;
    char* s;
    const char delims[] = "'\"(){}[];#+-*/%&^|~!.,?:><=";
    ujint off;
    ujint line = 0;
    DecloStack( Associa, keyword_map );

    init_lexwords (keyword_map);

    for (s = nextds_XFileB (xf, &match, delims);
         s;
         s = nextds_XFileB (xf, &match, delims))
    {
        AST* lo_ast;

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
                    TabStr ts = TabStr_XFileB (olay, 0);
                    ast = wsnode_AST (ast);
                    AffyTable( ast->txt, ts.sz+1 );
                    cat_TabStr (&ast->txt, &ts);
                    line += count_newlines (ast->txt.s);
                }
                off += olay->off;
                *olay = olay_XFileB (xf, off);
                if (!olay->buf.s[0])  break;

                olay->off = IdxEltTable( olay->buf, tods_XFileB (olay, 0) );
                if (olay->off > 0)
                {
                    TabStr ts = TabStr_XFileB (olay, 0);
                    Assoc* luk = lookup_Associa (keyword_map, &ts);

                    ast = app_AST (ast);

                    if (luk)
                    {
                        ast->kind = *(SyntaxKind*) val_of_Assoc (luk);
                    }
                    else
                    {
                        ast->kind = Syntax_Iden;
                        AffyTable( ast->txt, ts.sz+1 );
                        cat_TabStr (&ast->txt, &ts);
                    }
                    ast->line = line;
                    ast = joint_of_AST (ast);
                }

                off += olay->off;
                *olay = olay_XFileB (xf, off);
            }
        }

        lo_ast = split_of_AST (ast, 0);

        switch (match)
        {
        case '\0':
                /* End of file has been reached.*/
            break;
        case '\'':
            ast = app_AST (ast);
            ast->kind = Syntax_CharLit;
            ast->line = line;
            if (!parse_escaped (xf, &ast->txt, '\''))
                DBog1( "Gotta problem with single quotes! line:%u",
                       (uint) line );
            ast = joint_of_AST (ast);
            break;
        case '"':
            ast = app_AST (ast);
            ast->kind = Syntax_StringLit;
            ast->line = line;
            if (!parse_escaped (xf, &ast->txt, '"'))
                DBog1( "Gotta problem with double quotes! line:%u",
                       (uint) line );
            ast = joint_of_AST (ast);
            break;
        case '(':
            ast = app_AST (ast);
            ast->kind = Syntax_Parens;
            ast->line = line;
            break;
        case '{':
            ast = app_AST (ast);
            ast->kind = Syntax_Braces;
            ast->line = line;
            break;
        case '[':
            ast = app_AST (ast);
            ast->kind = Syntax_Brackets;
            ast->line = line;
            break;
        case ')':
        case '}':
        case ']':
            while (ast)
            {
                if ((match == ')' && ast->kind == Syntax_Parens) ||
                    (match == '}' && ast->kind == Syntax_Braces) ||
                    (match == ']' && ast->kind == Syntax_Brackets))
                {
                    ast = joint_of_AST (ast);
                    break;
                }
                ast = joint_of_AST (ast);
            }
            if (!ast)
            {
                DBog1( "Unmatched closing '%c'.", match );
                return;
            }
            break;
        case '#':
            ast = app_AST (ast);
            ast->kind = Syntax_Directive;
            ast->line = line;
            app_TabStr (&ast->txt, getlined_XFileB (xf, "\n"));
            ++ line;
            ast = joint_of_AST (ast);
            break;

#define LexiCase( c, k )  case c: \
            ast = app_AST (ast); \
            ast->kind = k; \
            ast->line = line; \
            ast = joint_of_AST (ast); \
            break;

#define Lex2Case( c, k1, k2 )  case c: \
            if (lo_ast && lo_ast->kind == k1) \
            { \
                lo_ast->kind = k2; \
            } \
            else \
            { \
                ast = app_AST (ast); \
                ast->kind = k1; \
                ast->line = line; \
                ast = joint_of_AST (ast); \
            } \
            break;

            Lex2Case( '+', Lexical_Add, Lexical_Inc );
            Lex2Case( '-', Lexical_Sub, Lexical_Dec );
        case '*':
            if (lo_ast && lo_ast->kind == Lexical_Div)
            {
                lo_ast->kind = Syntax_BlockComment;
                app_TabStr (&lo_ast->txt, getlined_XFileB (xf, "*/"));
                line += count_newlines (lo_ast->txt.s);
            }
            else
            {
                ast = app_AST (ast);
                ast->kind = Lexical_Mul;
                ast->line = line;
                ast = joint_of_AST (ast);
            }
            break;
        case '/':
            if (lo_ast && lo_ast->kind == Lexical_Div)
            {
                lo_ast->kind = Syntax_LineComment;
                app_TabStr (&lo_ast->txt, getlined_XFileB (xf, "\n"));
                ++ line;
            }
            else
            {
                ast = app_AST (ast);
                ast->kind = Lexical_Div;
                ast->line = line;
                ast = joint_of_AST (ast);
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
            if (lo_ast && lo_ast->kind == Lexical_GT)
            {
                lo_ast->kind = Lexical_RShift;
            }
            else if (lo_ast && lo_ast->kind == Lexical_Sub)
            {
                lo_ast->kind = Lexical_PMemb;
            }
            else
            {
                ast = app_AST (ast);
                ast->kind = Lexical_GT;
                ast = joint_of_AST (ast);
            }
            break;
            Lex2Case( '<', Lexical_LT, Lexical_LShift );
        case '=':
            if (lo_ast && lo_ast->kind == Lexical_Add)
                lo_ast->kind = Lexical_AddAssign;
            else if (lo_ast && lo_ast->kind == Lexical_Sub)
                lo_ast->kind = Lexical_SubAssign;
            else if (lo_ast && lo_ast->kind == Lexical_Mul)
                lo_ast->kind = Lexical_MulAssign;
            else if (lo_ast && lo_ast->kind == Lexical_Div)
                lo_ast->kind = Lexical_DivAssign;
            else if (lo_ast && lo_ast->kind == Lexical_Mod)
                lo_ast->kind = Lexical_ModAssign;
            else if (lo_ast && lo_ast->kind == Lexical_BitAnd)
                lo_ast->kind = Lexical_BitAndAssign;
            else if (lo_ast && lo_ast->kind == Lexical_BitXor)
                lo_ast->kind = Lexical_BitXorAssign;
            else if (lo_ast && lo_ast->kind == Lexical_BitOr)
                lo_ast->kind = Lexical_BitOrAssign;
            else if (lo_ast && lo_ast->kind == Lexical_Not)
                lo_ast->kind = Lexical_NotEq;
            else if (lo_ast && lo_ast->kind == Lexical_GT)
                lo_ast->kind = Lexical_GTEq;
            else if (lo_ast && lo_ast->kind == Lexical_RShift)
                lo_ast->kind = Lexical_RShiftAssign;
            else if (lo_ast && lo_ast->kind == Lexical_LT)
                lo_ast->kind = Lexical_LTEq;
            else if (lo_ast && lo_ast->kind == Lexical_LShift)
                lo_ast->kind = Lexical_LShiftAssign;
            else if (lo_ast && lo_ast->kind == Lexical_Assign)
                lo_ast->kind = Lexical_Eq;
            else
            {
                ast = app_AST (ast);
                ast->kind = Lexical_Assign;
                ast->line = line;
                ast = joint_of_AST (ast);
            }
            break;
#undef Lex2Case
#undef LexiCase
        }
    }
    while (ast)
    {
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
        ast = joint_of_AST (ast);
    }
    lose_Associa (keyword_map);
}

    void
build_stmts_AST (AST* ast)
{
    AST* pending = 0;
    SyntaxKind pending_kind = NSyntaxKinds;

    for (; ast; ast = split_of_AST (ast, 1))
    {
        AST* lo_ast = split_of_AST (ast, 0);

        if (pending)
        {
            if (pending_kind == NSyntaxKinds)
                pending_kind = split_of_AST (pending, 0) -> kind;
        }
        else
        {
            pending_kind = NSyntaxKinds;
        }

        if (!lo_ast)
        {
            Claim( !split_of_AST (ast, 1) );
            return;
        }
        switch  (lo_ast->kind)
        {
        case Syntax_Directive:
        case Syntax_LineComment:
        case Syntax_BlockComment:
            break;
        case Syntax_Parens:
            if (pending_kind == Lexical_For)
            {
                    /* Only gets first two statements in for loop.*/
                build_stmts_AST (lo_ast);
            }
            if (!pending)
            {
                pending = ast;
                pending_kind = lo_ast->kind;
            }
            break;
        case Syntax_Braces:
            build_stmts_AST (lo_ast);
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
                    /* TODO: Remember the damn ternary operator.*/
            }
            pending = 0;
            break;
        case Lexical_Semicolon:

            lo_ast->kind = Syntax_Stmt;
                /* Empty statement.*/
            if (!pending)  break;

                /*   \               \
                 *    p               p
                 *   / \             / \
                 *  a   0     =>   ';   2
                 *     / \         / \
                 *    b   1       a   0
                 *       / \         /
                 *     ';   2       b
                 *
                 * Where /';/ is the semicolon. It is changed to a statement.
                 * Numbered nodes are known to be Cons.
                 * Statement parts are just /a/ and /b/.
                 */
            join_AST (lo_ast, split_of_AST (pending, 0), 0);
            join_AST (lo_ast, split_of_AST (pending, 1), 1);
            join_AST (pending, lo_ast, 0);

            join_AST (pending, split_of_AST (ast, 1), 1);
            join_AST (joint_of_AST (ast), 0, 1);

            lo_ast->txt = ast->txt;
            ast->txt = dflt_TabStr ();
            lose_AST (ast);
            ast = pending;

            pending = 0;
            break;
        default:
            if (!pending)
            {
                pending = ast;
                pending_kind = lo_ast->kind;
            }
            break;
        }
    }
}

    void
load_ASTree (XFileB* xf, ASTree* t)
{
    DecloStack( CxCtx, ctx );
    ctx->ast = *t;
    init3_Associa (&ctx->type_lookup, sizeof(TabStr), sizeof(uint),
                   (Trit (*) (const void*, const void*)) swapped_TabStr);

    lex_AST (xf, t->root);

    build_stmts_AST (ctx->ast.root);

    *t = ctx->ast;
    lose_Associa (&ctx->type_lookup);
}

int main (int argc, char** argv)
{
    ASTree t;
    DecloStack( FileB, fb );

    init_sys_cx ();

    if (argc != 3)
    {
        DBog0( "Use 2 arguments." );
        return 1;
    }

    init_FileB (fb);
    open_FileB (fb, 0, argv[1]);

    init_ASTree (&t);
    load_ASTree (&fb->xo, &t);
    close_FileB (fb);

    seto_FileB (fb, 1);
    open_FileB (fb, 0, argv[2]);
    dump_ASTree (&fb->xo, &t);
    lose_FileB (fb);

    lose_ASTree (&t);
    lose_sys_cx ();
    return 0;
}

