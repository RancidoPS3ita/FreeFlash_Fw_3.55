#ifndef H_COMMON
#define H_COMMON

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <psl1ght/lv2.h>

#define HVSC_SYSCALL			811                  	// which syscall to overwrite with hvsc redirect
#define HVSC_SYSCALL_ADDR		0x8000000000195540ULL	// where above syscall is in lv2
#define NEW_POKE_SYSCALL		813                  	// which syscall to overwrite with new poke
#define NEW_POKE_SYSCALL_ADDR	0x8000000000195A68ULL	// where above syscall is in lv2

#define HV_BASE					0x8000000014000000ULL	// where in lv2 to map lv1
#define HV_SIZE					16 * 1024 * 1024		// size of lv1 memory to map/dump
#define DUMP_FILENAME			"/dev_hdd0/lv1.bin"


#endif
