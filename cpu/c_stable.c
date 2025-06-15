#include "c_stable.h"
#include "../cpu/e65816.h"
#include "../endmem.h"
#include "../macros.h"
#include "../video/newgfx.h"
#include "../video/newgfx16.h"
#include "se65816.h"
#include "table.h"

/* Sets the opcode tables
 * This function sets all the non-multiple entries */
static void settables(eop** table)
{
    // row 0
    table[0x00] = SA1COp00;
    table[0x01] = SA1COp01m8;
    table[0x02] = SA1COp02;
    table[0x03] = SA1COp03m8;
    table[0x04] = SA1COp04m8;
    table[0x05] = SA1COp05m8;
    table[0x06] = SA1COp06m8;
    table[0x07] = SA1COp07m8;
    table[0x08] = SA1COp08;
    table[0x09] = SA1COp09m8;
    table[0x0A] = SA1COp0Am8;
    table[0x0B] = SA1COp0B;
    table[0x0C] = SA1COp0Cm8;
    table[0x0D] = SA1COp0Dm8;
    table[0x0E] = SA1COp0Em8;
    table[0x0F] = SA1COp0Fm8;
    table[0x10] = SA1COp10;
    table[0x11] = SA1COp11m8;
    table[0x12] = SA1COp12m8;
    table[0x13] = SA1COp13m8;
    table[0x14] = SA1COp14m8;
    table[0x15] = SA1COp15m8;
    table[0x16] = SA1COp16m8;
    table[0x17] = SA1COp17m8;
    table[0x18] = SA1COp18;
    table[0x19] = SA1COp19m8;
    table[0x1A] = SA1COp1Am8;
    table[0x1B] = SA1COp1B;
    table[0x1C] = SA1COp1Cm8;
    table[0x1D] = SA1COp1Dm8;
    table[0x1E] = SA1COp1Em8;
    table[0x1F] = SA1COp1Fm8;
    table[0x20] = SA1COp20;
    table[0x21] = SA1COp21m8;
    table[0x22] = SA1COp22;
    table[0x23] = SA1COp23m8;
    table[0x24] = SA1COp24m8;
    table[0x25] = SA1COp25m8;
    table[0x26] = SA1COp26m8;
    table[0x27] = SA1COp27m8;
    table[0x28] = SA1COp28;
    table[0x29] = SA1COp29m8;
    table[0x2A] = SA1COp2Am8;
    table[0x2B] = SA1COp2B;
    table[0x2C] = SA1COp2Cm8;
    table[0x2D] = SA1COp2Dm8;
    table[0x2E] = SA1COp2Em8;
    table[0x2F] = SA1COp2Fm8;
    table[0x30] = SA1COp30;
    table[0x31] = SA1COp31m8;
    table[0x32] = SA1COp32m8;
    table[0x33] = SA1COp33m8;
    table[0x34] = SA1COp34m8;
    table[0x35] = SA1COp35m8;
    table[0x36] = SA1COp36m8;
    table[0x37] = SA1COp37m8;
    table[0x38] = SA1COp38;
    table[0x39] = SA1COp39m8;
    table[0x3A] = SA1COp3Am8;
    table[0x3B] = SA1COp3B;
    table[0x3C] = SA1COp3Cm8;
    table[0x3D] = SA1COp3Dm8;
    table[0x3E] = SA1COp3Em8;
    table[0x3F] = SA1COp3Fm8;
    table[0x40] = SA1COp40;
    table[0x41] = SA1COp41m8;
    table[0x42] = SA1COp42;
    table[0x43] = SA1COp43m8;
    table[0x44] = SA1COp44;
    table[0x45] = SA1COp45m8;
    table[0x46] = SA1COp46m8;
    table[0x47] = SA1COp47m8;
    table[0x48] = SA1COp48m8;
    table[0x49] = SA1COp49m8;
    table[0x4A] = SA1COp4Am8;
    table[0x4B] = SA1COp4B;
    table[0x4C] = SA1COp4C;
    table[0x4D] = SA1COp4Dm8;
    table[0x4E] = SA1COp4Em8;
    table[0x4F] = SA1COp4Fm8;
    table[0x50] = SA1COp50;
    table[0x51] = SA1COp51m8;
    table[0x52] = SA1COp52m8;
    table[0x53] = SA1COp53m8;
    table[0x54] = SA1COp54;
    table[0x55] = SA1COp55m8;
    table[0x56] = SA1COp56m8;
    table[0x57] = SA1COp57m8;
    table[0x58] = SA1COp58;
    table[0x59] = SA1COp59m8;
    table[0x5A] = SA1COp5Ax8;
    table[0x5B] = SA1COp5B;
    table[0x5C] = SA1COp5C;
    table[0x5D] = SA1COp5Dm8;
    table[0x5E] = SA1COp5Em8;
    table[0x5F] = SA1COp5Fm8;
    table[0x60] = SA1COp60;
    table[0x61] = SA1COp61m8nd;
    table[0x62] = SA1COp62;
    table[0x63] = SA1COp63m8nd;
    table[0x64] = SA1COp64m8;
    table[0x65] = SA1COp65m8nd;
    table[0x66] = SA1COp66m8;
    table[0x67] = SA1COp67m8nd;
    table[0x68] = SA1COp68m8;
    table[0x69] = SA1COp69m8nd;
    table[0x6A] = SA1COp6Am8;
    table[0x6B] = SA1COp6B;
    table[0x6C] = SA1COp6C;
    table[0x6D] = SA1COp6Dm8nd;
    table[0x6E] = SA1COp6Em8;
    table[0x6F] = SA1COp6Fm8nd;
    table[0x70] = SA1COp70;
    table[0x71] = SA1COp71m8nd;
    table[0x72] = SA1COp72m8nd;
    table[0x73] = SA1COp73m8nd;
    table[0x74] = SA1COp74m8;
    table[0x75] = SA1COp75m8nd;
    table[0x76] = SA1COp76m8;
    table[0x77] = SA1COp77m8nd;
    table[0x78] = SA1COp78;
    table[0x79] = SA1COp79m8nd;
    table[0x7A] = SA1COp7Ax8;
    table[0x7B] = SA1COp7B;
    table[0x7C] = SA1COp7C;
    table[0x7D] = SA1COp7Dm8nd;
    table[0x7E] = SA1COp7Em8;
    table[0x7F] = SA1COp7Fm8nd;
    table[0x80] = SA1COp80;
    table[0x81] = SA1COp81m8;
    table[0x82] = SA1COp82;
    table[0x83] = SA1COp83m8;
    table[0x84] = SA1COp84x8;
    table[0x85] = SA1COp85m8;
    table[0x86] = SA1COp86x8;
    table[0x87] = SA1COp87m8;
    table[0x88] = SA1COp88x8;
    table[0x89] = SA1COp89m8;
    table[0x8A] = SA1COp8Am8;
    table[0x8B] = SA1COp8B;
    table[0x8C] = SA1COp8Cx8;
    table[0x8D] = SA1COp8Dm8;
    table[0x8E] = SA1COp8Ex8;
    table[0x8F] = SA1COp8Fm8;
    table[0x90] = SA1COp90;
    table[0x91] = SA1COp91m8;
    table[0x92] = SA1COp92m8;
    table[0x93] = SA1COp93m8;
    table[0x94] = SA1COp94x8;
    table[0x95] = SA1COp95m8;
    table[0x96] = SA1COp96x8;
    table[0x97] = SA1COp97m8;
    table[0x98] = SA1COp98m8;
    table[0x99] = SA1COp99m8;
    table[0x9A] = SA1COp9A;
    table[0x9B] = SA1COp9Bx8;
    table[0x9C] = SA1COp9Cm8;
    table[0x9D] = SA1COp9Dm8;
    table[0x9E] = SA1COp9Em8;
    table[0x9F] = SA1COp9Fm8;
    table[0xA0] = SA1COpA0x8;
    table[0xA1] = SA1COpA1m8;
    table[0xA2] = SA1COpA2x8;
    table[0xA3] = SA1COpA3m8;
    table[0xA4] = SA1COpA4x8;
    table[0xA5] = SA1COpA5m8;
    table[0xA6] = SA1COpA6x8;
    table[0xA7] = SA1COpA7m8;
    table[0xA8] = SA1COpA8x8;
    table[0xA9] = SA1COpA9m8;
    table[0xAA] = SA1COpAAx8;
    table[0xAB] = SA1COpAB;
    table[0xAC] = SA1COpACx8;
    table[0xAD] = SA1COpADm8;
    table[0xAE] = SA1COpAEx8;
    table[0xAF] = SA1COpAFm8;
    table[0xB0] = SA1COpB0;
    table[0xB1] = SA1COpB1m8;
    table[0xB2] = SA1COpB2m8;
    table[0xB3] = SA1COpB3m8;
    table[0xB4] = SA1COpB4x8;
    table[0xB5] = SA1COpB5m8;
    table[0xB6] = SA1COpB6x8;
    table[0xB7] = SA1COpB7m8;
    table[0xB8] = SA1COpB8;
    table[0xB9] = SA1COpB9m8;
    table[0xBA] = SA1COpBAx8;
    table[0xBB] = SA1COpBBx8;
    table[0xBC] = SA1COpBCx8;
    table[0xBD] = SA1COpBDm8;
    table[0xBE] = SA1COpBEx8;
    table[0xBF] = SA1COpBFm8;
    table[0xC0] = SA1COpC0x8;
    table[0xC1] = SA1COpC1m8;
    table[0xC2] = SA1COpC2;
    table[0xC3] = SA1COpC3m8;
    table[0xC4] = SA1COpC4x8;
    table[0xC5] = SA1COpC5m8;
    table[0xC6] = SA1COpC6m8;
    table[0xC7] = SA1COpC7m8;
    table[0xC8] = SA1COpC8x8;
    table[0xC9] = SA1COpC9m8;
    table[0xCA] = SA1COpCAx8;
    table[0xCB] = SA1COpCB;
    table[0xCC] = SA1COpCCx8;
    table[0xCD] = SA1COpCDm8;
    table[0xCE] = SA1COpCEm8;
    table[0xCF] = SA1COpCFm8;
    table[0xD0] = SA1COpD0;
    table[0xD1] = SA1COpD1m8;
    table[0xD2] = SA1COpD2m8;
    table[0xD3] = SA1COpD3m8;
    table[0xD4] = SA1COpD4;
    table[0xD5] = SA1COpD5m8;
    table[0xD6] = SA1COpD6m8;
    table[0xD7] = SA1COpD7m8;
    table[0xD8] = SA1COpD8;
    table[0xD9] = SA1COpD9m8;
    table[0xDA] = SA1COpDAx8;
    table[0xDB] = SA1COpDB;
    table[0xDC] = SA1COpDC;
    table[0xDD] = SA1COpDDm8;
    table[0xDE] = SA1COpDEm8;
    table[0xDF] = SA1COpDFm8;
    table[0xE0] = SA1COpE0x8;
    table[0xE1] = SA1COpE1m8nd;
    table[0xE2] = SA1COpE2;
    table[0xE3] = SA1COpE3m8nd;
    table[0xE4] = SA1COpE4x8;
    table[0xE5] = SA1COpE5m8nd;
    table[0xE6] = SA1COpE6m8;
    table[0xE7] = SA1COpE7m8nd;
    table[0xE8] = SA1COpE8x8;
    table[0xE9] = SA1COpE9m8nd;
    table[0xEA] = SA1COpEA;
    table[0xEB] = SA1COpEB;
    table[0xEC] = SA1COpECx8;
    table[0xED] = SA1COpEDm8nd;
    table[0xEE] = SA1COpEEm8;
    table[0xEF] = SA1COpEFm8nd;
    table[0xF0] = SA1COpF0;
    table[0xF1] = SA1COpF1m8nd;
    table[0xF2] = SA1COpF2m8nd;
    table[0xF3] = SA1COpF3m8nd;
    table[0xF4] = SA1COpF4;
    table[0xF5] = SA1COpF5m8nd;
    table[0xF6] = SA1COpF6m8;
    table[0xF7] = SA1COpF7m8nd;
    table[0xF8] = SA1COpF8;
    table[0xF9] = SA1COpF9m8nd;
    table[0xFA] = SA1COpFAx8;
    table[0xFB] = SA1COpFB;
    table[0xFC] = SA1COpFC;
    table[0xFD] = SA1COpFDm8nd;
    table[0xFE] = SA1COpFEm8;
    table[0xFF] = SA1COpFFm8nd;
}

