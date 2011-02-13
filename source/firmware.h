/*
 * firmware.h -- FreeFlash (Writable Flash)
 *
 * Copyright (C) Miguel Boton (Waninkoko)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 */

#ifndef _FIRMWARE_H_
#define _FIRMWARE_H_

#include "types.h"

/* LV2 patch structure */
typedef struct {
	u64 address;
	u64 value;
} LV2Patch;

/* Firmware structure */
typedef struct {
	u32      version;
	u64      devlist;
	LV2Patch patches[32];
} Firmware;

/* Macros */
#define NB_FIRMWARE	(sizeof(FwTable) / sizeof(Firmware))


/* Firmware table */
Firmware FwTable[] = {
	/* 3.55 firmware */
	{
		.version = 35500,
		.devlist = 0x80000000003F6430ULL,
		.patches = {
			     { 0x8000000000169794ULL, 0x480000483C008001ULL },
			     { 0x8000000000169804ULL, 0x480001202F9F0000ULL },
			     { 0x800000000016C564ULL, 0x4800004C4092FEC4ULL },
			     { 0x800000000016C604ULL, 0x600000002FBA0000ULL },
			     { 0x800000000016C910ULL, 0x63BD06664BFFD67DULL },
			     { 0, 0 }
		}
	},
};

#endif	/* _FIRMWARE_H_ */
