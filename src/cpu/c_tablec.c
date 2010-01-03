#include "../endmem.h"
#include "../macros.h"
#include "c_tablec.h"
#include "e65816c.h"
#include "table.h"
#include "tablec.h"


/* Sets the opcode tables
 * This function sets all the non-multiple entries */
static void settables(eop** const table)
{
	// row 0
	table[0x00] = cCOp00;
	table[0x01] = cCOp01m8;
	table[0x02] = cCOp02;
	table[0x03] = cCOp03m8;
	table[0x04] = cCOp04m8;
	table[0x05] = cCOp05m8;
	table[0x06] = cCOp06m8;
	table[0x07] = cCOp07m8;
	table[0x08] = cCOp08;
	table[0x09] = cCOp09m8;
	table[0x0A] = cCOp0Am8;
	table[0x0B] = cCOp0B;
	table[0x0C] = cCOp0Cm8;
	table[0x0D] = cCOp0Dm8;
	table[0x0E] = cCOp0Em8;
	table[0x0F] = cCOp0Fm8;
	table[0x10] = cCOp10;
	table[0x11] = cCOp11m8;
	table[0x12] = cCOp12m8;
	table[0x13] = cCOp13m8;
	table[0x14] = cCOp14m8;
	table[0x15] = cCOp15m8;
	table[0x16] = cCOp16m8;
	table[0x17] = cCOp17m8;
	table[0x18] = cCOp18;
	table[0x19] = cCOp19m8;
	table[0x1A] = cCOp1Am8;
	table[0x1B] = cCOp1B;
	table[0x1C] = cCOp1Cm8;
	table[0x1D] = cCOp1Dm8;
	table[0x1E] = cCOp1Em8;
	table[0x1F] = cCOp1Fm8;
	table[0x20] = cCOp20;
	table[0x21] = cCOp21m8;
	table[0x22] = cCOp22;
	table[0x23] = cCOp23m8;
	table[0x24] = cCOp24m8;
	table[0x25] = cCOp25m8;
	table[0x26] = cCOp26m8;
	table[0x27] = cCOp27m8;
	table[0x28] = cCOp28;
	table[0x29] = cCOp29m8;
	table[0x2A] = cCOp2Am8;
	table[0x2B] = cCOp2B;
	table[0x2C] = cCOp2Cm8;
	table[0x2D] = cCOp2Dm8;
	table[0x2E] = cCOp2Em8;
	table[0x2F] = cCOp2Fm8;
	table[0x30] = cCOp30;
	table[0x31] = cCOp31m8;
	table[0x32] = cCOp32m8;
	table[0x33] = cCOp33m8;
	table[0x34] = cCOp34m8;
	table[0x35] = cCOp35m8;
	table[0x36] = cCOp36m8;
	table[0x37] = cCOp37m8;
	table[0x38] = cCOp38;
	table[0x39] = cCOp39m8;
	table[0x3A] = cCOp3Am8;
	table[0x3B] = cCOp3B;
	table[0x3C] = cCOp3Cm8;
	table[0x3D] = cCOp3Dm8;
	table[0x3E] = cCOp3Em8;
	table[0x3F] = cCOp3Fm8;
	table[0x40] = cCOp40;
	table[0x41] = cCOp41m8;
	table[0x42] = cCOp42;
	table[0x43] = cCOp43m8;
	table[0x44] = cCOp44;
	table[0x45] = cCOp45m8;
	table[0x46] = cCOp46m8;
	table[0x47] = cCOp47m8;
	table[0x48] = cCOp48m8;
	table[0x49] = cCOp49m8;
	table[0x4A] = cCOp4Am8;
	table[0x4B] = cCOp4B;
	table[0x4C] = cCOp4C;
	table[0x4D] = cCOp4Dm8;
	table[0x4E] = cCOp4Em8;
	table[0x4F] = cCOp4Fm8;
	table[0x50] = cCOp50;
	table[0x51] = cCOp51m8;
	table[0x52] = cCOp52m8;
	table[0x53] = cCOp53m8;
	table[0x54] = cCOp54;
	table[0x55] = cCOp55m8;
	table[0x56] = cCOp56m8;
	table[0x57] = cCOp57m8;
	table[0x58] = cCOp58;
	table[0x59] = cCOp59m8;
	table[0x5A] = cCOp5Ax8;
	table[0x5B] = cCOp5B;
	table[0x5C] = cCOp5C;
	table[0x5D] = cCOp5Dm8;
	table[0x5E] = cCOp5Em8;
	table[0x5F] = cCOp5Fm8;
	table[0x60] = cCOp60;
	table[0x61] = cCOp61m8nd;
	table[0x62] = cCOp62;
	table[0x63] = cCOp63m8nd;
	table[0x64] = cCOp64m8;
	table[0x65] = cCOp65m8nd;
	table[0x66] = cCOp66m8;
	table[0x67] = cCOp67m8nd;
	table[0x68] = cCOp68m8;
	table[0x69] = cCOp69m8nd;
	table[0x6A] = cCOp6Am8;
	table[0x6B] = cCOp6B;
	table[0x6C] = cCOp6C;
	table[0x6D] = cCOp6Dm8nd;
	table[0x6E] = cCOp6Em8;
	table[0x6F] = cCOp6Fm8nd;
	table[0x70] = cCOp70;
	table[0x71] = cCOp71m8nd;
	table[0x72] = cCOp72m8nd;
	table[0x73] = cCOp73m8nd;
	table[0x74] = cCOp74m8;
	table[0x75] = cCOp75m8nd;
	table[0x76] = cCOp76m8;
	table[0x77] = cCOp77m8nd;
	table[0x78] = cCOp78;
	table[0x79] = cCOp79m8nd;
	table[0x7A] = cCOp7Ax8;
	table[0x7B] = cCOp7B;
	table[0x7C] = cCOp7C;
	table[0x7D] = cCOp7Dm8nd;
	table[0x7E] = cCOp7Em8;
	table[0x7F] = cCOp7Fm8nd;
	table[0x80] = cCOp80;
	table[0x81] = cCOp81m8;
	table[0x82] = cCOp82;
	table[0x83] = cCOp83m8;
	table[0x84] = cCOp84x8;
	table[0x85] = cCOp85m8;
	table[0x86] = cCOp86x8;
	table[0x87] = cCOp87m8;
	table[0x88] = cCOp88x8;
	table[0x89] = cCOp89m8;
	table[0x8A] = cCOp8Am8;
	table[0x8B] = cCOp8B;
	table[0x8C] = cCOp8Cx8;
	table[0x8D] = cCOp8Dm8;
	table[0x8E] = cCOp8Ex8;
	table[0x8F] = cCOp8Fm8;
	table[0x90] = cCOp90;
	table[0x91] = cCOp91m8;
	table[0x92] = cCOp92m8;
	table[0x93] = cCOp93m8;
	table[0x94] = cCOp94x8;
	table[0x95] = cCOp95m8;
	table[0x96] = cCOp96x8;
	table[0x97] = cCOp97m8;
	table[0x98] = cCOp98m8;
	table[0x99] = cCOp99m8;
	table[0x9A] = cCOp9A;
	table[0x9B] = cCOp9Bx8;
	table[0x9C] = cCOp9Cm8;
	table[0x9D] = cCOp9Dm8;
	table[0x9E] = cCOp9Em8;
	table[0x9F] = cCOp9Fm8;
	table[0xA0] = cCOpA0x8;
	table[0xA1] = cCOpA1m8;
	table[0xA2] = cCOpA2x8;
	table[0xA3] = cCOpA3m8;
	table[0xA4] = cCOpA4x8;
	table[0xA5] = cCOpA5m8;
	table[0xA6] = cCOpA6x8;
	table[0xA7] = cCOpA7m8;
	table[0xA8] = cCOpA8x8;
	table[0xA9] = cCOpA9m8;
	table[0xAA] = cCOpAAx8;
	table[0xAB] = cCOpAB;
	table[0xAC] = cCOpACx8;
	table[0xAD] = cCOpADm8;
	table[0xAE] = cCOpAEx8;
	table[0xAF] = cCOpAFm8;
	table[0xB0] = cCOpB0;
	table[0xB1] = cCOpB1m8;
	table[0xB2] = cCOpB2m8;
	table[0xB3] = cCOpB3m8;
	table[0xB4] = cCOpB4x8;
	table[0xB5] = cCOpB5m8;
	table[0xB6] = cCOpB6x8;
	table[0xB7] = cCOpB7m8;
	table[0xB8] = cCOpB8;
	table[0xB9] = cCOpB9m8;
	table[0xBA] = cCOpBAx8;
	table[0xBB] = cCOpBBx8;
	table[0xBC] = cCOpBCx8;
	table[0xBD] = cCOpBDm8;
	table[0xBE] = cCOpBEx8;
	table[0xBF] = cCOpBFm8;
	table[0xC0] = cCOpC0x8;
	table[0xC1] = cCOpC1m8;
	table[0xC2] = cCOpC2;
	table[0xC3] = cCOpC3m8;
	table[0xC4] = cCOpC4x8;
	table[0xC5] = cCOpC5m8;
	table[0xC6] = cCOpC6m8;
	table[0xC7] = cCOpC7m8;
	table[0xC8] = cCOpC8x8;
	table[0xC9] = cCOpC9m8;
	table[0xCA] = cCOpCAx8;
	table[0xCB] = cCOpCB;
	table[0xCC] = cCOpCCx8;
	table[0xCD] = cCOpCDm8;
	table[0xCE] = cCOpCEm8;
	table[0xCF] = cCOpCFm8;
	table[0xD0] = cCOpD0;
	table[0xD1] = cCOpD1m8;
	table[0xD2] = cCOpD2m8;
	table[0xD3] = cCOpD3m8;
	table[0xD4] = cCOpD4;
	table[0xD5] = cCOpD5m8;
	table[0xD6] = cCOpD6m8;
	table[0xD7] = cCOpD7m8;
	table[0xD8] = cCOpD8;
	table[0xD9] = cCOpD9m8;
	table[0xDA] = cCOpDAx8;
	table[0xDB] = cCOpDB;
	table[0xDC] = cCOpDC;
	table[0xDD] = cCOpDDm8;
	table[0xDE] = cCOpDEm8;
	table[0xDF] = cCOpDFm8;
	table[0xE0] = cCOpE0x8;
	table[0xE1] = cCOpE1m8nd;
	table[0xE2] = cCOpE2;
	table[0xE3] = cCOpE3m8nd;
	table[0xE4] = cCOpE4x8;
	table[0xE5] = cCOpE5m8nd;
	table[0xE6] = cCOpE6m8;
	table[0xE7] = cCOpE7m8nd;
	table[0xE8] = cCOpE8x8;
	table[0xE9] = cCOpE9m8nd;
	table[0xEA] = cCOpEA;
	table[0xEB] = cCOpEB;
	table[0xEC] = cCOpECx8;
	table[0xED] = cCOpEDm8nd;
	table[0xEE] = cCOpEEm8;
	table[0xEF] = cCOpEFm8nd;
	table[0xF0] = cCOpF0;
	table[0xF1] = cCOpF1m8nd;
	table[0xF2] = cCOpF2m8nd;
	table[0xF3] = cCOpF3m8nd;
	table[0xF4] = cCOpF4;
	table[0xF5] = cCOpF5m8nd;
	table[0xF6] = cCOpF6m8;
	table[0xF7] = cCOpF7m8nd;
	table[0xF8] = cCOpF8;
	table[0xF9] = cCOpF9m8nd;
	table[0xFA] = cCOpFAx8;
	table[0xFB] = cCOpFB;
	table[0xFC] = cCOpFC;
	table[0xFD] = cCOpFDm8nd;
	table[0xFE] = cCOpFEm8;
	table[0xFF] = cCOpFFm8nd;
}