static void settablem16(eop** table)
{
    table[0x01] = SA1COp01m16;
    table[0x03] = SA1COp03m16;
    table[0x04] = SA1COp04m16;
    table[0x05] = SA1COp05m16;
    table[0x06] = SA1COp06m16;
    table[0x07] = SA1COp07m16;
    table[0x09] = SA1COp09m16;
    table[0x0A] = SA1COp0Am16;
    table[0x0C] = SA1COp0Cm16;
    table[0x0D] = SA1COp0Dm16;
    table[0x0E] = SA1COp0Em16;
    table[0x0F] = SA1COp0Fm16;
    table[0x11] = SA1COp11m16;
    table[0x12] = SA1COp12m16;
    table[0x13] = SA1COp13m16;
    table[0x14] = SA1COp14m16;
    table[0x15] = SA1COp15m16;
    table[0x16] = SA1COp16m16;
    table[0x17] = SA1COp17m16;
    table[0x19] = SA1COp19m16;
    table[0x1A] = SA1COp1Am16;
    table[0x1C] = SA1COp1Cm16;
    table[0x1D] = SA1COp1Dm16;
    table[0x1E] = SA1COp1Em16;
    table[0x1F] = SA1COp1Fm16;
    table[0x21] = SA1COp21m16;
    table[0x23] = SA1COp23m16;
    table[0x24] = SA1COp24m16;
    table[0x25] = SA1COp25m16;
    table[0x26] = SA1COp26m16;
    table[0x27] = SA1COp27m16;
    table[0x29] = SA1COp29m16;
    table[0x2A] = SA1COp2Am16;
    table[0x2C] = SA1COp2Cm16;
    table[0x2D] = SA1COp2Dm16;
    table[0x2E] = SA1COp2Em16;
    table[0x2F] = SA1COp2Fm16;
    table[0x31] = SA1COp31m16;
    table[0x32] = SA1COp32m16;
    table[0x33] = SA1COp33m16;
    table[0x34] = SA1COp34m16;
    table[0x35] = SA1COp35m16;
    table[0x36] = SA1COp36m16;
    table[0x37] = SA1COp37m16;
    table[0x39] = SA1COp39m16;
    table[0x3A] = SA1COp3Am16;
    table[0x3C] = SA1COp3Cm16;
    table[0x3D] = SA1COp3Dm16;
    table[0x3E] = SA1COp3Em16;
    table[0x3F] = SA1COp3Fm16;
    table[0x41] = SA1COp41m16;
    table[0x43] = SA1COp43m16;
    table[0x45] = SA1COp45m16;
    table[0x46] = SA1COp46m16;
    table[0x47] = SA1COp47m16;
    table[0x48] = SA1COp48m16;
    table[0x49] = SA1COp49m16;
    table[0x4A] = SA1COp4Am16;
    table[0x4D] = SA1COp4Dm16;
    table[0x4E] = SA1COp4Em16;
    table[0x4F] = SA1COp4Fm16;
    table[0x51] = SA1COp51m16;
    table[0x52] = SA1COp52m16;
    table[0x53] = SA1COp53m16;
    table[0x55] = SA1COp55m16;
    table[0x56] = SA1COp56m16;
    table[0x57] = SA1COp57m16;
    table[0x59] = SA1COp59m16;
    table[0x5D] = SA1COp5Dm16;
    table[0x5E] = SA1COp5Em16;
    table[0x5F] = SA1COp5Fm16;
    table[0x61] = SA1COp61m16nd;
    table[0x63] = SA1COp63m16nd;
    table[0x64] = SA1COp64m16;
    table[0x65] = SA1COp65m16nd;
    table[0x66] = SA1COp66m16;
    table[0x67] = SA1COp67m16nd;
    table[0x68] = SA1COp68m16;
    table[0x69] = SA1COp69m16nd;
    table[0x6A] = SA1COp6Am16;
    table[0x6D] = SA1COp6Dm16nd;
    table[0x6E] = SA1COp6Em16;
    table[0x6F] = SA1COp6Fm16nd;
    table[0x71] = SA1COp71m16nd;
    table[0x72] = SA1COp72m16nd;
    table[0x73] = SA1COp73m16nd;
    table[0x74] = SA1COp74m16;
    table[0x75] = SA1COp75m16nd;
    table[0x76] = SA1COp76m16;
    table[0x77] = SA1COp77m16nd;
    table[0x79] = SA1COp79m16nd;
    table[0x7D] = SA1COp7Dm16nd;
    table[0x7E] = SA1COp7Em16;
    table[0x7F] = SA1COp7Fm16nd;
    table[0x81] = SA1COp81m16;
    table[0x83] = SA1COp83m16;
    table[0x85] = SA1COp85m16;
    table[0x87] = SA1COp87m16;
    table[0x89] = SA1COp89m16;
    table[0x8A] = SA1COp8Am16;
    table[0x8D] = SA1COp8Dm16;
    table[0x8F] = SA1COp8Fm16;
    table[0x91] = SA1COp91m16;
    table[0x92] = SA1COp92m16;
    table[0x93] = SA1COp93m16;
    table[0x95] = SA1COp95m16;
    table[0x97] = SA1COp97m16;
    table[0x98] = SA1COp98m16;
    table[0x99] = SA1COp99m16;
    table[0x9C] = SA1COp9Cm16;
    table[0x9D] = SA1COp9Dm16;
    table[0x9E] = SA1COp9Em16;
    table[0x9F] = SA1COp9Fm16;
    table[0xA1] = SA1COpA1m16;
    table[0xA3] = SA1COpA3m16;
    table[0xA5] = SA1COpA5m16;
    table[0xA7] = SA1COpA7m16;
    table[0xA9] = SA1COpA9m16;
    table[0xAD] = SA1COpADm16;
    table[0xAF] = SA1COpAFm16;
    table[0xB1] = SA1COpB1m16;
    table[0xB2] = SA1COpB2m16;
    table[0xB3] = SA1COpB3m16;
    table[0xB5] = SA1COpB5m16;
    table[0xB7] = SA1COpB7m16;
    table[0xB9] = SA1COpB9m16;
    table[0xBD] = SA1COpBDm16;
    table[0xBF] = SA1COpBFm16;
    table[0xC1] = SA1COpC1m16;
    table[0xC3] = SA1COpC3m16;
    table[0xC5] = SA1COpC5m16;
    table[0xC6] = SA1COpC6m16;
    table[0xC7] = SA1COpC7m16;
    table[0xC9] = SA1COpC9m16;
    table[0xCD] = SA1COpCDm16;
    table[0xCE] = SA1COpCEm16;
    table[0xCF] = SA1COpCFm16;
    table[0xD1] = SA1COpD1m16;
    table[0xD2] = SA1COpD2m16;
    table[0xD3] = SA1COpD3m16;
    table[0xD5] = SA1COpD5m16;
    table[0xD6] = SA1COpD6m16;
    table[0xD7] = SA1COpD7m16;
    table[0xD9] = SA1COpD9m16;
    table[0xDD] = SA1COpDDm16;
    table[0xDE] = SA1COpDEm16;
    table[0xDF] = SA1COpDFm16;
    table[0xE1] = SA1COpE1m16nd;
    table[0xE3] = SA1COpE3m16nd;
    table[0xE5] = SA1COpE5m16nd;
    table[0xE6] = SA1COpE6m16;
    table[0xE7] = SA1COpE7m16nd;
    table[0xE9] = SA1COpE9m16nd;
    table[0xED] = SA1COpEDm16nd;
    table[0xEE] = SA1COpEEm16;
    table[0xEF] = SA1COpEFm16nd;
    table[0xF1] = SA1COpF1m16nd;
    table[0xF2] = SA1COpF2m16nd;
    table[0xF3] = SA1COpF3m16nd;
    table[0xF5] = SA1COpF5m16nd;
    table[0xF6] = SA1COpF6m16;
    table[0xF7] = SA1COpF7m16nd;
    table[0xF9] = SA1COpF9m16nd;
    table[0xFD] = SA1COpFDm16nd;
    table[0xFE] = SA1COpFEm16;
    table[0xFF] = SA1COpFFm16nd;
}

