# TC Syntax

Here is the tc syntax documentation. This is a work in progress, and it's not complete yet. It's also subject to change, as we continue to develop and improve TC. But it should give you a good idea of the syntax of TC, and how to use it to write code.

NOTE: All code is not runable. TC is in develepment and can change. Part of the code is also just that part like for example if it is a that prints Hello, World! it maybe is only the printing part and without for example tc.init(); and main function etc.
NOTE: All ASM is 86_64x ASM NASM syntax.

MIT License

## Program Structure

Every tc program start in the main function, which is the entry point of the program. The main function is defined as follows:

```tc
func main() {
    tc.init();
    // Your code here
}
```
The `tc.init()` function is the initialization function for TC, and it must be called at the beginning of the main function. It sets up the TC runtime and prepares the environment for your code to run. `tc` is a build in library that is essential for TC, and it provides many functions and features that you can use in your code. You can import other libraries and modules as well, but `tc` is always available for the compiler (TC2) and CoreSys Kernel (CSK). ;'s are an option you can make, you can also switch in code also.

## Comments
TC supports both line comments (LC) and block comments (BC). Here is how you can use them:

```tc
# This is a line comment. It starts with a '#' and continues until the end of the line.
// This also a line comment. It starts with a '//' and continues until the end of the line.
// This is a block comment. It starts with '//' and ends with '\\'. \\
```
Line comments are useful for adding brief explanations or notes to your code, while block comments are useful for adding longer explanations or for commenting out blocks of code during development. You can use either type of comment based on your preference and the context of your code. In TC it also exist a special type of comment called a "LICENSE comment". A LICENSE (LC) comment is a block comment that contains the license information for your code. It is typically placed at the top of your source file, and it provides information about the license under which your code is distributed. A LC starts with '^~' and ends with '~^'. For the license to be approved, it must be placed at the top of the file and must be properly formatted. Here is an the correct format for a LICENSE comment:  

```tc
^~
[PROJECT_NAME/PATH/TO/FILE HERE] [LICENSE NAME HERE] [NAME OF THE AUTHOR/AUTHORS/COMPANY HERE] [START YEAR HERE]-[THIS YEAR HERE]

Copyright (c) [START YEAR HERE]-[THIS YEAR HERE] [NAME OF THE AUTHOR/AUTHORS/COMPANY HERE]

[FULL LICENSE TEXT HERE]

Read more about the license here: [LINK TO THE LICENSE HERE]
ABF
This code/project is licensed under the [LICENSE NAME HERE] license. See the [PROJECT_NAME/PATH/TO/LICENSE/FILE HERE] file for more details.
~^ 
```

You can have up to 3 diffrent license websies, with the new rule that it must be seperated by ", ". Every link has it's https:// or http://, www if it have and the full link. If the license doesn't have a website write: "LWNF". Here is an example of a LICENSE comment for a file called "example.tc" that is licensed under the MIT License:

```tc
^~
CoreSys/Example/example.tc MIT License John Doe 2020-2024

Copyright (c) 2020-2024 John Doe

Copyright © 2026 John Doe

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Read more about the license here: https://opensource.org/licenses/MIT, https://mit-license.org/
ABF
This code/project is licensed under the MIT license. See the CoreSys/LICENSE file for more details.
~^ 
```

