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

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int w, h;
    char *name;
    Pixel **pix;
} Image;

int depth_param(int depth, int random) {

    /* 
     * depth = 1       only returns 1 leaf:            a
     * depth = 2       returns 1 branch:               (a ? b)
     * depth = 3       returns 3 branches, 4 leaves:   ((a ? b) ? (c ? d))
     * depth = 4       returns 7 branches, 8 leaves:   (((a ? b) ? (c ? d)) ? ((e ? f) ? (g ? h)))
     * 
     * here the depth is minimized at 2, and randomness is optionally added to increase the number
     * iterations (branches). random is maxed at 4, and if it's zero, then the same number of branches
     * are made each time the expression is generated.
     */

    if (depth < 2) depth = 2;
    if (random > 5) random = 5;
    return depth + (random > 0 ? rand() % random : 0);
}

Expr *make_expr(int depth) {
    Expr *e = malloc(sizeof(Expr));

    if (depth <= 1) {
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

void alloc_pix(Image *img) {
    img->pix = malloc(img->h * sizeof(Pixel *));
    if (!img->pix) {
        fprintf(stderr, "couldn't malloc rows 🙈💩💥\n");
        exit(1);
    }
    for (int i = 0; i < img->h; i += 1) {
        img->pix[i] = malloc(img->w * sizeof(Pixel));
        if (!img->pix[i]) {
            fprintf(stderr, "couldn't malloc row %d 🙈💩💥\n", i);
            exit(1);
        }
    }
}

void free_pix(Image *img) {
    for (int i = 0; i < img->h; i += 1) {
        free(img->pix[i]);
    }
    free(img->pix);
}

void save_bmp(Image *img) {
    FILE *f = fopen(img->name, "wb");
    if (!f) {
        fprintf(stderr, "couldn't open file %s for writing 🙈💩💥\n", img->name);
        exit(1);
    }

    /*  3 * width   3 bytes per pixel (rgb), so * width = num of bytes needed per row
     *  + 3         offset in order to round to nearest multiple of 4 after some bitwise math
     *  & (~3)      AND NOT 3, so keep all the bits except the last 2
     *
     *              3 = 0x00000003 = 00000000 00000000 00000000 00000011
     *             ~3 = 0xfffffffc = 11111111 11111111 11111111 11111100
     *              so when you AND all the bits against this, it keeps them all except last 2
     */

    int padded_row_size = (3 * img->w + 3) & (~3);
    int filesize = 54 + padded_row_size * img->h;

    unsigned char bmpfileheader[14] = {
        'B',            'M',
        filesize,       filesize>>8,    filesize>>16,   filesize>>24,
        0,              0,              0,              0,
        54,             0,              0,              0
    };

    unsigned char bmpinfoheader[40] = {
        40,             0,              0,              0,
        img->w,         img->w>>8,      img->w>>16,     img->w>>24,
        img->h,         img->h>>8,      img->h>>16,     img->h>>24,
        1,              0,              24,             0
    };

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    unsigned char padding[3] = {0,0,0};
    int padding_size = padded_row_size - img->w * 3;

    for (int y = img->h - 1; y >= 0; y -= 1) {
        for (int x = 0; x < img->w; x += 1) {
            unsigned char color[3] = {
                img->pix[y][x].b,
                img->pix[y][x].g,
                img->pix[y][x].r
            };
            fwrite(color, 1, 3, f);
        }
        fwrite(padding, 1, padding_size, f);
    }
    fclose(f);
}

void write_img(Image *img, Expr *ptrn, int c) {
    for (int y = 0; y < img->h; y += 1) {
        for (int x = 0; x < img->w; x += 1) {
            int val = eval_expr(ptrn, x, y, c);
            int thresh = 5;

            if (val % 256 > thresh) {
                img->pix[y][x] = (Pixel){0xff, 0xff, 0xff};
            } else {
                img->pix[y][x] = (Pixel){0x00, 0x00, 0x00};
            }
        }
    }
}

int main() {
    srand(time(NULL));

    int c = rand() % 30;
    int depth = depth_param(2, 4);
    Expr *e = make_expr(depth);

    Image out = {
        .w = 256,
        .h = 256,
        .name = "bitty.bmp"
    };

    alloc_pix(&out);
    write_img(&out, e, c);
    save_bmp(&out);
    free_pix(&out);

    printf("expression: ");
    print_expr(e);
    printf("\nwhere c = %d\n", c);
    free_expr(e);

    return 0;
}
