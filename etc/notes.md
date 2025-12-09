# notes to self

- currently got the random generator working to generate patterns like:
- `(a ? b)`
- where a and b could either be:
    - the x or y values accumulated over the 2d array
        - in the for loops in `write()`
    - or c, a constant over range 0 -- 99
    - with `?` being an operator enumerated in `enum Operator`

i'd like to extend the expression framework to generate more complex equations, like
- `((x ^ y) & (x << 2)) % 37`
- `((x * y) >> 3) ^ ((y - x) << 1)`
- `(((x | y) + (x ^ y)) & ((~x) << 1)) % 18`
- having a constant at the end to modulo is nice
- maybe i could make it so in `(a ? b)`, both a and b become their own `(a ? b)`
    - like where `a = (x & y)` and `b = (y >> 9)` and `? = |` so that:
    - `((x & y) | (y >> 9))`
    - or something
- but theres probably a more elegant way to generate these kinds of expressions
    - this approach feels too manual

- i have some sort of tree like structure in my mind where the branches are sets of parentheses
- inside the parentheses, there is an operator and an operand on either side
    - `( a ? b )`
- the operators can either be:
    - x or y from 2d img array
    - random constant
    - or another branch, i.e. another set of parentheses containing a sub expression
- the operands can be any from the enumerated list
- then another constant can be appended to the end, with an operator like modulo or bitshifter
- this approach with a template `((a ? b) ? (a ? b)) ? c` could produce things like:
    - `((x + x) - (y + y)) << 1`
    - `((x ^ y) & (x << 2)) % 37`
    - `(((x | y) + (x ^ y)) & ((~x) << 1)) % 18`

## parameters for cli later
- default is random, but user can input expression that will be passed to `int val` in `write()`
- maybe also `int thresh`
- come up with a framework for coloring
- width and height
- filename
- later filetype (bmp, xpm, png maybe)

### todo
- [ ] better expr generation
- [ ] parameterization
- [ ] xpm
- [ ] figure out good algorithms for coloring