This correctly LICENSE comment syntax. ABF stands for "Approved By FÈUE". FÈUE is the company behind TC, CoreSys and many more projects. If you want a license to be approved, you must follow the correct syntax and format for the LICENSE comment, and you must submit it for review to the FÈUE team. You can submit your LICENSE comment for review by contacting the FÈUE team directly through our website or social media channels. Once your LICENSE comment is reviewed and approved, it will be marked with "ABF" and it will be considered valid and compliant with our licensing policies. You can not use ABF else. License that are approved: 1. MIT, 2. Apache 2.0, 3. GPLv3, 4. LGPLv3, 5. BSD 2-Clause, 6. BSD 3-Clause, 7. MPL 2.0, 8. EPL 2.0, 9. AGPLv3, 10. CC0, 11. Artistic License 2.0, 12. Eclipse Public License 1.0, 13. ISC, 14. Zlib, 15. Python Software Foundation License, 16. Mozilla Public License 1.1, 17. GNU FDL, 18. CUA Office License, 19. Open Software License 3.0, 20. Common Development and Distribution License (CDDL), 21. Beerware, 22. WTFPL, 23. Unlicense, 24. Affero GPLv3, 25. Academic Free License, 26. Boost Software License, 27. CeCILL, 28. Creative Commons Attribution, 29. Creative Commons Attribution-ShareAlike, 30. ECL 2.0, 31. Eclipse Distribution License, 32. Eiffel Forum License, 33. Freetype License, 34. Frameworx License, 35. GNAT Modified GPL, 36. GNU Affero GPLv3, 37. GNU GPLv2, 38. GNU LGPLv2.1, 39. Haskell Language Report License, 40. IBM Public License, 41. Intel Open Source License, 42. Jabber Open Source License, 43. JSON License, 44. LAL 1.2, 45. LaTeX Project Public License, 46. Libpng License, 47. Lucent Public License, 48. Microsoft Public License, 49. MIT-Modern, 50. MIT No Attribution, 51. Motosoto License, 52. Multics License, 53. NASA Open Source Agreement, 54. Naumen Public License, 55. NCSA/University of Illinois License, 56. Nokia Open Source License, 57. NPOSL 3.0, 58. Open Data Commons Attribution License, 59. Open Data Commons Open Database License, 60. Open Data Commons Public Domain Dedication, 61. Open LDAP Public License, 62. OpenSSL License, 63. OSL 2.1, 64. PHP License, 65. PostgreSQL License, 66. Python License 2.0, 67. Q Public License, 68. Ricoh Source Code Public License, 69. Ruby License, 70. SAX-PD License, 71. Sleepycat License, 72. Sun Industry Standards Source License, 73. Sun Public License, 74. Sybase Open Watcom Public License, 75. University of California License, 76. University of Toronto License, 77. VIM License, 78. W3C License, 79. X11 License, 80. Xnet License, 81. Zope Public License, 82. AFL 3.0, 83. BSD 4-Clause, 84. BSD 0-Clause, 85. Clear BSD License, 86. Crystal License, 87. Do What The F*ck You Want License, 88. Erlang Public License, 89. Fair License, 90. Frameworx Public License, 91. GPL 1.0, 92. GNU Free Documentation License v1.2, 93. IBM InterConnect License, 94. IPA Font License, 95. LaTeX2e License, 96. Lucent Public License v1.02, 97. MIT X11, 98. Mozilla Public License 2.0, 99. Open LDAP Public License v2.8, 100. PostgreSQL License v1.0. The LWNF stands for "License Website not found". If you use this you can only use it ones (like not LWNF, LWNF etc) and you can't combind it with a website. You but must use a LC but it is strongly recommended by FÈUE.

## Variables

When you create a varibale the compiler will automatically on the first assignmeant.

Syntax Example:
```tc
tc.math.assign(name, value);
```

The value can be anything and any data type. You don't need data type definition symbols like "" or ' etc, but if you want for example anywhere in tc get the data from a varibale in a function or a diffrent varibale you will use $ for example:

```tc
tc.math.assign(a, Hello);
tc.math.assign(b, "$a, Hello");
tc.printf(b);
```

Gives us:

```bash
Hello, World!
```

in the console, but also if spaces are in the code use " " or ' '.

Rules:
1. No declaration keyword
2. Case sensitive
3. Default internal size: 64-bit

Valid names:
    x
    value
    counter
    myVariable etc

Invalid names:
5x
$ex ^

^ If you want the variable's name to be the value of for example ex then it a vaild name, but not if that is not the intended meaning. 

## Console and Graphics

TC uses CLUI (Command Line User Interpase) as defult but you can get a working GUI (Graphical User Interpase) with a init command. For example:

```tc
tc.graphics.init();
```

This will initialize the GUI subsystem for your TC program. Now you can use the tc.graphics functions.

## Functions

Functions is very importent for modern programming. So here is the function systax:

```tc
func name() {
    tc.func.init();
}
```

For example:
```tc
func example() {
    tc.func.init();
    tc.ret.init();
    ret 0;
}
```

With arguments you use this syntax for example:

```tc
func ret_value(x) {
    tc.func.init();
    tc.ret.init();
    ret x;
}
```
When you use arguments it's like defining a variable you don't need a data type definer like int, string etc.

## Return Statment

In programming we use return statments very much in our code so this the keyword for a return statment in TC:

```tc
tc.ret.init();
ret;
```

