#include "c_table.h"
#include "../cpu/e65816.h"
#include "../endmem.h"
#include "../macros.h"
#include "../video/newgfx.h"
#include "../video/newgfx16.h"
#include "table.h"

/* Sets the opcode tables
 * This function sets all the non-multiple entries */
static void settables(eop** table)
{
    // row 0
    table[0x00] = COp00;
    table[0x01] = COp01m8;
    table[0x02] = COp02;
    table[0x03] = COp03m8;
    table[0x04] = COp04m8;
    table[0x05] = COp05m8;
    table[0x06] = COp06m8;
    table[0x07] = COp07m8;
    table[0x08] = COp08;
    table[0x09] = COp09m8;
    table[0x0A] = COp0Am8;
    table[0x0B] = COp0B;
    table[0x0C] = COp0Cm8;
    table[0x0D] = COp0Dm8;
    table[0x0E] = COp0Em8;
    table[0x0F] = COp0Fm8;
    table[0x10] = COp10;
    table[0x11] = COp11m8;
    table[0x12] = COp12m8;
    table[0x13] = COp13m8;
    table[0x14] = COp14m8;
    table[0x15] = COp15m8;
    table[0x16] = COp16m8;
    table[0x17] = COp17m8;
    table[0x18] = COp18;
    table[0x19] = COp19m8;
    table[0x1A] = COp1Am8;
    table[0x1B] = COp1B;
    table[0x1C] = COp1Cm8;
    table[0x1D] = COp1Dm8;
    table[0x1E] = COp1Em8;
    table[0x1F] = COp1Fm8;
    table[0x20] = COp20;
    table[0x21] = COp21m8;
    table[0x22] = COp22;
    table[0x23] = COp23m8;
    table[0x24] = COp24m8;
    table[0x25] = COp25m8;
    table[0x26] = COp26m8;
    table[0x27] = COp27m8;
    table[0x28] = COp28;
    table[0x29] = COp29m8;
    table[0x2A] = COp2Am8;
    table[0x2B] = COp2B;
    table[0x2C] = COp2Cm8;
    table[0x2D] = COp2Dm8;
    table[0x2E] = COp2Em8;
    table[0x2F] = COp2Fm8;
    table[0x30] = COp30;
    table[0x31] = COp31m8;
    table[0x32] = COp32m8;
    table[0x33] = COp33m8;
    table[0x34] = COp34m8;
    table[0x35] = COp35m8;
    table[0x36] = COp36m8;
    table[0x37] = COp37m8;
    table[0x38] = COp38;
    table[0x39] = COp39m8;
    table[0x3A] = COp3Am8;
    table[0x3B] = COp3B;
    table[0x3C] = COp3Cm8;
    table[0x3D] = COp3Dm8;
    table[0x3E] = COp3Em8;
    table[0x3F] = COp3Fm8;
    table[0x40] = COp40;
    table[0x41] = COp41m8;
    table[0x42] = COp42;
    table[0x43] = COp43m8;
    table[0x44] = COp44;
    table[0x45] = COp45m8;
    table[0x46] = COp46m8;
    table[0x47] = COp47m8;
    table[0x48] = COp48m8;
    table[0x49] = COp49m8;
    table[0x4A] = COp4Am8;
    table[0x4B] = COp4B;
    table[0x4C] = COp4C;
    table[0x4D] = COp4Dm8;
    table[0x4E] = COp4Em8;
    table[0x4F] = COp4Fm8;
    table[0x50] = COp50;
    table[0x51] = COp51m8;
    table[0x52] = COp52m8;
    table[0x53] = COp53m8;
    table[0x54] = COp54;
    table[0x55] = COp55m8;
    table[0x56] = COp56m8;
    table[0x57] = COp57m8;
    table[0x58] = COp58;
    table[0x59] = COp59m8;
    table[0x5A] = COp5Ax8;
    table[0x5B] = COp5B;
    table[0x5C] = COp5C;
    table[0x5D] = COp5Dm8;
    table[0x5E] = COp5Em8;
    table[0x5F] = COp5Fm8;
    table[0x60] = COp60;
    table[0x61] = COp61m8nd;
    table[0x62] = COp62;
    table[0x63] = COp63m8nd;
    table[0x64] = COp64m8;
    table[0x65] = COp65m8nd;
    table[0x66] = COp66m8;
    table[0x67] = COp67m8nd;
    table[0x68] = COp68m8;
    table[0x69] = COp69m8nd;
    table[0x6A] = COp6Am8;
    table[0x6B] = COp6B;
    table[0x6C] = COp6C;
    table[0x6D] = COp6Dm8nd;
    table[0x6E] = COp6Em8;
    table[0x6F] = COp6Fm8nd;
    table[0x70] = COp70;
    table[0x71] = COp71m8nd;
    table[0x72] = COp72m8nd;
    table[0x73] = COp73m8nd;
    table[0x74] = COp74m8;
    table[0x75] = COp75m8nd;
    table[0x76] = COp76m8;
    table[0x77] = COp77m8nd;
    table[0x78] = COp78;
    table[0x79] = COp79m8nd;
    table[0x7A] = COp7Ax8;
    table[0x7B] = COp7B;
    table[0x7C] = COp7C;
    table[0x7D] = COp7Dm8nd;
    table[0x7E] = COp7Em8;
    table[0x7F] = COp7Fm8nd;
    table[0x80] = COp80;
    table[0x81] = COp81m8;
    table[0x82] = COp82;
    table[0x83] = COp83m8;
    table[0x84] = COp84x8;
    table[0x85] = COp85m8;
    table[0x86] = COp86x8;
    table[0x87] = COp87m8;
    table[0x88] = COp88x8;
    table[0x89] = COp89m8;
    table[0x8A] = COp8Am8;
    table[0x8B] = COp8B;
    table[0x8C] = COp8Cx8;
    table[0x8D] = COp8Dm8;
    table[0x8E] = COp8Ex8;
    table[0x8F] = COp8Fm8;
    table[0x90] = COp90;
    table[0x91] = COp91m8;
    table[0x92] = COp92m8;
    table[0x93] = COp93m8;
    table[0x94] = COp94x8;
    table[0x95] = COp95m8;
    table[0x96] = COp96x8;
    table[0x97] = COp97m8;
    table[0x98] = COp98m8;
    table[0x99] = COp99m8;
    table[0x9A] = COp9A;
    table[0x9B] = COp9Bx8;
    table[0x9C] = COp9Cm8;
    table[0x9D] = COp9Dm8;
    table[0x9E] = COp9Em8;
    table[0x9F] = COp9Fm8;
    table[0xA0] = COpA0x8;
    table[0xA1] = COpA1m8;
    table[0xA2] = COpA2x8;
    table[0xA3] = COpA3m8;
    table[0xA4] = COpA4x8;
    table[0xA5] = COpA5m8;
    table[0xA6] = COpA6x8;
    table[0xA7] = COpA7m8;
    table[0xA8] = COpA8x8;
    table[0xA9] = COpA9m8;
    table[0xAA] = COpAAx8;
    table[0xAB] = COpAB;
    table[0xAC] = COpACx8;
    table[0xAD] = COpADm8;
    table[0xAE] = COpAEx8;
    table[0xAF] = COpAFm8;
    table[0xB0] = COpB0;
    table[0xB1] = COpB1m8;
    table[0xB2] = COpB2m8;
    table[0xB3] = COpB3m8;
    table[0xB4] = COpB4x8;
    table[0xB5] = COpB5m8;
    table[0xB6] = COpB6x8;
    table[0xB7] = COpB7m8;
    table[0xB8] = COpB8;
    table[0xB9] = COpB9m8;
    table[0xBA] = COpBAx8;
    table[0xBB] = COpBBx8;
    table[0xBC] = COpBCx8;
    table[0xBD] = COpBDm8;
    table[0xBE] = COpBEx8;
    table[0xBF] = COpBFm8;
    table[0xC0] = COpC0x8;
    table[0xC1] = COpC1m8;
    table[0xC2] = COpC2;
    table[0xC3] = COpC3m8;
    table[0xC4] = COpC4x8;
    table[0xC5] = COpC5m8;
    table[0xC6] = COpC6m8;
    table[0xC7] = COpC7m8;
    table[0xC8] = COpC8x8;
    table[0xC9] = COpC9m8;
    table[0xCA] = COpCAx8;
    table[0xCB] = COpCB;
    table[0xCC] = COpCCx8;
    table[0xCD] = COpCDm8;
    table[0xCE] = COpCEm8;
    table[0xCF] = COpCFm8;
    table[0xD0] = COpD0;
    table[0xD1] = COpD1m8;
    table[0xD2] = COpD2m8;
    table[0xD3] = COpD3m8;
    table[0xD4] = COpD4;
    table[0xD5] = COpD5m8;
    table[0xD6] = COpD6m8;
    table[0xD7] = COpD7m8;
    table[0xD8] = COpD8;
    table[0xD9] = COpD9m8;
    table[0xDA] = COpDAx8;
    table[0xDB] = COpDB;
    table[0xDC] = COpDC;
    table[0xDD] = COpDDm8;
    table[0xDE] = COpDEm8;
    table[0xDF] = COpDFm8;
    table[0xE0] = COpE0x8;
    table[0xE1] = COpE1m8nd;
    table[0xE2] = COpE2;
    table[0xE3] = COpE3m8nd;
    table[0xE4] = COpE4x8;
    table[0xE5] = COpE5m8nd;
    table[0xE6] = COpE6m8;
    table[0xE7] = COpE7m8nd;
    table[0xE8] = COpE8x8;
    table[0xE9] = COpE9m8nd;
    table[0xEA] = COpEA;
    table[0xEB] = COpEB;
    table[0xEC] = COpECx8;
    table[0xED] = COpEDm8nd;
    table[0xEE] = COpEEm8;
    table[0xEF] = COpEFm8nd;
    table[0xF0] = COpF0;
    table[0xF1] = COpF1m8nd;
    table[0xF2] = COpF2m8nd;
    table[0xF3] = COpF3m8nd;
    table[0xF4] = COpF4;
    table[0xF5] = COpF5m8nd;
    table[0xF6] = COpF6m8;
    table[0xF7] = COpF7m8nd;
    table[0xF8] = COpF8;
    table[0xF9] = COpF9m8nd;
    table[0xFA] = COpFAx8;
    table[0xFB] = COpFB;
    table[0xFC] = COpFC;
    table[0xFD] = COpFDm8nd;
    table[0xFE] = COpFEm8;
    table[0xFF] = COpFFm8nd;
}

