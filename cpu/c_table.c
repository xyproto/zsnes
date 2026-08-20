#include "c_table.h"
#include "c_ops65816.h"
#include "../endmem.h"
#ifndef lengthof
#define lengthof(x) (sizeof(x) / sizeof *(x))
#endif
#ifndef endof
#define endof(x) ((x) + lengthof(x))
#endif
#include "../video/newgfx.h"
#include "../video/newgfx16.h"
#include "table.h"

/* Sets the opcode tables
 * This function sets all the non-multiple entries */
static void settables(opfn** table)
{
    // row 0
    table[0x00] = c_COp00;
    table[0x01] = c_COp01m8;
    table[0x02] = c_COp02;
    table[0x03] = c_COp03m8;
    table[0x04] = c_COp04m8;
    table[0x05] = c_COp05m8;
    table[0x06] = c_COp06m8;
    table[0x07] = c_COp07m8;
    table[0x08] = c_COp08;
    table[0x09] = c_COp09m8;
    table[0x0A] = c_COp0Am8;
    table[0x0B] = c_COp0B;
    table[0x0C] = c_COp0Cm8;
    table[0x0D] = c_COp0Dm8;
    table[0x0E] = c_COp0Em8;
    table[0x0F] = c_COp0Fm8;
    table[0x10] = c_COp10;
    table[0x11] = c_COp11m8;
    table[0x12] = c_COp12m8;
    table[0x13] = c_COp13m8;
    table[0x14] = c_COp14m8;
    table[0x15] = c_COp15m8;
    table[0x16] = c_COp16m8;
    table[0x17] = c_COp17m8;
    table[0x18] = c_COp18;
    table[0x19] = c_COp19m8;
    table[0x1A] = c_COp1Am8;
    table[0x1B] = c_COp1B;
    table[0x1C] = c_COp1Cm8;
    table[0x1D] = c_COp1Dm8;
    table[0x1E] = c_COp1Em8;
    table[0x1F] = c_COp1Fm8;
    table[0x20] = c_COp20;
    table[0x21] = c_COp21m8;
    table[0x22] = c_COp22;
    table[0x23] = c_COp23m8;
    table[0x24] = c_COp24m8;
    table[0x25] = c_COp25m8;
    table[0x26] = c_COp26m8;
    table[0x27] = c_COp27m8;
    table[0x28] = c_COp28;
    table[0x29] = c_COp29m8;
    table[0x2A] = c_COp2Am8;
    table[0x2B] = c_COp2B;
    table[0x2C] = c_COp2Cm8;
    table[0x2D] = c_COp2Dm8;
    table[0x2E] = c_COp2Em8;
    table[0x2F] = c_COp2Fm8;
    table[0x30] = c_COp30;
    table[0x31] = c_COp31m8;
    table[0x32] = c_COp32m8;
    table[0x33] = c_COp33m8;
    table[0x34] = c_COp34m8;
    table[0x35] = c_COp35m8;
    table[0x36] = c_COp36m8;
    table[0x37] = c_COp37m8;
    table[0x38] = c_COp38;
    table[0x39] = c_COp39m8;
    table[0x3A] = c_COp3Am8;
    table[0x3B] = c_COp3B;
    table[0x3C] = c_COp3Cm8;
    table[0x3D] = c_COp3Dm8;
    table[0x3E] = c_COp3Em8;
    table[0x3F] = c_COp3Fm8;
    table[0x40] = c_COp40;
    table[0x41] = c_COp41m8;
    table[0x42] = c_COp42;
    table[0x43] = c_COp43m8;
    table[0x44] = c_COp44;
    table[0x45] = c_COp45m8;
    table[0x46] = c_COp46m8;
    table[0x47] = c_COp47m8;
    table[0x48] = c_COp48m8;
    table[0x49] = c_COp49m8;
    table[0x4A] = c_COp4Am8;
    table[0x4B] = c_COp4B;
    table[0x4C] = c_COp4C;
    table[0x4D] = c_COp4Dm8;
    table[0x4E] = c_COp4Em8;
    table[0x4F] = c_COp4Fm8;
    table[0x50] = c_COp50;
    table[0x51] = c_COp51m8;
    table[0x52] = c_COp52m8;
    table[0x53] = c_COp53m8;
    table[0x54] = c_COp54;
    table[0x55] = c_COp55m8;
    table[0x56] = c_COp56m8;
    table[0x57] = c_COp57m8;
    table[0x58] = c_COp58;
    table[0x59] = c_COp59m8;
    table[0x5A] = c_COp5Ax8;
    table[0x5B] = c_COp5B;
    table[0x5C] = c_COp5C;
    table[0x5D] = c_COp5Dm8;
    table[0x5E] = c_COp5Em8;
    table[0x5F] = c_COp5Fm8;
    table[0x60] = c_COp60;
    table[0x61] = c_COp61m8nd;
    table[0x62] = c_COp62;
    table[0x63] = c_COp63m8nd;
    table[0x64] = c_COp64m8;
    table[0x65] = c_COp65m8nd;
    table[0x66] = c_COp66m8;
    table[0x67] = c_COp67m8nd;
    table[0x68] = c_COp68m8;
    table[0x69] = c_COp69m8nd;
    table[0x6A] = c_COp6Am8;
    table[0x6B] = c_COp6B;
    table[0x6C] = c_COp6C;
    table[0x6D] = c_COp6Dm8nd;
    table[0x6E] = c_COp6Em8;
    table[0x6F] = c_COp6Fm8nd;
    table[0x70] = c_COp70;
    table[0x71] = c_COp71m8nd;
    table[0x72] = c_COp72m8nd;
    table[0x73] = c_COp73m8nd;
    table[0x74] = c_COp74m8;
    table[0x75] = c_COp75m8nd;
    table[0x76] = c_COp76m8;
    table[0x77] = c_COp77m8nd;
    table[0x78] = c_COp78;
    table[0x79] = c_COp79m8nd;
    table[0x7A] = c_COp7Ax8;
    table[0x7B] = c_COp7B;
    table[0x7C] = c_COp7C;
    table[0x7D] = c_COp7Dm8nd;
    table[0x7E] = c_COp7Em8;
    table[0x7F] = c_COp7Fm8nd;
    table[0x80] = c_COp80;
    table[0x81] = c_COp81m8;
    table[0x82] = c_COp82;
    table[0x83] = c_COp83m8;
    table[0x84] = c_COp84x8;
    table[0x85] = c_COp85m8;
    table[0x86] = c_COp86x8;
    table[0x87] = c_COp87m8;
    table[0x88] = c_COp88x8;
    table[0x89] = c_COp89m8;
    table[0x8A] = c_COp8Am8;
    table[0x8B] = c_COp8B;
    table[0x8C] = c_COp8Cx8;
    table[0x8D] = c_COp8Dm8;
    table[0x8E] = c_COp8Ex8;
    table[0x8F] = c_COp8Fm8;
    table[0x90] = c_COp90;
    table[0x91] = c_COp91m8;
    table[0x92] = c_COp92m8;
    table[0x93] = c_COp93m8;
    table[0x94] = c_COp94x8;
    table[0x95] = c_COp95m8;
    table[0x96] = c_COp96x8;
    table[0x97] = c_COp97m8;
    table[0x98] = c_COp98m8;
    table[0x99] = c_COp99m8;
    table[0x9A] = c_COp9A;
    table[0x9B] = c_COp9Bx8;
    table[0x9C] = c_COp9Cm8;
    table[0x9D] = c_COp9Dm8;
    table[0x9E] = c_COp9Em8;
    table[0x9F] = c_COp9Fm8;
    table[0xA0] = c_COpA0x8;
    table[0xA1] = c_COpA1m8;
    table[0xA2] = c_COpA2x8;
    table[0xA3] = c_COpA3m8;
    table[0xA4] = c_COpA4x8;
    table[0xA5] = c_COpA5m8;
    table[0xA6] = c_COpA6x8;
    table[0xA7] = c_COpA7m8;
    table[0xA8] = c_COpA8x8;
    table[0xA9] = c_COpA9m8;
    table[0xAA] = c_COpAAx8;
    table[0xAB] = c_COpAB;
    table[0xAC] = c_COpACx8;
    table[0xAD] = c_COpADm8;
    table[0xAE] = c_COpAEx8;
    table[0xAF] = c_COpAFm8;
    table[0xB0] = c_COpB0;
    table[0xB1] = c_COpB1m8;
    table[0xB2] = c_COpB2m8;
    table[0xB3] = c_COpB3m8;
    table[0xB4] = c_COpB4x8;
    table[0xB5] = c_COpB5m8;
    table[0xB6] = c_COpB6x8;
    table[0xB7] = c_COpB7m8;
    table[0xB8] = c_COpB8;
    table[0xB9] = c_COpB9m8;
    table[0xBA] = c_COpBAx8;
    table[0xBB] = c_COpBBx8;
    table[0xBC] = c_COpBCx8;
    table[0xBD] = c_COpBDm8;
    table[0xBE] = c_COpBEx8;
    table[0xBF] = c_COpBFm8;
    table[0xC0] = c_COpC0x8;
    table[0xC1] = c_COpC1m8;
    table[0xC2] = c_COpC2;
    table[0xC3] = c_COpC3m8;
    table[0xC4] = c_COpC4x8;
    table[0xC5] = c_COpC5m8;
    table[0xC6] = c_COpC6m8;
    table[0xC7] = c_COpC7m8;
    table[0xC8] = c_COpC8x8;
    table[0xC9] = c_COpC9m8;
    table[0xCA] = c_COpCAx8;
    table[0xCB] = c_COpCB;
    table[0xCC] = c_COpCCx8;
    table[0xCD] = c_COpCDm8;
    table[0xCE] = c_COpCEm8;
    table[0xCF] = c_COpCFm8;
    table[0xD0] = c_COpD0;
    table[0xD1] = c_COpD1m8;
    table[0xD2] = c_COpD2m8;
    table[0xD3] = c_COpD3m8;
    table[0xD4] = c_COpD4;
    table[0xD5] = c_COpD5m8;
    table[0xD6] = c_COpD6m8;
    table[0xD7] = c_COpD7m8;
    table[0xD8] = c_COpD8;
    table[0xD9] = c_COpD9m8;
    table[0xDA] = c_COpDAx8;
    table[0xDB] = c_COpDB;
    table[0xDC] = c_COpDC;
    table[0xDD] = c_COpDDm8;
    table[0xDE] = c_COpDEm8;
    table[0xDF] = c_COpDFm8;
    table[0xE0] = c_COpE0x8;
    table[0xE1] = c_COpE1m8nd;
    table[0xE2] = c_COpE2;
    table[0xE3] = c_COpE3m8nd;
    table[0xE4] = c_COpE4x8;
    table[0xE5] = c_COpE5m8nd;
    table[0xE6] = c_COpE6m8;
    table[0xE7] = c_COpE7m8nd;
    table[0xE8] = c_COpE8x8;
    table[0xE9] = c_COpE9m8nd;
    table[0xEA] = c_COpEA;
    table[0xEB] = c_COpEB;
    table[0xEC] = c_COpECx8;
    table[0xED] = c_COpEDm8nd;
    table[0xEE] = c_COpEEm8;
    table[0xEF] = c_COpEFm8nd;
    table[0xF0] = c_COpF0;
    table[0xF1] = c_COpF1m8nd;
    table[0xF2] = c_COpF2m8nd;
    table[0xF3] = c_COpF3m8nd;
    table[0xF4] = c_COpF4;
    table[0xF5] = c_COpF5m8nd;
    table[0xF6] = c_COpF6m8;
    table[0xF7] = c_COpF7m8nd;
    table[0xF8] = c_COpF8;
    table[0xF9] = c_COpF9m8nd;
    table[0xFA] = c_COpFAx8;
    table[0xFB] = c_COpFB;
    table[0xFC] = c_COpFC;
    table[0xFD] = c_COpFDm8nd;
    table[0xFE] = c_COpFEm8;
    table[0xFF] = c_COpFFm8nd;
}

