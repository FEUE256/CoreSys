# CoreSys & Co Version

- CoreSys Version: 0.5.1v Dev
- MC Version: 1.1v Release
- GPT Tool Version: 2.5 Release
- Project (Everything around CoreSyS including website etc.) 0.4.7v

CSC Version: 0.1.3.5v
(For Devs remember to change CS_VER in include/kernel/version.h and docs/history update)

CSC (CoreSys & Co)

Fun fact the CSC 9.0v has only 9000 (excluded EDK Headers) lines of code

0.1.3.5 CSCS gh-lang:
github-linguist
    77.82%  1657729    C
    19.67%  419003     HTML
    1.13%   24015      Makefile
    
0.1.3.5 CSC cloc:
cloc . --exclude-dir=.git,bin,.venv,dist
     379 text files.
     333 unique files.
     109 files ignored.

github.com/AlDanial/cloc v 2.06  T=22.86 s (14.6 files/s, 4409.1 lines/s)
----------------------------------------------------------------------------------
Language                        files          blank        comment           code
----------------------------------------------------------------------------------
C                                  50           4456           4769          22875
C/C++ Header                      104           7423           4355          20232
JSON                               54              0              0          16574
HTML                                5           4131             11           9592
Markdown                           60            797              0           1985
Other
----------------------------------------------------------------------------------
SUM:                              333          17398           9377          74035
----------------------------------------------------------------------------------
(Removed EDK)

0.1.2.8v CSC gh-lang:
github-linguist
    91.90%  19806864   C
    4.67%   1005575    Assembly
    1.94%   418993     HTML

0.1.2.8 CSC cloc:
 cloc . --exclude-dir=.git,bin,.venv,dist
    3539 text files.
    2158 unique files.                                          
    1412 files ignored.

github.com/AlDanial/cloc v 2.06  T=52.50 s (41.1 files/s, 10429.6 lines/s)
----------------------------------------------------------------------------------
Language                        files          blank        comment           code
----------------------------------------------------------------------------------
C/C++ Header                     1010          48399         154471         111986
C                                 473          28936          59230          82041
JSON                               51              0              0          16448
HTML                                5           4131             11           9592
Assembly                          356           1576           9157           6316
Other
----------------------------------------------------------------------------------
SUM:                             2158          85922         223104         238491
----------------------------------------------------------------------------------

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
Other
----------------------------------------------------------------------------------
SUM:                             2139          84762         222980         233918
----------------------------------------------------------------------------------

0.1.2.6v CSC gh-lang:
github-linguist
    91.89%  19755611   C
    4.68%   1005575    Assembly
    1.95%   418993     HTML

9.0 CSC
cloc . --exclude-dir=.git,bin,.venv,dist
    3311 text files.
    1963 unique files.
    1355 files ignored.

github.com/AlDanial/cloc v 2.06  T=108.03 s (18.2 files/s, 4397.4 lines/s)
----------------------------------------------------------------------------------
Language                        files          blank        comment           code
----------------------------------------------------------------------------------
C/C++ Header                      941          45591         152100         104153
C                                 436          25662          55218          64606
Assembly                          354           1560           9148           6265
TNSDL                             144           1164              9           5944
Markdown                           54            546              0           1648
make                                8             88             87            251
Other
----------------------------------------------------------------------------------
SUM:                             1963          74718         216646         183707
----------------------------------------------------------------------------------
