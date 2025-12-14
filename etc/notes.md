# notes to self

ideas
- make it modular?
    - expression generator as its own binary that writes to stdout
    - pipe the output into the image generator
    - make script to do that and open with an image viewer
        ```sh
        #!/bin/sh

        expressia | bip && xdg-open *.bmp
        ```
    - i'd probably have to rewrite the whole thing to be able to parse the literal expressions
        - since i'm using internal enums for all the components and converting them to * / + - etc symbols later in order to print
- i could make bop more robust and make an interface for format conversion with `magick`
    - i could also add a viewer option

## parameters for cli later
- default is random, but user can input expression that will be passed to `int val` in `write()`
- maybe also `int thresh`
- come up with a framework for coloring
- width and height
- filename
- later filetype (bmp, xpm, png maybe)

### todo
- [x] better expr generation
- [/] parameterization
- [ ] xpm
- [ ] figure out good algorithms for coloring
- [ ] figure out user input for expressions with input sanitization
