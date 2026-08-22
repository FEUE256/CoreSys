#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdint.h>

/* =========================================================
 * EMBEDDED TC RUNTIME
 * ========================================================= */

static const char *TC_RUNTIME =
"#ifndef TC_RUNTIME_H\n"
"#define TC_RUNTIME_H\n"
"\n"
"typedef int64_t tc_val;\n"
"\n"
"/* ---------------------------------------------------------\n"
" * Core\n"
" * --------------------------------------------------------- */\n"
"\n"
"static void tc_func_init(void)\n"
"{\n"
"}\n"
"\n"
"static void tc_ret_init(void)\n"
"{\n"
"}\n"
"\n"
"static void tc_loops_init(void)\n"
"{\n"
"}\n"
"\n"
"static void tc_graphics_init(void)\n"
"{\n"
"}\n"
"\n"
"static void tc_init(void)\n"
"{\n"
"    tc_func_init();\n"
"    tc_ret_init();\n"
"    tc_loops_init();\n"
"    tc_graphics_init();\n"
"}\n"
"\n"
"static void tc_noop(void)\n"
"{\n"
"}\n"
"\n"
"/* ---------------------------------------------------------\n"
" * Printing\n"
" * --------------------------------------------------------- */\n"
"\n"
"static void tc_printf(const char *fmt, ...)\n"
"{\n"
"    va_list ap;\n"
"    va_start(ap, fmt);\n"
"    vprintf(fmt, ap);\n"
"    va_end(ap);\n"
"}\n"
"\n"
"static void tc_print(const char *str)\n"
"{\n"
"    if (str != NULL)\n"
"        fputs(str, stdout);\n"
"}\n"
"\n"
"/* ---------------------------------------------------------\n"
" * String conversion\n"
" * --------------------------------------------------------- */\n"
"\n"
"static tc_val tc_str_to_val(const char *str)\n"
"{\n"
"    if (str == NULL)\n"
"        return 0;\n"
"\n"
"    char *end = NULL;\n"
"    long long value = strtoll(str, &end, 0);\n"
"\n"
"    if (end == str)\n"
"        return 0;\n"
"\n"
"    return (tc_val)value;\n"
"}\n"
"\n"
"static const char *tc_val_to_str(tc_val value)\n"
"{\n"
"    static char buf[4096];\n"
"    snprintf(buf, sizeof(buf), \"%lld\", (long long)value);\n"
"    return buf;\n"
"}\n"
"\n"
"static const char *tc_str_interp(const char *fmt, ...)\n"
"{\n"
"    static char buf[4096];\n"
"    va_list ap;\n"
"    va_start(ap, fmt);\n"
"    vsnprintf(buf, sizeof(buf), fmt, ap);\n"
"    va_end(ap);\n"
"    return buf;\n"
"}\n"
"\n"
"/* ---------------------------------------------------------\n"
" * Math\n"
" * --------------------------------------------------------- */\n"
"\n"
"static tc_val tc_math_add(tc_val a, tc_val b)\n"
"{\n"
"    return a + b;\n"
"}\n"
"\n"
"static tc_val tc_math_subtract(tc_val a, tc_val b)\n"
"{\n"
"    return a - b;\n"
"}\n"
"\n"
"static tc_val tc_math_multiply(tc_val a, tc_val b)\n"
"{\n"
"    return a * b;\n"
"}\n"
"\n"
"static tc_val tc_math_divide(tc_val a, tc_val b)\n"
"{\n"
"    if (b == 0)\n"
"        return 0;\n"
"    return a / b;\n"
"}\n"
"\n"
"static tc_val tc_math_modulo(tc_val a, tc_val b)\n"
"{\n"
"    if (b == 0)\n"
"        return 0;\n"
"    return a % b;\n"
"}\n"
"\n"
"static tc_val tc_math_equal(tc_val a, tc_val b)\n"
"{\n"
"    return a == b;\n"
"}\n"
"\n"
"static tc_val tc_math_not_equal(tc_val a, tc_val b)\n"
"{\n"
"    return a != b;\n"
"}\n"
"\n"
"static tc_val tc_math_less_then(tc_val a, tc_val b)\n"
"{\n"
"    return a < b;\n"
"}\n"
"\n"
"static tc_val tc_math_greater_then(tc_val a, tc_val b)\n"
"{\n"
"    return a > b;\n"
"}\n"
"\n"
"static tc_val tc_math_greater_then_or_equal(tc_val a, tc_val b)\n"
"{\n"
"    return a >= b;\n"
"}\n"
"\n"
"static tc_val tc_math_less_then_or_equal(tc_val a, tc_val b)\n"
"{\n"
"    return a <= b;\n"
"}\n"
"\n"
"#endif\n"
;

/* =========================================================
 * UTILITIES
 * ========================================================= */

static void die(const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);

    fprintf(stderr, "tc2 error: ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");

    va_end(ap);
    exit(1);
}

static char *xstrdup(const char *s)
{
    char *r = strdup(s);

    if (!r)
        die("out of memory");

    return r;
}

static char *xstrndup(const char *s, size_t n)
{
    char *r = strndup(s, n);

    if (!r)
        die("out of memory");

    return r;
}

/* =========================================================
 * LEXER
 * ========================================================= */

typedef enum {
    TK_EOF = 0,
    TK_IDENT,
    TK_NUMBER,
    TK_STRING,
    TK_DOT,
    TK_COMMA,
    TK_SEMICOLON,
    TK_LPAREN,
    TK_RPAREN,
    TK_LBRACE,
    TK_RBRACE,
    TK_COLON,
    TK_DOLLAR,
    TK_EQUALS,

    TK_ASMKW,
    TK_CKEYWORD,

    TK_FUNC,
    TK_RET,
    TK_IF,
    TK_ELSE,
    TK_LOOP,
    TK_JUMP
} TKind;

typedef struct {
    TKind kind;
    char *val;
    int line;
} Token;

typedef struct {
    const char *src;
    size_t pos;
    int line;
    Token cur;
} Lexer;

static void token_free(Token *t)
{
    free(t->val);
    t->val = NULL;
}

static Token lex_next(Lexer *lx);

static void lexer_init(Lexer *lx, const char *src)
{
    lx->src = src;
    lx->pos = 0;
    lx->line = 1;
    lx->cur = lex_next(lx);
}

static char lx_advance(Lexer *lx)
{
    char c = lx->src[lx->pos++];

    if (c == '\n')
        lx->line++;

    return c;
}

static Token make_tok(TKind k, const char *v, int line)
{
    Token t;

    t.kind = k;
    t.val = xstrdup(v);
    t.line = line;

    return t;
}