For example:
```tc
tc.ret.init();
ret 5;
```
or
```tc
tc.ret.init();
ret x;
```

This compiles to in ASM:
```asm
mov rax, value
ret
```

## Numbers

These are the supported number types in TC:

1. Base 10:
```tc
tc.math.assign(x, 10);
```

2. Base 16:
```tc
tc.math.assign(x, 0xFF);
```

3. Base 2:
```tc
tc.math.assign(x, 0b1010);
```

## Operators
Math:
```tc
tc.math.add;
tc.math.subtract;
tc.math.multiply;
tc.math.divide;
tc.math.modulo;
```

Example:
```tc
tc.math.assign(x, tc.math.add(5, 9));
```

Assignment:
```tc
tc.math.assign();
```

Comparison:
```tc
tc.math.equal();
tc.math.not_equal();
tc.math.less_then();
tc.math.greater_then();
tc.math.greater_then_or_equal();
tc.math.less_then_or_equal();
```

## Expressions

Example:
```tc
tc.math.assign(x, 5);
tc.math.assign(y, tc.math.add($x, 10));
tc.math.assign(z, tc.math.multiply($y, 2));
```

## Function Calls

Syntax: 
```tc
name();
```

Example:
```tc
tc.print();
```

Assigning variable with function return value:
```tc
tc.math.assign(result, tc.math.add(7, 9));
```

## Programming language injection

This is an optional advanced feature. It is called CI (Code Injection). This is the syntax for CI:
```tc
tc.lang.init();
tc.math.assign(tc.lang.CODE, 0b1)
[Programming Language Name] {
    # CODE
}
tc.math.assign(tc.lang.CODE, 0b0)
tc.math.assign(tc.lang.STATUS, 0b1)
tc.lang.run(tc.lang.STATUS); # Run the code
```

This is an example for ASM:
```tc
tc.lang.init();
tc.math.assign(tc.lang.CODE, 0b1)
asm {
    section .data
        msg db "Hello, World!", 10  ; 10 = newline
        len equ $ - msg           ; message length

    section .text
        global _start

    _start:
        ; write(1, msg, len)
        mov rax, 1        ; syscall: write
        mov rdi, 1        ; file descriptor: stdout
        mov rsi, msg      ; message address
        mov rdx, len      ; message length
        syscall

        ; exit(0)
        mov rax, 60       ; syscall: exit
        xor rdi, rdi      ; exit code 0
        syscall
}
tc.math.assign(tc.lang.CODE, 0b0)
tc.math.assign(tc.lang.STATUS, 0b1)
tc.lang.run(tc.lang.STATUS); # Runs the code
```

And then in the console:
```bash
Hello, World!
```

## Whitespace

Ignored excpt for separation.

Valid:
```tc
tc.math.assign(x,10)
```

Valid:
```tc
tc.math.assign(x, 10)
```

## Required Chatacters

Statment end (Optinal, but recummended):
```tc
;
```

Block start:
```tc
{
```

Block end:
```tc
}
```

## Entry Point

Required function:
```tc
func main() {

}
```

This function name can be change with the -e [FUNCTION NAME] when you compile with TC2

## Bare Minimum Program

This is the smallest possible code with defult settings:
```tc
func main() {
    tc.init();
    ret 0;
}
```

## How get data from a variable
```tc
tc.math.assign(x, 8); # x = 8
tc.math.assign(y, x); # y = 8
tc.math.assign(z, $x); # z = 8
tc.math.assign(a, tc.math.access(x)) // a = 8
```
Here we can see that it don't matter if you have a $ before the variable name or not. You can also use `tc.math.access(x)` to access the data from the variable x.

## Example Full Program File
Info: 
    Filename: test.tc 
    Location: MyTestFolder/test.tc
    License: MIT License
    License File: MyTestFolder/LICENSE

Example File:
```tc
^~
CoreSys/Example/example.tc MIT License John Doe 2020-2024

Copyright (c) 2020-2024 John Doe

Copyright © 2026 John Doe

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

Read more about the license here: https://opensource.org/licenses/MIT, https://mit-license.org/
ABF
This code/project is licensed under the MIT license. See the CoreSys/LICENSE file for more details.
~^ 

func main {
    tc.init(); # Init 

    tc.math.assign(x, 58); # Assigns x to be 58 
    tc.math.assign(y, 23); # Assigns y to be 23
    tc.math.assign(z, tc.math.add(x, y)); # Assigns z to be x + y

    # Prints z to the console
    tc.printf(z) # Prints 81 to the console

    ret $z; # Returns the value of z (x + y)
}
```

