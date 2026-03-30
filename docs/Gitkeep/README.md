# Why does some folders have a .gitkeep file

The reson:

    ./bin/Debug/CoreSys/bootloader
    ./bin/Debug/CoreSys/init
    ./bin/Debug/CoreSys/kernel
    ./bin/Debug/GPT
    ./bin/Debug/MC
    ./bin/Release/CoreSys
    ./bin/Release/GPT
    ./bin/Release/MC
    And others...

Haves a .gitkeep file in them is just so Git (The program we use for handle our code versioning and uploading to GitHub (Github is the service the CoreSys project is on; on the internet)) can track the file so they can be pushed to the internet.

TL;DR:

They exsist so the folders is also pushed to the internet not just skipping it because CoreSys is not build.
