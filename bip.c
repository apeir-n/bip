/***************************************
 ***************** bip *****************
 ***************************************
 ***** algorithmic image generator *****
 ***************************************
 ***************************************
 ***************************************
 ************* BSD 2-Clause ************
 ******* see LICENSE for details *******
 ***************************************/

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <getopt.h>
#include <string.h>

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
    VAR_C
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

/* function declarations */
void die(const char *fmt, ...);
int clip(int val, int min, int max);
Expr *make_expr(int depth);
int eval_expr(Expr *e, int x, int y, int c);
char *str_expr(Expr *e);
void app_expr(Expr *e, char *buf, size_t size);
void free_expr(Expr *e);
int black_pix(Pixel p);
void alloc_pix(Image *img);
void free_pix(Image *img);
void write_img(Image *img, Expr *e, int c, int thresh);
void save_bmp(Image *img);
void save_xpm(Image *img, char *expr_str, int c);
void usage(void);
void parse_args(int argc, char *argv[], Image *out, int *depth, int *random, int *thresh, int *xpm, int *name_provided, int *name_allocated);

/* function implementations */
void die(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    fprintf(stderr, "\n");
    va_end(args);
    exit(1);
}

int clip(int val, int min, int max) {
    if (val < min) return min;
    if (val > max) return max;
    return val;
}

