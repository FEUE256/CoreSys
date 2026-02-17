ENTRY(efi_main)
OUTPUT_FORMAT(pei-x86-64)
SECTIONS {
  . = 0x100000;
  .text : { *(.text*) }
  .rodata : { *(.rodata*) }
  .data : { *(.data*) }
  .bss : { *(.bss*) }
}