Result:
81 has been printed in the console.

## File Extension
Technical C source file:
```inputfile
program.tc
```
Compiled output:
```outputfile
program.run
program.o
program.asm
program.s
program.c
```

## Jumps and Labels
When you jump you just go to where the lable is. Lable syntax:
```tc
lable: 
    # CODE

    ret x;
```
Jump Syntax:
```tc
lable: 
    # CODE

    ret x;
jump lable;
```
This makes a inf loop.

## If/else statments
If/else statments are very importent coding part. If/else syntax:
```tc
if (bolean statment) {
    # If true 
} else {
    # If false
}
```
Elseif's can be made with many if/else statments.

## Loops
A loops is for when you want the thing done many times in a row. Syntax:
```tc
loop (end=x, index=y) {
    tc.loops.init();
    start:
        #CODE
        tc.math.add(index, 1);
        if (tc.math.equal(index, end)) {
            ret 0;
        } else {
            jump start;
        }
    ret 0;
}
```
Example:
```tc
loop (end=5, index=0) {
    tc.loops.init();
    start:
        tc.print(Index: $index);
        tc.math.add(index, 1);
        if (tc.math.equal(index, end)) {
            ret 0;
        } else {
            jump start;
        }
    ret 0;
}
```
Example on a inf loop:
```tc
loop:
    ret 0;
jump loop;
```

## Reserved Keywords
Current:
```tc
func
ret
asm
c
c#
c++
if
else
loop
```

## TC Syntax Abbreviations and the meanings
    TC – Technical C (programming language under development)
    ASM – Assembly (x86_64 NASM syntax)
    LC – Line Comment
    BC – Block Comment
    LICENSE comment (LC) – Special comment for license info
    ABF – Approved By FÈUE (marks license approval)
    CLUI – Command Line User Interpase
    GUI – Graphical User Interpase
    CI – Code Injection
    LWNF – License Website Not Found
    TC2 – Compiler for TC
    CSK – CoreSys Kernel
    FÈUE – Company behind TC, CoreSys, and other projects
    RT - Runtime
    TR - Technical C Runtime
    API - Application Programming Interface

## TC Standard Tool Chain

### Core Initialization
- `tc.init();` – Initializes the TC runtime.

### Math / Variable Operations
- `tc.math.assign(name, value);` – Assigns a value to a variable.
- `tc.math.add(a, b);` – Adds two values.
- `tc.math.subtract(a, b);` – Subtracts `b` from `a`.
- `tc.math.multiply(a, b);` – Multiplies two values.
- `tc.math.divide(a, b);` – Divides `a` by `b`.
- `tc.math.modulo(a, b);` – Modulus operation (`a % b`).
- `tc.math.equal(a, b);` – Returns true if `a == b`.
- `tc.math.not_equal(a, b);` – Returns true if `a != b`.
- `tc.math.less_then(a, b);` – Returns true if `a < b`.
- `tc.math.greater_then(a, b);` – Returns true if `a > b`.
- `tc.math.greater_then_or_equal(a, b);` – Returns true if `a >= b`.
- `tc.math.less_then_or_equal(a, b);` – Returns true if `a <= b`.

### Variable Access
- `$var` – Accesses the value of a variable.

### Printing / Output
- `tc.printf(value);` – Prints value to console.
- `tc.print(value);` – Alias for printing.

### Functions
- `tc.func.init();` – Initializes function context.
- `tc.ret.init();` – Prepares return statement.
- `ret value;` – Returns a value from a function.

### Loops
- `tc.loops.init();` – Initializes loop control.

### Graphics / GUI
- `tc.graphics.init();` – Initializes GUI subsystem.

### Code Injection / Multi-language
- `tc.lang.init();` – Starts code injection block.
- `tc.math.assign(tc.lang.CODE, 0b1);` – Activates code injection.
- `tc.math.assign(tc.lang.CODE, 0b0);` – Deactivates code injection.
- `tc.math.assign(tc.lang.STATUS, 0b1);` – Marks injection ready to run.
- `tc.lang.run(tc.lang.STATUS);` – Executes injected code.

By: Nils Efverman, CEO FÈUE