static Token lex_next(Lexer *lx)
{
again:

    while (lx->src[lx->pos] &&
           isspace((unsigned char)lx->src[lx->pos]))
        lx_advance(lx);

    if (!lx->src[lx->pos])
        return make_tok(TK_EOF, "", lx->line);

    int line = lx->line;
    char c = lx->src[lx->pos];

    /* ---------------------------------------------------------
     * License block
     * ^~ ... ~^
     * --------------------------------------------------------- */

    if (c == '^' && lx->src[lx->pos + 1] == '~') {
        lx->pos += 2;

        while (lx->src[lx->pos]) {
            if (lx->src[lx->pos] == '~' &&
                lx->src[lx->pos + 1] == '^') {
                lx->pos += 2;
                break;
            }

            if (lx->src[lx->pos] == '\n')
                lx->line++;

            lx->pos++;
        }

        goto again;
    }

    /* ---------------------------------------------------------
     * # line comment
     * --------------------------------------------------------- */

    if (c == '#') {
        while (lx->src[lx->pos] &&
               lx->src[lx->pos] != '\n')
            lx->pos++;

        goto again;
    }

    /* ---------------------------------------------------------
     * // ... \\ block comment
     * --------------------------------------------------------- */

    if (c == '/' && lx->src[lx->pos + 1] == '/') {
        lx->pos += 2;

        while (lx->src[lx->pos]) {
            if (lx->src[lx->pos] == '\\' &&
                lx->src[lx->pos + 1] == '\\') {
                lx->pos += 2;
                break;
            }

            if (lx->src[lx->pos] == '\n')
                lx->line++;

            lx->pos++;
        }

        goto again;
    }

    /* ---------------------------------------------------------
     * Number literals
     * --------------------------------------------------------- */

    if (isdigit((unsigned char)c)) {
        size_t start = lx->pos;

        if (c == '0' &&
            (lx->src[lx->pos + 1] == 'x' ||
             lx->src[lx->pos + 1] == 'X')) {

            lx->pos += 2;

            while (isxdigit((unsigned char)lx->src[lx->pos]))
                lx->pos++;

        } else if (c == '0' &&
                   (lx->src[lx->pos + 1] == 'b' ||
                    lx->src[lx->pos + 1] == 'B')) {

            lx->pos += 2;

            while (lx->src[lx->pos] == '0' ||
                   lx->src[lx->pos] == '1')
                lx->pos++;

        } else {

            while (isdigit((unsigned char)lx->src[lx->pos]))
                lx->pos++;
        }

        char *v = xstrndup(
            lx->src + start,
            lx->pos - start
        );

        Token t = make_tok(TK_NUMBER, v, line);

        free(v);

        return t;
    }

    /* ---------------------------------------------------------
     * String literals
     * --------------------------------------------------------- */

    if (c == '"' || c == '\'') {
        char delim = c;

        lx->pos++;

        size_t start = lx->pos;

        while (lx->src[lx->pos] &&
               lx->src[lx->pos] != delim) {

            if (lx->src[lx->pos] == '\n')
                lx->line++;

            lx->pos++;
        }

        char *v = xstrndup(
            lx->src + start,
            lx->pos - start
        );

        if (lx->src[lx->pos] == delim)
            lx->pos++;

        Token t = make_tok(TK_STRING, v, line);

        free(v);

        return t;
    }

    /* ---------------------------------------------------------
     * Identifiers / keywords
     * --------------------------------------------------------- */

    if (isalpha((unsigned char)c) || c == '_') {
        size_t start = lx->pos;

        while (isalnum((unsigned char)lx->src[lx->pos]) ||
               lx->src[lx->pos] == '_')
            lx->pos++;

        char *v = xstrndup(
            lx->src + start,
            lx->pos - start
        );

        TKind k = TK_IDENT;

        if (!strcmp(v, "func"))
            k = TK_FUNC;
        else if (!strcmp(v, "ret"))
            k = TK_RET;
        else if (!strcmp(v, "if"))
            k = TK_IF;
        else if (!strcmp(v, "else"))
            k = TK_ELSE;
        else if (!strcmp(v, "loop"))
            k = TK_LOOP;
        else if (!strcmp(v, "jump"))
            k = TK_JUMP;
        else if (!strcmp(v, "asm"))
            k = TK_ASMKW;
        else if (!strcmp(v, "c"))
            k = TK_CKEYWORD;

        Token t = make_tok(k, v, line);

        free(v);

        return t;
    }

    /* ---------------------------------------------------------
     * Single-character tokens
     * --------------------------------------------------------- */

    lx->pos++;

    switch (c) {
        case '.':
            return make_tok(TK_DOT, ".", line);

        case ',':
            return make_tok(TK_COMMA, ",", line);

        case ';':
            return make_tok(TK_SEMICOLON, ";", line);

        case '(':
            return make_tok(TK_LPAREN, "(", line);

        case ')':
            return make_tok(TK_RPAREN, ")", line);

        case '{':
            return make_tok(TK_LBRACE, "{", line);

        case '}':
            return make_tok(TK_RBRACE, "}", line);

        case ':':
            return make_tok(TK_COLON, ":", line);

        case '$':
            return make_tok(TK_DOLLAR, "$", line);

        case '=':
            return make_tok(TK_EQUALS, "=", line);

        default:
            goto again;
    }
}

static Token lx_consume(Lexer *lx)
{
    Token t = lx->cur;

    lx->cur = lex_next(lx);

    return t;
}

static TKind lx_kind(Lexer *lx)
{
    return lx->cur.kind;
}

static Token lx_expect(
    Lexer *lx,
    TKind k,
    const char *what
)
{
    if (lx->cur.kind != k)
        die(
            "line %d: expected %s, got '%s'",
            lx->cur.line,
            what,
            lx->cur.val
        );

    return lx_consume(lx);
}

static void lx_skip_semi(Lexer *lx)
{
    if (lx->cur.kind == TK_SEMICOLON) {
        token_free(&lx->cur);
        lx->cur = lex_next(lx);
    }
}

/* =========================================================
 * TYPES
 * ========================================================= */

typedef enum {
    TC_TYPE_INT,
    TC_TYPE_STR,
    TC_TYPE_CHAR,
    TC_TYPE_FLOAT,
    TC_TYPE_BOOL
} TCType;

static const char *tctype_to_c(TCType t)
{
    switch (t) {
        case TC_TYPE_STR:
            return "const char *";

        case TC_TYPE_CHAR:
            return "char";

        case TC_TYPE_FLOAT:
            return "double";

        case TC_TYPE_BOOL:
            return "int";

        default:
            return "tc_val";
    }
}

/* =========================================================
 * STRING BUILDER
 * ========================================================= */

typedef struct {
    char *buf;
    size_t len;
    size_t cap;
} SB;