static void settablem16(eop** table)
{
    table[0x01] = COp01m16;
    table[0x03] = COp03m16;
    table[0x04] = COp04m16;
    table[0x05] = COp05m16;
    table[0x06] = COp06m16;
    table[0x07] = COp07m16;
    table[0x09] = COp09m16;
    table[0x0A] = COp0Am16;
    table[0x0C] = COp0Cm16;
    table[0x0D] = COp0Dm16;
    table[0x0E] = COp0Em16;
    table[0x0F] = COp0Fm16;
    table[0x11] = COp11m16;
    table[0x12] = COp12m16;
    table[0x13] = COp13m16;
    table[0x14] = COp14m16;
    table[0x15] = COp15m16;
    table[0x16] = COp16m16;
    table[0x17] = COp17m16;
    table[0x19] = COp19m16;
    table[0x1A] = COp1Am16;
    table[0x1C] = COp1Cm16;
    table[0x1D] = COp1Dm16;
    table[0x1E] = COp1Em16;
    table[0x1F] = COp1Fm16;
    table[0x21] = COp21m16;
    table[0x23] = COp23m16;
    table[0x24] = COp24m16;
    table[0x25] = COp25m16;
    table[0x26] = COp26m16;
    table[0x27] = COp27m16;
    table[0x29] = COp29m16;
    table[0x2A] = COp2Am16;
    table[0x2C] = COp2Cm16;
    table[0x2D] = COp2Dm16;
    table[0x2E] = COp2Em16;
    table[0x2F] = COp2Fm16;
    table[0x31] = COp31m16;
    table[0x32] = COp32m16;
    table[0x33] = COp33m16;
    table[0x34] = COp34m16;
    table[0x35] = COp35m16;
    table[0x36] = COp36m16;
    table[0x37] = COp37m16;
    table[0x39] = COp39m16;
    table[0x3A] = COp3Am16;
    table[0x3C] = COp3Cm16;
    table[0x3D] = COp3Dm16;
    table[0x3E] = COp3Em16;
    table[0x3F] = COp3Fm16;
    table[0x41] = COp41m16;
    table[0x43] = COp43m16;
    table[0x45] = COp45m16;
    table[0x46] = COp46m16;
    table[0x47] = COp47m16;
    table[0x48] = COp48m16;
    table[0x49] = COp49m16;
    table[0x4A] = COp4Am16;
    table[0x4D] = COp4Dm16;
    table[0x4E] = COp4Em16;
    table[0x4F] = COp4Fm16;
    table[0x51] = COp51m16;
    table[0x52] = COp52m16;
    table[0x53] = COp53m16;
    table[0x55] = COp55m16;
    table[0x56] = COp56m16;
    table[0x57] = COp57m16;
    table[0x59] = COp59m16;
    table[0x5D] = COp5Dm16;
    table[0x5E] = COp5Em16;
    table[0x5F] = COp5Fm16;
    table[0x61] = COp61m16nd;
    table[0x63] = COp63m16nd;
    table[0x64] = COp64m16;
    table[0x65] = COp65m16nd;
    table[0x66] = COp66m16;
    table[0x67] = COp67m16nd;
    table[0x68] = COp68m16;
    table[0x69] = COp69m16nd;
    table[0x6A] = COp6Am16;
    table[0x6D] = COp6Dm16nd;
    table[0x6E] = COp6Em16;
    table[0x6F] = COp6Fm16nd;
    table[0x71] = COp71m16nd;
    table[0x72] = COp72m16nd;
    table[0x73] = COp73m16nd;
    table[0x74] = COp74m16;
    table[0x75] = COp75m16nd;
    table[0x76] = COp76m16;
    table[0x77] = COp77m16nd;
    table[0x79] = COp79m16nd;
    table[0x7D] = COp7Dm16nd;
    table[0x7E] = COp7Em16;
    table[0x7F] = COp7Fm16nd;
    table[0x81] = COp81m16;
    table[0x83] = COp83m16;
    table[0x85] = COp85m16;
    table[0x87] = COp87m16;
    table[0x89] = COp89m16;
    table[0x8A] = COp8Am16;
    table[0x8D] = COp8Dm16;
    table[0x8F] = COp8Fm16;
    table[0x91] = COp91m16;
    table[0x92] = COp92m16;
    table[0x93] = COp93m16;
    table[0x95] = COp95m16;
    table[0x97] = COp97m16;
    table[0x98] = COp98m16;
    table[0x99] = COp99m16;
    table[0x9C] = COp9Cm16;
    table[0x9D] = COp9Dm16;
    table[0x9E] = COp9Em16;
    table[0x9F] = COp9Fm16;
    table[0xA1] = COpA1m16;
    table[0xA3] = COpA3m16;
    table[0xA5] = COpA5m16;
    table[0xA7] = COpA7m16;
    table[0xA9] = COpA9m16;
    table[0xAD] = COpADm16;
    table[0xAF] = COpAFm16;
    table[0xB1] = COpB1m16;
    table[0xB2] = COpB2m16;
    table[0xB3] = COpB3m16;
    table[0xB5] = COpB5m16;
    table[0xB7] = COpB7m16;
    table[0xB9] = COpB9m16;
    table[0xBD] = COpBDm16;
    table[0xBF] = COpBFm16;
    table[0xC1] = COpC1m16;
    table[0xC3] = COpC3m16;
    table[0xC5] = COpC5m16;
    table[0xC6] = COpC6m16;
    table[0xC7] = COpC7m16;
    table[0xC9] = COpC9m16;
    table[0xCD] = COpCDm16;
    table[0xCE] = COpCEm16;
    table[0xCF] = COpCFm16;
    table[0xD1] = COpD1m16;
    table[0xD2] = COpD2m16;
    table[0xD3] = COpD3m16;
    table[0xD5] = COpD5m16;
    table[0xD6] = COpD6m16;
    table[0xD7] = COpD7m16;
    table[0xD9] = COpD9m16;
    table[0xDD] = COpDDm16;
    table[0xDE] = COpDEm16;
    table[0xDF] = COpDFm16;
    table[0xE1] = COpE1m16nd;
    table[0xE3] = COpE3m16nd;
    table[0xE5] = COpE5m16nd;
    table[0xE6] = COpE6m16;
    table[0xE7] = COpE7m16nd;
    table[0xE9] = COpE9m16nd;
    table[0xED] = COpEDm16nd;
    table[0xEE] = COpEEm16;
    table[0xEF] = COpEFm16nd;
    table[0xF1] = COpF1m16nd;
    table[0xF2] = COpF2m16nd;
    table[0xF3] = COpF3m16nd;
    table[0xF5] = COpF5m16nd;
    table[0xF6] = COpF6m16;
    table[0xF7] = COpF7m16nd;
    table[0xF9] = COpF9m16nd;
    table[0xFD] = COpFDm16nd;
    table[0xFE] = COpFEm16;
    table[0xFF] = COpFFm16nd;
}

