#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int w, h;
    char *name;
    Pixel **pix;
} Image;

void save(Image *img) {
    FILE *f = fopen(img->name, "wb");

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

void write(Image *img) {
    for (int y = 0; y < img->h; y += 1) {
        for (int x = 0; x < img->w; x += 1) {
            int val = (x & y) % 18;

            if (val > 12) {
                img->pix[y][x].r = 0xFF;
                img->pix[y][x].g = 0xFF;
                img->pix[y][x].b = 0xFF;
            } else {
                img->pix[y][x].r = 0x00;
                img->pix[y][x].g = 0x00;
                img->pix[y][x].b = 0x00;
            }
            /* if (val > 12) {
                img[y][x].r = (x ^ y) ^ 0xFF;
                img[y][x].g = (x | y) ^ 0xFF;
                img[y][x].b = (x & y) ^ 0xFF;
            } else {
                img[y][x].r = (x ^ y) & 0xFF;
                img[y][x].g = (x | y) & 0xFF;
                img[y][x].b = (x & y) & 0xFF;
            } */
        }
    }
}

void palloc(Image *img) {
    img->pix = malloc(img->h * sizeof(Pixel *));
    if (!img->pix) {
        fprintf(stderr, "uh ohs... couldn't malloc rows! 🙈💩💥\n");
        exit(1);
    }
    for (int i = 0; i < img->h; i += 1) {
        img->pix[i] = malloc(img->w * sizeof(Pixel));
        if (!img->pix[i]) {
            fprintf(stderr, "uh ohs... couldn't malloc row %d! 🙈💩💥\n", i);
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
    Image out = {
        .w = 256,
        .h = 256,
        .name = "bitty.bmp"
    };

    palloc(&out);
    write(&out);
    save(&out);
    pfree(&out);

    return 0;
}