static void sb_init(SB *sb)
{
    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void sb_free(SB *sb)
{
    free(sb->buf);

    sb->buf = NULL;
    sb->len = 0;
    sb->cap = 0;
}

static void sb_grow(SB *sb, size_t need)
{
    if (sb->len + need < sb->cap)
        return;

    size_t nc = sb->cap ? sb->cap * 2 : 256;

    while (nc < sb->len + need)
        nc *= 2;

    char *nb = realloc(sb->buf, nc);

    if (!nb)
        die("out of memory");

    sb->buf = nb;
    sb->cap = nc;
}

static void sb_append(SB *sb, const char *s)
{
    size_t n = strlen(s);

    sb_grow(sb, n + 1);

    memcpy(
        sb->buf + sb->len,
        s,
        n + 1
    );

    sb->len += n;
}

static void sb_appendf(
    SB *sb,
    const char *fmt,
    ...
)
{
    char tmp[4096];

    va_list ap;

    va_start(ap, fmt);

    vsnprintf(
        tmp,
        sizeof(tmp),
        fmt,
        ap
    );

    va_end(ap);

    sb_append(sb, tmp);
}

/* =========================================================
 * SCOPE
 * ========================================================= */

#define MAX_VARS 512

typedef struct Scope {
    char *vars[MAX_VARS];
    int nvars;
    struct Scope *parent;
} Scope;

static Scope *scope_new(Scope *parent)
{
    Scope *s = calloc(1, sizeof(Scope));

    if (!s)
        die("out of memory");

    s->parent = parent;

    return s;
}

static void scope_free(Scope *s)
{
    for (int i = 0; i < s->nvars; i++)
        free(s->vars[i]);

    free(s);
}

static int scope_declare(
    Scope *s,
    const char *name
)
{
    for (int i = 0; i < s->nvars; i++) {
        if (!strcmp(s->vars[i], name))
            return 0;
    }

    if (s->parent) {
        for (Scope *p = s->parent; p; p = p->parent) {
            for (int i = 0; i < p->nvars; i++) {
                if (!strcmp(p->vars[i], name))
                    return 0;
            }
        }
    }

    if (s->nvars >= MAX_VARS)
        die("too many variables in scope");

    s->vars[s->nvars++] = xstrdup(name);

    return 1;
}

/* =========================================================
 * VARIABLE TYPE TRACKING
 * ========================================================= */

typedef struct VarInfo {
    char *name;
    TCType type;
    struct VarInfo *next;
} VarInfo;


/*
 * Type table used by the compiler.
 */
typedef struct TypeEntry {
    char *name;
    TCType type;
    struct TypeEntry *next;
} TypeEntry;

static TypeEntry *type_table = NULL;

static void type_set(
    const char *name,
    TCType type
)
{
    for (TypeEntry *e = type_table; e; e = e->next) {
        if (!strcmp(e->name, name)) {
            e->type = type;
            return;
        }
    }

    TypeEntry *e = calloc(1, sizeof(TypeEntry));

    if (!e)
        die("out of memory");

    e->name = xstrdup(name);
    e->type = type;
    e->next = type_table;

    type_table = e;
}

static TCType type_get(
    const char *name
)
{
    for (TypeEntry *e = type_table; e; e = e->next) {
        if (!strcmp(e->name, name))
            return e->type;
    }

    return TC_TYPE_INT;
}

static void type_table_free(void)
{
    TypeEntry *e = type_table;

    while (e) {
        TypeEntry *next = e->next;

        free(e->name);
        free(e);

        e = next;
    }

    type_table = NULL;
}

/* =========================================================
 * FORWARD DECLARATIONS
 * ========================================================= */

static void parse_stmt_list(
    Lexer *lx,
    SB *out,
    Scope *sc,
    int indent
);

static void parse_expr_to_c(
    Lexer *lx,
    SB *out,
    Scope *sc
);

static TCType parse_expr_type(
    Lexer *lx,
    Scope *sc
);

/* =========================================================
 * TYPE DETECTION
 * ========================================================= */

static TCType detect_type(Lexer *lx)
{
    if (lx_kind(lx) == TK_STRING)
        return TC_TYPE_STR;

    if (lx_kind(lx) == TK_NUMBER)
        return TC_TYPE_INT;

    if (lx_kind(lx) == TK_IDENT) {
        if (!strcmp(lx->cur.val, "true") ||
            !strcmp(lx->cur.val, "false"))
            return TC_TYPE_BOOL;

        return type_get(lx->cur.val);
    }

    return TC_TYPE_INT;
}

/* =========================================================
 * STRING HELPERS
 * ========================================================= */

static void emit_string(
    SB *out,
    const char *s
)
{
    int has_interp = 0;

    for (const char *p = s; *p; p++) {
        if (*p == '$') {
            has_interp = 1;
            break;
        }

        if (*p == '%' &&
            isalpha((unsigned char)*(p + 1))) {
            has_interp = 1;
            break;
        }
    }

    if (!has_interp) {
        sb_append(out, "\"");

        for (const char *p = s; *p; p++) {
            if (*p == '%') {
                sb_append(out, "%%");
            } else if (*p == '"') {
                sb_append(out, "\\\"");
            } else if (*p == '\\') {
                p++;

                if (!*p)
                    break;

                if (*p == 'n')
                    sb_append(out, "\\n");
                else if (*p == 't')
                    sb_append(out, "\\t");
                else if (*p == 'r')
                    sb_append(out, "\\r");
                else if (*p == '\\')
                    sb_append(out, "\\\\");
                else if (*p == '"')
                    sb_append(out, "\\\"");
                else {
                    char tmp[3] = {'\\', *p, 0};
                    sb_append(out, tmp);
                }
            } else {
                char tmp[2] = {*p, 0};
                sb_append(out, tmp);
            }
        }

        sb_append(out, "\"");

        return;
    }

    char varnames[32][128];
    int nvarnames = 0;

    SB fmt;
    sb_init(&fmt);

    sb_append(&fmt, "\"");

    for (const char *p = s; *p; p++) {

        if (*p == '$' ||
            (*p == '%' &&
             isalpha((unsigned char)*(p + 1)))) {

            p++;

            char vname[128] = {0};
            int vi = 0;

            while ((isalnum((unsigned char)*p) ||
                    *p == '_') &&
                   vi < 127) {

                vname[vi++] = *p;
                p++;
            }

            p--;

            sb_append(&fmt, "%s");

            if (nvarnames < 32) {
                snprintf(
                    varnames[nvarnames],
                    sizeof(varnames[nvarnames]),
                    "%s",
                    vname
                );

                nvarnames++;
            }

        } else if (*p == '%') {

            sb_append(&fmt, "%%");

        } else if (*p == '"') {

            sb_append(&fmt, "\\\"");

        } else if (*p == '\\') {

            p++;

            if (!*p)
                break;

            if (*p == 'n')
                sb_append(&fmt, "\\n");
            else if (*p == 't')
                sb_append(&fmt, "\\t");
            else if (*p == 'r')
                sb_append(&fmt, "\\r");
            else if (*p == '\\')
                sb_append(&fmt, "\\\\");
            else if (*p == '"')
                sb_append(&fmt, "\\\"");
            else {
                char tmp[3] = {'\\', *p, 0};
                sb_append(&fmt, tmp);
            }

        } else {

            char tmp[2] = {*p, 0};
            sb_append(&fmt, tmp);
        }
    }

    sb_append(&fmt, "\"");

    sb_appendf(
        out,
        "tc_str_interp(%s",
        fmt.buf
    );

    for (int i = 0; i < nvarnames; i++)
        sb_appendf(
            out,
            ", %s",
            varnames[i]
        );

    sb_append(out, ")");

    sb_free(&fmt);
}

static void emit_raw_string(
    SB *out,
    const char *s
)
{
    sb_append(out, "\"");

    for (const char *p = s; *p; p++) {

        if (*p == '"') {
            sb_append(out, "\\\"");
        } else if (*p == '\\') {

            p++;

            if (!*p)
                break;

            if (*p == 'n')
                sb_append(out, "\\n");
            else if (*p == 't')
                sb_append(out, "\\t");
            else if (*p == 'r')
                sb_append(out, "\\r");
            else if (*p == '\\')
                sb_append(out, "\\\\");
            else if (*p == '"')
                sb_append(out, "\\\"");
            else {
                char tmp[3] = {'\\', *p, 0};
                sb_append(out, tmp);
            }

        } else {

            char tmp[2] = {*p, 0};
            sb_append(out, tmp);
        }
    }

    sb_append(out, "\"");
}

/* =========================================================
 * DOTTED NAMES
 * ========================================================= */

static void read_dotted_name(
    Lexer *lx,
    char *buf,
    size_t bufsz
)
{
    buf[0] = 0;

    if (lx_kind(lx) != TK_IDENT &&
        lx_kind(lx) != TK_CKEYWORD &&
        lx_kind(lx) != TK_ASMKW) {

        die(
            "line %d: expected identifier, got '%s'",
            lx->cur.line,
            lx->cur.val
        );
    }

    strncat(
        buf,
        lx->cur.val,
        bufsz - strlen(buf) - 1
    );

    lx_consume(lx);

    while (lx_kind(lx) == TK_DOT) {
        lx_consume(lx);

        if (lx_kind(lx) != TK_IDENT &&
            lx_kind(lx) != TK_CKEYWORD &&
            lx_kind(lx) != TK_ASMKW) {

            die(
                "line %d: expected identifier after '.'",
                lx->cur.line
            );
        }

        strncat(
            buf,
            ".",
            bufsz - strlen(buf) - 1
        );

        strncat(
            buf,
            lx->cur.val,
            bufsz - strlen(buf) - 1
        );

        lx_consume(lx);
    }
}

/* =========================================================
 * TC CALL MAPPING
 * ========================================================= */

static const char *map_tc_call(
    const char *name
)
{
    if (!strcmp(name, "tc.init"))
        return "tc_init";

    if (!strcmp(name, "tc.func.init"))
        return "tc_func_init";

    if (!strcmp(name, "tc.ret.init"))
        return "tc_ret_init";

    if (!strcmp(name, "tc.loops.init"))
        return "tc_loops_init";

    if (!strcmp(name, "tc.graphics.init"))
        return "tc_graphics_init";

    if (!strcmp(name, "tc.printf"))
        return "tc_printf";

    if (!strcmp(name, "tc.print"))
        return "tc_print";

    if (!strcmp(name, "tc.math.add"))
        return "tc_math_add";

    if (!strcmp(name, "tc.math.subtract"))
        return "tc_math_subtract";

    if (!strcmp(name, "tc.math.multiply"))
        return "tc_math_multiply";

    if (!strcmp(name, "tc.math.divide"))
        return "tc_math_divide";

    if (!strcmp(name, "tc.math.modulo"))
        return "tc_math_modulo";

    if (!strcmp(name, "tc.math.equal"))
        return "tc_math_equal";

    if (!strcmp(name, "tc.math.not_equal"))
        return "tc_math_not_equal";

    if (!strcmp(name, "tc.math.less_then"))
        return "tc_math_less_then";

    if (!strcmp(name, "tc.math.greater_then"))
        return "tc_math_greater_then";

    if (!strcmp(name, "tc.math.greater_then_or_equal"))
        return "tc_math_greater_then_or_equal";

    if (!strcmp(name, "tc.math.less_then_or_equal"))
        return "tc_math_less_then_or_equal";

    if (!strcmp(name, "tc.math.assign"))
        return "__tc_assign__";

    if (!strcmp(name, "tc.lang.init"))
        return "tc_noop";

    if (!strcmp(name, "tc.lang.run"))
        return "tc_noop";

    return NULL;
}

/* =========================================================
 * NUMBER EMISSION
 * ========================================================= */

static void emit_number(
    SB *out,
    const char *v
)
{
    if (v[0] == '0' &&
        (v[1] == 'b' || v[1] == 'B')) {

        unsigned long long val = 0;

        for (const char *p = v + 2; *p; p++)
            val = val * 2ULL +
                  (unsigned long long)(*p - '0');

        sb_appendf(
            out,
            "%lluLL",
            val
        );

        return;
    }

    if (v[0] == '0' &&
        (v[1] == 'x' || v[1] == 'X')) {

        sb_appendf(
            out,
            "%sLL",
            v
        );

        return;
    }

    sb_append(out, v);
}

/* =========================================================
 * EXPRESSION TYPE
 * ========================================================= */

static TCType parse_expr_type(Lexer *lx, Scope *sc)
{
    (void)sc;

    if (lx_kind(lx) == TK_STRING) {
        return TC_TYPE_STR;
    }

    if (lx_kind(lx) == TK_NUMBER) {
        const char *v = lx->cur.val;

        for (const char *p = v; *p; p++) {
            if (*p == '.') {
                return TC_TYPE_FLOAT;
            }
        }

        return TC_TYPE_INT;
    }

    if (lx_kind(lx) == TK_DOLLAR) {
        lx_consume(lx);

        Token t = lx_expect(lx, TK_IDENT, "variable name after $");

        TCType type = TC_TYPE_INT;

        /*
         * Variables are represented as tc_val unless their
         * declaration was explicitly known as another type.
         *
         * For now, dereferenced variables are therefore treated
         * as integer values.
         */
        token_free(&t);
        return type;
    }

    if (lx_kind(lx) == TK_IDENT ||
        lx_kind(lx) == TK_CKEYWORD ||
        lx_kind(lx) == TK_ASMKW) {

        char name[256] = {0};
        read_dotted_name(lx, name, sizeof(name));

        if (!strcmp(name, "true") || !strcmp(name, "false")) {
            return TC_TYPE_BOOL;
        }

        /*
         * A function call returning a value is tc_val by default.
         */
        if (lx_kind(lx) == TK_LPAREN) {
            return TC_TYPE_INT;
        }

        return TC_TYPE_INT;
    }

    return TC_TYPE_INT;
}

/* =========================================================
 * EXPRESSION CONVERSION
 * ========================================================= */

static void parse_expr_as_val(
    Lexer *lx,
    SB *out,
    Scope *sc
)
{
    TCType type = parse_expr_type(lx, sc);

    SB expr;
    sb_init(&expr);

    parse_expr_to_c(
        lx,
        &expr,
        sc
    );

    if (type == TC_TYPE_STR) {
        sb_appendf(
            out,
            "tc_str_to_val(%s)",
            expr.buf
        );
    } else {
        sb_append(out, expr.buf);
    }

    sb_free(&expr);
}

static void parse_expr_as_str(
    Lexer *lx,
    SB *out,
    Scope *sc
)
{
    TCType type = parse_expr_type(lx, sc);

    SB expr;
    sb_init(&expr);

    parse_expr_to_c(
        lx,
        &expr,
        sc
    );

    if (type == TC_TYPE_STR) {
        sb_append(out, expr.buf);
    } else {
        sb_appendf(
            out,
            "tc_val_to_str(%s)",
            expr.buf
        );
    }

    sb_free(&expr);
}

/* =========================================================
 * EXPRESSION PARSER
 * ========================================================= */

static void parse_expr_to_c(Lexer *lx, SB *out, Scope *sc)
{
    /* ---------------------------------------------------------
     * $variable
     * --------------------------------------------------------- */
    if (lx_kind(lx) == TK_DOLLAR) {
        lx_consume(lx);

        Token t = lx_expect(lx, TK_IDENT,
                            "variable name after $");

        sb_append(out, t.val);

        token_free(&t);
        return;
    }

    /* ---------------------------------------------------------
     * number
     * --------------------------------------------------------- */
    if (lx_kind(lx) == TK_NUMBER) {
        Token t = lx_consume(lx);

        emit_number(out, t.val);

        token_free(&t);
        return;
    }

    /* ---------------------------------------------------------
     * string
     * --------------------------------------------------------- */
    if (lx_kind(lx) == TK_STRING) {
        Token t = lx_consume(lx);

        emit_string(out, t.val);

        token_free(&t);
        return;
    }

    /* ---------------------------------------------------------
     * identifier / dotted name
     * --------------------------------------------------------- */
    if (lx_kind(lx) == TK_IDENT ||
        lx_kind(lx) == TK_CKEYWORD ||
        lx_kind(lx) == TK_ASMKW)
    {
        char name[256] = {0};

        read_dotted_name(lx, name, sizeof(name));

        /*
         * Plain identifier.
         *
         * Example:
         *
         *     helper
         */
        if (lx_kind(lx) != TK_LPAREN) {
            sb_append(out, name);
            return;
        }

        /*
         * Function call.
         */
        lx_consume(lx);

        const char *cname = map_tc_call(name);

        /* -----------------------------------------------------
         * tc.math.assign(...)
         *
         * The '(' has ALREADY been consumed above.
         * ----------------------------------------------------- */
        if (cname && !strcmp(cname, "__tc_assign__")) {

            char varname[128] = {0};

            /*
             * Optional $
             *
             * tc.math.assign($x, ...)
             */
            if (lx_kind(lx) == TK_DOLLAR)
                lx_consume(lx);

            Token vt = lx_expect(
                lx,
                TK_IDENT,
                "variable name"
            );

            strncpy(
                varname,
                vt.val,
                sizeof(varname) - 1
            );

            token_free(&vt);

            lx_expect(lx, TK_COMMA, ",");

            TCType type = detect_type(lx);

            SB val;
            sb_init(&val);

            /*
             * Assignment value MUST NOT be another function call.
             *
             * Therefore:
             *
             *     tc.math.assign(x, tc.math.add(1, 2))
             *
             * is rejected.
             */
            if (lx_kind(lx) == TK_IDENT ||
                lx_kind(lx) == TK_CKEYWORD ||
                lx_kind(lx) == TK_ASMKW)
            {
                char check[256] = {0};

                read_dotted_name(
                    lx,
                    check,
                    sizeof(check)
                );

                if (lx_kind(lx) == TK_LPAREN) {
                    die(
                        "line %d: function calls cannot be used "
                        "inside function arguments",
                        lx->cur.line
                    );
                }

                sb_append(&val, check);
            }
            else {
                parse_expr_to_c(lx, &val, sc);
            }

            lx_expect(lx, TK_RPAREN, ")");

            int is_new =
                scope_declare(sc, varname);

            if (is_new) {
                sb_appendf(
                    out,
                    "%s %s = %s",
                    tctype_to_c(type),
                    varname,
                    val.buf
                );
            }
            else {
                sb_appendf(
                    out,
                    "%s = %s",
                    varname,
                    val.buf
                );
            }

            sb_free(&val);

            return;
        }

        /*
         * Normal function call.
         *
         * Example:
         *
         *     tc.math.add("$x", 5)
         *
         * Arguments can be values, but NOT calls.
         */
        if (!cname)
            cname = name;

        sb_appendf(out, "%s(", cname);

        int first = 1;

        while (lx_kind(lx) != TK_RPAREN &&
               lx_kind(lx) != TK_EOF)
        {
            if (!first)
                sb_append(out, ", ");

            first = 0;

            /*
             * Detect nested call BEFORE parsing the argument.
             */
            if (lx_kind(lx) == TK_IDENT ||
                lx_kind(lx) == TK_CKEYWORD ||
                lx_kind(lx) == TK_ASMKW)
            {
                char argname[256] = {0};

                read_dotted_name(
                    lx,
                    argname,
                    sizeof(argname)
                );

                if (lx_kind(lx) == TK_LPAREN) {
                    die(
                        "line %d: function calls cannot be "
                        "used inside function arguments",
                        lx->cur.line
                    );
                }

                sb_append(out, argname);
            }
            else {
                parse_expr_to_c(lx, out, sc);
            }

            if (lx_kind(lx) == TK_COMMA)
                lx_consume(lx);
        }

        lx_expect(lx, TK_RPAREN, ")");

        sb_append(out, ")");

        return;
    }

    die(
        "line %d: unexpected token '%s' in expression",
        lx->cur.line,
        lx->cur.val
    );
}

/* =========================================================
 * INDENT
 * ========================================================= */

static void emit_indent(
    SB *out,
    int indent
)
{
    for (int i = 0; i < indent; i++)
        sb_append(out, "    ");
}

/* =========================================================
 * RAW C BLOCK
 * ========================================================= */

static char *read_raw_block(Lexer *lx)
{
    size_t start = lx->pos;

    int depth = 1;

    int in_string = 0;
    int in_char = 0;
    int escape = 0;

    int in_line_comment = 0;
    int in_block_comment = 0;

    while (lx->src[lx->pos] &&
           depth > 0) {

        char c = lx->src[lx->pos];
        char n = lx->src[lx->pos + 1];

        if (in_line_comment) {
            if (c == '\n') {
                in_line_comment = 0;
                lx->line++;
            }

            lx->pos++;

            continue;
        }

        if (in_block_comment) {

            if (c == '*' && n == '/') {
                in_block_comment = 0;
                lx->pos += 2;
                continue;
            }

            if (c == '\n')
                lx->line++;

            lx->pos++;

            continue;
        }

        if (in_string) {

            if (escape)
                escape = 0;
            else if (c == '\\')
                escape = 1;
            else if (c == '"')
                in_string = 0;
            else if (c == '\n')
                lx->line++;

            lx->pos++;

            continue;
        }

        if (in_char) {

            if (escape)
                escape = 0;
            else if (c == '\\')
                escape = 1;
            else if (c == '\'')
                in_char = 0;
            else if (c == '\n')
                lx->line++;

            lx->pos++;

            continue;
        }

        if (c == '/' && n == '/') {
            in_line_comment = 1;
            lx->pos += 2;
            continue;
        }

        if (c == '/' && n == '*') {
            in_block_comment = 1;
            lx->pos += 2;
            continue;
        }

        if (c == '"') {
            in_string = 1;
            lx->pos++;
            continue;
        }

        if (c == '\'') {
            in_char = 1;
            lx->pos++;
            continue;
        }

        if (c == '{')
            depth++;
        else if (c == '}') {
            depth--;

            if (depth == 0)
                break;
        }

        if (c == '\n')
            lx->line++;

        lx->pos++;
    }

    if (depth != 0)
        die(
            "line %d: unterminated C block",
            lx->line
        );

    char *raw = xstrndup(
        lx->src + start,
        lx->pos - start
    );

    if (lx->src[lx->pos] == '}')
        lx->pos++;

    token_free(&lx->cur);
    lx->cur = lex_next(lx);

    return raw;
}

/* =========================================================
 * PARAMETER LIST
 * ========================================================= */

static void parse_param_list(
    Lexer *lx,
    SB *out
)
{
    lx_expect(
        lx,
        TK_LPAREN,
        "("
    );

    sb_append(out, "(");

    int first = 1;

    while (lx_kind(lx) != TK_RPAREN &&
           lx_kind(lx) != TK_EOF) {

        if (!first)
            sb_append(out, ", ");

        first = 0;

        Token t =
            lx_expect(
                lx,
                TK_IDENT,
                "parameter name"
            );

        sb_appendf(
            out,
            "tc_val %s",
            t.val
        );

        type_set(
            t.val,
            TC_TYPE_INT
        );

        token_free(&t);

        if (lx_kind(lx) == TK_COMMA)
            lx_consume(lx);
    }

    lx_expect(
        lx,
        TK_RPAREN,
        ")"
    );

    sb_append(out, ")");
}

/* =========================================================
 * LOOP
 * ========================================================= */

static void parse_loop(
    Lexer *lx,
    SB *out,
    Scope *sc,
    int indent
)
{
    /*
     * loop:
     *     ...
     */
    if (lx_kind(lx) == TK_COLON) {

        lx_consume(lx);

        sb_append(
            out,
            "while (1) {\n"
        );

        parse_stmt_list(
            lx,
            out,
            sc,
            indent + 1
        );

        emit_indent(
            out,
            indent
        );

        sb_append(
            out,
            "}\n"
        );

        return;
    }

    /*
     * loop(end = ..., index = ...)
     */
    lx_expect(
        lx,
        TK_LPAREN,
        "("
    );

    Token end_kw =
        lx_expect(
            lx,
            TK_IDENT,
            "end"
        );

    lx_expect(
        lx,
        TK_EQUALS,
        "="
    );

    SB end_val;
    sb_init(&end_val);

    parse_expr_to_c(
        lx,
        &end_val,
        sc
    );

    lx_expect(
        lx,
        TK_COMMA,
        ","
    );

    Token idx_kw =
        lx_expect(
            lx,
            TK_IDENT,
            "index"
        );

    lx_expect(
        lx,
        TK_EQUALS,
        "="
    );

    SB idx_val;
    sb_init(&idx_val);

    parse_expr_to_c(
        lx,
        &idx_val,
        sc
    );

    lx_expect(
        lx,
        TK_RPAREN,
        ")"
    );

    sb_appendf(
        out,
        "for (tc_val %s = %s; "
        "tc_math_less_then(%s, %s); "
        "%s = tc_math_add(%s, 1)) {\n",
        idx_kw.val,
        idx_val.buf,
        idx_kw.val,
        end_val.buf,
        idx_kw.val,
        idx_kw.val
    );

    scope_declare(
        sc,
        idx_kw.val
    );

    type_set(
        idx_kw.val,
        TC_TYPE_INT
    );

    lx_expect(
        lx,
        TK_LBRACE,
        "{"
    );

    Scope *inner =
        scope_new(sc);

    parse_stmt_list(
        lx,
        out,
        inner,
        indent + 1
    );

    scope_free(inner);

    lx_expect(
        lx,
        TK_RBRACE,
        "}"
    );

    emit_indent(
        out,
        indent
    );

    sb_append(
        out,
        "}\n"
    );

    token_free(&end_kw);
    token_free(&idx_kw);

    sb_free(&end_val);
    sb_free(&idx_val);
}

/* =========================================================
 * STATEMENT PARSER
 * ========================================================= */

static void parse_stmt(
    Lexer *lx,
    SB *out,
    Scope *sc,
    int indent
)
{
    /* ---------------------------------------------------------
     * $x = expression
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_DOLLAR) {

        lx_consume(lx);

        Token vt =
            lx_expect(
                lx,
                TK_IDENT,
                "variable name after $"
            );

        lx_expect(
            lx,
            TK_EQUALS,
            "="
        );

        TCType type =
            detect_type(lx);

        SB val;
        sb_init(&val);

        parse_expr_to_c(
            lx,
            &val,
            sc
        );

        int is_new =
            scope_declare(
                sc,
                vt.val
            );

        type_set(
            vt.val,
            type
        );

        emit_indent(
            out,
            indent
        );

        if (is_new) {

            sb_appendf(
                out,
                "%s %s = %s;\n",
                tctype_to_c(type),
                vt.val,
                val.buf
            );

        } else {

            /*
             * If assigning a numeric expression into a
             * string variable, explicitly convert it.
             */
            if (type_get(vt.val) == TC_TYPE_STR &&
                type != TC_TYPE_STR) {

                sb_appendf(
                    out,
                    "%s = tc_val_to_str(%s);\n",
                    vt.val,
                    val.buf
                );

            } else if (type_get(vt.val) != TC_TYPE_STR &&
                       type == TC_TYPE_STR) {

                sb_appendf(
                    out,
                    "%s = tc_str_to_val(%s);\n",
                    vt.val,
                    val.buf
                );

            } else {

                sb_appendf(
                    out,
                    "%s = %s;\n",
                    vt.val,
                    val.buf
                );
            }
        }

        sb_free(&val);
        token_free(&vt);

        lx_skip_semi(lx);

        return;
    }

    /* ---------------------------------------------------------
     * ret
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_RET) {

        lx_consume(lx);

        emit_indent(
            out,
            indent
        );

        if (lx_kind(lx) == TK_SEMICOLON ||
            lx_kind(lx) == TK_RBRACE ||
            lx_kind(lx) == TK_EOF) {

            sb_append(
                out,
                "return 0;\n"
            );

        } else {

            TCType type =
                detect_type(lx);

            SB expr;
            sb_init(&expr);

            parse_expr_to_c(
                lx,
                &expr,
                sc
            );

            if (type == TC_TYPE_STR) {

                sb_appendf(
                    out,
                    "return (int)tc_str_to_val(%s);\n",
                    expr.buf
                );

            } else {

                sb_appendf(
                    out,
                    "return (int)(%s);\n",
                    expr.buf
                );
            }

            sb_free(&expr);
        }

        lx_skip_semi(lx);

        return;
    }

    /* ---------------------------------------------------------
     * jump
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_JUMP) {

        lx_consume(lx);

        Token t =
            lx_expect(
                lx,
                TK_IDENT,
                "label name"
            );

        emit_indent(
            out,
            indent
        );

        sb_appendf(
            out,
            "goto %s;\n",
            t.val
        );

        token_free(&t);

        lx_skip_semi(lx);

        return;
    }

    /* ---------------------------------------------------------
     * if / else
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_IF) {

        lx_consume(lx);

        lx_expect(
            lx,
            TK_LPAREN,
            "("
        );

        emit_indent(
            out,
            indent
        );

        sb_append(
            out,
            "if ("
        );

        parse_expr_as_val(
            lx,
            out,
            sc
        );

        lx_expect(
            lx,
            TK_RPAREN,
            ")"
        );

        sb_append(
            out,
            ") {\n"
        );

        lx_expect(
            lx,
            TK_LBRACE,
            "{"
        );

        Scope *inner =
            scope_new(sc);

        parse_stmt_list(
            lx,
            out,
            inner,
            indent + 1
        );

        scope_free(inner);

        lx_expect(
            lx,
            TK_RBRACE,
            "}"
        );

        emit_indent(
            out,
            indent
        );

        sb_append(
            out,
            "}"
        );

        if (lx_kind(lx) == TK_ELSE) {

            lx_consume(lx);

            sb_append(
                out,
                " else {\n"
            );

            lx_expect(
                lx,
                TK_LBRACE,
                "{"
            );

            Scope *el =
                scope_new(sc);

            parse_stmt_list(
                lx,
                out,
                el,
                indent + 1
            );

            scope_free(el);

            lx_expect(
                lx,
                TK_RBRACE,
                "}"
            );

            emit_indent(
                out,
                indent
            );

            sb_append(
                out,
                "}"
            );
        }

        sb_append(
            out,
            "\n"
        );

        return;
    }

    /* ---------------------------------------------------------
     * loop
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_LOOP) {

        lx_consume(lx);

        emit_indent(
            out,
            indent
        );

        parse_loop(
            lx,
            out,
            sc,
            indent
        );

        return;
    }

    /* ---------------------------------------------------------
     * asm { }
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_ASMKW) {

        lx_consume(lx);

        char *raw =
            read_raw_block(lx);

        emit_indent(
            out,
            indent
        );

        sb_append(
            out,
            "__asm__ volatile(\n"
        );

        sb_append(
            out,
            "\".intel_syntax noprefix\\n\"\n"
        );

        char *p = raw;

        while (*p) {

            char *nl =
                strchr(p, '\n');

            size_t len =
                nl ?
                (size_t)(nl - p) :
                strlen(p);

            while (len > 0 &&
                   p[len - 1] == '\r')
                len--;

            emit_indent(
                out,
                indent + 1
            );

            sb_append(
                out,
                "\""
            );

            for (size_t i = 0; i < len; i++) {

                unsigned char ch =
                    (unsigned char)p[i];

                switch (ch) {

                    case '\\':
                        sb_append(
                            out,
                            "\\\\"
                        );
                        break;

                    case '"':
                        sb_append(
                            out,
                            "\\\""
                        );
                        break;

                    case '\t':
                        sb_append(
                            out,
                            "\\t"
                        );
                        break;

                    case '\r':
                        break;

                    default:

                        if (ch < 32) {

                            sb_appendf(
                                out,
                                "\\x%02X",
                                ch
                            );

                        } else {

                            char tmp[2] = {
                                (char)ch,
                                0
                            };

                            sb_append(
                                out,
                                tmp
                            );
                        }

                        break;
                }
            }

            sb_append(
                out,
                "\\n\"\n"
            );

            if (nl)
                p = nl + 1;
            else
                break;
        }

        emit_indent(
            out,
            indent
        );

        sb_append(
            out,
            "\".att_syntax\\n\"\n"
        );

        sb_append(
            out,
            ");\n"
        );

        free(raw);

        return;
    }

    /* ---------------------------------------------------------
     * c { }
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_CKEYWORD) {

        lx_consume(lx);

        if (lx_kind(lx) == TK_LBRACE) {

            char *raw =
                read_raw_block(lx);

            char *content = raw;

            if (*content == '\n')
                content++;

            emit_indent(
                out,
                indent
            );

            sb_append(
                out,
                content
            );

            sb_append(
                out,
                "\n"
            );

            free(raw);

            return;
        }

        SB expr;
        sb_init(&expr);

        sb_append(
            &expr,
            "c"
        );

        while (lx_kind(lx) == TK_DOT) {

            lx_consume(lx);

            sb_append(
                &expr,
                "."
            );

            Token t =
                lx_expect(
                    lx,
                    TK_IDENT,
                    "identifier"
                );

            sb_append(
                &expr,
                t.val
            );

            token_free(&t);
        }

        emit_indent(
            out,
            indent
        );

        sb_append(
            out,
            expr.buf
        );

        sb_free(&expr);

        if (lx_kind(lx) == TK_LPAREN) {

            sb_append(
                out,
                "("
            );

            lx_consume(lx);

            int first = 1;

            while (lx_kind(lx) != TK_RPAREN &&
                   lx_kind(lx) != TK_EOF) {

                if (!first)
                    sb_append(
                        out,
                        ", "
                    );

                first = 0;

                parse_expr_to_c(
                    lx,
                    out,
                    sc
                );

                if (lx_kind(lx) == TK_COMMA)
                    lx_consume(lx);
            }

            lx_expect(
                lx,
                TK_RPAREN,
                ")"
            );

            sb_append(
                out,
                ")"
            );
        }

        sb_append(
            out,
            ";\n"
        );

        lx_skip_semi(lx);

        return;
    }

    /* ---------------------------------------------------------
     * Identifier:
     * label / dotted call / function call
     * --------------------------------------------------------- */

    if (lx_kind(lx) == TK_IDENT) {

        char name[256] = {0};

        read_dotted_name(
            lx,
            name,
            sizeof(name)
        );

        /* label */
        if (lx_kind(lx) == TK_COLON) {

            lx_consume(lx);

            emit_indent(
                out,
                indent
            );

            sb_appendf(
                out,
                "%s:\n",
                name
            );

            return;
        }

        const char *cname =
            map_tc_call(name);

        if (cname &&
            !strcmp(cname, "__tc_assign__")) {

            lx_expect(
                lx,
                TK_LPAREN,
                "("
            );

            char varname[128] = {0};

            if (lx_kind(lx) == TK_DOLLAR)
                lx_consume(lx);

            Token vt =
                lx_expect(
                    lx,
                    TK_IDENT,
                    "variable name"
                );

            strncpy(
                varname,
                vt.val,
                sizeof(varname) - 1
            );

            token_free(&vt);

            lx_expect(
                lx,
                TK_COMMA,
                ","
            );

            TCType type =
                detect_type(lx);

            SB val;
            sb_init(&val);

            parse_expr_to_c(
                lx,
                &val,
                sc
            );

            lx_expect(
                lx,
                TK_RPAREN,
                ")"
            );

            int is_new =
                scope_declare(
                    sc,
                    varname
                );

            type_set(
                varname,
                type
            );

            emit_indent(
                out,
                indent
            );

            if (is_new) {

                sb_appendf(
                    out,
                    "%s %s = %s;\n",
                    tctype_to_c(type),
                    varname,
                    val.buf
                );

            } else {

                sb_appendf(
                    out,
                    "%s = %s;\n",
                    varname,
                    val.buf
                );
            }

            sb_free(&val);

            lx_skip_semi(lx);

            return;
        }

        if (!cname)
            cname = name;

        emit_indent(
            out,
            indent
        );

        if (lx_kind(lx) == TK_LPAREN) {

            lx_consume(lx);

            sb_appendf(
                out,
                "%s(",
                cname
            );

            int first = 1;

            int is_printf =
                !strcmp(cname, "tc_printf");

            int is_print =
                !strcmp(cname, "tc_print");

            int is_math =
                !strncmp(name, "tc.math.", 8);

            while (lx_kind(lx) != TK_RPAREN &&
                   lx_kind(lx) != TK_EOF) {

                if (!first)
                    sb_append(
                        out,
                        ", "
                    );

                if (first &&
                    is_printf &&
                    lx_kind(lx) == TK_STRING) {

                    Token t =
                        lx_consume(lx);

                    emit_raw_string(
                        out,
                        t.val
                    );

                    token_free(&t);

                } else if (is_math) {

                    parse_expr_as_val(
                        lx,
                        out,
                        sc
                    );

                } else if (is_print) {

                    parse_expr_as_str(
                        lx,
                        out,
                        sc
                    );

                } else {

                    parse_expr_to_c(
                        lx,
                        out,
                        sc
                    );
                }

                first = 0;

                if (lx_kind(lx) == TK_COMMA)
                    lx_consume(lx);
            }

            lx_expect(
                lx,
                TK_RPAREN,
                ")"
            );

            sb_append(
                out,
                ");\n"
            );

        } else {

            /*
             * A bare function call.
             */
            sb_appendf(
                out,
                "%s();\n",
                cname
            );
        }

        lx_skip_semi(lx);

        return;
    }

    fprintf(
        stderr,
        "tc2 warning: line %d: "
        "skipping unexpected token '%s'\n",
        lx->cur.line,
        lx->cur.val
    );

    lx_consume(lx);
}

