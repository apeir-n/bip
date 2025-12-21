/***************************************
 ***************** bip *****************
 ***** algorithmic image generator *****
 ***************************************
 ***************************************
 ************* BSD 2-Clause ************
 ******* see LICENSE for details *******
 ***************************************
 ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <getopt.h>
#include <string.h>

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

    const unsigned char bmpfileheader[14] = {
        'B',            'M',
        filesize,       filesize>>8,    filesize>>16,   filesize>>24,
        0,              0,              0,              0,
        54,             0,              0,              0
    };

    const unsigned char bmpinfoheader[40] = {
        40,             0,              0,              0,
        img->w,         img->w>>8,      img->w>>16,     img->w>>24,
        img->h,         img->h>>8,      img->h>>16,     img->h>>24,
        1,              0,              24,             0
    };

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    const unsigned char padding[3] = {0,0,0};
    int padding_size = padded_row_size - img->w * 3;

    for (int y = img->h - 1; y >= 0; y -= 1) {
        for (int x = 0; x < img->w; x += 1) {
            const unsigned char color[3] = {
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

void write_img(Image *img, Expr *ptrn, int c, int thresh) {
    for (int y = 0; y < img->h; y += 1) {
        for (int x = 0; x < img->w; x += 1) {
            int val = eval_expr(ptrn, x, y, c);
            /* int val = (((x | y) | (x | x)) ^ ((x << y) ^ (x ^ y))); */

            if (val % 256 > thresh) {
                img->pix[y][x] = (Pixel){0xff, 0xff, 0xff};
            } else {
                img->pix[y][x] = (Pixel){0x00, 0x00, 0x00};
            }
        }
    }
}

void usage() {
    printf("\n\033[96m");
    printf("      ██░             ██░\n");
    printf("      ██░\n");
    printf("      ██████░     ████░       ████████░\n");
    printf("    ██░     ██░     ██░       ██░     ██░\n");
    printf("    ██░     ██░   ██░       ██░       ██░\n");
    printf("  ██░     ██░   ██░         ██░     ██░\n");
    printf("  ████████░     ████░       ████████░\n");
    printf("                          ██░\n");
    printf("                          ██░\n");
    printf("  \033[94mbip - algorithmic image generator\n");
    printf("  \033[96m================= = =  =  =   =   =\n\n");
    printf("  usage: \033[94mbip [\033[4;92moptions\033[0;94m]\033[96m\n\n");
    printf("  \033[94mbip\033[96m generates an expression using a recursive tree structure,\n");
    printf("  which is evaluated and written to each pixel in the output file.\n\n");
    printf("  \033[4;92moptions\033[0;96m:\n\n");
    printf("      \033[93m--width,  -w \033[95m<int> \033[91m(256)          \033[96m- width in pixels\n");
    printf("      \033[93m--height, -i \033[95m<int> \033[91m(256)          \033[96m- height in pixels\n");
    printf("      \033[93m--depth,  -d \033[95m<int> \033[91m(2)            \033[96m- number of iterations in tree\n");
    printf("      \033[93m--random, -r \033[95m<int> \033[91m(4)            \033[96m- random offset to depth param\n");
    printf("      \033[93m--thresh, -t \033[95m<int> \033[91m(5)            \033[96m- threshold for white/black pixels\n");
    printf("      \033[93m--name,   -n \033[95m<str> \033[91m(\"bitty.bmp\")  \033[96m- name of output image file\n");
    printf("      \033[93m--help,   -h                      \033[96m- displays this help message\n\n");
    printf("  \033[4;92mexamples\033[0;96m:\n\n");
    printf("      \033[94mbip \033[93m--depth\033[94m=\033[91m4 \033[93m--random\033[94m=\033[91m0 \033[93m--name\033[94m=\033[91mboppajam\033[96m\n\n");
    printf("  this will generate an image named 'boppajam.bmp' using an expression\n");
    printf("  that's always the same length, at a recursion depth of 4.\n\n");
    printf("      \033[94mbip \033[93m-d \033[91m2 \033[93m-r \033[91m6 \033[93m-w \033[91m1024 \033[93m-h \033[91m1024\033[96m\n\n");
    printf("  this will generate a 1024x1024 size image that uses an expr with a minimum\n");
    printf("  recursion depth of 2, but can go much deeper with a high randomness level.\n\n");
    printf("  \033[4;92minfo\033[0;96m:\n");
    printf("      - the image will be spit out into the directory from which \033[94mbip\033[96m was called\n");
    printf("      - the '.bmp' extension is automatically added to the filename\n");
    printf("      - depth can be thought of as the length and complexity of the expression\n");
    printf("      - random is added to depth, and the max recursion depth is given by the sum\n");
    printf("          - i.e., if depth=3 and random=2, the recursion depth min=3 and max=5\n");
    printf("      - setting random to 0 will disable it, so all exprs will be the same length\n\n");
    printf("  for more details on how the recursive tree structure works, see:\n");
    printf("  \033[95mhttps://github.com/apeir-n/bip/blob/master/README.md\033[0m\n");
}

