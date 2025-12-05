#include <stdio.h>
#include <stdint.h>

typedef struct {
    unsigned char r, g, b;
} Pixel;

void save(const char *filename, int width, int height, Pixel pixels[height][width]) {
    FILE *f = fopen(filename, "wb");

    /*  3 * width   3 bytes per pixel (rgb), so * width = num of bytes needed per row
     *  + 3         offset in order to round to nearest multiple of 4 after some bitwise math
     *  & (~3)      AND NOT 3, so keep all the bits except the last 2
     *
     *              3 = 0x00000003 = 00000000 00000000 00000000 00000011
     *             ~3 = 0xfffffffc = 11111111 11111111 11111111 11111100
     *              so when you AND all the bits against this, it keeps them all except last 2
     */
    int padded_row_size = (3 * width + 3) & (~3);
    int filesize = 54 + padded_row_size * height;

    unsigned char bmpfileheader[14] = {
        'B',            'M',
        filesize,       filesize>>8,    filesize>>16,   filesize>>24,
        0,              0,              0,              0,
        54,             0,              0,              0
    };

    unsigned char bmpinfoheader[40] = {
        40,             0,              0,              0,
        width,          width>>8,       width>>16,      width>>24,
        height,         height>>8,      height>>16,     height>>24,
        1,              0,              24,             0
    };

    fwrite(bmpfileheader, 1, 14, f);
    fwrite(bmpinfoheader, 1, 40, f);

    unsigned char padding[3] = {0,0,0};
    int padding_size = padded_row_size - width * 3;

    for (int y = height - 1; y >= 0; y -= 1) {
        for (int x = 0; x < width; x += 1) {
            unsigned char color[3] = {
                pixels[y][x].b,
                pixels[y][x].g,
                pixels[y][x].r
            };
            fwrite(color, 1, 3, f);
        }
        fwrite(padding, 1, padding_size, f);
    }
    fclose(f);
}

void write(int h, int w, Pixel img[h][w]) {
    for (int y = 0; y < h; y += 1) {
        for (int x = 0; x < w; x += 1) {
            int val = (x & y) % 18;

            if (val > 12) {
                img[y][x].r = 0xFF;
                img[y][x].g = 0xFF;
                img[y][x].b = 0xFF;
            } else {
                img[y][x].r = 0x00;
                img[y][x].g = 0x00;
                img[y][x].b = 0x00;
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

int main() {
    int h = 256, w = 256;
    Pixel img[h][w];

    write(h, w, img);

    save("bitty.bmp", w, h, img);
    return 0;
}
