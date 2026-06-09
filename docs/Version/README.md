# CoreSys & Co Version

- CoreSys Version: 0.4.6v Dev
- MC Version: 1.1v Release
- GPT Tool Version: 2.5 Release
- Project (Everything around CoreSyS including website etc.) 0.4.5v

CSC Version: 0.1.2.7v
(For Devs remember to change CS_VER in include/kernel/version.h and docs/history update)

CSC (CoreSys & Co)

Fun fact the CSC 9.0v has only 9000 (excluded EDK Headers) lines of code

0.1.2.6v CSC cloc:
cloc . --exclude-dir=.git,bin,.venv,dist
    3520 text files.
    2139 unique files.                                          
    1411 files ignored.

----------------------------------------------------------------------------------
Language                        files          blank        comment           code
----------------------------------------------------------------------------------
C/C++ Header                      997          47979         154374         110441
C                                 473          28914          59221          82027
JSON                               51              0              0          16447
HTML                                5           4131             11           9592
Assembly                          356           1576           9157           6316
TNSDL                             144           1164              9           5944
Markdown                           57            584              0           1386
make                               15            185            133            547
CSS                                 4             53              1            265
Bourne Shell                       11             67             15            243
Text                                5             11              0            195
Python                              4             55             56            153
PHP                                 3             10              0            152
YAML                                6             20              1            118
JavaScript                          2             10              2             38
Linker Script                       4              0              0             37
Fortran 77                          1              3              0             16
Windows Message File                1              0              0              1
----------------------------------------------------------------------------------
SUM:                             2139          84762         222980         233918
----------------------------------------------------------------------------------

0.1.2.6v CSC gh-lang:
github-linguist
    91.89%  19755611   C
    4.68%   1005575    Assembly
    1.95%   418993     HTML
    1.23%   265295     C++
    0.11%   23144      Makefile
    0.06%   12198      Python
    0.03%   6296       Shell
    0.03%   6171       Roff
    0.02%   4410       CSS
    0.01%   1181       JavaScript
