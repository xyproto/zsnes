#include "c_tablec.h"
#include "../endmem.h"
#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof *(x))
#endif
#ifndef endof
#define endof(x) ((x) + lengthof(x))
#endif
#include "c_ops65816_dbg.h"
#include "table.h"

/* Sets the opcode tables
 * This function sets all the non-multiple entries */
static void settables(opfn** const table)
{
    // row 0
    table[0x00] = c_dbgCOp00;
    table[0x01] = c_dbgCOp01m8;
    table[0x02] = c_dbgCOp02;
    table[0x03] = c_dbgCOp03m8;
    table[0x04] = c_dbgCOp04m8;
    table[0x05] = c_dbgCOp05m8;
    table[0x06] = c_dbgCOp06m8;
    table[0x07] = c_dbgCOp07m8;
    table[0x08] = c_dbgCOp08;
    table[0x09] = c_dbgCOp09m8;
    table[0x0A] = c_dbgCOp0Am8;
    table[0x0B] = c_dbgCOp0B;
    table[0x0C] = c_dbgCOp0Cm8;
    table[0x0D] = c_dbgCOp0Dm8;
    table[0x0E] = c_dbgCOp0Em8;
    table[0x0F] = c_dbgCOp0Fm8;
    table[0x10] = c_dbgCOp10;
    table[0x11] = c_dbgCOp11m8;
    table[0x12] = c_dbgCOp12m8;
    table[0x13] = c_dbgCOp13m8;
    table[0x14] = c_dbgCOp14m8;
    table[0x15] = c_dbgCOp15m8;
    table[0x16] = c_dbgCOp16m8;
    table[0x17] = c_dbgCOp17m8;
    table[0x18] = c_dbgCOp18;
    table[0x19] = c_dbgCOp19m8;
    table[0x1A] = c_dbgCOp1Am8;
    table[0x1B] = c_dbgCOp1B;
    table[0x1C] = c_dbgCOp1Cm8;
    table[0x1D] = c_dbgCOp1Dm8;
    table[0x1E] = c_dbgCOp1Em8;
    table[0x1F] = c_dbgCOp1Fm8;
    table[0x20] = c_dbgCOp20;
    table[0x21] = c_dbgCOp21m8;
    table[0x22] = c_dbgCOp22;
    table[0x23] = c_dbgCOp23m8;
    table[0x24] = c_dbgCOp24m8;
    table[0x25] = c_dbgCOp25m8;
    table[0x26] = c_dbgCOp26m8;
    table[0x27] = c_dbgCOp27m8;
    table[0x28] = c_dbgCOp28;
    table[0x29] = c_dbgCOp29m8;
    table[0x2A] = c_dbgCOp2Am8;
    table[0x2B] = c_dbgCOp2B;
    table[0x2C] = c_dbgCOp2Cm8;
    table[0x2D] = c_dbgCOp2Dm8;
    table[0x2E] = c_dbgCOp2Em8;
    table[0x2F] = c_dbgCOp2Fm8;
    table[0x30] = c_dbgCOp30;
    table[0x31] = c_dbgCOp31m8;
    table[0x32] = c_dbgCOp32m8;
    table[0x33] = c_dbgCOp33m8;
    table[0x34] = c_dbgCOp34m8;
    table[0x35] = c_dbgCOp35m8;
    table[0x36] = c_dbgCOp36m8;
    table[0x37] = c_dbgCOp37m8;
    table[0x38] = c_dbgCOp38;
    table[0x39] = c_dbgCOp39m8;
    table[0x3A] = c_dbgCOp3Am8;
    table[0x3B] = c_dbgCOp3B;
    table[0x3C] = c_dbgCOp3Cm8;
    table[0x3D] = c_dbgCOp3Dm8;
    table[0x3E] = c_dbgCOp3Em8;
    table[0x3F] = c_dbgCOp3Fm8;
    table[0x40] = c_dbgCOp40;
    table[0x41] = c_dbgCOp41m8;
    table[0x42] = c_dbgCOp42;
    table[0x43] = c_dbgCOp43m8;
    table[0x44] = c_dbgCOp44;
    table[0x45] = c_dbgCOp45m8;
    table[0x46] = c_dbgCOp46m8;
    table[0x47] = c_dbgCOp47m8;
    table[0x48] = c_dbgCOp48m8;
    table[0x49] = c_dbgCOp49m8;
    table[0x4A] = c_dbgCOp4Am8;
    table[0x4B] = c_dbgCOp4B;
    table[0x4C] = c_dbgCOp4C;
    table[0x4D] = c_dbgCOp4Dm8;
    table[0x4E] = c_dbgCOp4Em8;
    table[0x4F] = c_dbgCOp4Fm8;
    table[0x50] = c_dbgCOp50;
    table[0x51] = c_dbgCOp51m8;
    table[0x52] = c_dbgCOp52m8;
    table[0x53] = c_dbgCOp53m8;
    table[0x54] = c_dbgCOp54;
    table[0x55] = c_dbgCOp55m8;
    table[0x56] = c_dbgCOp56m8;
    table[0x57] = c_dbgCOp57m8;
    table[0x58] = c_dbgCOp58;
    table[0x59] = c_dbgCOp59m8;
    table[0x5A] = c_dbgCOp5Ax8;
    table[0x5B] = c_dbgCOp5B;
    table[0x5C] = c_dbgCOp5C;
    table[0x5D] = c_dbgCOp5Dm8;
    table[0x5E] = c_dbgCOp5Em8;
    table[0x5F] = c_dbgCOp5Fm8;
    table[0x60] = c_dbgCOp60;
    table[0x61] = c_dbgCOp61m8nd;
    table[0x62] = c_dbgCOp62;
    table[0x63] = c_dbgCOp63m8nd;
    table[0x64] = c_dbgCOp64m8;
    table[0x65] = c_dbgCOp65m8nd;
    table[0x66] = c_dbgCOp66m8;
    table[0x67] = c_dbgCOp67m8nd;
    table[0x68] = c_dbgCOp68m8;
    table[0x69] = c_dbgCOp69m8nd;
    table[0x6A] = c_dbgCOp6Am8;
    table[0x6B] = c_dbgCOp6B;
    table[0x6C] = c_dbgCOp6C;
    table[0x6D] = c_dbgCOp6Dm8nd;
    table[0x6E] = c_dbgCOp6Em8;
    table[0x6F] = c_dbgCOp6Fm8nd;
    table[0x70] = c_dbgCOp70;
    table[0x71] = c_dbgCOp71m8nd;
    table[0x72] = c_dbgCOp72m8nd;
    table[0x73] = c_dbgCOp73m8nd;
    table[0x74] = c_dbgCOp74m8;
    table[0x75] = c_dbgCOp75m8nd;
    table[0x76] = c_dbgCOp76m8;
    table[0x77] = c_dbgCOp77m8nd;
    table[0x78] = c_dbgCOp78;
    table[0x79] = c_dbgCOp79m8nd;
    table[0x7A] = c_dbgCOp7Ax8;
    table[0x7B] = c_dbgCOp7B;
    table[0x7C] = c_dbgCOp7C;
    table[0x7D] = c_dbgCOp7Dm8nd;
    table[0x7E] = c_dbgCOp7Em8;
    table[0x7F] = c_dbgCOp7Fm8nd;
    table[0x80] = c_dbgCOp80;
    table[0x81] = c_dbgCOp81m8;
    table[0x82] = c_dbgCOp82;
    table[0x83] = c_dbgCOp83m8;
    table[0x84] = c_dbgCOp84x8;
    table[0x85] = c_dbgCOp85m8;
    table[0x86] = c_dbgCOp86x8;
    table[0x87] = c_dbgCOp87m8;
    table[0x88] = c_dbgCOp88x8;
    table[0x89] = c_dbgCOp89m8;
    table[0x8A] = c_dbgCOp8Am8;
    table[0x8B] = c_dbgCOp8B;
    table[0x8C] = c_dbgCOp8Cx8;
    table[0x8D] = c_dbgCOp8Dm8;
    table[0x8E] = c_dbgCOp8Ex8;
    table[0x8F] = c_dbgCOp8Fm8;
    table[0x90] = c_dbgCOp90;
    table[0x91] = c_dbgCOp91m8;
    table[0x92] = c_dbgCOp92m8;
    table[0x93] = c_dbgCOp93m8;
    table[0x94] = c_dbgCOp94x8;
    table[0x95] = c_dbgCOp95m8;
    table[0x96] = c_dbgCOp96x8;
    table[0x97] = c_dbgCOp97m8;
    table[0x98] = c_dbgCOp98m8;
    table[0x99] = c_dbgCOp99m8;
    table[0x9A] = c_dbgCOp9A;
    table[0x9B] = c_dbgCOp9Bx8;
    table[0x9C] = c_dbgCOp9Cm8;
    table[0x9D] = c_dbgCOp9Dm8;
    table[0x9E] = c_dbgCOp9Em8;
    table[0x9F] = c_dbgCOp9Fm8;
    table[0xA0] = c_dbgCOpA0x8;
    table[0xA1] = c_dbgCOpA1m8;
    table[0xA2] = c_dbgCOpA2x8;
    table[0xA3] = c_dbgCOpA3m8;
    table[0xA4] = c_dbgCOpA4x8;
    table[0xA5] = c_dbgCOpA5m8;
    table[0xA6] = c_dbgCOpA6x8;
    table[0xA7] = c_dbgCOpA7m8;
    table[0xA8] = c_dbgCOpA8x8;
    table[0xA9] = c_dbgCOpA9m8;
    table[0xAA] = c_dbgCOpAAx8;
    table[0xAB] = c_dbgCOpAB;
    table[0xAC] = c_dbgCOpACx8;
    table[0xAD] = c_dbgCOpADm8;
    table[0xAE] = c_dbgCOpAEx8;
    table[0xAF] = c_dbgCOpAFm8;
    table[0xB0] = c_dbgCOpB0;
    table[0xB1] = c_dbgCOpB1m8;
    table[0xB2] = c_dbgCOpB2m8;
    table[0xB3] = c_dbgCOpB3m8;
    table[0xB4] = c_dbgCOpB4x8;
    table[0xB5] = c_dbgCOpB5m8;
    table[0xB6] = c_dbgCOpB6x8;
    table[0xB7] = c_dbgCOpB7m8;
    table[0xB8] = c_dbgCOpB8;
    table[0xB9] = c_dbgCOpB9m8;
    table[0xBA] = c_dbgCOpBAx8;
    table[0xBB] = c_dbgCOpBBx8;
    table[0xBC] = c_dbgCOpBCx8;
    table[0xBD] = c_dbgCOpBDm8;
    table[0xBE] = c_dbgCOpBEx8;
    table[0xBF] = c_dbgCOpBFm8;
    table[0xC0] = c_dbgCOpC0x8;
    table[0xC1] = c_dbgCOpC1m8;
    table[0xC2] = c_dbgCOpC2;
    table[0xC3] = c_dbgCOpC3m8;
    table[0xC4] = c_dbgCOpC4x8;
    table[0xC5] = c_dbgCOpC5m8;
    table[0xC6] = c_dbgCOpC6m8;
    table[0xC7] = c_dbgCOpC7m8;
    table[0xC8] = c_dbgCOpC8x8;
    table[0xC9] = c_dbgCOpC9m8;
    table[0xCA] = c_dbgCOpCAx8;
    table[0xCB] = c_dbgCOpCB;
    table[0xCC] = c_dbgCOpCCx8;
    table[0xCD] = c_dbgCOpCDm8;
    table[0xCE] = c_dbgCOpCEm8;
    table[0xCF] = c_dbgCOpCFm8;
    table[0xD0] = c_dbgCOpD0;
    table[0xD1] = c_dbgCOpD1m8;
    table[0xD2] = c_dbgCOpD2m8;
    table[0xD3] = c_dbgCOpD3m8;
    table[0xD4] = c_dbgCOpD4;
    table[0xD5] = c_dbgCOpD5m8;
    table[0xD6] = c_dbgCOpD6m8;
    table[0xD7] = c_dbgCOpD7m8;
    table[0xD8] = c_dbgCOpD8;
    table[0xD9] = c_dbgCOpD9m8;
    table[0xDA] = c_dbgCOpDAx8;
    table[0xDB] = c_dbgCOpDB;
    table[0xDC] = c_dbgCOpDC;
    table[0xDD] = c_dbgCOpDDm8;
    table[0xDE] = c_dbgCOpDEm8;
    table[0xDF] = c_dbgCOpDFm8;
    table[0xE0] = c_dbgCOpE0x8;
    table[0xE1] = c_dbgCOpE1m8nd;
    table[0xE2] = c_dbgCOpE2;
    table[0xE3] = c_dbgCOpE3m8nd;
    table[0xE4] = c_dbgCOpE4x8;
    table[0xE5] = c_dbgCOpE5m8nd;
    table[0xE6] = c_dbgCOpE6m8;
    table[0xE7] = c_dbgCOpE7m8nd;
    table[0xE8] = c_dbgCOpE8x8;
    table[0xE9] = c_dbgCOpE9m8nd;
    table[0xEA] = c_dbgCOpEA;
    table[0xEB] = c_dbgCOpEB;
    table[0xEC] = c_dbgCOpECx8;
    table[0xED] = c_dbgCOpEDm8nd;
    table[0xEE] = c_dbgCOpEEm8;
    table[0xEF] = c_dbgCOpEFm8nd;
    table[0xF0] = c_dbgCOpF0;
    table[0xF1] = c_dbgCOpF1m8nd;
    table[0xF2] = c_dbgCOpF2m8nd;
    table[0xF3] = c_dbgCOpF3m8nd;
    table[0xF4] = c_dbgCOpF4;
    table[0xF5] = c_dbgCOpF5m8nd;
    table[0xF6] = c_dbgCOpF6m8;
    table[0xF7] = c_dbgCOpF7m8nd;
    table[0xF8] = c_dbgCOpF8;
    table[0xF9] = c_dbgCOpF9m8nd;
    table[0xFA] = c_dbgCOpFAx8;
    table[0xFB] = c_dbgCOpFB;
    table[0xFC] = c_dbgCOpFC;
    table[0xFD] = c_dbgCOpFDm8nd;
    table[0xFE] = c_dbgCOpFEm8;
    table[0xFF] = c_dbgCOpFFm8nd;
}