static void settablex16(eop** table)
{
    table[0x5A] = SA1COp5Ax16;
    table[0x7A] = SA1COp7Ax16;
    table[0x84] = SA1COp84x16;
    table[0x86] = SA1COp86x16;
    table[0x88] = SA1COp88x16;
    table[0x8C] = SA1COp8Cx16;
    table[0x8E] = SA1COp8Ex16;
    table[0x94] = SA1COp94x16;
    table[0x96] = SA1COp96x16;
    table[0x9B] = SA1COp9Bx16;
    table[0xA0] = SA1COpA0x16;
    table[0xA2] = SA1COpA2x16;
    table[0xA4] = SA1COpA4x16;
    table[0xA6] = SA1COpA6x16;
    table[0xA8] = SA1COpA8x16;
    table[0xAA] = SA1COpAAx16;
    table[0xAC] = SA1COpACx16;
    table[0xAE] = SA1COpAEx16;
    table[0xB4] = SA1COpB4x16;
    table[0xB6] = SA1COpB6x16;
    table[0xBA] = SA1COpBAx16;
    table[0xBB] = SA1COpBBx16;
    table[0xBC] = SA1COpBCx16;
    table[0xBE] = SA1COpBEx16;
    table[0xC0] = SA1COpC0x16;
    table[0xC4] = SA1COpC4x16;
    table[0xC8] = SA1COpC8x16;
    table[0xCA] = SA1COpCAx16;
    table[0xCC] = SA1COpCCx16;
    table[0xDA] = SA1COpDAx16;
    table[0xE0] = SA1COpE0x16;
    table[0xE4] = SA1COpE4x16;
    table[0xE8] = SA1COpE8x16;
    table[0xEC] = SA1COpECx16;
    table[0xFA] = SA1COpFAx16;
}