/* =========================================================
 * STATEMENT LIST
 * ========================================================= */

static void parse_stmt_list(
    Lexer *lx,
    SB *out,
    Scope *sc,
    int indent
)
{
    while (lx_kind(lx) != TK_RBRACE &&
           lx_kind(lx) != TK_EOF) {

        parse_stmt(
            lx,
            out,
            sc,
            indent
        );
    }
}

/* =========================================================
 * TOP LEVEL
 * ========================================================= */

static void parse_program(
    Lexer *lx,
    SB *out
)
{
    sb_append(
        out,
        "/* =====================================================\n"
        " * TC2 GENERATED C FILE\n"
        " * ===================================================== */\n"
        "\n"
    );

    sb_append(
        out,
        TC_RUNTIME
    );

    sb_append(
        out,
        "\n"
        "/* =====================================================\n"
        " * TC PROGRAM\n"
        " * ===================================================== */\n"
        "\n"
    );

    while (lx_kind(lx) != TK_EOF) {

        if (lx_kind(lx) == TK_FUNC) {

            lx_consume(lx);

            Token fname =
                lx_expect(
                    lx,
                    TK_IDENT,
                    "function name"
                );

            int is_main =
                !strcmp(
                    fname.val,
                    "main"
                );

            sb_appendf(
                out,
                "%s %s",
                is_main ? "int" : "tc_val",
                fname.val
            );

            if (lx_kind(lx) == TK_LPAREN) {

                if (is_main) {

                    lx_consume(lx);

                    while (lx_kind(lx) != TK_RPAREN &&
                           lx_kind(lx) != TK_EOF) {

                        lx_consume(lx);
                    }

                    lx_expect(
                        lx,
                        TK_RPAREN,
                        ")"
                    );

                    sb_append(
                        out,
                        "(void)"
                    );

                } else {

                    SB params;
                    sb_init(&params);

                    parse_param_list(
                        lx,
                        &params
                    );

                    sb_append(
                        out,
                        params.buf
                    );

                    sb_free(&params);
                }

            } else {

                sb_append(
                    out,
                    is_main ?
                    "(void)" :
                    "()"
                );
            }

            sb_append(
                out,
                " {\n"
            );

            lx_expect(
                lx,
                TK_LBRACE,
                "{"
            );

            Scope *sc =
                scope_new(NULL);

            parse_stmt_list(
                lx,
                out,
                sc,
                1
            );

            scope_free(sc);

            lx_expect(
                lx,
                TK_RBRACE,
                "}"
            );

            /*
             * Ensure main has a valid C return.
             */
            if (is_main) {
                /*
                 * The TC source normally supplies ret.
                 * Nothing is added here so explicit ret
                 * semantics are preserved.
                 */
            }

            sb_append(
                out,
                "}\n\n"
            );

            token_free(&fname);

            continue;
        }

        fprintf(
            stderr,
            "tc2 warning: line %d: "
            "skipping top-level token '%s'\n",
            lx->cur.line,
            lx->cur.val
        );

        lx_consume(lx);
    }
}