static void settablem16(opfn** table)
{
    table[0x01] = c_COp01m16;
    table[0x03] = c_COp03m16;
    table[0x04] = c_COp04m16;
    table[0x05] = c_COp05m16;
    table[0x06] = c_COp06m16;
    table[0x07] = c_COp07m16;
    table[0x09] = c_COp09m16;
    table[0x0A] = c_COp0Am16;
    table[0x0C] = c_COp0Cm16;
    table[0x0D] = c_COp0Dm16;
    table[0x0E] = c_COp0Em16;
    table[0x0F] = c_COp0Fm16;
    table[0x11] = c_COp11m16;
    table[0x12] = c_COp12m16;
    table[0x13] = c_COp13m16;
    table[0x14] = c_COp14m16;
    table[0x15] = c_COp15m16;
    table[0x16] = c_COp16m16;
    table[0x17] = c_COp17m16;
    table[0x19] = c_COp19m16;
    table[0x1A] = c_COp1Am16;
    table[0x1C] = c_COp1Cm16;
    table[0x1D] = c_COp1Dm16;
    table[0x1E] = c_COp1Em16;
    table[0x1F] = c_COp1Fm16;
    table[0x21] = c_COp21m16;
    table[0x23] = c_COp23m16;
    table[0x24] = c_COp24m16;
    table[0x25] = c_COp25m16;
    table[0x26] = c_COp26m16;
    table[0x27] = c_COp27m16;
    table[0x29] = c_COp29m16;
    table[0x2A] = c_COp2Am16;
    table[0x2C] = c_COp2Cm16;
    table[0x2D] = c_COp2Dm16;
    table[0x2E] = c_COp2Em16;
    table[0x2F] = c_COp2Fm16;
    table[0x31] = c_COp31m16;
    table[0x32] = c_COp32m16;
    table[0x33] = c_COp33m16;
    table[0x34] = c_COp34m16;
    table[0x35] = c_COp35m16;
    table[0x36] = c_COp36m16;
    table[0x37] = c_COp37m16;
    table[0x39] = c_COp39m16;
    table[0x3A] = c_COp3Am16;
    table[0x3C] = c_COp3Cm16;
    table[0x3D] = c_COp3Dm16;
    table[0x3E] = c_COp3Em16;
    table[0x3F] = c_COp3Fm16;
    table[0x41] = c_COp41m16;
    table[0x43] = c_COp43m16;
    table[0x45] = c_COp45m16;
    table[0x46] = c_COp46m16;
    table[0x47] = c_COp47m16;
    table[0x48] = c_COp48m16;
    table[0x49] = c_COp49m16;
    table[0x4A] = c_COp4Am16;
    table[0x4D] = c_COp4Dm16;
    table[0x4E] = c_COp4Em16;
    table[0x4F] = c_COp4Fm16;
    table[0x51] = c_COp51m16;
    table[0x52] = c_COp52m16;
    table[0x53] = c_COp53m16;
    table[0x55] = c_COp55m16;
    table[0x56] = c_COp56m16;
    table[0x57] = c_COp57m16;
    table[0x59] = c_COp59m16;
    table[0x5D] = c_COp5Dm16;
    table[0x5E] = c_COp5Em16;
    table[0x5F] = c_COp5Fm16;
    table[0x61] = c_COp61m16nd;
    table[0x63] = c_COp63m16nd;
    table[0x64] = c_COp64m16;
    table[0x65] = c_COp65m16nd;
    table[0x66] = c_COp66m16;
    table[0x67] = c_COp67m16nd;
    table[0x68] = c_COp68m16;
    table[0x69] = c_COp69m16nd;
    table[0x6A] = c_COp6Am16;
    table[0x6D] = c_COp6Dm16nd;
    table[0x6E] = c_COp6Em16;
    table[0x6F] = c_COp6Fm16nd;
    table[0x71] = c_COp71m16nd;
    table[0x72] = c_COp72m16nd;
    table[0x73] = c_COp73m16nd;
    table[0x74] = c_COp74m16;
    table[0x75] = c_COp75m16nd;
    table[0x76] = c_COp76m16;
    table[0x77] = c_COp77m16nd;
    table[0x79] = c_COp79m16nd;
    table[0x7D] = c_COp7Dm16nd;
    table[0x7E] = c_COp7Em16;
    table[0x7F] = c_COp7Fm16nd;
    table[0x81] = c_COp81m16;
    table[0x83] = c_COp83m16;
    table[0x85] = c_COp85m16;
    table[0x87] = c_COp87m16;
    table[0x89] = c_COp89m16;
    table[0x8A] = c_COp8Am16;
    table[0x8D] = c_COp8Dm16;
    table[0x8F] = c_COp8Fm16;
    table[0x91] = c_COp91m16;
    table[0x92] = c_COp92m16;
    table[0x93] = c_COp93m16;
    table[0x95] = c_COp95m16;
    table[0x97] = c_COp97m16;
    table[0x98] = c_COp98m16;
    table[0x99] = c_COp99m16;
    table[0x9C] = c_COp9Cm16;
    table[0x9D] = c_COp9Dm16;
    table[0x9E] = c_COp9Em16;
    table[0x9F] = c_COp9Fm16;
    table[0xA1] = c_COpA1m16;
    table[0xA3] = c_COpA3m16;
    table[0xA5] = c_COpA5m16;
    table[0xA7] = c_COpA7m16;
    table[0xA9] = c_COpA9m16;
    table[0xAD] = c_COpADm16;
    table[0xAF] = c_COpAFm16;
    table[0xB1] = c_COpB1m16;
    table[0xB2] = c_COpB2m16;
    table[0xB3] = c_COpB3m16;
    table[0xB5] = c_COpB5m16;
    table[0xB7] = c_COpB7m16;
    table[0xB9] = c_COpB9m16;
    table[0xBD] = c_COpBDm16;
    table[0xBF] = c_COpBFm16;
    table[0xC1] = c_COpC1m16;
    table[0xC3] = c_COpC3m16;
    table[0xC5] = c_COpC5m16;
    table[0xC6] = c_COpC6m16;
    table[0xC7] = c_COpC7m16;
    table[0xC9] = c_COpC9m16;
    table[0xCD] = c_COpCDm16;
    table[0xCE] = c_COpCEm16;
    table[0xCF] = c_COpCFm16;
    table[0xD1] = c_COpD1m16;
    table[0xD2] = c_COpD2m16;
    table[0xD3] = c_COpD3m16;
    table[0xD5] = c_COpD5m16;
    table[0xD6] = c_COpD6m16;
    table[0xD7] = c_COpD7m16;
    table[0xD9] = c_COpD9m16;
    table[0xDD] = c_COpDDm16;
    table[0xDE] = c_COpDEm16;
    table[0xDF] = c_COpDFm16;
    table[0xE1] = c_COpE1m16nd;
    table[0xE3] = c_COpE3m16nd;
    table[0xE5] = c_COpE5m16nd;
    table[0xE6] = c_COpE6m16;
    table[0xE7] = c_COpE7m16nd;
    table[0xE9] = c_COpE9m16nd;
    table[0xED] = c_COpEDm16nd;
    table[0xEE] = c_COpEEm16;
    table[0xEF] = c_COpEFm16nd;
    table[0xF1] = c_COpF1m16nd;
    table[0xF2] = c_COpF2m16nd;
    table[0xF3] = c_COpF3m16nd;
    table[0xF5] = c_COpF5m16nd;
    table[0xF6] = c_COpF6m16;
    table[0xF7] = c_COpF7m16nd;
    table[0xF9] = c_COpF9m16nd;
    table[0xFD] = c_COpFDm16nd;
    table[0xFE] = c_COpFEm16;
    table[0xFF] = c_COpFFm16nd;
}