void inittablec(void)
{
	// set tablead  (NVMXDIZC) (  MXD   )
	for (u4 i = 0; i != lengthof(tableadc); ++i)
	{
		static eop** const tableXc[] =
		{
			tableAc, // ---
			tableEc, // --D
			tableCc, // -X-
			tableGc, // -XD
			tableBc, // M--
			tableFc, // M-D
			tableDc, // MX-
			tableHc  // MXD
		};
		tableadc[i] = tableXc[(i & 0x38) >> 3];
	}

	// Set CPU addresses
	// First, set all addresses to invalid
	// XXX This is probably pointless, the following settables() overwrite all entries
	for (eop** i = tableAc; i != endof(tableAc); ++i) *i = eopINVALID;
	for (eop** i = tableBc; i != endof(tableBc); ++i) *i = eopINVALID;
	for (eop** i = tableCc; i != endof(tableCc); ++i) *i = eopINVALID;
	for (eop** i = tableDc; i != endof(tableDc); ++i) *i = eopINVALID;
	for (eop** i = tableEc; i != endof(tableEc); ++i) *i = eopINVALID;
	for (eop** i = tableFc; i != endof(tableFc); ++i) *i = eopINVALID;
	for (eop** i = tableGc; i != endof(tableGc); ++i) *i = eopINVALID;
	for (eop** i = tableHc; i != endof(tableHc); ++i) *i = eopINVALID;

	settables(tableAc);
	settables(tableBc);
	settables(tableCc);
	settables(tableDc);
	settables(tableEc);
	settables(tableFc);
	settables(tableGc);
	settables(tableHc);

	// set proper functions
	asm volatile("call %P0" :: "X" (settablem16),  "D" (tableAc) : "memory"); // Table addresses (M:0,X:0,D:0)
	asm volatile("call %P0" :: "X" (settablex16),  "D" (tableAc) : "memory");

	asm volatile("call %P0" :: "X" (settablex16),  "D" (tableBc) : "memory"); // Table addresses (M:1,X:0,D:0)

	asm volatile("call %P0" :: "X" (settablem16),  "D" (tableCc) : "memory"); // Table addresses (M:0,X:1,D:0)

	asm volatile("call %P0" :: "X" (settablem16),  "D" (tableEc) : "memory"); // Table addresses (M:0,X:0,D:1)
	asm volatile("call %P0" :: "X" (settableDm16), "D" (tableEc) : "memory");
	asm volatile("call %P0" :: "X" (settablex16),  "D" (tableEc) : "memory");

	asm volatile("call %P0" :: "X" (settablex16),  "D" (tableFc) : "memory"); // Table addresses (M:1,X:0,D:1)
	asm volatile("call %P0" :: "X" (settableDm8),  "D" (tableFc) : "memory");

	asm volatile("call %P0" :: "X" (settablem16),  "D" (tableGc) : "memory"); // Table addresses (M:0,X:1,D:1)
	asm volatile("call %P0" :: "X" (settableDm16), "D" (tableGc) : "memory");

	asm volatile("call %P0" :: "X" (settableDm8),  "D" (tableHc) : "memory"); // Table addresses (M:1,X:1,D:1)
}
