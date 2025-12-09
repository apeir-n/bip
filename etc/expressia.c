#include <stdio.h>
#include <stdlib.h>
#include <time.h>

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

Expr *make_expr(int depth) {
    Expr *e = malloc(sizeof(Expr));

    /* the rand() % 4 part makes it more unpredictable, without it, the depth param denotes the number of nodes */
    /* if (depth <= 1) { */
    if (depth <= 1 || rand() % 4 == 0) {
        /* base case: make a leaf */
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

void print_expr(Expr *e) {
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
}

void free_expr(Expr *e) {
    if (e->kind == EXPR_OP) {
        free_expr(e->op_expr.l);
        free_expr(e->op_expr.r);
    }
    free(e);
}

int main() {
    srand(time(NULL));

    Expr *e = make_expr(4);
    printf("expression: ");
    print_expr(e);
    printf("\n");
    free_expr(e);
    return 0;
}