static void settableDm8(eop** table)
{
    table[0x61] = SA1COp61m8d;
    table[0x63] = SA1COp63m8d;
    table[0x65] = SA1COp65m8d;
    table[0x67] = SA1COp67m8d;
    table[0x69] = SA1COp69m8d;
    table[0x6D] = SA1COp6Dm8d;
    table[0x6F] = SA1COp6Fm8d;
    table[0x71] = SA1COp71m8d;
    table[0x72] = SA1COp72m8d;
    table[0x73] = SA1COp73m8d;
    table[0x75] = SA1COp75m8d;
    table[0x77] = SA1COp77m8d;
    table[0x79] = SA1COp79m8d;
    table[0x7D] = SA1COp7Dm8d;
    table[0x7F] = SA1COp7Fm8d;
    table[0xE1] = SA1COpE1m8d;
    table[0xE3] = SA1COpE3m8d;
    table[0xE5] = SA1COpE5m8d;
    table[0xE7] = SA1COpE7m8d;
    table[0xE9] = SA1COpE9m8d;
    table[0xED] = SA1COpEDm8d;
    table[0xEF] = SA1COpEFm8d;
    table[0xF1] = SA1COpF1m8d;
    table[0xF2] = SA1COpF2m8d;
    table[0xF3] = SA1COpF3m8d;
    table[0xF5] = SA1COpF5m8d;
    table[0xF7] = SA1COpF7m8d;
    table[0xF9] = SA1COpF9m8d;
    table[0xFD] = SA1COpFDm8d;
    table[0xFF] = SA1COpFFm8d;
}