static void settablex16(eop** table)
{
    table[0x5A] = COp5Ax16;
    table[0x7A] = COp7Ax16;
    table[0x84] = COp84x16;
    table[0x86] = COp86x16;
    table[0x88] = COp88x16;
    table[0x8C] = COp8Cx16;
    table[0x8E] = COp8Ex16;
    table[0x94] = COp94x16;
    table[0x96] = COp96x16;
    table[0x9B] = COp9Bx16;
    table[0xA0] = COpA0x16;
    table[0xA2] = COpA2x16;
    table[0xA4] = COpA4x16;
    table[0xA6] = COpA6x16;
    table[0xA8] = COpA8x16;
    table[0xAA] = COpAAx16;
    table[0xAC] = COpACx16;
    table[0xAE] = COpAEx16;
    table[0xB4] = COpB4x16;
    table[0xB6] = COpB6x16;
    table[0xBA] = COpBAx16;
    table[0xBB] = COpBBx16;
    table[0xBC] = COpBCx16;
    table[0xBE] = COpBEx16;
    table[0xC0] = COpC0x16;
    table[0xC4] = COpC4x16;
    table[0xC8] = COpC8x16;
    table[0xCA] = COpCAx16;
    table[0xCC] = COpCCx16;
    table[0xDA] = COpDAx16;
    table[0xE0] = COpE0x16;
    table[0xE4] = COpE4x16;
    table[0xE8] = COpE8x16;
    table[0xEC] = COpECx16;
    table[0xFA] = COpFAx16;
}

