/**
  ******************************************************************************
  * File Name          : terminal.h
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#ifndef _TERMINAL_H_
#define _TERMINAL_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include "usart.h"

#define CACHE_QUEUE_LEN    128
typedef struct{
		uint16_t index_r;
		uint16_t index_w;
		uint16_t w_tmp;
		uint16_t r_tmp;
		uint8_t buf[CACHE_QUEUE_LEN];
}cache_queue;

#define init_queue(_queue) do{ \
		memset(&_queue, 0, sizeof(cache_queue)); \
		_queue.index_r = sizeof(_queue.buf)-1; \
}while(0);
#define macro_queue_read(index,buf,_queue)  do{ \
			memset(&buf, 0, sizeof(buf)); \
			for(index=0; index<sizeof(buf); index++) \
			{ \
					_queue.r_tmp = _queue.index_r+1; \
					if(_queue.r_tmp>=sizeof(_queue.buf)) _queue.r_tmp=0; \
					if(_queue.r_tmp == _queue.index_w) break;  /* empty */ \
					buf[index] = _queue.buf[_queue.r_tmp]; \
					_queue.index_r = _queue.r_tmp; \
			} \
}while(0);
#if 0
#define macro_queue_write(_byte,_queue)  do{ \
			_queue.w_tmp = _queue.index_w+1; \
			if(_queue.w_tmp>=sizeof(_queue.buf)) _queue.w_tmp=0; \
			_queue.buf[_queue.w_tmp] = _byte; \
			if(_queue.w_tmp != _queue.index_r) _queue.index_w=_queue.w_tmp; \
}while(0);
#else
#define macro_queue_write(_byte,_queue)  do{ \
			_queue.buf[_queue.index_w] = _byte; \
			_queue.w_tmp = _queue.index_w+1; \
			if(_queue.w_tmp>=sizeof(_queue.buf)) _queue.w_tmp=0; \
			if(_queue.w_tmp != _queue.index_r) _queue.index_w=_queue.w_tmp; \
}while(0);
#endif

extern cache_queue cache;
extern cache_queue cache1_terminal;
extern cache_queue cache1_terminal_tx;
extern cache_queue cache_board;
extern uint8_t terminal_rx_flag;

static __inline uint16_t cache_queue_read_bak(cache_queue* _cache, uint8_t buf[], const uint16_t buf_size)
{
		uint16_t index=0;
		memset(buf, 0, buf_size);
#if 0
			//memset(&buf, 0, sizeof(buf));
			for(index=0; index<buf_size; index++)
			{
					cache.r_tmp = cache.index_r+1;
					if(cache.r_tmp>=sizeof(cache.buf)) cache.r_tmp=0;
					if(cache.r_tmp == cache.index_w) break;  // empty
					buf[index] = cache.buf[cache.r_tmp];
					cache.index_r = cache.r_tmp;
			}
#else
		for(index=0; index<buf_size; index++)
		{
				_cache->r_tmp = _cache->index_r+1;
				if(_cache->r_tmp>=sizeof(_cache->buf)) _cache->r_tmp=0;
				if(_cache->r_tmp == _cache->index_w) break;  // empty
				buf[index] = _cache->buf[_cache->r_tmp];
				_cache->index_r = _cache->r_tmp;
		}
#endif
		return index;
}
static __inline uint16_t cache_queue_read(cache_queue* _cache, uint8_t buf[], const uint16_t buf_size)
{
		uint16_t index=0;
		memset(buf, 0, buf_size);
		for(index=0; index<buf_size; index++)
		{
				buf[index] = _cache->buf[_cache->index_r];
				_cache->r_tmp = _cache->index_r+1;
				if(_cache->r_tmp>=sizeof(_cache->buf)) _cache->r_tmp=0;
				if(_cache->index_r == _cache->index_w) break;  // empty
				_cache->index_r = _cache->r_tmp;
		}
		return index;
}


static __inline int cache_queue_read_byte(cache_queue* _cache, uint8_t* _byte)
{
		int ret=-1;
		__asm("CPSID  I");
		_cache->r_tmp = _cache->index_r+1;
		if(_cache->r_tmp>=sizeof(_cache->buf)) _cache->r_tmp=0;
		if(_cache->r_tmp != _cache->index_w)
		{
				*_byte = _cache->buf[_cache->r_tmp];
				_cache->index_r = _cache->r_tmp;
				ret = 0;
		}
		__asm("CPSIE  I");
		return ret;
}


extern void terminal2_data_process(uint8_t data);
extern void terminal1_data_process(uint8_t data);

#endif //_TERMINAL_H_