static void settablem16(opfn** const table)
{
    table[0x01] = c_dbgCOp01m16;
    table[0x03] = c_dbgCOp03m16;
    table[0x04] = c_dbgCOp04m16;
    table[0x05] = c_dbgCOp05m16;
    table[0x06] = c_dbgCOp06m16;
    table[0x07] = c_dbgCOp07m16;
    table[0x09] = c_dbgCOp09m16;
    table[0x0A] = c_dbgCOp0Am16;
    table[0x0C] = c_dbgCOp0Cm16;
    table[0x0D] = c_dbgCOp0Dm16;
    table[0x0E] = c_dbgCOp0Em16;
    table[0x0F] = c_dbgCOp0Fm16;
    table[0x11] = c_dbgCOp11m16;
    table[0x12] = c_dbgCOp12m16;
    table[0x13] = c_dbgCOp13m16;
    table[0x14] = c_dbgCOp14m16;
    table[0x15] = c_dbgCOp15m16;
    table[0x16] = c_dbgCOp16m16;
    table[0x17] = c_dbgCOp17m16;
    table[0x19] = c_dbgCOp19m16;
    table[0x1A] = c_dbgCOp1Am16;
    table[0x1C] = c_dbgCOp1Cm16;
    table[0x1D] = c_dbgCOp1Dm16;
    table[0x1E] = c_dbgCOp1Em16;
    table[0x1F] = c_dbgCOp1Fm16;
    table[0x21] = c_dbgCOp21m16;
    table[0x23] = c_dbgCOp23m16;
    table[0x24] = c_dbgCOp24m16;
    table[0x25] = c_dbgCOp25m16;
    table[0x26] = c_dbgCOp26m16;
    table[0x27] = c_dbgCOp27m16;
    table[0x29] = c_dbgCOp29m16;
    table[0x2A] = c_dbgCOp2Am16;
    table[0x2C] = c_dbgCOp2Cm16;
    table[0x2D] = c_dbgCOp2Dm16;
    table[0x2E] = c_dbgCOp2Em16;
    table[0x2F] = c_dbgCOp2Fm16;
    table[0x31] = c_dbgCOp31m16;
    table[0x32] = c_dbgCOp32m16;
    table[0x33] = c_dbgCOp33m16;
    table[0x34] = c_dbgCOp34m16;
    table[0x35] = c_dbgCOp35m16;
    table[0x36] = c_dbgCOp36m16;
    table[0x37] = c_dbgCOp37m16;
    table[0x39] = c_dbgCOp39m16;
    table[0x3A] = c_dbgCOp3Am16;
    table[0x3C] = c_dbgCOp3Cm16;
    table[0x3D] = c_dbgCOp3Dm16;
    table[0x3E] = c_dbgCOp3Em16;
    table[0x3F] = c_dbgCOp3Fm16;
    table[0x41] = c_dbgCOp41m16;
    table[0x43] = c_dbgCOp43m16;
    table[0x45] = c_dbgCOp45m16;
    table[0x46] = c_dbgCOp46m16;
    table[0x47] = c_dbgCOp47m16;
    table[0x48] = c_dbgCOp48m16;
    table[0x49] = c_dbgCOp49m16;
    table[0x4A] = c_dbgCOp4Am16;
    table[0x4D] = c_dbgCOp4Dm16;
    table[0x4E] = c_dbgCOp4Em16;
    table[0x4F] = c_dbgCOp4Fm16;
    table[0x51] = c_dbgCOp51m16;
    table[0x52] = c_dbgCOp52m16;
    table[0x53] = c_dbgCOp53m16;
    table[0x55] = c_dbgCOp55m16;
    table[0x56] = c_dbgCOp56m16;
    table[0x57] = c_dbgCOp57m16;
    table[0x59] = c_dbgCOp59m16;
    table[0x5D] = c_dbgCOp5Dm16;
    table[0x5E] = c_dbgCOp5Em16;
    table[0x5F] = c_dbgCOp5Fm16;
    table[0x61] = c_dbgCOp61m16nd;
    table[0x63] = c_dbgCOp63m16nd;
    table[0x64] = c_dbgCOp64m16;
    table[0x65] = c_dbgCOp65m16nd;
    table[0x66] = c_dbgCOp66m16;
    table[0x67] = c_dbgCOp67m16nd;
    table[0x68] = c_dbgCOp68m16;
    table[0x69] = c_dbgCOp69m16nd;
    table[0x6A] = c_dbgCOp6Am16;
    table[0x6D] = c_dbgCOp6Dm16nd;
    table[0x6E] = c_dbgCOp6Em16;
    table[0x6F] = c_dbgCOp6Fm16nd;
    table[0x71] = c_dbgCOp71m16nd;
    table[0x72] = c_dbgCOp72m16nd;
    table[0x73] = c_dbgCOp73m16nd;
    table[0x74] = c_dbgCOp74m16;
    table[0x75] = c_dbgCOp75m16nd;
    table[0x76] = c_dbgCOp76m16;
    table[0x77] = c_dbgCOp77m16nd;
    table[0x79] = c_dbgCOp79m16nd;
    table[0x7D] = c_dbgCOp7Dm16nd;
    table[0x7E] = c_dbgCOp7Em16;
    table[0x7F] = c_dbgCOp7Fm16nd;
    table[0x81] = c_dbgCOp81m16;
    table[0x83] = c_dbgCOp83m16;
    table[0x85] = c_dbgCOp85m16;
    table[0x87] = c_dbgCOp87m16;
    table[0x89] = c_dbgCOp89m16;
    table[0x8A] = c_dbgCOp8Am16;
    table[0x8D] = c_dbgCOp8Dm16;
    table[0x8F] = c_dbgCOp8Fm16;
    table[0x91] = c_dbgCOp91m16;
    table[0x92] = c_dbgCOp92m16;
    table[0x93] = c_dbgCOp93m16;
    table[0x95] = c_dbgCOp95m16;
    table[0x97] = c_dbgCOp97m16;
    table[0x98] = c_dbgCOp98m16;
    table[0x99] = c_dbgCOp99m16;
    table[0x9C] = c_dbgCOp9Cm16;
    table[0x9D] = c_dbgCOp9Dm16;
    table[0x9E] = c_dbgCOp9Em16;
    table[0x9F] = c_dbgCOp9Fm16;
    table[0xA1] = c_dbgCOpA1m16;
    table[0xA3] = c_dbgCOpA3m16;
    table[0xA5] = c_dbgCOpA5m16;
    table[0xA7] = c_dbgCOpA7m16;
    table[0xA9] = c_dbgCOpA9m16;
    table[0xAD] = c_dbgCOpADm16;
    table[0xAF] = c_dbgCOpAFm16;
    table[0xB1] = c_dbgCOpB1m16;
    table[0xB2] = c_dbgCOpB2m16;
    table[0xB3] = c_dbgCOpB3m16;
    table[0xB5] = c_dbgCOpB5m16;
    table[0xB7] = c_dbgCOpB7m16;
    table[0xB9] = c_dbgCOpB9m16;
    table[0xBD] = c_dbgCOpBDm16;
    table[0xBF] = c_dbgCOpBFm16;
    table[0xC1] = c_dbgCOpC1m16;
    table[0xC3] = c_dbgCOpC3m16;
    table[0xC5] = c_dbgCOpC5m16;
    table[0xC6] = c_dbgCOpC6m16;
    table[0xC7] = c_dbgCOpC7m16;
    table[0xC9] = c_dbgCOpC9m16;
    table[0xCD] = c_dbgCOpCDm16;
    table[0xCE] = c_dbgCOpCEm16;
    table[0xCF] = c_dbgCOpCFm16;
    table[0xD1] = c_dbgCOpD1m16;
    table[0xD2] = c_dbgCOpD2m16;
    table[0xD3] = c_dbgCOpD3m16;
    table[0xD5] = c_dbgCOpD5m16;
    table[0xD6] = c_dbgCOpD6m16;
    table[0xD7] = c_dbgCOpD7m16;
    table[0xD9] = c_dbgCOpD9m16;
    table[0xDD] = c_dbgCOpDDm16;
    table[0xDE] = c_dbgCOpDEm16;
    table[0xDF] = c_dbgCOpDFm16;
    table[0xE1] = c_dbgCOpE1m16nd;
    table[0xE3] = c_dbgCOpE3m16nd;
    table[0xE5] = c_dbgCOpE5m16nd;
    table[0xE6] = c_dbgCOpE6m16;
    table[0xE7] = c_dbgCOpE7m16nd;
    table[0xE9] = c_dbgCOpE9m16nd;
    table[0xED] = c_dbgCOpEDm16nd;
    table[0xEE] = c_dbgCOpEEm16;
    table[0xEF] = c_dbgCOpEFm16nd;
    table[0xF1] = c_dbgCOpF1m16nd;
    table[0xF2] = c_dbgCOpF2m16nd;
    table[0xF3] = c_dbgCOpF3m16nd;
    table[0xF5] = c_dbgCOpF5m16nd;
    table[0xF6] = c_dbgCOpF6m16;
    table[0xF7] = c_dbgCOpF7m16nd;
    table[0xF9] = c_dbgCOpF9m16nd;
    table[0xFD] = c_dbgCOpFDm16nd;
    table[0xFE] = c_dbgCOpFEm16;
    table[0xFF] = c_dbgCOpFFm16nd;
}