static void settableDm8(eop** table)
{
    table[0x61] = COp61m8d;
    table[0x63] = COp63m8d;
    table[0x65] = COp65m8d;
    table[0x67] = COp67m8d;
    table[0x69] = COp69m8d;
    table[0x6D] = COp6Dm8d;
    table[0x6F] = COp6Fm8d;
    table[0x71] = COp71m8d;
    table[0x72] = COp72m8d;
    table[0x73] = COp73m8d;
    table[0x75] = COp75m8d;
    table[0x77] = COp77m8d;
    table[0x79] = COp79m8d;
    table[0x7D] = COp7Dm8d;
    table[0x7F] = COp7Fm8d;
    table[0xE1] = COpE1m8d;
    table[0xE3] = COpE3m8d;
    table[0xE5] = COpE5m8d;
    table[0xE7] = COpE7m8d;
    table[0xE9] = COpE9m8d;
    table[0xED] = COpEDm8d;
    table[0xEF] = COpEFm8d;
    table[0xF1] = COpF1m8d;
    table[0xF2] = COpF2m8d;
    table[0xF3] = COpF3m8d;
    table[0xF5] = COpF5m8d;
    table[0xF7] = COpF7m8d;
    table[0xF9] = COpF9m8d;
    table[0xFD] = COpFDm8d;
    table[0xFF] = COpFFm8d;
}

