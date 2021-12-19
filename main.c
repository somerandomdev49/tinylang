#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "bytecode.h"
#include "common.h"

enum {
    token_type_eof,
    token_type_id,
    token_type_num,
};

typedef struct {
    u8 type;
    u16 value;
} token_t;

const char *tok_to_str(token_t *t) {
    static char s[32];
    /**/ if(t->type == token_type_num)
        snprintf(s, 32, "num %d", t->value);
    else if(t->type == token_type_eof)
        snprintf(s, 32, "eof");
    else
        snprintf(s, 32, "'%c'", t->value);
    return s;
}

token_t NIL_TOK = { 0, 0 };
token_t tnext(FILE *f) {
    token_t tok = NIL_TOK;
    static char overlook = 0;
again:;
    int c = overlook == 0 ? fgetc(f) : overlook;
    overlook = 0;
    switch(c) {
        case 'a'...'z':
        case 'A'...'Z':
            tok.type = token_type_id;
            tok.value = c;
            break;

        case '0'...'9':
            tok.type = token_type_num;
            tok.value = tok.value * 10 + (c - '0');
            goto again;

        case ' ':
        case '\n':
        case '\t':
        case '\r':
            if(!tok.type) goto again;
            break;

        default:
            if(tok.type == 0) {
                tok.type = c;
                tok.value = c;
            } else { overlook = c; }
            break;
    }
    return tok;
}

static u1 error_count = 0;
void error(const char *msg) {
    fprintf(stderr, "\033[0;31mError #%d: %s\033[0;0m\n", ++error_count, msg);
}

enum {
    parse_type_none,
    parse_type_expr,
    parse_type_stmt,
    parse_type_term,
    parse_type_fact,
    parse_type_comp,
    parse_type_eqls,
    parse_type_asgn,
    parse_type_atom,
};

const char *parse_type_convert_to_string(u8 type) {
    static const char *map[] = {
        "none", "expr", "stmt",
        "term", "fact", "comp",
        "eqls", "asgn", "atom",
    };
    return map[type];
}

void emit0(FILE *f, u8 opc) {
    fprintf(stderr, "  \033[0;32m%s[%.2Xh]\033[0;0m\n", ins_convert_to_string(opc), opc);
    fputc(opc, f);
}

void emit1(FILE *f, u8 opc, u8 opr) {
    if(opc >= 0x26 && opc <= 0x30) {
        static const char regs[] = "AXYZ";
        fprintf(stderr, "  \033[0;32m%s[%.2Xh] \033[0;33m%c\033[0;0m\n",
            ins_convert_to_string(opc), opc, regs[opr]);
    } else {
        fprintf(stderr, "  \033[0;32m%s[%.2Xh] \033[0;33m%d\033[0;0m\n",
            ins_convert_to_string(opc), opc, opr);
    }
    fputc(opc, f);
    fputc(opr, f);
}

void emit2(FILE *f, u8 opc, u16 opr) {
    if(opc >= 0x02 && opc <= 0x09) {
        fprintf(stderr, "  \033[0;32m%s[%.2Xh] \033[0;33m0x%.4X\033[0;0m\n",
            ins_convert_to_string(opc), opc, opr);
    } else {
        fprintf(stderr, "  \033[0;32m%s[%.2Xh] \033[0;33m%d\033[0;0m\n",
            ins_convert_to_string(opc), opc, opr);
    }
    fputc(opc, f);
    fputc(opr & 0xFF, f);
    fputc((opr & 0xFF00) >> 8, f);
}

#define VAR_COUNT 52

typedef struct {
    FILE *o, *f;
    token_t prev;
    int type;
    u16 vars[VAR_COUNT];
    u16 last;
} parser_info_t;