static void settablex16(opfn** const table)
{
    table[0x5A] = c_dbgCOp5Ax16;
    table[0x7A] = c_dbgCOp7Ax16;
    table[0x84] = c_dbgCOp84x16;
    table[0x86] = c_dbgCOp86x16;
    table[0x88] = c_dbgCOp88x16;
    table[0x8C] = c_dbgCOp8Cx16;
    table[0x8E] = c_dbgCOp8Ex16;
    table[0x94] = c_dbgCOp94x16;
    table[0x96] = c_dbgCOp96x16;
    table[0x9B] = c_dbgCOp9Bx16;
    table[0xA0] = c_dbgCOpA0x16;
    table[0xA2] = c_dbgCOpA2x16;
    table[0xA4] = c_dbgCOpA4x16;
    table[0xA6] = c_dbgCOpA6x16;
    table[0xA8] = c_dbgCOpA8x16;
    table[0xAA] = c_dbgCOpAAx16;
    table[0xAC] = c_dbgCOpACx16;
    table[0xAE] = c_dbgCOpAEx16;
    table[0xB4] = c_dbgCOpB4x16;
    table[0xB6] = c_dbgCOpB6x16;
    table[0xBA] = c_dbgCOpBAx16;
    table[0xBB] = c_dbgCOpBBx16;
    table[0xBC] = c_dbgCOpBCx16;
    table[0xBE] = c_dbgCOpBEx16;
    table[0xC0] = c_dbgCOpC0x16;
    table[0xC4] = c_dbgCOpC4x16;
    table[0xC8] = c_dbgCOpC8x16;
    table[0xCA] = c_dbgCOpCAx16;
    table[0xCC] = c_dbgCOpCCx16;
    table[0xDA] = c_dbgCOpDAx16;
    table[0xE0] = c_dbgCOpE0x16;
    table[0xE4] = c_dbgCOpE4x16;
    table[0xE8] = c_dbgCOpE8x16;
    table[0xEC] = c_dbgCOpECx16;
    table[0xFA] = c_dbgCOpFAx16;
}