static void settableDm16(eop** table)
{
    table[0x61] = COp61m16d;
    table[0x63] = COp63m16d;
    table[0x65] = COp65m16d;
    table[0x67] = COp67m16d;
    table[0x69] = COp69m16d;
    table[0x6D] = COp6Dm16d;
    table[0x6F] = COp6Fm16d;
    table[0x71] = COp71m16d;
    table[0x72] = COp72m16d;
    table[0x73] = COp73m16d;
    table[0x75] = COp75m16d;
    table[0x77] = COp77m16d;
    table[0x79] = COp79m16d;
    table[0x7D] = COp7Dm16d;
    table[0x7F] = COp7Fm16d;
    table[0xE1] = COpE1m16d;
    table[0xE3] = COpE3m16d;
    table[0xE5] = COpE5m16d;
    table[0xE7] = COpE7m16d;
    table[0xE9] = COpE9m16d;
    table[0xED] = COpEDm16d;
    table[0xEF] = COpEFm16d;
    table[0xF1] = COpF1m16d;
    table[0xF2] = COpF2m16d;
    table[0xF3] = COpF3m16d;
    table[0xF5] = COpF5m16d;
    table[0xF7] = COpF7m16d;
    table[0xF9] = COpF9m16d;
    table[0xFD] = COpFDm16d;
    table[0xFF] = COpFFm16d;
}

