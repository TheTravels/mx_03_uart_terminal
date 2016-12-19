/**
  ******************************************************************************
  * File Name          : play.h
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#ifndef _PLAY_H_
#define _PLAY_H_

#include <stdint.h>
#include <string.h>
#include <stdio.h>

#define  TICK__NULL     0
#define  TICK_MUL_4     1023
#define  TICK_MUL_3     767
#define  TICK_MUL_2     511
#define  TICK_DIV_1     255
#define  TICK_DIV_2     127
#define  TICK_DIV_4     63
#define  TICK_DIV_8     31
#define  TICK_DIV_16    15
#define  TICK_DIV_32    7

#define  VALUE_LOW      0
#define  VALUE_NOR      1
#define  VALUE_HIGH     2

typedef struct{
		uint8_t toneH:4;
		uint8_t tone:4;
		uint8_t beat;  // tick/256;
		uint8_t hold;  // tick/128
}note;

typedef struct{
		uint8_t versions;
		uint16_t len;
		note *data;
}notes;

#endif //_PLAY_H_
