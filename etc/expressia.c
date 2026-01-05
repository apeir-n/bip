#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* types & enums */
enum Operator {
    OP_ADD,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_AND,
    OP_OOR,
    OP_XOR,
    OP_SHL,
    OP_SHR
};

enum Operand {
    VAR_X,
    VAR_Y,
    VAR_C /* const */
};

typedef enum {
    EXPR_OP,
    EXPR_VAL
} ExprKind;

typedef struct Expr {
    ExprKind kind;
    union {
        struct {
            enum Operator op;
            struct Expr *l;
            struct Expr *r;
        } op_expr;
        enum Operand val_expr;
    };
} Expr;

/* function declarations */
int clip(int val, int min, int max);
Expr *make_expr(int depth);
int eval_expr(Expr *e, int x, int y, int c);
char *str_expr(Expr *e);
void app_expr(Expr *e, char *buf, size_t size);
void free_expr(Expr *e);

/* function implementations */
int clip(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

Expr *make_expr(int depth) {
    Expr *e = malloc(sizeof(Expr));

    if (!e) {
        fprintf(stderr, "couldn't malloc expression 🙈💩💥\n");
        exit(1);
    }

    /* base case: make a leaf */
    if (depth <= 1) {
        e->kind = EXPR_VAL;
        e->val_expr = rand() % 3;
        return e;
    }

    /* recursive case: make a branch */
    e->kind = EXPR_OP;
    e->op_expr.op = rand() % 10;
    e->op_expr.l = make_expr(depth - 1);
    e->op_expr.r = make_expr(depth - 1);
    return e;
}

int eval_expr(Expr *e, int x, int y, int c) {
    if (e->kind == EXPR_VAL) {
        switch (e->val_expr) {
            case VAR_X: return x;
            case VAR_Y: return y;
            case VAR_C: return c;
            default: return 0;
        }
    }

    int a = eval_expr(e->op_expr.l, x, y, c);
    int b = eval_expr(e->op_expr.r, x, y, c);

    switch (e->op_expr.op) {
        case OP_ADD: return a + b;
        case OP_SUB: return a - b;
        case OP_MUL: return a * b;
        case OP_DIV: return b == 0 ? 0 : a / b;
        case OP_MOD: return b == 0 ? 0 : a % b;
        case OP_AND: return a & b;
        case OP_OOR: return a | b;
        case OP_XOR: return a ^ b;
        case OP_SHL: return a << (b % 8);
        case OP_SHR: return a >> (b % 8);
        default: return 0;
    }
}

/* void print_expr(Expr *e) {
    if (e->kind == EXPR_VAL) {
        switch (e->val_expr) {
            case VAR_X: printf("x"); break;
            case VAR_Y: printf("y"); break;
            case VAR_C: printf("c"); break;
        }
        return;
    }
    
    printf("(");
    print_expr(e->op_expr.l);

    switch (e->op_expr.op) {
        case OP_ADD: printf(" + ");  break;
        case OP_SUB: printf(" - ");  break;
        case OP_MUL: printf(" * ");  break;
        case OP_DIV: printf(" / ");  break;
        case OP_MOD: printf(" %% "); break;
        case OP_AND: printf(" & ");  break;
        case OP_OOR: printf(" | ");  break;
        case OP_XOR: printf(" ^ ");  break;
        case OP_SHL: printf(" << "); break;
        case OP_SHR: printf(" >> "); break;
    }

    print_expr(e->op_expr.r);
    printf(")");
} */

/* make buffer for expr string */
char *str_expr(Expr *e) {
    char *buf = malloc(1024);
    if (!buf) return NULL;
    buf[0] = '\0';

    app_expr(e, buf, 1024);
    return buf;
}

/* append strings to buffer */
void app_expr(Expr *e, char *buf, size_t size) {
    if (e->kind == EXPR_VAL) {
        const char *var =
            (e->val_expr == VAR_X) ? "x" :
            (e->val_expr == VAR_Y) ? "y" :
            (e->val_expr == VAR_C) ? "c" : "?";
        strncat(buf, var, size - strlen(buf) - 1);
        return;
    }

    strncat(buf, "(", size - strlen(buf) - 1);
    app_expr(e->op_expr.l, buf, size);

    const char *op =
        (e->op_expr.op == OP_ADD) ? " + " :
        (e->op_expr.op == OP_SUB) ? " - " :
        (e->op_expr.op == OP_MUL) ? " * " :
        (e->op_expr.op == OP_DIV) ? " / " :
        (e->op_expr.op == OP_MOD) ? " % " :
        (e->op_expr.op == OP_AND) ? " & " :
        (e->op_expr.op == OP_OOR) ? " | " :
        (e->op_expr.op == OP_XOR) ? " ^ " :
        (e->op_expr.op == OP_SHL) ? " << " :
        (e->op_expr.op == OP_SHR) ? " >> " : " ? ";
    strncat(buf, op, size - strlen(buf) - 1);

    app_expr(e->op_expr.r, buf, size);
    strncat(buf, ")", size - strlen(buf) - 1);
}

void free_expr(Expr *e) {
    if (e->kind == EXPR_OP) {
        free_expr(e->op_expr.l);
        free_expr(e->op_expr.r);
    }
    free(e);
}

int main() {
    /* seed the random functions */
    srand(time(NULL));

    /* defaults */
    int c = rand() % 30;
    int depth = 2;
    int random = 4;
    int randepth = depth + (random > 0 ? rand() % random : 0);
    Expr *e = make_expr(randepth);

    char *expr_str = str_expr(e);
    if (!expr_str) return 1;

    printf("expr: %s\n", expr_str);
    printf("where c = %d\n", c);

    free(expr_str);
    free_expr(e);

    /* printf("expression: ");
    print_expr(e);
    printf("\nwhere c = %d\n", c);
    free_expr(e); */

    return 0;
}
