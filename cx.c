
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
    ,Syntax_Plain
    ,Syntax_LineComment
    ,Syntax_BlockComment
    ,Syntax_Directive
    ,Syntax_Char
    ,Syntax_String
    ,Syntax_Parens
    ,Syntax_Braces
    ,Syntax_Brackets
    ,Syntax_Stmt
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
    case Syntax_Plain:
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
    default:
        DBog0( "No Good!" );
        break;
    };
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
            --off;
        }
        if (escaped)
            app_TabStr (t, delims);
        else
            return true;
    }
    return false;
}

    void
load_ASTree (XFileB* xf, ASTree* t)
{
    AST* ast = t->root;
    char match = 0;
    char* s;
    DeclTable( char, txtq );

        /* First parse:
         * - line/block comment
         * - directive (perhaps this should happen later)
         * - char, string
         * - parentheses, braces, brackets
         * - semicolon
         */
    for (s = nextds_XFileB (xf, &match, "'\"(){}[];/#");
         s;
         s = nextds_XFileB (xf, &match, "'\"(){}[];/#"))
    {
        if (s[0])
            app_TabStr (&txtq, s);

        if (match == '/')
        {
            bool comment = false;
            char c = 0;
            if (load_char_XFileB (xf, &c))
            {
                if (c == '/')  comment = true;
                else if (c == '*')  comment = true;
                else  -- xf->off;
            }

            if (comment)
            {
                match = c;
            }
            else
            {
                xf->buf.s[xf->off-1] = (byte) '/';
                app_TabStr (&txtq, "/");
                continue;
            }
        }

        if (txtq.sz > 0)
        {
            ast = app_AST (ast);
            ast->kind = Syntax_Plain;
            CopyTable( ast->txt, txtq );
            txtq.sz = 0;
            ast = joint_of_AST (ast);
        }

        switch (match)
        {
        case '\0':
                /* End of file has been reached.*/
            break;
        case '\'':
            ast = app_AST (ast);
            ast->kind = Syntax_Char;
            if (!parse_escaped (xf, &ast->txt, '\''))
                DBog0( "Gotta problem with single quotes!" );
            ast = joint_of_AST (ast);
            break;
        case '"':
            ast = app_AST (ast);
            ast->kind = Syntax_String;
            if (!parse_escaped (xf, &ast->txt, '"'))
                DBog0( "Gotta problem with double quotes!" );
            ast = joint_of_AST (ast);
            break;
        case '(':
            ast = app_AST (ast);
            ast->kind = Syntax_Parens;
            break;
        case '{':
            ast = app_AST (ast);
            ast->kind = Syntax_Braces;
            break;
        case '[':
            ast = app_AST (ast);
            ast->kind = Syntax_Brackets;
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
                /* TODO: Insert me at the right spot
                 * and as the right thing!
                 */
            if (1)
            {
                ast = app_AST (ast);
                ast->kind = Syntax_Plain;
                app_TabStr (&ast->txt, ";");
                ast = joint_of_AST (ast);
            }
            else
            {
                AST* b = ast;
                while (ast->kind == Syntax_Cons)
                {
                    side_of_AST (ast, 0);
                    b = ast;
                    ast = joint_of_AST (ast);
                }
            }
            break;
        case '/':
            ast = app_AST (ast);
            ast->kind = Syntax_LineComment;
            app_TabStr (&ast->txt, getlined_XFileB (xf, "\n"));
            ast = joint_of_AST (ast);
            break;
        case '*':
            ast = app_AST (ast);
            ast->kind = Syntax_BlockComment;
            app_TabStr (&ast->txt, getlined_XFileB (xf, "*/"));
            ast = joint_of_AST (ast);
            break;
        case '#':
            ast = app_AST (ast);
            ast->kind = Syntax_Directive;
            app_TabStr (&ast->txt, getlined_XFileB (xf, "\n"));
            ast = joint_of_AST (ast);
            break;
        }
    }
    while (ast)
    {
        switch (ast->kind)
        {
        case Syntax_Parens:
            DBog0( "Unclosed '('." );
            break;
        case Syntax_Braces:
            DBog0( "Unclosed '{'." );
            break;
        case Syntax_Brackets:
            DBog0( "Unclosed '['." );
            break;
        default:
            break;
        }
        ast = joint_of_AST (ast);
    }
    LoseTable( txtq );
}

int main (int argc, char** argv)
{
    ASTree t;
    init_sys_cx ();
    (void) argc;
    (void) argv;

    init_ASTree (&t);
    load_ASTree (stdin_XFileB (), &t);
    dump_ASTree (stdout_OFileB (), &t);

    lose_ASTree (&t);
    lose_sys_cx ();
    return 0;
}

