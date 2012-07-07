
#include "fileb.h"
#include "bstree.h"
#include "sys-cx.h"
#include "table.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct ASTree ASTree;
typedef struct AST AST;

typedef
enum SyntaxKind
{   Syntax_Root
    ,Syntax_Cons
    ,Syntax_WhiteSpace
    ,Syntax_Iden
    ,Syntax_LineComment
    ,Syntax_BlockComment
    ,Syntax_Directive
    ,Syntax_Char
    ,Syntax_String
    ,Syntax_Parens
    ,Syntax_Braces
    ,Syntax_Brackets
    ,Syntax_Stmt

    ,Lexical_Add
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

    ,NSyntaxKinds
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


    AST
dflt_AST ()
{
    AST ast;
    ast.kind = NSyntaxKinds;
    ast.bst.split[0] = 0;
    ast.bst.split[1] = 0;
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
side_of_AST (AST* ast, Bit side)
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
set_side_AST (AST* a, AST* b, Bit side)
{
    a->bst.split[side] = &b->bst;
    b->bst.joint       = &a->bst;
}

    void
init_ASTree (ASTree* t)
{
    init_BSTree (&t->bst, &t->sentinel, NULL);
    t->root = make_AST ();
    t->root->kind = Syntax_Root;
    root_for_BSTree (&t->bst, &t->root->bst);
}


static
    void
lose_AST (BSTNode* bst)
{
    AST* ast = CastUp( AST, bst, bst );
    LoseTable( ast->txt );
    free (ast);
}

    void
lose_ASTree (ASTree* ast)
{
    lose_BSTree (&ast->bst, lose_AST);
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
            dump_AST (of, side_of_AST (ast, 0));
            ast = side_of_AST (ast, 1);
        } while (ast);
        break;
    case Syntax_WhiteSpace:
        Claim( ast->txt.sz > 0 );
        dump_cstr_OFileB (of, ast->txt.s);
        break;
    case Syntax_Iden:
        Claim( ast->txt.sz > 0 );
        dump_cstr_OFileB (of, ast->txt.s);
        break;
    case Syntax_Char:
        dump_char_OFileB (of, '\'');
        Claim( ast->txt.sz > 0 );
        dump_cstr_OFileB (of, ast->txt.s);
        dump_char_OFileB (of, '\'');
        break;
    case Syntax_String:
        dump_char_OFileB (of, '"');
        Claim( ast->txt.sz > 0 );
        dump_cstr_OFileB (of, ast->txt.s);
        dump_char_OFileB (of, '"');
        break;
    case Syntax_Parens:
        dump_char_OFileB (of, '(');
        dump_AST (of, side_of_AST (ast, 0));
        dump_AST (of, side_of_AST (ast, 1));
        dump_char_OFileB (of, ')');
        break;
    case Syntax_Braces:
        dump_char_OFileB (of, '{');
        dump_AST (of, side_of_AST (ast, 0));
        dump_AST (of, side_of_AST (ast, 1));
        dump_char_OFileB (of, '}');
        break;
    case Syntax_Brackets:
        dump_char_OFileB (of, '[');
        dump_AST (of, side_of_AST (ast, 0));
        dump_AST (of, side_of_AST (ast, 1));
        dump_char_OFileB (of, ']');
        break;
    case Syntax_Stmt:
        dump_AST (of, side_of_AST (ast, 0));
        dump_AST (of, side_of_AST (ast, 1));
        dump_char_OFileB (of, ';');
        break;
    case Syntax_LineComment:
        dump_cstr_OFileB (of, "//");
        dump_cstr_OFileB (of, ast->txt.s);
        dump_char_OFileB (of, '\n');
        break;
    case Syntax_BlockComment:
        dump_cstr_OFileB (of, "/*");
        dump_cstr_OFileB (of, ast->txt.s);
        dump_cstr_OFileB (of, "*/");
        break;
    case Syntax_Directive:
        dump_char_OFileB (of, '#');
        dump_cstr_OFileB (of, ast->txt.s);
        dump_char_OFileB (of, '\n');
        break;
#define LexiCase( s, k )  case k: \
        dump_cstr_OFileB (of, s); \
        break;

        LexiCase( "+"  , Lexical_Add );
        LexiCase( "++" , Lexical_Inc );
        LexiCase( "-"  , Lexical_Sub );
        LexiCase( "--" , Lexical_Dec );
        LexiCase( "*"  , Lexical_Mul );
        LexiCase( "/"  , Lexical_Div );
        LexiCase( "%"  , Lexical_Mod );
        LexiCase( "&"  , Lexical_BitAnd );
        LexiCase( "&&" , Lexical_And );
        LexiCase( "^"  , Lexical_BitXor );
        LexiCase( "|"  , Lexical_BitOr );
        LexiCase( "||" , Lexical_Or );
        LexiCase( "~"  , Lexical_BitNot );
        LexiCase( "!"  , Lexical_Not );
        LexiCase( "."  , Lexical_Dot );
        LexiCase( ","  , Lexical_Comma );
        LexiCase( "?"  , Lexical_Question );
        LexiCase( ":"  , Lexical_Colon );
        LexiCase( ">"  , Lexical_GT );
        LexiCase( "->" , Lexical_PMemb );
        LexiCase( ">>" , Lexical_RShift );
        LexiCase( "<"  , Lexical_LT );
        LexiCase( "<<" , Lexical_LShift );
        LexiCase( "="  , Lexical_Assign );
        LexiCase( "+=" , Lexical_AddAssign );
        LexiCase( "-=" , Lexical_SubAssign );
        LexiCase( "*=" , Lexical_MulAssign );
        LexiCase( "/=" , Lexical_DivAssign );
        LexiCase( "%=" , Lexical_ModAssign );
        LexiCase( "&=" , Lexical_BitAndAssign );
        LexiCase( "^=" , Lexical_BitXorAssign );
        LexiCase( "|=" , Lexical_BitOrAssign );
        LexiCase( "!=" , Lexical_NotEq );
        LexiCase( ">=" , Lexical_GTEq );
        LexiCase( ">>=", Lexical_RShiftAssign );
        LexiCase( "<=" , Lexical_LTEq );
        LexiCase( "<<=", Lexical_LShiftAssign );
        LexiCase( "==" , Lexical_Eq );
#undef LexiCase
    default:
        DBog0( "No Good!" );
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

    void
load_ASTree (XFileB* xf, ASTree* t)
{
    AST* ast = t->root;
    char match = 0;
    char* s;
    const char delims[] = "'\"(){}[];#+-*/%&^|~!.,?:><=";
    ujint off;
    ujint line = 0;

    mayflush_XFileB (xf, Yes);

        /* Tokenize while dealing with
         * - line/block comment
         * - directive (perhaps this should happen later)
         * - char, string
         * - parentheses, braces, brackets
         * - statement ending with semicolon
         */
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

            while (olay->buf.sz > 1)
            {
                skipds_XFileB (olay, 0);
                if (olay->off > 0)
                {
                    TabStr ts = TabStr_XFileB (olay, 0);
                    ast = app_AST (ast);
                    ast->kind = Syntax_WhiteSpace;
                    ast->line = line;
                    cat_TabStr (&ast->txt, &ts);
                    line += count_newlines (ast->txt.s);
                    ast = joint_of_AST (ast);
                }
                off += olay->off;
                *olay = olay_XFileB (xf, off);
                if (olay->buf.sz <= 1)  break;

                olay->off = IdxEltTable( olay->buf, tods_XFileB (olay, 0));
                if (olay->off > 0)
                {
                    TabStr ts = TabStr_XFileB (olay, 0);
                    ast = app_AST (ast);
                    ast->kind = Syntax_Iden;
                    ast->line = line;
                    cat_TabStr (&ast->txt, &ts);
                    ast = joint_of_AST (ast);
                }

                off += olay->off;
                *olay = olay_XFileB (xf, off);
            }
        }

        lo_ast = side_of_AST (ast, 0);

        switch (match)
        {
        case '\0':
                /* End of file has been reached.*/
            break;
        case '\'':
            ast = app_AST (ast);
            ast->kind = Syntax_Char;
            ast->line = line;
            if (!parse_escaped (xf, &ast->txt, '\''))
                DBog1( "Gotta problem with single quotes! line:%u",
                       (uint) line );
            ast = joint_of_AST (ast);
            break;
        case '"':
            ast = app_AST (ast);
            ast->kind = Syntax_String;
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
        case ';':
            while (ast->kind == Syntax_Cons && lo_ast->kind != Syntax_Stmt)
            {
                side_of_AST (ast, 0);
                lo_ast = ast;
                ast = joint_of_AST (ast);
            }
            if (ast->kind == Syntax_Cons)  ast = lo_ast;

            lo_ast = make_AST ();
            lo_ast->kind = Syntax_Stmt;
            lo_ast->line = line;
            join_BSTNode (&lo_ast->bst, ast->bst.split[0], 0);
            join_BSTNode (&lo_ast->bst, ast->bst.split[1], 1);
            ast->bst.split[0] = 0;
            ast->bst.split[1] = 0;
            join_BSTNode (&ast->bst, &lo_ast->bst, 0);
            ast = joint_of_AST (lo_ast);
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