/* =========================================================
 * FILE I/O
 * ========================================================= */

static char *read_file(
    const char *path
)
{
    FILE *f =
        fopen(path, "rb");

    if (!f)
        die(
            "cannot open '%s'",
            path
        );

    if (fseek(f, 0, SEEK_END) != 0)
        die(
            "cannot seek '%s'",
            path
        );

    long sz = ftell(f);

    if (sz < 0)
        die(
            "cannot determine size of '%s'",
            path
        );

    rewind(f);

    char *buf =
        malloc((size_t)sz + 1);

    if (!buf)
        die("out of memory");

    size_t got =
        fread(
            buf,
            1,
            (size_t)sz,
            f
        );

    if ((long)got != sz)
        die(
            "read error on '%s'",
            path
        );

    buf[sz] = 0;

    fclose(f);

    return buf;
}

static void write_file(
    const char *path,
    const char *data
)
{
    FILE *f =
        fopen(path, "wb");

    if (!f)
        die(
            "cannot write '%s'",
            path
        );

    fputs(
        data,
        f
    );

    fclose(f);
}

static char *replace_ext(
    const char *path,
    const char *newext
)
{
    const char *dot =
        strrchr(path, '.');

    size_t base_len =
        dot ?
        (size_t)(dot - path) :
        strlen(path);

    size_t total =
        base_len +
        strlen(newext) +
        1;

    char *r =
        malloc(total);

    if (!r)
        die("out of memory");

    memcpy(
        r,
        path,
        base_len
    );

    strcpy(
        r + base_len,
        newext
    );

    return r;
}

