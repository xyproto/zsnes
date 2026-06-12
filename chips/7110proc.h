#ifndef H7110PROC_H
#define H7110PROC_H

#include "../types.h"

/* Init / reset functions (called via asm_call() from cpu/c_regs.c and cpu/c_regsw.c) */
void SPC7110init(void);
void SPC7110Reset(void);
void initSPC7110regs(void);

/* SPC7110 data registers (referenced from gblvars.h / save-state code) */
extern u4 SPCMultA;
extern u4 SPCMultB;
extern u4 SPCDivEnd;
extern u4 SPCMulRes;
extern u4 SPCDivRes;
extern u4 SPC7110BankA;
extern u4 SPC7110RTCStat;
extern u1 SPC7110RTC[16];
extern u1 SPC7110RTCB[16];
extern u4 SPCROMPtr;
extern u4* SPCROMtoI;
extern u4 SPCROMAdj;
extern u4 SPCROMInc;
extern u4 SPCROMCom;
extern u4 SPCCheckFix;
extern u4 SPCSignedVal;
extern u1 SPCCompressionRegs[13];
extern u4 PHnum2writespc7110reg;

/* Register read handlers (registered via REGPTR in initSPC7110regs) */
void SPC4800(void);
void SPC4801(void);
void SPC4802(void);
void SPC4803(void);
void SPC4804(void);
void SPC4805(void);
void SPC4806(void);
void SPC4807(void);
void SPC4808(void);
void SPC4809(void);
void SPC480A(void);
void SPC480B(void);
void SPC480C(void);
void SPC4810(void);
void SPC4811(void);
void SPC4812(void);
void SPC4813(void);
void SPC4814(void);
void SPC4815(void);
void SPC4816(void);
void SPC4817(void);
void SPC4818(void);
void SPC481A(void);
void SPC4820(void);
void SPC4821(void);
void SPC4822(void);
void SPC4823(void);
void SPC4824(void);
void SPC4825(void);
void SPC4826(void);
void SPC4827(void);
void SPC4828(void);
void SPC4829(void);
void SPC482A(void);
void SPC482B(void);
void SPC482C(void);
void SPC482D(void);
void SPC482E(void);
void SPC482F(void);
void SPC4831(void);
void SPC4832(void);
void SPC4833(void);
void SPC4834(void);
void SPC4840(void);
void SPC4841(void);
void SPC4842(void);
void SPC4850(void);
void SPC4851(void);
void SPC4852(void);
void SPC4853(void);
void SPC4854(void);
void SPC4855(void);
void SPC4856(void);
void SPC4857(void);
void SPC4858(void);
void SPC4859(void);
void SPC485A(void);
void SPC485B(void);
void SPC485C(void);
void SPC485D(void);
void SPC485E(void);
void SPC485F(void);

/* Register write handlers (registered via REGPTW in SPC7110Reset) */
void SPC4801w(void);
void SPC4802w(void);
void SPC4803w(void);
void SPC4804w(void);
void SPC4805w(void);
void SPC4806w(void);
void SPC4807w(void);
void SPC4808w(void);
void SPC4809w(void);
void SPC480Aw(void);
void SPC480Bw(void);
void SPC4811w(void);
void SPC4812w(void);
void SPC4813w(void);
void SPC4814w(void);
void SPC4815w(void);
void SPC4816w(void);
void SPC4817w(void);
void SPC4818w(void);
void SPC4820w(void);
void SPC4821w(void);
void SPC4822w(void);
void SPC4823w(void);
void SPC4824w(void);
void SPC4825w(void);
void SPC4826w(void);
void SPC4827w(void);
void SPC482Ew(void);
void SPC4831w(void);
void SPC4832w(void);
void SPC4833w(void);
void SPC4840w(void);
void SPC4841w(void);
void SPC4842w(void);

/* Memory-bank access functions (used in cpu/memtable.c mrwp entries) */
u1 SPC7110ReadSRAM8b(u4 addr);
void SPC7110WriteSRAM8b(u4 addr, u1 val);
u2 SPC7110ReadSRAM16b(u4 addr);
void SPC7110WriteSRAM16b(u4 addr, u2 val);

u1 memaccessspc7110r8(u4 addr);
void memaccessspc7110w8(u4 addr, u1 val);
u2 memaccessspc7110r16(u4 addr);
void memaccessspc7110w16(u4 addr, u2 val);

#endif