static void settablex16(opfn** table)
{
    table[0x5A] = c_COp5Ax16;
    table[0x7A] = c_COp7Ax16;
    table[0x84] = c_COp84x16;
    table[0x86] = c_COp86x16;
    table[0x88] = c_COp88x16;
    table[0x8C] = c_COp8Cx16;
    table[0x8E] = c_COp8Ex16;
    table[0x94] = c_COp94x16;
    table[0x96] = c_COp96x16;
    table[0x9B] = c_COp9Bx16;
    table[0xA0] = c_COpA0x16;
    table[0xA2] = c_COpA2x16;
    table[0xA4] = c_COpA4x16;
    table[0xA6] = c_COpA6x16;
    table[0xA8] = c_COpA8x16;
    table[0xAA] = c_COpAAx16;
    table[0xAC] = c_COpACx16;
    table[0xAE] = c_COpAEx16;
    table[0xB4] = c_COpB4x16;
    table[0xB6] = c_COpB6x16;
    table[0xBA] = c_COpBAx16;
    table[0xBB] = c_COpBBx16;
    table[0xBC] = c_COpBCx16;
    table[0xBE] = c_COpBEx16;
    table[0xC0] = c_COpC0x16;
    table[0xC4] = c_COpC4x16;
    table[0xC8] = c_COpC8x16;
    table[0xCA] = c_COpCAx16;
    table[0xCC] = c_COpCCx16;
    table[0xDA] = c_COpDAx16;
    table[0xE0] = c_COpE0x16;
    table[0xE4] = c_COpE4x16;
    table[0xE8] = c_COpE8x16;
    table[0xEC] = c_COpECx16;
    table[0xFA] = c_COpFAx16;
}