/* =========================================================
 * MAIN
 * ========================================================= */

int main(
    int argc,
    char **argv
)
{
    const char *infile = NULL;
    const char *outfile = NULL;

    for (int i = 1; i < argc; i++) {

        if (!strcmp(argv[i], "--out") &&
            i + 1 < argc) {

            outfile = argv[++i];

        } else if (!strcmp(argv[i], "--help") ||
                   !strcmp(argv[i], "-h")) {

            printf(
                "Usage: tc2 <source.tc> "
                "[--out output.c]\n"
            );

            return 0;

        } else {

            infile = argv[i];
        }
    }

    if (!infile) {

        fprintf(
            stderr,
            "Usage: tc2 <source.tc> "
            "[--out output.c]\n"
        );

        return 1;
    }

    char *src =
        read_file(infile);

    Lexer lx;

    lexer_init(
        &lx,
        src
    );

    SB out;

    sb_init(&out);

    parse_program(
        &lx,
        &out
    );

    char *dest =
        outfile ?
        xstrdup(outfile) :
        replace_ext(
            infile,
            ".c"
        );

    write_file(
        dest,
        out.buf
    );

    printf(
        "tc2: %s -> %s\n",
        infile,
        dest
    );

    token_free(&lx.cur);

    sb_free(&out);

    type_table_free();

    free(src);
    free(dest);

    return 0;
}
