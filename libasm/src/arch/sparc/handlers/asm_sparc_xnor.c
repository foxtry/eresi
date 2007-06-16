/*
**
** $Id: asm_sparc_xnor.c,v 1.4 2007-06-16 20:24:26 strauss Exp $
**
*/
#include "libasm.h"

int
asm_sparc_xnor(asm_instr * ins, u_char * buf, u_int len,
		     asm_processor * proc)
{
  struct s_decode_format3 opcode;
  struct s_asm_proc_sparc *inter;
  sparc_convert_format3(&opcode, buf);

  inter = proc->internals;
  ins->instr = inter->op2_table[opcode.op3];
  
  ins->type = ASM_TYPE_ARITH;

  ins->nb_op = 3;
  asm_sparc_op_fetch(&ins->op1, buf, ASM_SP_OTYPE_REGISTER, ins);
  ins->op1.base_reg = opcode.rd;
  asm_sparc_op_fetch(&ins->op3, buf, ASM_SP_OTYPE_REGISTER, ins);
  ins->op3.base_reg = opcode.rs1;

  if (opcode.i == 0) {
    asm_sparc_op_fetch(&ins->op2, buf, ASM_SP_OTYPE_REGISTER, ins);
    ins->op2.base_reg = opcode.rs2;
  }
  else {
    asm_sparc_op_fetch(&ins->op2, buf, ASM_SP_OTYPE_IMMEDIATE, ins);
    ins->op2.imm = opcode.imm;
  }

  if (ins->op2.type == ASM_SP_OTYPE_REGISTER &&
	   ins->op2.base_reg == ASM_REG_G0) {

    ins->instr = ASM_SP_NOT;
    if (ins->op3.base_reg == ins->op1.base_reg)
      ins->nb_op = 1;
    else {
      ins->nb_op = 2;
      ins->op2 = ins->op3;
    }
  }
  
  return 4;

}