static void settableDm8(opfn** table)
{
    table[0x61] = c_COp61m8d;
    table[0x63] = c_COp63m8d;
    table[0x65] = c_COp65m8d;
    table[0x67] = c_COp67m8d;
    table[0x69] = c_COp69m8d;
    table[0x6D] = c_COp6Dm8d;
    table[0x6F] = c_COp6Fm8d;
    table[0x71] = c_COp71m8d;
    table[0x72] = c_COp72m8d;
    table[0x73] = c_COp73m8d;
    table[0x75] = c_COp75m8d;
    table[0x77] = c_COp77m8d;
    table[0x79] = c_COp79m8d;
    table[0x7D] = c_COp7Dm8d;
    table[0x7F] = c_COp7Fm8d;
    table[0xE1] = c_COpE1m8d;
    table[0xE3] = c_COpE3m8d;
    table[0xE5] = c_COpE5m8d;
    table[0xE7] = c_COpE7m8d;
    table[0xE9] = c_COpE9m8d;
    table[0xED] = c_COpEDm8d;
    table[0xEF] = c_COpEFm8d;
    table[0xF1] = c_COpF1m8d;
    table[0xF2] = c_COpF2m8d;
    table[0xF3] = c_COpF3m8d;
    table[0xF5] = c_COpF5m8d;
    table[0xF7] = c_COpF7m8d;
    table[0xF9] = c_COpF9m8d;
    table[0xFD] = c_COpFDm8d;
    table[0xFF] = c_COpFFm8d;
}