int main(int argc, char *argv[]) {

    /* seed the random functions */
    srand(time(NULL));

    /* defaults */
    int c = rand() % 30;
    int depth = 2;
    int random = 4;
    int thresh = 5;
    int size = 256;
    int name_allocated = 0;

    Image out = {
        .w = size,
        .h = size,
        .name = "bitty.bmp"
    };

    /* declare and parse args */
    static struct option long_options[] = {
        {"width",   required_argument,  0,  'w'},
        {"height",  required_argument,  0,  'i'},
        {"depth",   required_argument,  0,  'd'},
        {"random",  required_argument,  0,  'r'},
        {"thresh",  required_argument,  0,  't'},
        {"name",    required_argument,  0,  'n'},
        {"help",    no_argument,        0,  'h'},
        {0,         0,                  0,   0 },
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "w:i:d:r:n:h", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w': out.w = clip(atoi(optarg), 16, 8192); break;
            case 'i': out.h = clip(atoi(optarg), 16, 8192); break;
            case 'd': depth = clip(atoi(optarg), 2, 12); break;
            case 'r': random = clip(atoi(optarg), 0, 6); break;
            case 't': thresh = clip(atoi(optarg), 1, 255); break;
            case 'n':
                {
                    /* sanitize */
                    if (!optarg || strlen(optarg) == 0) { fprintf(stderr, "filename can't be empty\n"); exit(1); }
                    if (strstr(optarg, "/") || strstr(optarg, "..")) { fprintf(stderr, "filename can't include a / or ..\n"); exit(1); }
                    if (strlen(optarg) > 240) { fprintf(stderr, "filename is too long\n"); exit(1); }

                    /* find the correct size to allocate depending on whether '.bmp' extension was added from cli */
                    const char *ext = ".bmp";
                    size_t len = strlen(optarg);
                    size_t extlen = strlen(ext);
                    if (len >= extlen && strcmp(optarg + len - extlen, ext) == 0) {
                        out.name = optarg;
                        name_allocated = 0;
                    } else {
                        out.name = malloc(len + extlen + 1);
                        if (!out.name) {
                            fprintf(stderr, "couldn't malloc filename\n");
                            exit(1);
                        }
                        snprintf(out.name, len + extlen + 1, "%s%s", optarg, ext);
                        name_allocated = 1;
                    }
                }
                break;
            case 'h': usage(); return 0;
            default:
                printf("usage:\n");
                usage();
                return 1;
        }
    }

    /* do the image stuff */
    int randepth = depth + (random > 0 ? rand() % random : 0);
    Expr *e = make_expr(randepth);
    alloc_pix(&out);
    write_img(&out, e, c, thresh);
    save_bmp(&out);
    free_pix(&out);

    if (name_allocated) {
        free(out.name);
    }

    /* print the expression */
    printf("expression: ");
    print_expr(e);
    printf("\nwhere c = %d\n", c);
    free_expr(e);

    return 0;
}