// Currently no register allocation done,
// just pushes/pops the used register.
void codegen(parser_info_t *info) {
    token_t t = NIL_TOK;
    char n;
again:
    // fprintf(stderr, "codegen(%s)\n", parse_type_convert_to_string(info->type));
    switch(info->type) {
    case parse_type_atom:
        t = info->prev.type ? info->prev : tnext(info->f);
        if(t.type == token_type_id) {
            n = t.value;
            info->prev = t = tnext(info->f);
            if(t.type == '=') {
                // t = info->prev.type ? info->prev : tnext(info->f);
                info->type = parse_type_asgn;
                goto again;
            } else {
                /**/ if(t.value == 'T')
                    emit2(info->o, ins_isa, TINYLANG_CONST_TRUE );
                else if(t.value == 'F')
                    emit2(info->o, ins_isa, TINYLANG_CONST_FALSE);
                else
                    emit2(info->o, ins_lda, info->vars[n]);
            }
        } else if(t.type == token_type_num) {
            emit2(info->o, ins_isa, t.value);
        } else if(t.type == '(') {
            codegen(info);
            t = tnext(info->f);
            if(t.type != ')') {
                error("Parser: Expected a closing parenthesis.");
            }
        } else {
            printf("Unexpected '%s'\n", tok_to_str(&t));
            error("Parser: Expected an identifer, a number or a parenthesis.");
        }
        break;
    case parse_type_asgn:
        info->type = parse_type_expr;
        info->prev = NIL_TOK;
        codegen(info);
        emit2(info->o, ins_sta, info->vars[n] = info->last);
        info->last += 2;
        break;
    case parse_type_fact:
        info->type = parse_type_atom;
        codegen(info);
        info->prev = t = info->prev.type ? info->prev : tnext(info->f);
        if(t.type != '*' && t.type != '/') break;
        emit0(info->o, ins_pha);
        while(t.type == '*' || t.type == '/') {
            info->prev.type = 0;
            u8 n = t.type;
            info->type = parse_type_atom;
            codegen(info);
            info->prev = t = info->prev.type ? info->prev : tnext(info->f);
            emit0(info->o, ins_max);
            emit0(info->o, ins_pla);
            emit1(info->o, n == '*' ? ins_mul : ins_div, reg_x);
        }
        break;
    case parse_type_term:
        info->type = parse_type_fact;
        codegen(info);
        info->prev = t = info->prev.type ? info->prev : tnext(info->f);
        if(t.value != '+' && t.value != '-') break;
        emit0(info->o, ins_pha);
        while(t.value == '+' || t.value == '-') {
            info->prev.type = 0;
            u8 n = t.type;
            info->type = parse_type_fact;
            codegen(info);
            info->prev = t = info->prev.type ? info->prev : tnext(info->f);
            emit0(info->o, ins_max);
            emit0(info->o, ins_pla);
            emit1(info->o, n == '+' ? ins_add : ins_sub, reg_x);
        }
        break;
    case parse_type_comp:
        info->type = parse_type_term;
        codegen(info);
        info->prev = t = info->prev.type ? info->prev : tnext(info->f);
        if(t.type != '>' && t.type != '<') break;
        emit0(info->o, ins_pha);
        while(t.type == '>' || t.type == '<') {
            info->prev.type = 0;
            u8 n = t.type;
            info->type = parse_type_term;
            codegen(info);
            info->prev = t = info->prev.type ? info->prev : tnext(info->f);
            emit0(info->o, ins_max);
            emit0(info->o, ins_pla);
            emit1(info->o, ins_cmp, reg_x);
            emit1(info->o, ins_biti, n == '>' ? flag_plus : flag_minus);
        }
        break;
    case parse_type_eqls:
        info->type = parse_type_comp;
        codegen(info);
        info->prev = t = info->prev.type ? info->prev : tnext(info->f);
        if(t.type != '?' && t.type != '!') break;
        emit0(info->o, ins_pha);
        while(t.type == '?' || t.type == '!') {
            info->prev.type = 0;
            u8 n = t.type;
            info->type = parse_type_comp;
            codegen(info);
            info->prev = t = info->prev.type ? info->prev : tnext(info->f);
            emit0(info->o, ins_max);
            emit0(info->o, ins_pla);
            emit1(info->o, n == '?' ? ins_nxr : ins_xor, reg_x);
        }
        break;

    case parse_type_expr:
        info->type = parse_type_eqls;
        goto again;

    case parse_type_stmt:
        info->type = parse_type_expr;
        codegen(info);
        t = info->prev.type ? info->prev : tnext(info->f);
        if(t.type != ';') error("Expected ';'.");
        else info->prev = t;
        break;
    default:
        fprintf(stderr, "Parse Type: %d\n", info->type);
        error("Parser(Internal,Fatal): Unknown parse type.");
        exit(1);
        break;
    }
}

int main(int argc, char *argv[]) {
    FILE *in = fopen("test.tl", "r");
    if(!in) {
        error("Failed opening file!");
        return 1;
    }

    FILE *out = fopen("test.bin", "wb");
    if(!out) {
        error("Failed opening file!");
        return 1;
    }

    parser_info_t info;
    info.f = in;
    info.o = out;
    info.last = 0x2000;
    info.prev = NIL_TOK;
    info.type = 0;
    for(u8 i = 0; i < VAR_COUNT; ++i)
        info.vars[i] = 0;

    for(int c;;) {
        for(c=fgetc(info.f);;c=fgetc(info.f))
            if(c != ' ' && c != '\n' && c != '\t' && c != '\r')
                break;

        if(c == EOF) break;
        ungetc(c, info.f);

        info.type = parse_type_stmt;
        codegen(&info);
        info.prev = NIL_TOK;
    }

    emit0(info.o, ins_hlt);

    fclose(in);
    fclose(out);

    if(error_count != 0) {
        fprintf(stderr, "Failed to compile due to %d errors.\n", error_count);
        return 1;
    }
    return 0;
}
