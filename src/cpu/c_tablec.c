#include "../endmem.h"
#include "../macros.h"
#include "c_tablec.h"
#include "table.h"
#include "tablec.h"


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

	asm volatile("call %P0" :: "X" (settables), "D" (tableAc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableBc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableCc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableDc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableEc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableFc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableGc) : "memory");
	asm volatile("call %P0" :: "X" (settables), "D" (tableHc) : "memory");

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