static void settableDm8(opfn** const table)
{
    table[0x61] = c_dbgCOp61m8d;
    table[0x63] = c_dbgCOp63m8d;
    table[0x65] = c_dbgCOp65m8d;
    table[0x67] = c_dbgCOp67m8d;
    table[0x69] = c_dbgCOp69m8d;
    table[0x6D] = c_dbgCOp6Dm8d;
    table[0x6F] = c_dbgCOp6Fm8d;
    table[0x71] = c_dbgCOp71m8d;
    table[0x72] = c_dbgCOp72m8d;
    table[0x73] = c_dbgCOp73m8d;
    table[0x75] = c_dbgCOp75m8d;
    table[0x77] = c_dbgCOp77m8d;
    table[0x79] = c_dbgCOp79m8d;
    table[0x7D] = c_dbgCOp7Dm8d;
    table[0x7F] = c_dbgCOp7Fm8d;
    table[0xE1] = c_dbgCOpE1m8d;
    table[0xE3] = c_dbgCOpE3m8d;
    table[0xE5] = c_dbgCOpE5m8d;
    table[0xE7] = c_dbgCOpE7m8d;
    table[0xE9] = c_dbgCOpE9m8d;
    table[0xED] = c_dbgCOpEDm8d;
    table[0xEF] = c_dbgCOpEFm8d;
    table[0xF1] = c_dbgCOpF1m8d;
    table[0xF2] = c_dbgCOpF2m8d;
    table[0xF3] = c_dbgCOpF3m8d;
    table[0xF5] = c_dbgCOpF5m8d;
    table[0xF7] = c_dbgCOpF7m8d;
    table[0xF9] = c_dbgCOpF9m8d;
    table[0xFD] = c_dbgCOpFDm8d;
    table[0xFF] = c_dbgCOpFFm8d;
}

