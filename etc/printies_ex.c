void hellwal_usage(const char *name)
{
    printf("Usage:\n");
    printf("  %s -i <image> [OPTIONS]\n\n", name);
    printf("Options:\n");
    printf("  -i, --image <image>                Set image file\n");
    printf("  -d, --dark                         Set dark mode (default)\n");
    printf("  -l, --light                        Set light mode\n");
    printf("  -c, --color                        Enable colorized mode (experimental)\n");
    printf("  -v, --invert                       Invert colors in the palette\n");
    printf("  -m, --neon-mode                    Enhance colors for a neon effect\n");
    printf("  -r, --random                       Pick random image or theme\n");
    printf("  -q, --quiet                        Suppress output\n");
    printf("  -j, --json                         Prints colors to stdout in json format, it's skipping templates\n");
    printf("  -s, --script             <script>  Execute script after running hellwal\n");
    printf("  -f, --template-folder    <dir>     Set folder containing templates\n");
    printf("  -o, --output             <dir>     Set output folder for generated templates\n");
    printf("  -t, --theme              <file>    Set theme file or name\n");
    printf("  -k, --theme-folder       <dir>     Set folder containing themes\n");
    printf("  -g, --gray-scale         <value>   Apply grayscale filter   (0-1) (float)\n");
    printf("  -n, --dark-offset        <value>   Adjust darkness offset   (0-1) (float)\n");
    printf("  -b, --bright-offset      <value>   Adjust brightness offset (0-1) (float)\n");
    printf("  --check-contrast                   Ensure colors are readable against the background\n");
    printf("  --preview                          Preview current terminal colorscheme\n");
    printf("  --preview-small                    Preview current terminal colorscheme - small factor\n");
    printf("  --debug                            Enable debug mode\n");
    printf("  --version                          Print version and exit\n");
    printf("  --no-cache                         Disable caching\n");
    printf("  --skip-term-colors                 Skip setting colors to the terminal\n");
    printf("  --skip-luminance-sort              Skip sorting colors before applying\n");
    printf("  --static-background \"#hex\"         Set static background color\n");
    printf("  --static-foreground \"#hex\"         Set static foreground color\n");
    printf("  -h, --help                         Display this help and exit\n\n");
    printf("Defaults:\n");
    printf("  Template folder: ~/.config/hellwal/templates\n");
    printf("  Theme folder: ~/.config/hellwal/themes\n");
    printf("  Output folder: ~/.cache/hellwal/\n\n");
}

void print_term_colors()
{
    for (int i = 0; i < PALETTE_SIZE; i++)
    {
        // set foreground color
        printf("\033[38;5;%dm", i);
        printf(" FG %2d ", i);

        // reset and set background color
        printf("\033[0m");
        printf("\033[48;5;%dm", i);
        printf(" BG %2d ", i);

        // reset again
        printf("\033[0m\n");
    }
    printf("\n");

    print_term_colors_small();

    // reset at the end
    printf("\033[0m\n");
}

void print_term_colors_small()
{
    for (int i=0; i<PALETTE_SIZE; i++)
    {
        if (i == PALETTE_SIZE/2)
            printf("\n");

        //printf("\033[38;5;%dm", i); // if you want to set foreground color
        printf("\033[48;5;%dm", i);   // if you want to set background color
        printf("   ");                // two spaces as a "block" of a color
    }
    // reset at the end
    printf("\033[0m\n");
}

/* Writes color as block to stdout - it does not perform new line by itself */
void print_color(RGB col)
{
    if (ARGS.QUIET != 0)
        return;

    char *color_block = "   ";                  // color_block is 3 spaces wide
                                                // color_block + ↓↓↓↓↓↓
    /* Write color from as colored block */
    fprintf(stdout, "\x1b[48;2;%d;%d;%dm%s\033[0m", col.R, col.G, col.B, color_block);
}