static void settableDm16(eop** table)
{
    table[0x61] = SA1COp61m16d;
    table[0x63] = SA1COp63m16d;
    table[0x65] = SA1COp65m16d;
    table[0x67] = SA1COp67m16d;
    table[0x69] = SA1COp69m16d;
    table[0x6D] = SA1COp6Dm16d;
    table[0x6F] = SA1COp6Fm16d;
    table[0x71] = SA1COp71m16d;
    table[0x72] = SA1COp72m16d;
    table[0x73] = SA1COp73m16d;
    table[0x75] = SA1COp75m16d;
    table[0x77] = SA1COp77m16d;
    table[0x79] = SA1COp79m16d;
    table[0x7D] = SA1COp7Dm16d;
    table[0x7F] = SA1COp7Fm16d;
    table[0xE1] = SA1COpE1m16d;
    table[0xE3] = SA1COpE3m16d;
    table[0xE5] = SA1COpE5m16d;
    table[0xE7] = SA1COpE7m16d;
    table[0xE9] = SA1COpE9m16d;
    table[0xED] = SA1COpEDm16d;
    table[0xEF] = SA1COpEFm16d;
    table[0xF1] = SA1COpF1m16d;
    table[0xF2] = SA1COpF2m16d;
    table[0xF3] = SA1COpF3m16d;
    table[0xF5] = SA1COpF5m16d;
    table[0xF7] = SA1COpF7m16d;
    table[0xF9] = SA1COpF9m16d;
    table[0xFD] = SA1COpFDm16d;
    table[0xFF] = SA1COpFFm16d;
}

