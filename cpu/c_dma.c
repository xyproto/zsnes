#include "c_dma.h"
#include "../initc.h"
#include "../ui.h"
#include "memtable.h"

u1 AddrNoIncr = 0;

static u1 read_reg(eop* const reg, u2 const address)
{
    u1 al;
    asm volatile("call %A1"
                 : "=a"(al)
                 : "rm"(reg), "c"(address)
                 : "cc", "memory", "ebx");
    return al;
}

static void transdmappu2cpu(u1 const al, DMAInfo* const esi)
{
    // set address increment value
    s4 const addrincr = al & 0x08 ? 0 : al & 0x10 ? -1
                                                  : // Automatic decrement
        1; // Automatic increment

    // get address order to be written
    static u1 const addrwrite[][4] = {
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

    u2 const dx = esi->count;
    u1 const curbank = esi->bank;
    u2 cx = esi->offset;
    esi->count = 0;

#if 0 // XXX seems to be unused in the loop
	u1 const* const esi = (cx & 0x8000 ? snesmmap : snesmap2)[curbank];
#endif

    // Do loop
    u4 edx = dx != 0 ? dx : 65536;
    while (edx > 4) {
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
    if (--edx != 0) {
        memw8no_rom(curbank, cx, read_reg(regptrb, cx));
        cx += addrincr;
        if (--edx != 0) {
            memw8no_rom(curbank, cx, read_reg(regptrc, cx));
            cx += addrincr;
            if (--edx != 0) {
                memw8no_rom(curbank, cx, read_reg(regptrd, cx));
                cx += addrincr;
            }
        }
    }

    esi->offset = cx;
}

static inline void write_reg(eop* const reg, u2 const address, u1 const val)
{
    asm volatile("call %A0" ::"rm"(reg), "c"(address), "a"(val)
                 : "cc", "memory", "ebx");
}

static void transdma(DMAInfo* const esi)
{
    u1 const al = esi->control;
    if (al & 0x80) {
        transdmappu2cpu(al, esi);
        return;
    }

    // Set address increment value
    s4 const addrincr = al & 0x08 ? 0 : al & 0x10 ? -1
                                                  : // Automatic decrement
        1; // Automatic increment
    AddrNoIncr = addrincr == 0;

    // Get address order to be written
    u1 mode = al & 0x07;
    if (mode == 5)
        mode -= 4; // Mode 5 DMA
    static u1 const addrwrite[][4] = {
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

    u2 const dx = esi->count;
    u1 const curbank = esi->bank;
    u2 cx = esi->offset;
    esi->count = 0;

#if 0 // XXX seems to be unused in the loop
	u1 const* const esi = (cx & 0x8000 ? snesmmap : snesmap2)[curbank];
#endif

    // Do loop
    u4 edx = dx != 0 ? dx : 65536;
    while (edx > 4) {
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
    if (--edx != 0) {
        u1 const valb = memr8(curbank, cx);
        write_reg(regptrb, cx += addrincr, valb);
        if (--edx != 0) {
            u1 const valc = memr8(curbank, cx);
            write_reg(regptrc, cx += addrincr, valc);
            if (--edx != 0) {
                u1 const vald = memr8(curbank, cx);
                write_reg(regptrd, cx += addrincr, vald);
            }
        }
    }

    esi->offset = cx;
    AddrNoIncr = 0;
}

void c_reg420Bw(u4 eax)
{
    DMAInfo* esi = dmadata;
    for (eax &= 0xFF; eax != 0; ++esi, eax >>= 1) {
        if (eax & 0x01)
            transdma(esi);
    }
}

void setuphdma(u4 const ah, HDMAInfo* const edx, DMAInfo* const esi)
{
    // transfer old address to new address
    u2 const ax = esi->offset;
    esi->hdma_table = ax;
    edx->addr_inc = ax;

    // Get address order to be written
    u1 mode = esi->control & 0x07;
    if (mode >= 5)
        mode -= 4; // Mode 5, 6 or 7
    static u1 const addrnumt[] = { 1, 2, 2, 4, 4, 4, 4, 4 };
    edx->count = addrnumt[mode];
    static u1 const addrwrite[][4] = {
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

    // Get pointers
    u2 const base_addr = 0x2100 + esi->destination;
    for (u4 i = 0; i != lengthof(edx->dst_reg); ++i) {
        u2 bx = base_addr + edi[i]; // PPU memory - 21xx
        if (bx == 0x2118 || bx == 0x2119)
            bx = 0x2200; // Bad hack _Demo_
        edx->dst_reg[i] = REGPTW(bx);
    }

    esi->hdma_line_counter = 0;
    hdmatype |= ah;
}

void c_reg420Cw(u4 eax)
{
    u1 const al = eax;
    curhdma = al;
    // [sneed] fix games that use double HDMA.
    if (curypos < resolutn && (!(INTEnab & 0x10) || (80 <= HIRQLoc && HIRQLoc <= 176))) {
        nexthdma = al;
        if (al != 0x00) {
            DMAInfo* esi = dmadata;
            HDMAInfo* edx = hdmadata;
            for (u1 i = 0x01; i != 0; ++esi, ++edx, i <<= 1) {
                if (al & i)
                    setuphdma(i, edx, esi);
            }
        }
    }
    if (nohdmaframe == 1)
        ++hdmadelay;
    hdmarestart = 0;
}

static void setuphdmars(HDMAInfo* const edx, DMAInfo const* const esi)
{
    // get address order to be written
    u1 mode = esi->control & 0x07;
    if (mode >= 5)
        mode -= 4; // Mode 5, 6 or 7;
    static u1 const addrnumt[] = { 1, 2, 2, 4, 4, 4, 4, 4 };
    edx->count = addrnumt[mode];
    static u1 const addrwrite[][4] = {
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

    // Get pointers
    u2 const base_addr = 0x2100 + esi->destination;
    for (u4 i = 0; i != lengthof(edx->dst_reg); ++i) {
        u2 bx = base_addr + edi[i]; // PPU memory - 21xx
        if (bx == 0x2118 || bx == 0x2119)
            bx = 0x2200; // Bad hack _Demo_
        edx->dst_reg[i] = REGPTW(bx);
    }
}

void starthdma(void)
{
    u1 const al = curhdma;
    nexthdma = al;
    if (al == 0x00)
        return;

    DMAInfo* esi = dmadata;
    HDMAInfo* edx = hdmadata;
    for (u1 i = 0x01; i != 0; ++esi, ++edx, i <<= 1) {
        if (al & i)
            setuphdma(i, edx, esi);
    }
}

static void hdmatype2indirect(HDMAInfo const* const edx, DMAInfo* const esi)
{
    u1 tempdecr = edx->count;
    eop* const* reg = edx->dst_reg;
    do {
        u2 const cx = esi->count++; // increment/decrement/keep pointer location
        u1 const al = memr8(esi->hdma_bank, cx);
        write_reg(*reg, cx, al);
    } while (++reg, --tempdecr != 0);

    --esi->hdma_line_counter;
}

static void indirectaddr(u4 const ah, HDMAInfo* const edx, DMAInfo* const esi)
{
    if ((esi->hdma_line_counter & 0x7F) == 0) {
        if (!(hdmatype & ah))
            edx->addr_inc += 2;
        hdmatype &= ~ah;

        u1 const al = memr8(esi->bank, edx->addr_inc++);
        esi->hdma_line_counter = al;
        esi->count = memr16(esi->bank, edx->addr_inc);
        if (al == 0) {
            nexthdma ^= ah;
            esi->hdma_table = edx->addr_inc;
            return;
        }

        if (esi->hdma_line_counter > 0x80)
            goto hdmatype2indirect;

        u1 tempdecr = edx->count;
        u2 cx = esi->count; // increment/decrement/keep pointer location
        eop* const* reg = edx->dst_reg;
        do {
            u1 const al = memr8(esi->hdma_bank, cx);
            write_reg(*reg, cx, al);
        } while (++cx, ++reg, --tempdecr != 0);
    } else if (esi->hdma_line_counter & 0x80) {
    hdmatype2indirect:
        hdmatype2indirect(edx, esi);
        return;
    }

    esi->hdma_table = edx->addr_inc;
    --esi->hdma_line_counter;
}

static void hdmatype2(HDMAInfo* const edx, DMAInfo* const esi)
{
    u1 tempdecr = edx->count;
    eop* const* reg = edx->dst_reg;
    do {
        u2 const cx = edx->addr_inc++; // increment/decrement/keep pointer location
        u1 const al = memr8(esi->bank, cx);
        write_reg(*reg, cx, al);
    } while (++reg, --tempdecr != 0);

    esi->hdma_table = edx->addr_inc;
    --esi->hdma_line_counter;
}

static void dohdma(u4 const ah, HDMAInfo* const edx, DMAInfo* const esi)
{
    if (esi->control & 0x40) {
        indirectaddr(ah, edx, esi);
        return;
    }

    if ((esi->hdma_line_counter & 0x7F) == 0) {
        if (!(esi->hdma_line_counter & 0x80) && !(hdmatype & ah))
            edx->addr_inc += edx->count; // Increment
        hdmatype &= ~ah;

        u1 const al = memr8(esi->bank, edx->addr_inc++);
        esi->hdma_line_counter = al;

        if (al == 0) {
            nexthdma ^= ah;
            esi->hdma_table = edx->addr_inc;
            return;
        }

        if (esi->hdma_line_counter > 0x80)
            goto hdmatype2;

        u1 tempdecr = edx->count;
        u2 cx = edx->addr_inc;
        eop* const* reg = edx->dst_reg;
        do {
            u1 const al = memr8(esi->bank, cx);
            write_reg(*reg, cx, al);
        } while (++cx, ++reg, --tempdecr != 0);
    } else if (esi->hdma_line_counter & 0x80) {
    hdmatype2:
        hdmatype2(edx, esi);
        return;
    }

    esi->hdma_table = edx->addr_inc;
    --esi->hdma_line_counter;
}

static void exechdmars(void)
{
    u1 const al = nexthdma;
    if (al != 0x00) {
        DMAInfo* esi = dmadata;
        HDMAInfo* edx = hdmadata;
        for (u1 i = 0x01; i != 0; ++esi, ++edx, i <<= 1) {
            if (!(al & i))
                continue;
            setuphdmars(edx, esi);
            dohdma(i, edx, esi);
        }
    }
    hdmarestart = 0;
}

void exechdma(void)
{
    if (hdmarestart == 1) {
        exechdmars();
        return;
    }

    u1 const al = nexthdma;
    if (al == 0x00)
        return;

    DMAInfo* esi = dmadata;
    HDMAInfo* edx = hdmadata;
    for (u1 i = 0x01; i != 0; ++esi, ++edx, i <<= 1) {
        if (al & i)
            dohdma(i, edx, esi);
    }
}
