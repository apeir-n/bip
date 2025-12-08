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

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int w, h;
    char *name;
    Pixel **pix;
} Image;

typedef struct {
    enum Operator op;
    enum Operand l;
    enum Operand r;
    int constant;
} Expr;

void save(Image *img) {
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

int getop(enum Operand which, int x, int y, int c) {
    switch (which) {
        case VAR_X: return x;
        case VAR_Y: return y;
        case VAR_C: return c;
        default: return 0;
    }
}

int eval(Expr e, int x, int y) {
    int a = getop(e.l, x, y, e.constant);
    int b = getop(e.r, x, y, e.constant);

    switch (e.op) {
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

/*******************************************************
 *************** DEBUG *********************************
 *******************************************************/

const char* opname(int op) {
    switch (op) {
        case OP_ADD: return "ADD";
        case OP_SUB: return "SUB";
        case OP_MUL: return "MUL";
        case OP_DIV: return "DIV";
        case OP_MOD: return "MOD";
        case OP_AND: return "AND";
        case OP_OOR: return "OOR";
        case OP_XOR: return "XOR";
        case OP_SHL: return "SHL";
        case OP_SHR: return "SHR";
        default: return "???";
    }
}

const char* varname(int var) {
    switch (var) {
        case VAR_X: return "X";
        case VAR_Y: return "Y";
        case VAR_C: return "C";
        default: return "???";
    }
}
/*******************************************************
 *************** DEBUG *********************************
 *******************************************************/

Expr randexpr() {
    Expr e;
    e.op = rand() % 10;
    e.l = rand() % 3;
    e.r = rand() % 3;
    e.constant = rand() % 16;
    return e;
}

void write(Image *img) {
    /* Expr ptrn = randexpr(); */
    Expr ptrn = { OP_SHR, VAR_X, VAR_Y, 18 };

    for (int y = 0; y < img->h; y += 1) {
        for (int x = 0; x < img->w; x += 1) {
            int val = eval(ptrn, x, y);
            int thresh = 12;

            if (val % 256 > thresh) {
                img->pix[y][x] = (Pixel){0xff, 0xff, 0xff};
            } else {
                img->pix[y][x] = (Pixel){0x00, 0x00, 0x00};
            }
        }
    }
}

void palloc(Image *img) {
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

void pfree(Image *img) {
    for (int i = 0; i < img->h; i += 1) {
        free(img->pix[i]);
    }
    free(img->pix);
}

int main() {
    srand(time(NULL));
    /* Image out = {
        .w = 256,
        .h = 256,
        .name = "bitty.bmp"
    };

    palloc(&out);
    write(&out);
    save(&out);
    pfree(&out); */


    Expr e = randexpr();

    printf("op: %s\nleft: %s\nright: %s\nconst:%d\n", opname(e.op), varname(e.l), varname(e.r), e.constant);

    int randint = rand();

    printf("randint: %d\n", randint);

    return 0;
}
