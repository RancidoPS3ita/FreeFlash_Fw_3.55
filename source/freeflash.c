
/*
 * freeflash.c -- FreeFlash (Writable Flash)
 *
 * Copyright (C) Miguel Boton (Waninkoko)
 *
 * This software is distributed under the terms of the GNU General Public
 * License ("GPL") version 3, as published by the Free Software Foundation.
 * 
 * Ported to the firmware 3.55 by Rancid(o)
 * Included lv1dumper (from flukes1) to poke lv2 without the system shutting down
 */


#include "common.h"
#include "peek_poke.h"
#include "hvcall.h"
#include "mm.h"

#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#include "image.h"
#include "syscalls.h"
#include "system.h"
#include "video.h"
#include <sysmodule/sysmodule.h>

#include "firmware.h"

#include "imgError.bin.h"
#include "imgOk.bin.h"

#define SLEEP_TIME		6

/* Flash strings */
#define STR_FLASH		0x5F666C6173680000ULL
#define STR_FLASHX		0x5F666C6173685800ULL
#define STR_FFLASH		0x5F66666C61736800ULL

/* Mount parameters */
#define MOUNT_DEVICE		"CELL_FS_IOS:BUILTIN_FLSH1"
#define MOUNT_FILESYSTEM	"CELL_FS_FAT"
#define MOUNT_POINT		"/dev_flash"


u64 mmap_lpar_addr;

int map_lv1() {
	int result = lv1_undocumented_function_114(0, 0xC, HV_SIZE, &mmap_lpar_addr);
	if (result != 0) {
		
		return 0;
	}
	
	result = mm_map_lpar_memory_region(mmap_lpar_addr, HV_BASE, HV_SIZE, 0xC, 0);
	if (result) {
		
		return 0;
	}
	
	return 1;
}

void unmap_lv1() {
	if (mmap_lpar_addr != 0)
		lv1_undocumented_function_115(mmap_lpar_addr);
}

void dump_lv1() {
	if (mmap_lpar_addr == 0)
		return; // lv1 not mapped
	
	FILE *f = fopen(DUMP_FILENAME, "wb");
	u64 quad;
	for (u64 i = (u64)HV_BASE; i < HV_BASE + HV_SIZE; i += 8) {
		quad = lv2_peek(i);
		fwrite(&quad, 8, 1, f);
	}
	fclose(f);
}

void patch_lv2_protection() {
	// changes protected area of lv2 to first byte only
	lv1_poke(0x363a78, 0x0000000000000001ULL);
	lv1_poke(0x363a80, 0xe0d251b556c59f05ULL);
	lv1_poke(0x363a88, 0xc232fcad552c80d7ULL);
	lv1_poke(0x363a90, 0x65140cd200000000ULL);
}


/* Console firmware */
static Firmware *fw = NULL;

/* Resources */
static PngDatas images[2];


static s32 Modules_Load(void)
{
	/* Load PNGDEC module */
	return SysLoadModule(SYSMODULE_PNGDEC);
}

static s32 Resources_Load(void)
{
	s32 ret;

	/* Load images */
	ret = Image_LoadPNG(&images[0], imgError_bin, sizeof(imgError_bin));
	if (ret)
		return ret;

	ret = Image_LoadPNG(&images[1], imgOk_bin, sizeof(imgOk_bin));
	if (ret)
		return ret;

	return 0;
}

static s32 Fw_CheckVersion(void)
{
	u64 ver;
	u32 i;

	/* TODO: Get firmware version without reading a fixed address */
	
	/* Read firmware version */
	ver = 35500; // No peek, set a static number, dirty but work

	/* Find firmware */
	for (i = 0;  i < NB_FIRMWARE; i++) {
		Firmware *firm = &FwTable[i];

		/* Version match */
		if (firm->version == ver) {
			fw = firm;
			return 0;
		}
	}

	return -EINVAL;
}

static void Fw_Lv2Patch(void)
{
	u32 i;

	/* Apply patch table */
	for (i = 0;; i++) {
		LV2Patch *patch = &fw->patches[i];

		/* Table end */
		if (!patch->address)
			break;

		/* Apply patch */
		LV2_Poke(patch->address, patch->value);
	}
}

static s32 Devlist_Find(u64 val)
{
	u64 ptr = fw->devlist;
	u32 i;

	/* Find loop */
	for (i = 0; i < 16; i++) {
		u64 v;

		/* Read value */
		v = LV2_Peek(ptr);

		/* Value match */
		if (v == val)
			return 0;

		/* Next address */
		ptr += 0x100;
	}

	return -EINVAL;
}

static s32 Devlist_Patch(u64 orig, u64 new)
{
	u64 ptr = fw->devlist;
	s32 ret = -EINVAL;

	u32 i;

	/* Patch loop */
	for (i = 0; i < 16; i++) {
		u64 v;

		/* Read value */
		v = LV2_Peek(ptr);

		/* Value match */
		if (v == orig) {
			/* Patch address */
			LV2_Poke(ptr, new);

			/* Return success */
			ret = 0;
		}

		/* Next address */
		ptr += 0x100;
	}

	return ret;
}

static void Image_Draw(PngDatas *image)
{
	s32 x, y;

	/* Wait for the previous flip */
	Video_WaitFlip();

	/* Clear screen */
	Video_Clear(COLOR_XRGB_BLACK);

	/* Calculate coordinates */
	x = (Video_GetWidth()  - image->width)  / 2;
	y = (Video_GetHeight() - image->height) / 2;

	/* Draw image */
	Image_DrawPNG(image, x, y);

	/* Flip buffers */
	Video_Flip();
}


s32 main(s32 argc, const char *argv[])
{
	

	
	install_new_poke();

	
	if (!map_lv1()) {
		remove_new_poke();
		exit(0);
	} else {
		
		patch_lv2_protection();

		
		remove_new_poke();
	}
{
	s32 ret;

	/* Load modules */
	ret = Modules_Load();
	if (ret)
		goto out;

	/* Initialize system */
	ret = System_Init();
	if (ret)
		goto out;

	/* Initialize video */
	ret = Video_Init();
	if (ret)
		goto out;

	/* Load resources */
	ret = Resources_Load();
	if (ret)
		goto out;

	/* Check firmware version */
	ret = Fw_CheckVersion();
	if (ret)
		goto error;

	/* Find "fflash" */
	ret = Devlist_Find(STR_FFLASH);
	if (!ret)
		goto error;

	/* Patch LV2 */
	Fw_Lv2Patch();

	/* Rename flash */
	ret = Devlist_Patch(STR_FLASH, STR_FLASHX);
	if (ret)
		goto error;

	/* Mount flash */
	ret = LV2_Mount(MOUNT_DEVICE, MOUNT_FILESYSTEM, MOUNT_POINT, 0, 0, 0, NULL, 0);

	/* Rename new flash to "dev_fflash" */
	Devlist_Patch(STR_FLASH, STR_FFLASH);

	/* Restore flash */
	Devlist_Patch(STR_FLASHX, STR_FLASH);

	/* Mount error */
	if (ret)
		goto error;

	/* Draw OK image */
	Image_Draw(&images[1]);
	sleep(SLEEP_TIME);

	goto out;



error:
	/* Draw error image */
	Image_Draw(&images[0]);
	sleep(SLEEP_TIME);

out:
	return 0;
}
}
