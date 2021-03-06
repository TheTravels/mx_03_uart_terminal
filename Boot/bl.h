/****************************************************************************
 *
 *   Copyright (c) 2012-2014 PX4 Development Team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name PX4 nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/**
 * @file bl.h
 *
 * Common bootloader definitions.
 */

#pragma once

#include <stdint.h>
/***************************************************************************** 
 * Generic bootloader functions.
 */

/* board info forwarded from board-specific code to booloader */
struct boardinfo {
	uint32_t	board_type;
	uint32_t	board_rev;
	uint32_t	fw_size;
	uint32_t	systick_mhz;		/* systick input clock */

};// __attribute__((packed));

extern struct boardinfo board_info;

extern void jump_to_app(void);
extern void bootloader(unsigned timeout);
extern void delay(unsigned msec);

#define BL_WAIT_MAGIC	0x19710317		/* magic number in PWR regs to wait in bootloader */

/* generic timers */
#define NTIMERS		4
#define TIMER_BL_WAIT	0
#define TIMER_CIN	1
#define TIMER_LED	2
#define TIMER_DELAY	3
extern volatile unsigned timer[NTIMERS];	/* each timer decrements every millisecond if > 0 */

/* generic receive buffer for async reads */
extern void buf_put(uint8_t b);
extern int buf_get(void);

/***************************************************************************** 
 * Chip/board functions.
 */

/* LEDs */
#define LED_ACTIVITY	1
#define LED_BOOTLOADER	2

#ifdef BOOT_DELAY_ADDRESS
# define BOOT_DELAY_SIGNATURE1	0x92c2ecff
# define BOOT_DELAY_SIGNATURE2	0xc5057d5d
# define BOOT_DELAY_MAX		30
#endif

extern void led_on(unsigned led);
extern void led_off(unsigned led);
extern void led_toggle(unsigned led);

/* flash helpers from main_*.c */
extern uint32_t flash_func_sector_size(unsigned sector);
extern void flash_func_erase_sector(unsigned sector);
extern void flash_func_write_word(uint32_t address, uint32_t word);
extern uint32_t flash_func_read_word(uint32_t address);
extern uint32_t flash_func_read_otp(uint32_t address);
extern uint32_t flash_func_read_sn(uint32_t address);

/*****************************************************************************
 * Interface in/output.
 */

extern void cinit(void *config);
extern void cfini(void);
extern int cin(void);
extern void cout(uint8_t *buf, unsigned len);
