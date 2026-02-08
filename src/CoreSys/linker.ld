/* Minimal linker script for CoreSys EFI image */

ENTRY(efi_main)

SECTIONS
{
    /* Start address is irrelevant for UEFI PE/COFF, but keep .text at beginning */
    .text : ALIGN(4K) {
        *(.text)
    }

    .rodata : ALIGN(4K) {
        *(.rodata)
    }

    .data : ALIGN(4K) {
        *(.data)
    }

    .bss : ALIGN(4K) {
        __bss_start__ = .;
        *(.bss)
        *(COMMON)
        __bss_end__ = .;
    }
}