static void settableDm16(opfn** const table)
{
    table[0x61] = c_dbgCOp61m16d;
    table[0x63] = c_dbgCOp63m16d;
    table[0x65] = c_dbgCOp65m16d;
    table[0x67] = c_dbgCOp67m16d;
    table[0x69] = c_dbgCOp69m16d;
    table[0x6D] = c_dbgCOp6Dm16d;
    table[0x6F] = c_dbgCOp6Fm16d;
    table[0x71] = c_dbgCOp71m16d;
    table[0x72] = c_dbgCOp72m16d;
    table[0x73] = c_dbgCOp73m16d;
    table[0x75] = c_dbgCOp75m16d;
    table[0x77] = c_dbgCOp77m16d;
    table[0x79] = c_dbgCOp79m16d;
    table[0x7D] = c_dbgCOp7Dm16d;
    table[0x7F] = c_dbgCOp7Fm16d;
    table[0xE1] = c_dbgCOpE1m16d;
    table[0xE3] = c_dbgCOpE3m16d;
    table[0xE5] = c_dbgCOpE5m16d;
    table[0xE7] = c_dbgCOpE7m16d;
    table[0xE9] = c_dbgCOpE9m16d;
    table[0xED] = c_dbgCOpEDm16d;
    table[0xEF] = c_dbgCOpEFm16d;
    table[0xF1] = c_dbgCOpF1m16d;
    table[0xF2] = c_dbgCOpF2m16d;
    table[0xF3] = c_dbgCOpF3m16d;
    table[0xF5] = c_dbgCOpF5m16d;
    table[0xF7] = c_dbgCOpF7m16d;
    table[0xF9] = c_dbgCOpF9m16d;
    table[0xFD] = c_dbgCOpFDm16d;
    table[0xFF] = c_dbgCOpFFm16d;
}

