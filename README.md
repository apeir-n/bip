# bip

`bip` is an algorithmic image generator that can be used to make interesting patterns with pixels. The magic in `bip` lies in the recursive tree structure it uses to generate an expression, which is then evaluated to a single integer for each pixel, and written to the 2d array to make the output image. The image will be output to the current working directory from which `bip` was called, and then can be opened with an image viewer. This repo has a `build` script that when run, will automatically compile, execute, and open the image in an image viewer. After it's been compiled, you can use `build` to just run the program and open the resulting image with `./build run`.

This project was just an exercise for me while I try learning c. If you have advice for better programming practices or whatever, let me know.
