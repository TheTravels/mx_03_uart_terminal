/**
  ******************************************************************************
  * File Name          : terminal.c
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "usart.h"
#include <string.h>
#include "terminal.h"

cache_queue cache;
cache_queue cache1_terminal;
cache_queue cache1_terminal_tx;
cache_queue cache_board;
uint8_t terminal_rx_flag = 0;
//uint8_t aRxBuffer;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
//#if 0
//		HAL_UART_Transmit(&huart2,&aRxBuffer,1,0);
//#else
//		cache.w_tmp = cache.index_w+1;
//		if(cache.w_tmp>=sizeof(cache.buf)) cache.w_tmp=0;
//		cache.buf[cache.w_tmp] = aRxBuffer;
//		if(cache.w_tmp != cache.index_r) cache.index_w=cache.w_tmp;
//#endif
//		HAL_UART_Receive_IT(&huart2,&aRxBuffer,1);
		__HAL_UART_ENABLE_IT(UartHandle, UART_IT_RXNE);
}
 
void terminal2_data_process(uint8_t data)
{
	//if(__HAL_UART_GET_FLAG(&huart2, UART_FLAG_RXNE) == RESET)
	{
			//HAL_UART_Transmit(&huart2,&data,1,0);
			//aRxBuffer = data;
			//data = huart2.Instance->RDR & 0xFF;
			//flag = 1;
#if 0
			cache.w_tmp = cache.index_w+1;
			if(cache.w_tmp>=sizeof(cache.buf)) cache.w_tmp=0;
			cache.buf[cache.w_tmp] = data;
			if(cache.w_tmp != cache.index_r) cache.index_w=cache.w_tmp;
#else
			macro_queue_write(data,cache);
#endif
			//HAL_GPIO_TogglePin(GPIOA, SYS_LED_Pin|SIG_LED_Pin);
	}
}

void terminal1_data_process(uint8_t data)
{
		__asm("CPSID  I");
		macro_queue_write(data,cache1_terminal);
		__asm("CPSIE  I");
		terminal_rx_flag = 1;
		//HAL_GPIO_TogglePin(sign_GPIO_Port, sign_Pin); 
		//HAL_UART_Transmit(&huart2, &data, 1, 1);
}

uint8_t rx_data=0;
static uint8_t busy_flag=0;
int fputc_bak(int c, FILE *f) // printf
{
//	uint8_t data=c;
//	HAL_UART_Transmit(&huart1,&data,1,100);
//	return c;
//	if(busy_flag) return c;
//	__asm("CPSID  I");
//	busy_flag = 1;
//	__asm("CPSIE  I");
#if 1
	uint8_t data=c;
	//HAL_UART_Transmit(&huart2,&data,1,100);
	HAL_UART_Transmit(&huart2,&data,1,100);
#else
	rx_data=c;
	HAL_UART_Transmit_DMA(&huart2, &rx_data, 1);
#endif
//	__asm("CPSID  I");
//	busy_flag = 0;
//	__asm("CPSIE  I");
	return c;
}
int fputc(int c, FILE *f) // printf
{
#if 1
	uint8_t data=c;
	//HAL_UART_Transmit(&huart2,&data,1,100);
	HAL_UART_Transmit(&huart2,&data,1,100);
#else
	uint8_t data=c;
	__asm("CPSID  I");
	macro_queue_write(data,cache1_terminal_tx);
	__asm("CPSIE  I");
#endif
	return c;
}
/* USER CODE END 0 */
//void uart_put_string(char *str)
//{
//	HAL_UART_Transmit(&huart2,(uint8_t *)str, strlen(str),50);
//	//__HAL_UART_ENABLE_IT(&huart2,UART_IT_RXNE);
//	//HAL_UART_Receive_IT(&huart2,&aRxBuffer,1);
//}