// Generate OpCode Table
void inittable(void)
{
    // set up mosaic
#ifdef __MSDOS__
    mosjmptab[0] = mosdraw2;
    mosjmptab[1] = mosdraw3;
    mosjmptab[2] = mosdraw4;
    mosjmptab[3] = mosdraw5;
    mosjmptab[4] = mosdraw6;
    mosjmptab[5] = mosdraw7;
    mosjmptab[6] = mosdraw8;
    mosjmptab[7] = mosdraw9;
    mosjmptab[8] = mosdraw10;
    mosjmptab[9] = mosdraw11;
    mosjmptab[10] = mosdraw12;
    mosjmptab[11] = mosdraw13;
    mosjmptab[12] = mosdraw14;
    mosjmptab[13] = mosdraw15;
    mosjmptab[14] = mosdraw16;
#endif

    mosjmptab16b[0] = mosdraw216b;
    mosjmptab16b[1] = mosdraw316b;
    mosjmptab16b[2] = mosdraw416b;
    mosjmptab16b[3] = mosdraw516b;
    mosjmptab16b[4] = mosdraw616b;
    mosjmptab16b[5] = mosdraw716b;
    mosjmptab16b[6] = mosdraw816b;
    mosjmptab16b[7] = mosdraw916b;
    mosjmptab16b[8] = mosdraw1016b;
    mosjmptab16b[9] = mosdraw1116b;
    mosjmptab16b[10] = mosdraw1216b;
    mosjmptab16b[11] = mosdraw1316b;
    mosjmptab16b[12] = mosdraw1416b;
    mosjmptab16b[13] = mosdraw1516b;
    mosjmptab16b[14] = mosdraw1616b;

    mosjmptab16bt[0] = mosdraw216bt;
    mosjmptab16bt[1] = mosdraw316bt;
    mosjmptab16bt[2] = mosdraw416bt;
    mosjmptab16bt[3] = mosdraw516bt;
    mosjmptab16bt[4] = mosdraw616bt;
    mosjmptab16bt[5] = mosdraw716bt;
    mosjmptab16bt[6] = mosdraw816bt;
    mosjmptab16bt[7] = mosdraw916bt;
    mosjmptab16bt[8] = mosdraw1016bt;
    mosjmptab16bt[9] = mosdraw1116bt;
    mosjmptab16bt[10] = mosdraw1216bt;
    mosjmptab16bt[11] = mosdraw1316bt;
    mosjmptab16bt[12] = mosdraw1416bt;
    mosjmptab16bt[13] = mosdraw1516bt;
    mosjmptab16bt[14] = mosdraw1616bt;

    mosjmptab16btms[0] = mosdraw216btms;
    mosjmptab16btms[1] = mosdraw316btms;
    mosjmptab16btms[2] = mosdraw416btms;
    mosjmptab16btms[3] = mosdraw516btms;
    mosjmptab16btms[4] = mosdraw616btms;
    mosjmptab16btms[5] = mosdraw716btms;
    mosjmptab16btms[6] = mosdraw816btms;
    mosjmptab16btms[7] = mosdraw916btms;
    mosjmptab16btms[8] = mosdraw1016btms;
    mosjmptab16btms[9] = mosdraw1116btms;
    mosjmptab16btms[10] = mosdraw1216btms;
    mosjmptab16btms[11] = mosdraw1316btms;
    mosjmptab16btms[12] = mosdraw1416btms;
    mosjmptab16btms[13] = mosdraw1516btms;
    mosjmptab16btms[14] = mosdraw1616btms;

    mosjmptab16bntms[0] = mosdraw216bntms;
    mosjmptab16bntms[1] = mosdraw316bntms;
    mosjmptab16bntms[2] = mosdraw416bntms;
    mosjmptab16bntms[3] = mosdraw516bntms;
    mosjmptab16bntms[4] = mosdraw616bntms;
    mosjmptab16bntms[5] = mosdraw716bntms;
    mosjmptab16bntms[6] = mosdraw816bntms;
    mosjmptab16bntms[7] = mosdraw916bntms;
    mosjmptab16bntms[8] = mosdraw1016bntms;
    mosjmptab16bntms[9] = mosdraw1116bntms;
    mosjmptab16bntms[10] = mosdraw1216bntms;
    mosjmptab16bntms[11] = mosdraw1316bntms;
    mosjmptab16bntms[12] = mosdraw1416bntms;
    mosjmptab16bntms[13] = mosdraw1516bntms;
    mosjmptab16bntms[14] = mosdraw1616bntms;

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
    for (eop** i = tableA; i != endof(tableA); ++i)
        *i = eopINVALID;
    for (eop** i = tableB; i != endof(tableB); ++i)
        *i = eopINVALID;
    for (eop** i = tableC; i != endof(tableC); ++i)
        *i = eopINVALID;
    for (eop** i = tableD; i != endof(tableD); ++i)
        *i = eopINVALID;
    for (eop** i = tableE; i != endof(tableE); ++i)
        *i = eopINVALID;
    for (eop** i = tableF; i != endof(tableF); ++i)
        *i = eopINVALID;
    for (eop** i = tableG; i != endof(tableG); ++i)
        *i = eopINVALID;
    for (eop** i = tableH; i != endof(tableH); ++i)
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