static void settableDm16(opfn** table)
{
    table[0x61] = c_COp61m16d;
    table[0x63] = c_COp63m16d;
    table[0x65] = c_COp65m16d;
    table[0x67] = c_COp67m16d;
    table[0x69] = c_COp69m16d;
    table[0x6D] = c_COp6Dm16d;
    table[0x6F] = c_COp6Fm16d;
    table[0x71] = c_COp71m16d;
    table[0x72] = c_COp72m16d;
    table[0x73] = c_COp73m16d;
    table[0x75] = c_COp75m16d;
    table[0x77] = c_COp77m16d;
    table[0x79] = c_COp79m16d;
    table[0x7D] = c_COp7Dm16d;
    table[0x7F] = c_COp7Fm16d;
    table[0xE1] = c_COpE1m16d;
    table[0xE3] = c_COpE3m16d;
    table[0xE5] = c_COpE5m16d;
    table[0xE7] = c_COpE7m16d;
    table[0xE9] = c_COpE9m16d;
    table[0xED] = c_COpEDm16d;
    table[0xEF] = c_COpEFm16d;
    table[0xF1] = c_COpF1m16d;
    table[0xF2] = c_COpF2m16d;
    table[0xF3] = c_COpF3m16d;
    table[0xF5] = c_COpF5m16d;
    table[0xF7] = c_COpF7m16d;
    table[0xF9] = c_COpF9m16d;
    table[0xFD] = c_COpFDm16d;
    table[0xFF] = c_COpFFm16d;
}

