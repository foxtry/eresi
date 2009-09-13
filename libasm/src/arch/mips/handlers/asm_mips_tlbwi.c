/**
* @file libasm/src/arch/mips/handlers/asm_mips_tlbwi.c
** @ingroup MIPS_instrs
*/
/* Adam 'pi3' Zabrocki */
/* Manuel Martin - 2007 */

#include <libasm.h>

/* TLBWI */

int asm_mips_tlbwi(asm_instr *ins, u_char *buf, u_int len,
                  asm_processor *proc)
{
   ins->instr = ASM_MIPS_TLBWI;
   ins->type = ASM_TYPE_ARCH | ASM_TYPE_CONTROL;
   asm_mips_operand_fetch(&ins->op[0], buf, ASM_MIPS_OTYPE_NOOP, ins);

   /* Exceptions: Coprocessor Unusable */

   return 4;
}