void inittablec(void)
{
    // set tablead  (NVMXDIZC) (  MXD   )
    for (u4 i = 0; i != lengthof(tableadc); ++i) {
        static eop** const tableXc[] = {
            tableAc, // ---
            tableEc, // --D
            tableCc, // -X-
            tableGc, // -XD
            tableBc, // M--
            tableFc, // M-D
            tableDc, // MX-
            tableHc // MXD
        };
        tableadc[i] = tableXc[(i & 0x38) >> 3];
    }

    // Set CPU addresses
    // First, set all addresses to invalid
    // XXX This is probably pointless, the following settables() overwrite all entries
    for (opfn** i = tableAc; i != endof(tableAc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableBc; i != endof(tableBc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableCc; i != endof(tableCc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableDc; i != endof(tableDc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableEc; i != endof(tableEc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableFc; i != endof(tableFc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableGc; i != endof(tableGc); ++i)
        *i = eopINVALID;
    for (opfn** i = tableHc; i != endof(tableHc); ++i)
        *i = eopINVALID;

    settables(tableAc);
    settables(tableBc);
    settables(tableCc);
    settables(tableDc);
    settables(tableEc);
    settables(tableFc);
    settables(tableGc);
    settables(tableHc);

    // set proper functions
    settablem16(tableAc); // Table addresses (M:0,X:0,D:0)
    settablex16(tableAc);

    settablex16(tableBc); // Table addresses (M:1,X:0,D:0)

    settablem16(tableCc); // Table addresses (M:0,X:1,D:0)

    settablem16(tableEc); // Table addresses (M:0,X:0,D:1)
    settableDm16(tableEc);
    settablex16(tableEc);

    settablex16(tableFc); // Table addresses (M:1,X:0,D:1)
    settableDm8(tableFc);

    settablem16(tableGc); // Table addresses (M:0,X:1,D:1)
    settableDm16(tableGc);

    settableDm8(tableHc); // Table addresses (M:1,X:1,D:1)
}