// Generate OpCode Table
void inittable(void)
{
    // set ngpalcon4b
    for (u4 i = 0; i != 32; ++i) {
        u1 const bl = (u1)i >> 2 << 4;
        ngpalcon4b[i] = bl * 0x01010101U;
    }

    // set ngpalcon2b
    for (u4 i = 0; i != 32; ++i) {
        u1 const bl = (u1)i >> 2 << 2;
        ngpalcon2b[i] = bl * 0x01010101U;
    }

    // set tablead  (NVMXDIZC) (  MXD   )
    for (u4 i = 0; i != lengthof(tablead); ++i) {
        static eop** const tableX[] = {
            tableA, // ---
            tableE, // --D
            tableC, // -X-
            tableG, // -XD
            tableB, // M--
            tableF, // M-D
            tableD, // MX-
            tableH // MXD
        };
        tablead[i] = tableX[(i & 0x38) >> 3];
    }

    // Set CPU addresses
    // First, set all addresses to invalid
    // XXX This is probably pointless, the following settables() overwrite all entries
    for (opfn** i = tableA; i != endof(tableA); ++i)
        *i = eopINVALID;
    for (opfn** i = tableB; i != endof(tableB); ++i)
        *i = eopINVALID;
    for (opfn** i = tableC; i != endof(tableC); ++i)
        *i = eopINVALID;
    for (opfn** i = tableD; i != endof(tableD); ++i)
        *i = eopINVALID;
    for (opfn** i = tableE; i != endof(tableE); ++i)
        *i = eopINVALID;
    for (opfn** i = tableF; i != endof(tableF); ++i)
        *i = eopINVALID;
    for (opfn** i = tableG; i != endof(tableG); ++i)
        *i = eopINVALID;
    for (opfn** i = tableH; i != endof(tableH); ++i)
        *i = eopINVALID;

    // XXX All initialisations below seem to have no effect on the emulator
    settables(tableA);
    settables(tableB);
    settables(tableC);
    settables(tableD);
    settables(tableE);
    settables(tableF);
    settables(tableG);
    settables(tableH);

    // set proper functions
    settablem16(tableA); // Table addresses (M:0,X:0,D:0)
    settablex16(tableA);

    settablex16(tableB); // Table addresses (M:1,X:0,D:0)

    settablem16(tableC); // Table addresses (M:0,X:1,D:0)

    settablem16(tableE); // Table addresses (M:0,X:0,D:1)
    settableDm16(tableE);
    settablex16(tableE);

    settablex16(tableF); // Table addresses (M:1,X:0,D:1)
    settableDm8(tableF);

    settablem16(tableG); // Table addresses (M:0,X:1,D:1)
    settableDm16(tableG);

    settableDm8(tableH); // Table addresses (M:1,X:1,D:1)
}

