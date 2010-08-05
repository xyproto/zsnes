#include "../ui.h"
#include "c_dma.h"
#include "dma.h"
#include "memtable.h"


static u1 read_reg(eop* const reg, u2 const address)
{
	u1 al;
	asm volatile("call %A1" : "=a" (al) : "rm" (reg), "c" (address) : "cc", "memory", "ebx");
	return al;
}


static void transdmappu2cpu(u1 const al, DMAInfo* const esi)
{
	// set address increment value
	s4 const addrincr =
		al & 0x08 ?  0 :
		al & 0x10 ? -1 : // Automatic decrement
		1;               // Automatic increment

	// get address order to be written
	static u1 const addrwrite[][4] =
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 1 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 1, 1 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 }
	};
	u1 const* const edi = addrwrite[al & 0x07];

	// Pointer address of registers
	eop* const regptr_ = REGPTR(0x2100 + esi->destination + edi[0]); // PPU memory - 21xx
	eop* const regptrb = REGPTR(0x2100 + esi->destination + edi[1]); // PPU memory - 21xx
	eop* const regptrc = REGPTR(0x2100 + esi->destination + edi[2]); // PPU memory - 21xx
	eop* const regptrd = REGPTR(0x2100 + esi->destination + edi[3]); // PPU memory - 21xx

	u2 const dx      = esi->count;
	u1 const curbank = esi->bank;
	u2       cx      = esi->offset;
	esi->count = 0;

#if 0 // XXX seems to be unused in the loop
	u1 const* const esi = (cx & 0x8000 ? snesmmap : snesmap2)[curbank];
#endif

	// Do loop
	u4 edx = dx != 0 ? dx : 65536;
	while (edx > 4)
	{
		memw8no_rom(curbank, cx, read_reg(regptr_, cx));
		cx += addrincr;
		memw8no_rom(curbank, cx, read_reg(regptrb, cx));
		cx += addrincr;
		memw8no_rom(curbank, cx, read_reg(regptrc, cx));
		cx += addrincr;
		memw8no_rom(curbank, cx, read_reg(regptrd, cx));
		cx += addrincr;
		edx -= 4;
	}
	memw8no_rom(curbank, cx, read_reg(regptr_, cx));
	cx += addrincr;
	if (--edx != 0)
	{
		memw8no_rom(curbank, cx, read_reg(regptrb, cx));
		cx += addrincr;
		if (--edx != 0)
		{
			memw8no_rom(curbank, cx, read_reg(regptrc, cx));
			cx += addrincr;
			if (--edx != 0)
			{
				memw8no_rom(curbank, cx, read_reg(regptrd, cx));
				cx += addrincr;
			}
		}
	}

	esi->offset = cx;
}


static inline void write_reg(eop* const reg, u2 const address, u1 const val)
{
	asm volatile("call %A0" :: "rm" (reg), "c" (address), "a" (val) : "cc", "memory", "ebx");
}


void transdma(DMAInfo* const esi)
{
	u1 const al = esi->control;
	if (al & 0x80)
	{
		transdmappu2cpu(al, esi);
		return;
	}

	// Set address increment value
	s4 const addrincr =
		al & 0x08 ?  0 :
		al & 0x10 ? -1 : // Automatic decrement
		1;               // Automatic increment
	AddrNoIncr = addrincr == 0;

	// Get address order to be written
	u1 mode = al & 0x07;
	if (mode == 5) mode -= 4; // Mode 5 DMA
	static u1 const addrwrite[][4] =
	{
		{ 0, 0, 0, 0 },
		{ 0, 1, 0, 1 },
		{ 0, 0, 0, 0 },
		{ 0, 0, 1, 1 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 },
		{ 0, 1, 2, 3 }
	};
	u1 const* const edi = addrwrite[mode];

	// Pointer address of registers
	eop* const regptra = REGPTW(0x2100 + esi->destination + edi[0]); // PPU memory - 21xx
	eop* const regptrb = REGPTW(0x2100 + esi->destination + edi[1]); // PPU memory - 21xx
	eop* const regptrc = REGPTW(0x2100 + esi->destination + edi[2]); // PPU memory - 21xx
	eop* const regptrd = REGPTW(0x2100 + esi->destination + edi[3]); // PPU memory - 21xx

	u2 const dx      = esi->count;
	u1 const curbank = esi->bank;
	u2       cx      = esi->offset;
	esi->count = 0;

#if 0 // XXX seems to be unused in the loop
	u1 const* const esi = (cx & 0x8000 ? snesmmap : snesmap2)[curbank];
#endif

	// Do loop
	u4 edx = dx != 0 ? dx : 65536;
	while (edx > 4)
	{
		u1 const vala = memr8(curbank, cx);
		write_reg(regptra, cx += addrincr, vala);
		u1 const valb = memr8(curbank, cx);
		write_reg(regptrb, cx += addrincr, valb);
		u1 const valc = memr8(curbank, cx);
		write_reg(regptrc, cx += addrincr, valc);
		u1 const vald = memr8(curbank, cx);
		write_reg(regptrd, cx += addrincr, vald);
		edx -= 4;
	}
	u1 const vala = memr8(curbank, cx);
	write_reg(regptra, cx += addrincr, vala);
	if (--edx != 0)
	{
		u1 const valb = memr8(curbank, cx);
		write_reg(regptrb, cx += addrincr, valb);
		if (--edx != 0)
		{
			u1 const valc = memr8(curbank, cx);
			write_reg(regptrc, cx += addrincr, valc);
			if (--edx != 0)
			{
				u1 const vald = memr8(curbank, cx);
				write_reg(regptrd, cx += addrincr, vald);
			}
		}
	}

	esi->offset = cx;
	AddrNoIncr  = 0;
}
