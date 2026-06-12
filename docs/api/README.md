# API

The CoreSys API has two modes km/um (kernel- / user-) and bm/im (boot- / initmode) the bm/im file is bootloader/init specified (include/API/CoreSys.h) and km/um is kernel/user specified (kernel/include/CoreSys.h)

The CIOS API is not anything CIOS uses but if you had it whould be in fwm/fw (firmware mode / firmware "mode")

### UM

User Mode is where everything that you the end user uses via the kernel um (so not eg the extra functions in the bootloader).

### KM

Kernel Mode is the place the kernel runs at; the highest access mode with acces to HAL (eg outb) directly without halcalls.

### BM

Boot Mode is not an ordinary mode outside CoreSys, but its the mode where the bootloader operates with access to fw tools like (SystemTable and ImageHandle).

### IM

Init Mode is not an ordinary mode outside CoreSys. Its like bm but for the init stage that also have access to fw tools.

## KM/UM API

The KU API as we will now say for the KM/UM API has a calling table as our main API calling technice.

Calling table:
CS_CORE  
├── CS_HAL  
│   ├── HAL_DEV_NULL  
│   │   Args: ret_t code  
│   │   Return: ret_t  
│   │  
│   ├── HAL_EXECUTE_COMMAND  
│   │   Args: set_t let_t* cmd  
│   │   Return: num_t  
│   │  
│   ├── HAL_TTY_LOOP  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_TTY_WRITE  
│   │   Args: set_t let_t* s  
│   │   Return: nret_t  
│   │  
│   ├── HAL_TTY_PUTC  
│   │   Args: let_t c  
│   │   Return: nret_t  
│   │  
│   ├── HAL_OUTB  
│   │   Args: unum16_t port, unum8_t val  
│   │   Return: nret_t  
│   │  
│   ├── HAL_INB  
│   │   Args: unum16_t port  
│   │   Return: unum8_t  
│   │  
│   ├── HAL_OUTW  
│   │   Args: unum16_t port, unum16_t val  
│   │   Return: nret_t  
│   │  
│   ├── HAL_INW  
│   │   Args: unum16_t port  
│   │   Return: unum16_t  
│   │  
│   ├── HAL_SERIAL_TX_EMPTY  
│   │   Args: nret_t  
│   │   Return: unum8_t  
│   │  
│   ├── HAL_SERIAL_RECEIVED  
│   │   Args: nret_t  
│   │   Return: unum8_t  
│   │  
│   ├── HAL_SERIAL_WRITE_let_t  
│   │   Args: let_t c  
│   │   Return: nret_t  
│   │  
│   ├── HAL_A_let_t_PRnum_t  
│   │   Args: let_t c  
│   │   Return: nret_t  
│   │  
│   ├── HAL_SERIAL_WRITE  
│   │   Args: set_t let_t* s  
│   │   Return: nret_t  
│   │  
│   ├── HAL_SERIAL_WRITE_U64  
│   │   Args: unum64_t v  
│   │   Return: nret_t  
│   │  
│   ├── HAL_SERIAL_WRITE_PTR  
│   │   Args: set_t nret_t* p  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_t  
│   │   Args: set_t let_t* s  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_t_let_t  
│   │   Args: let_t c  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_t_U64  
│   │   Args: unum64_t v  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_t_U8  
│   │   Args: unum8_t v  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_tF  
│   │   Args: set_t let_t* s  
│   │   Return: nret_t  
│   │  
│   ├── HAL_SERIAL_READ_let_t  
│   │   Args: nret_t  
│   │   Return: let_t  
│   │  
│   ├── HAL_KREAD  
│   │   Args: nret_t  
│   │   Return: let_t  
│   │  
│   ├── HAL_SERIAL_CLEAR  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KCLEAR  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_K_CLEAR  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KSTRLEN  
│   │   Args: set_t let_t* s  
│   │   Return: cap_t  
│   │  
│   ├── HAL_KPRnum_t_STR  
│   │   Args: set_t let_t* s  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_t_Unum_t  
│   │   Args: unum64_t v, uncon_t base  
│   │   Return: nret_t  
│   │  
│   ├── HAL_KPRnum_t_num_t  
│   │   Args: num64_t v  
│   │   Return: nret_t  
│   │  
│   ├── HAL_INIT_SERIAL  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_DEINIT_SERIAL  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_LED_DEMO  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   ├── HAL_SHUTDOWN  
│   │   Args: nret_t  
│   │   Return: nret_t  
│   │  
│   └── HAL_REBOOT  
│       Args: nret_t  
│       Return: nret_t  
│  
└── CS_SYS  
    ├── SYS_DEV_NULL  
    │   Args: nret_t  
    │   Return: num_t  
    │  
    ├── SYS_READ  
    │   Args: nret_t  
    │   Return: let_t  
    │  
    ├── SYS_WRITE  
    │   Args: set_t let_t* s  
    │   Return: nret_t  
    │  
    ├── SYS_SHUTDOWN  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    ├── SYS_REBOOT  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    ├── SYS_INIT  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    ├── SYS_DEINIT  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    ├── SYS_CLEAR  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    ├── SYS_REINIT  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    ├── SYS_HALT  
    │   Args: nret_t  
    │   Return: nret_t  
    │  
    └── SYS_SF  
        Args: set_t let_t* s  
        Return: nret_t  