/* Cycles per 65816 opcode, from cpu/table.asm. */
u1 cpucycle[256] = {
     8,  6,  8,  4,  5,  3,  5,  6,  3,  2,  2,  4,  6,  4,  6,  5,
     2,  5,  5,  7,  5,  4,  6,  6,  2,  4,  2,  2,  6,  4,  7,  5,
     6,  6,  8,  4,  3,  3,  5,  6,  4,  2,  2,  5,  4,  4,  6,  5,
     2,  5,  5,  7,  4,  4,  6,  6,  2,  4,  2,  2,  4,  4,  7,  5,
     7,  6,  2,  4,  7,  3,  5,  6,  3,  2,  2,  3,  3,  4,  6,  5,
     2,  5,  5,  7,  7,  4,  6,  6,  2,  4,  3,  2,  4,  4,  7,  5,
     6,  6,  6,  4,  3,  3,  5,  6,  4,  2,  2,  6,  5,  4,  6,  5,
     2,  5,  5,  7,  4,  4,  6,  6,  2,  4,  4,  2,  6,  4,  7,  5,
     2,  6,  3,  4,  3,  3,  3,  6,  2,  2,  2,  3,  4,  4,  4,  5,
     2,  6,  5,  7,  4,  4,  4,  6,  2,  5,  2,  2,  4,  5,  5,  5,
     2,  6,  2,  4,  3,  3,  3,  6,  2,  2,  2,  4,  4,  4,  4,  5,
     2,  5,  5,  7,  4,  4,  4,  6,  2,  4,  2,  2,  4,  4,  4,  5,
     2,  6,  3,  4,  3,  3,  5,  6,  2,  2,  2,  3,  4,  4,  4,  5,
     2,  5,  5,  7,  6,  4,  6,  6,  2,  4,  3,  3,  6,  4,  7,  5,
     2,  6,  3,  4,  3,  3,  5,  6,  2,  2,  2,  3,  4,  4,  6,  5,
     2,  5,  5,  7,  5,  4,  6,  6,  2,  4,  4,  2,  6,  4,  7,  5,
};

/* Invalid-opcode handler: the assembly was a bare `ret`, so this does nothing
   but take the register block the dispatcher hands every opcode body. */
void eopINVALID(zreg* const r)
{
    (void)r;
}
