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
 * @file flash.c
 *
 * Common bootloader logic.
 *
 * Aside from the header includes below, this file should have no board-specific logic.
 */
#include <stdint.h>
#include <stdio.h>
#include "stm32f0xx_hal.h"
#include "hw_config.h"
#include "../Boot/flash.h"
#include "../Boot/bl.h"
#include "main.h"
#include "../uart/terminal.h"

#if 0
struct boardinfo board_info = {
	.board_type	= BOARD_TYPE,
	.board_rev	= 0,
	.fw_size	= APP_SIZE_MAX,

	.systick_mhz	= OSC_FREQ,
};
#else
struct boardinfo board_info = {
	BOARD_TYPE,
	0,
	APP_SIZE_MAX,
	OSC_FREQ,
};
#endif


uint32_t flash_func_sector_size(unsigned sector)
{
	if (sector < BOARD_FLASH_SECTORS)
		return FLASH_SECTOR_SIZE;
	return 0;
}
static FLASH_EraseInitTypeDef EraseInitStruct;
uint32_t Address = 0, PAGEError = 0;
void flash_func_erase_sector(unsigned sector)
{
#if 0
	if (sector < BOARD_FLASH_SECTORS)
		flash_erase_page(APP_LOAD_ADDRESS + (sector * FLASH_SECTOR_SIZE));
#else
	if (sector >= BOARD_FLASH_SECTORS) return;
  /* Fill EraseInit structure*/
  EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
  //EraseInitStruct.PageAddress = APP_LOAD_ADDRESS;
	EraseInitStruct.PageAddress = APP_LOAD_ADDRESS + (sector * FLASH_SECTOR_SIZE);
  EraseInitStruct.NbPages     = 1;//(FLASH_USER_END_ADDR - FLASH_USER_START_ADDR) / FLASH_PAGE_SIZE;
  if (HAL_FLASHEx_Erase(&EraseInitStruct, &PAGEError) != HAL_OK)
  {
    /* Infinite loop */
		;//HAL_FLASH_Lock();
  }
#endif
}

void flash_func_write_word(uint32_t address, uint32_t word)
{
	//flash_program_word(address + APP_LOAD_ADDRESS, word);
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word);
}

uint32_t flash_func_read_word(uint32_t address)
{
	return *(uint32_t *)(address + APP_LOAD_ADDRESS);
}

uint32_t flash_func_read_otp(uint32_t address)
{
	return 0;
}
uint32_t flash_func_read_sn(uint32_t address)
{
    return 0;
}

void flash_unlock(void)
{
		HAL_FLASH_Unlock();
}
void flash_lock(void)
{
		HAL_FLASH_Lock();
}

//-------------------------------- LED
void led_on(unsigned led)
{
		HAL_GPIO_WritePin(sign_GPIO_Port, sign_Pin, GPIO_PIN_RESET);
}

void led_off(unsigned led)
{
		HAL_GPIO_WritePin(sign_GPIO_Port, sign_Pin, GPIO_PIN_SET);
}

void led_toggle(unsigned led)
{
		HAL_GPIO_TogglePin(sign_GPIO_Port, sign_Pin); 
}

//-----------------------

void uart_cinit(void *config)
{
}

void uart_cfini(void)
{
//	usart_disable(usart);
}

int uart_cin(void)
{
//	int c = -1;

//	if (USART_SR(usart) & USART_SR_RXNE)
//		c = usart_recv(usart);
//	return c;
	uint8_t _byte=0;
	int ret=-1;
	//printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
	//__asm("CPSID  I");
	ret = cache_queue_read_byte(&cache_board, &_byte);
	//__asm("CPSIE  I");
	if(0==ret)
	{
			ret = _byte;
			//printf("\r\n%s  %d [%d]\r\n", __func__, ret, _byte);
	}
	else
	{
			osDelay(1);
	}
	return ret;
}
extern void boot_send(const char *data, const uint16_t len);
void uart_cout(uint8_t *buf, unsigned len)
{
//	while (len--)
//		usart_send_blocking(usart, *buf++);
		boot_send(buf, len);
}

/* StartBootTask function */
void StartBootTask(void const * argument)
{
  /* USER CODE BEGIN StartBootTask */
  /* Infinite loop */
	printf("\r\n%s  %s [%s]\r\n", __func__, __DATE__, __TIME__);
	osDelay(100);
	if(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin))
	{
			printf("\r\n%s  %s KEY UP\r\n", __func__, __DATE__);
			// vTaskSuspendAll();
			// __asm("CPSID  I");
			// boot
	}
	printf("\r\n%s  %s KEY DOWN\r\n", __func__, __DATE__);
	osDelay(10);
	if(HAL_GPIO_ReadPin(KEY_GPIO_Port, KEY_Pin))
	{
			printf("\r\n%s  %s KEY UP\r\n", __func__, __DATE__);
			// vTaskSuspendAll();
			// __asm("CPSID  I");
			// boot
	}
  for(;;)
  {
		/* run the bootloader, possibly coming back after the timeout */
		bootloader(0);
		/* look to see if we can boot the app */
		jump_to_app();
    osDelay(1);
  }
  /* USER CODE END StartBootTask */
}

