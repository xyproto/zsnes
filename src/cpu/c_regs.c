#include "../asm_call.h"
#include "../chips/7110proc.h"
#include "../chips/sa1regs.h"
#include "../chips/sfxproc.h"
#include "../gblvars.h"
#include "../initc.h"
#include "../macros.h"
#include "../ui.h"
#include "c_regs.h"
#include "regs.h"


void initregr(void)
{
	// Fill register pointer with invalid register accesses
	for (eop** i = regptra; i != endof(regptra); ++i) *i = regINVALID;

	// Set all valid register accesses
	REGPTR(0x2100) = reg2100r;
	REGPTR(0x2134) = reg2134r;
	REGPTR(0x2135) = reg2135r;
	REGPTR(0x2136) = reg2136r;
	REGPTR(0x2137) = reg2137r;
	REGPTR(0x2138) = reg2138r;
	REGPTR(0x2139) = reg2139r;
	REGPTR(0x213A) = reg213Ar;
	REGPTR(0x213B) = reg213Br;
	REGPTR(0x213C) = reg213Cr;
	REGPTR(0x213D) = reg213Dr;
	REGPTR(0x213E) = reg213Er;
	REGPTR(0x213F) = reg213Fr;
	REGPTR(0x2140) = reg2140r;
	REGPTR(0x2141) = reg2141r;
	REGPTR(0x2142) = reg2142r;
	REGPTR(0x2143) = reg2143r;
	REGPTR(0x2144) = reg2140r;
	REGPTR(0x2145) = reg2141r;
	REGPTR(0x2146) = reg2142r;
	REGPTR(0x2147) = reg2143r;
	REGPTR(0x2148) = reg2140r;
	REGPTR(0x2149) = reg2141r;
	REGPTR(0x214A) = reg2142r;
	REGPTR(0x214B) = reg2143r;
	REGPTR(0x214C) = reg2140r;
	REGPTR(0x214D) = reg2141r;
	REGPTR(0x214E) = reg2142r;
	REGPTR(0x214F) = reg2143r;
	REGPTR(0x2150) = reg2140r;
	REGPTR(0x2151) = reg2141r;
	REGPTR(0x2152) = reg2142r;
	REGPTR(0x2153) = reg2143r;
	REGPTR(0x2154) = reg2140r;
	REGPTR(0x2155) = reg2141r;
	REGPTR(0x2156) = reg2142r;
	REGPTR(0x2157) = reg2143r;
	REGPTR(0x2158) = reg2140r;
	REGPTR(0x2159) = reg2141r;
	REGPTR(0x215A) = reg2142r;
	REGPTR(0x215B) = reg2143r;
	REGPTR(0x215C) = reg2140r;
	REGPTR(0x215D) = reg2141r;
	REGPTR(0x215E) = reg2142r;
	REGPTR(0x215F) = reg2143r;
	REGPTR(0x2160) = reg2140r;
	REGPTR(0x2161) = reg2141r;
	REGPTR(0x2162) = reg2142r;
	REGPTR(0x2163) = reg2143r;
	REGPTR(0x2164) = reg2140r;
	REGPTR(0x2165) = reg2141r;
	REGPTR(0x2166) = reg2142r;
	REGPTR(0x2167) = reg2143r;
	REGPTR(0x2168) = reg2140r;
	REGPTR(0x2169) = reg2141r;
	REGPTR(0x216A) = reg2142r;
	REGPTR(0x216B) = reg2143r;
	REGPTR(0x216C) = reg2140r;
	REGPTR(0x216D) = reg2141r;
	REGPTR(0x216E) = reg2142r;
	REGPTR(0x216F) = reg2143r;
	REGPTR(0x2170) = reg2140r;
	REGPTR(0x2171) = reg2141r;
	REGPTR(0x2172) = reg2142r;
	REGPTR(0x2173) = reg2143r;
	REGPTR(0x2174) = reg2140r;
	REGPTR(0x2175) = reg2141r;
	REGPTR(0x2176) = reg2142r;
	REGPTR(0x2177) = reg2143r;
	REGPTR(0x2178) = reg2140r;
	REGPTR(0x2179) = reg2141r;
	REGPTR(0x217A) = reg2142r;
	REGPTR(0x217B) = reg2143r;
	REGPTR(0x217C) = reg2140r;
	REGPTR(0x217D) = reg2141r;
	REGPTR(0x217E) = reg2142r;
	REGPTR(0x217F) = reg2143r;
	REGPTR(0x2180) = reg2180r;
	REGPTR(0x21C2) = reg21C2r;
	REGPTR(0x21C3) = reg21C3r;

	REGPTR(0x4016) = reg4016r;
	REGPTR(0x4017) = reg4017r;

	REGPTR(0x4100) = reg4100r;

	REGPTR(0x420A) = reg420Ar;
	REGPTR(0x420B) = reg420Br;
	REGPTR(0x420C) = reg420Cr;
	REGPTR(0x420D) = reg420Dr;
	REGPTR(0x420E) = reg420Er;
	REGPTR(0x420F) = reg420Fr;

	REGPTR(0x4210) = reg4210r;
	REGPTR(0x4211) = reg4211r;
	REGPTR(0x4212) = reg4212r;
	REGPTR(0x4213) = reg4213r;
	REGPTR(0x4214) = reg4214r;
	REGPTR(0x4215) = reg4215r;
	REGPTR(0x4216) = reg4216r;
	REGPTR(0x4217) = reg4217r;
	REGPTR(0x4218) = reg4218r;
	REGPTR(0x4219) = reg4219r;
	REGPTR(0x421A) = reg421Ar;
	REGPTR(0x421B) = reg421Br;
	REGPTR(0x421C) = reg421Cr;
	REGPTR(0x421D) = reg421Dr;
	REGPTR(0x421E) = reg421Er;
	REGPTR(0x421F) = reg421Fr;

	for (u4 i = 0x4300; i != 0x4380; ++i) REGPTR(i) = reg43XXr;

	if (SFXEnable)     asm_call(initsfxregsr);
	if (SA1Enable)     asm_call(initSA1regs);
	if (SDD1Enable)    asm_call(initSDD1regs);
	if (SPC7110Enable) asm_call(initSPC7110regs);
	if (RTCEnable)     asm_call(RTCReset);
}
