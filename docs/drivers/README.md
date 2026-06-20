# Guide to custom drivers

Include drivers/your/driver/main.h in kernel/main.c

Create your driver in drivers/..../main.h

Use pragma once and then just C code for your driver and then just impiment a driver-brige in drivers/tty/cmd.h