// Generate OpCode Table
void SA1inittable(void)
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

    // set SA1tablead  (NVMXDIZC) (  MXD   )
    for (u4 i = 0; i != lengthof(SA1tablead); ++i) {
        static eop** const tableX[] = {
            SA1tableA, // ---
            SA1tableE, // --D
            SA1tableC, // -X-
            SA1tableG, // -XD
            SA1tableB, // M--
            SA1tableF, // M-D
            SA1tableD, // MX-
            SA1tableH // MXD
        };
        SA1tablead[i] = tableX[(i & 0x38) >> 3];
    }

    // Set CPU addresses
    // First, set all addresses to invalid
    // XXX This is probably pointless, the following settables() overwrite all entries
    for (eop** i = SA1tableA; i != endof(SA1tableA); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableB; i != endof(SA1tableB); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableC; i != endof(SA1tableC); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableD; i != endof(SA1tableD); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableE; i != endof(SA1tableE); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableF; i != endof(SA1tableF); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableG; i != endof(SA1tableG); ++i)
        *i = eopINVALID;
    for (eop** i = SA1tableH; i != endof(SA1tableH); ++i)
        *i = eopINVALID;

    // XXX All initialisations below seem to have no effect on the emulator
    settables(SA1tableA);
    settables(SA1tableB);
    settables(SA1tableC);
    settables(SA1tableD);
    settables(SA1tableE);
    settables(SA1tableF);
    settables(SA1tableG);
    settables(SA1tableH);

    // set proper functions
    settablem16(SA1tableA); // Table addresses (M:0,X:0,D:0)
    settablex16(SA1tableA);

    settablex16(SA1tableB); // Table addresses (M:1,X:0,D:0)

    settablem16(SA1tableC); // Table addresses (M:0,X:1,D:0)

    settablem16(SA1tableE); // Table addresses (M:0,X:0,D:1)
    settableDm16(SA1tableE);
    settablex16(SA1tableE);

    settablex16(SA1tableF); // Table addresses (M:1,X:0,D:1)
    settableDm8(SA1tableF);

    settablem16(SA1tableG); // Table addresses (M:0,X:1,D:1)
    settableDm16(SA1tableG);
    settableDm8(SA1tableH); // Table addresses (M:1,X:1,D:1)
}