Expr *make_expr(int depth) {
    Expr *e = malloc(sizeof(Expr));
    if (!e) die("resource allocation failed");

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

/* test if pixel is black for xpm function */
int black_pix(Pixel p) {
    return p.r == 0x00 && p.g == 0x00 && p.b == 0x00;
}

void alloc_pix(Image *img) {
    img->pix = malloc(img->h * sizeof(Pixel *));
    if (!img->pix) die("resource allocation failed");

    for (int i = 0; i < img->h; i += 1) {
        img->pix[i] = malloc(img->w * sizeof(Pixel));
        if (!img->pix[i]) die("resource allocation failed");
    }
}

void free_pix(Image *img) {
    for (int i = 0; i < img->h; i += 1) {
        free(img->pix[i]);
    }
    free(img->pix);
}

void write_img(Image *img, Expr *e, int c, int thresh) {
    for (int y = 0; y < img->h; y += 1) {
        for (int x = 0; x < img->w; x += 1) {
            int val = eval_expr(e, x, y, c);

            if (val % 256 > thresh) {
                img->pix[y][x] = (Pixel){0xff, 0xff, 0xff};
            } else {
                img->pix[y][x] = (Pixel){0x00, 0x00, 0x00};
            }
        }
    }
}

void save_bmp(Image *img) {
    FILE *f = fopen(img->name, "wb");
    if (!f) die("couldn't open file for writing");

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

void save_xpm(Image *img, char *expr_str, int c) {
    FILE *f = fopen(img->name, "w");
    if (!f) die("couldn't open file for writing");

    fprintf(f, "/* XPM */\n");
    fprintf(f, "/* %s, c = %d */\n", expr_str, c);
    fprintf(f, "static const char *image[] = {\n");
    fprintf(f, "\"%d %d 2 1\",\n", img->h, img->w);
    fprintf(f, "\". c #000000\",\n");
    fprintf(f, "\"  c #ffffff\",\n");

    for (int y = 0; y < img->h; y += 1) {
        fprintf(f, "\"");
        for (int x = 0; x < img->w; x += 1) {
            const unsigned char color[3] = {
                img->pix[y][x].r,
                img->pix[y][x].g,
                img->pix[y][x].b,
            };

            if (black_pix(img->pix[y][x])) {
                fputc('.', f);
            } else {
                fputc(' ', f);
            }
        }
        fprintf(f, "\",\n");
    }
    fprintf(f, "};\n");
    fclose(f);
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
    printf("      \033[93m--xpm,    -x                      \033[96m- generates a .xpm file instead of .bmp\n");
    printf("      \033[93m--name,   -n \033[95m<str> \033[91m(\"bitty.bmp\")  \033[96m- name of output image file\n");
    printf("      \033[93m--help,   -h                      \033[96m- displays this help message\n\n");
    printf("  \033[4;92mexamples\033[0;96m:\n\n");
    printf("      \033[94mbip \033[93m--depth\033[94m=\033[91m4 \033[93m--random\033[94m=\033[91m0 \033[93m--name\033[94m=\033[91mboppajam\033[96m\n\n");
    printf("  this will generate an image named 'boppajam.bmp' using an expression\n");
    printf("  that's always the same length, at a recursion depth of 4.\n\n");
    printf("      \033[94mbip \033[93m-d \033[91m2 \033[93m-r \033[91m6 \033[93m-w \033[91m1024 \033[93m-i \033[91m1024\033[96m \033[93m-x\n\n");
    printf("  this will generate a 1024x1024 size XPM that uses an expr with a minimum\n");
    printf("  recursion depth of 2, but can go much deeper with a high randomness level.\n\n");
    printf("  \033[4;92minfo\033[0;96m:\n");
    printf("      - the image will be spit out into the directory from which \033[94mbip\033[96m was called\n");
    printf("      - the file extension (.bmp or .xpm) is automatically added to the filename\n");
    printf("      - depth can be thought of as the length and complexity of the expression\n");
    printf("      - random is added to depth, and the max recursion depth is given by the sum\n");
    printf("          - i.e., if depth=3 and random=2, the recursion depth min=3 and max=5\n");
    printf("      - setting random to 0 will disable it, so all exprs will be the same length\n\n");
    printf("  for more details on how the recursive tree structure works, see:\n");
    printf("  \033[95mhttps://github.com/apeir-n/bip/blob/master/README.md\033[0m\n");
}

void parse_args(int argc, char *argv[], Image *out, int *depth, int *random, int *thresh, int *xpm, int *name_provided, int *name_allocated) {
    static struct option long_options[] = {
        {"width",   required_argument,  0,  'w'},
        {"height",  required_argument,  0,  'i'},
        {"depth",   required_argument,  0,  'd'},
        {"random",  required_argument,  0,  'r'},
        {"thresh",  required_argument,  0,  't'},
        {"name",    required_argument,  0,  'n'},
        {"xpm",     no_argument,        0,  'x'},
        {"help",    no_argument,        0,  'h'},
        {0,         0,                  0,   0 },
    };

    int opt;
    while ((opt = getopt_long(argc, argv, "w:i:d:r:t:n:xh", long_options, NULL)) != -1) {
        switch (opt) {
            case 'w': out->w = clip(atoi(optarg), 16, 8192); break;
            case 'i': out->h = clip(atoi(optarg), 16, 8192); break;
            case 'd': *depth = clip(atoi(optarg), 2, 12); break;
            case 'r': *random = clip(atoi(optarg), 0, 6); break;
            case 't': *thresh = clip(atoi(optarg), 1, 255); break;
            case 'x': *xpm = 1; break;
            case 'n': {
                if (!optarg || strlen(optarg) == 0)                 die("filename is empty");
                if (strstr(optarg, "/") || strstr(optarg, ".."))    die("filename can't include a / or ..");
                if (strlen(optarg) > 240)                           die("filename is too long");

                const char *ext = xpm ? ".xpm" : ".bmp";
                size_t len = strlen(optarg);
                size_t extlen = strlen(ext);

                /* if the user gives a file extension, copy it; else add it in (according to filetype from xpm ternary above) */
                if (len >= extlen && strcmp(optarg + len - extlen, ext) == 0) {
                    out->name = strdup(optarg);
                } else {
                    out->name = malloc(len + extlen + 1);
                    if (!out->name) die("resource allocation failed");
                    snprintf(out->name, len + extlen + 1, "%s%s", optarg, ext);
                }
                *name_allocated = 1;
                *name_provided = 1;
            } break;
            case 'h':
                usage();
                exit(0);
            default:
                printf("usage:\n");
                usage();
                exit(1);
        }
    }
}

int main(int argc, char *argv[]) {
    srand(time(NULL));

    /* defaults */
    int c = rand() % 30;
    int depth = 2;
    int random = 4;
    int thresh = 5;
    int size = 256;
    int name_allocated = 0;
    int name_provided = 0;
    int xpm = 0;
    Image out = {
        .w = size,
        .h = size,
        .name = NULL,
    };

    /* cli args */
    parse_args(
            argc,
            argv,
            &out,
            &depth,
            &random,
            &thresh,
            &xpm,
            &name_provided,
            &name_allocated
            );

    if (!name_provided) {
        out.name = strdup(xpm ? "bitty.xpm" : "bitty.bmp");
        if (!out.name) die("resource allocation failed");
        name_allocated = 1;
    }

    /* prepare expr stuff */
    int randepth = depth + (random > 0 ? rand() % random : 0);
    Expr *e = make_expr(randepth);
    char *expr_str = str_expr(e);
    if (!expr_str) die("resource allocation failed");

    /* do image stuff */
    alloc_pix(&out);
    write_img(&out, e, c, thresh);
    if (xpm) save_xpm(&out, expr_str, c);
    else save_bmp(&out);

    /* print expr */
    printf("%s, c = %d\n", expr_str, c);

    /* cleanup */
    free_pix(&out);
    if (name_allocated) free(out.name);
    free(expr_str);
    free_expr(e);

    /* goodbye */
    return 0;
}
