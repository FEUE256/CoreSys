#pragma once

#define CS_X86_64 

typedef struct regs_t regs_t;

extern void _cs_asm_init_main(void);
extern void _cs_asm_ud2_main(void);
extern void _cs_asm_int3_main(void);
extern void _cs_asm_get_regs(regs_t *regs);
