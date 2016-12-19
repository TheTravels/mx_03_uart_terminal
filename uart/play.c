/**
  ******************************************************************************
  * File Name          : play.c
  ******************************************************************************
  */
/* Includes ------------------------------------------------------------------*/
#include "play.h"
#include "cmsis_os.h"

typedef union {
	uint8_t buf[3];
	struct {
		uint8_t r;
		uint8_t g;
		uint8_t b;
	}rgb;
}RGB;

//static RGB rgb_list_l[][8]={
//	{{255,0,0},{205,50,0},{155,100,0},{105,150,0},{55,200,0},{5,250,0},{0,255,50},{0,205,100}},
//	{{255,0,0},{205,50,0},{155,100,0},{105,150,0},{55,200,0},{5,250,0},{0,255,50},{0,205,100}}, // -8
//};
//static RGB rgb_list_h[][8]={
//	{{255,0,0},{205,50,0},{155,100,0},{105,150,0},{55,200,0},{5,250,0},{0,255,50},{0,205,100}}, // +8
//	{{255,0,0},{205,50,0},{155,100,0},{105,150,0},{55,200,0},{5,250,0},{0,255,50},{0,205,100}}, // +16
//};
static RGB rgb_list[3][8]={
	{{0,0,0},{0xFF,0,0},{0xFF,0x7F,0},{0xFF,0xFF,0},{0,0xFF,0},{0,0xFF,0xFF},{0,0,0xFF},{0x8B,0,0xFF}},
	{{0,0,0},{0xFF,0,0},{0xFF,0x7F,0},{0xFF,0xFF,0},{0,0xFF,0},{0,0xFF,0xFF},{0,0,0xFF},{0x8B,0,0xFF}},
	{{0,0,0},{0xFF,0,0},{0xFF,0x7F,0},{0xFF,0xFF,0},{0,0xFF,0},{0,0xFF,0xFF},{0,0,0xFF},{0x8B,0,0xFF}}
};
static const RGB rgb_list_base[8]={
	// ºì³È»ÆÂÌÇàÀ¶×Ï
	{0,0,0},{0xFF,0,0},{0xFF,0x7F,0},{0xFF,0xFF,0},{0,0xFF,0},{0,0xFF,0xFF},{0,0,0xFF},{0x8B,0,0xFF}
};

static note buf[]={
	{VALUE_LOW, 5, TICK_DIV_1, TICK__NULL}, // |
	{VALUE_NOR, 1, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 1, TICK_DIV_1, TICK__NULL}, // |
	{VALUE_NOR, 1, TICK_DIV_1, TICK_DIV_1}, 
	{VALUE_NOR, 3, TICK_DIV_1, TICK__NULL}, // |
	{VALUE_NOR, 2, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 1, TICK_DIV_1, TICK__NULL}, // |
	{VALUE_NOR, 2, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 3, TICK_DIV_1, TICK__NULL}, // |
	{VALUE_NOR, 1, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 1, TICK_DIV_1, TICK__NULL}, // |
	{VALUE_NOR, 3, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 5, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 6, TICK_DIV_1, TICK_MUL_2},
	{VALUE_NOR, 6, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 0, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 6, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 5, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 3, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 3, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 1, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 2, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 1, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 2, TICK_DIV_1, TICK_DIV_1},
	{VALUE_NOR, 3, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 1, TICK_DIV_1, TICK_DIV_1},
	{VALUE_LOW, 6, TICK_DIV_1, TICK__NULL},
	{VALUE_LOW, 6, TICK_DIV_1, TICK_DIV_1},
	{VALUE_LOW, 5, TICK_DIV_1, TICK__NULL},
	{VALUE_NOR, 1, TICK_DIV_1, TICK_MUL_2},
//	{VALUE_NOR, 1, TICK_DIV_1, TICK_DIV_1},
//	{VALUE_NOR, 0, TICK_DIV_1, TICK__NULL},
//	{VALUE_NOR, 0, TICK_DIV_1, TICK__NULL},
};

const uint16_t tick = 500; //ms
//const float tick = 1000.0f/TICK_DIV_1; //ms
static float brightness = 50.0f;
void BrushOutTone(const note *_note, uint8_t r, uint8_t g, uint8_t b)
{
		uint16_t delay = 0;
		uint16_t i=0;
		float brigh=1.0f;
		delay = (tick*(_note->beat+_note->hold))/TICK_DIV_1;
		delay /= 2;
		delay = 256;
		r = 200;
		g = 0;
		b = 0;
		//delay = (_note->beat+_note->hold);
		// +++
		for(i=0; i<delay/2; i++)
		{
				brigh = 2.0f*i/delay;
				BrushOut(20, r*brigh, g*brigh, b*brigh);
				osDelay(2);
		}
		// ---
		for(i=delay; i>0; i--)
		{
				brigh = 1.0f*i/delay;
				BrushOut(20, r*brigh, g*brigh, b*brigh);
				osDelay(2);
		}
}
void BrushOutTest(const note *_note, const float R, const float G, const float B)
{
		uint16_t delay = 0;
		uint16_t i=0;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		float brigh=1.0f;
		float scale = 1.0f;
		float _tick = (float)tick/TICK_DIV_1; //ms
		scale = brightness/256.0f;
		//float R=200.0f,G=100.0f,B=1.0f;
		//delay = 128;
		delay = _tick*_note->beat*2/3+20; //(tick*_note->beat)/TICK_DIV_1/2;
		r = 200;
		g = 100;
		b = 0;
		// +++
		for(i=20; i<delay; i++)
		{
				//brigh = 2.0f*i/delay;
				r = R*i*scale/delay;
				g = G*i*scale/delay;
				b = B*i*scale/delay;
				BrushOut(20, r, g, b);
				osDelay(1);
				//HAL_Delay(2);
		}
		// ---
		//delay = 256;
		//delay = (tick*(_note->beat+_note->hold))/TICK_DIV_1;
		delay = _tick*_note->beat/3+_tick*_note->hold; //(uint16_t)(tick*(_note->beat/2+_note->hold))+1;
		delay += 50;
		for(i=delay; i>50; i--)
		{
				r = R*i*scale/delay;
				g = G*i*scale/delay;
				b = B*i*scale/delay;
				BrushOut(20, r, g, b);
				osDelay(1);
				//HAL_Delay(4);
		}
}
void BrushOutTest_bak2(const note *_note, const float R, const float G, const float B)
{
		uint16_t delay = 0;
		uint16_t i=0;
		uint8_t r;
		uint8_t g;
		uint8_t b;
		float brigh=1.0f;
		float _tick = 4.0f;//(float)tick/TICK_DIV_1; //ms
		//float R=200.0f,G=100.0f,B=1.0f;
		delay = 128;
		r = 200;
		g = 100;
		b = 0;
		// +++
		for(i=0; i<delay; i++)
		{
				r = R*i/delay;
				g = G*i/delay;
				b = B*i/delay;
				BrushOut(20, r, g, b);
				osDelay(2);
		}
		// ---
		delay = 500;
		for(i=delay; i>0; i--)
		{
				r = R*i/delay;
				g = G*i/delay;
				b = B*i/delay;
				BrushOut(20, r, g, b);
				osDelay(2);
		}
}
typedef void (*send)(uint8_t data[3]);
//void my_play(send _send)
//void my_play(void (*_Outone)(uint8_t data[3]))
void my_play(void)
{
		uint16_t len = sizeof(buf)/sizeof(note);
		uint16_t i=0;
		const RGB *rgb=0;
		note *_note=0;
		uint16_t delay = 0;
		for(i=0; i<len; i++)
		{
				_note = &buf[i];
				//_note->tone = 6;
				rgb = &rgb_list[_note->toneH][_note->tone];
				//rgb = &rgb_list_base[_note->tone];
#if 0
				//_Outone(rgb->buf);
				BrushOut(20, rgb->rgb.r, rgb->rgb.g, rgb->rgb.b);
				delay = (tick*(_note->beat+_note->hold))/TICK_DIV_1;
				osDelay(delay);
#else
			//while(1)BrushOutTone(_note, rgb->rgb.r, rgb->rgb.g, rgb->rgb.b);
			//while(1) BrushOutTest(_note, rgb->rgb.r, rgb->rgb.g, rgb->rgb.b);
			//BrushOutTest(_note, 0, 000, 200);
			//printf("\r\n%s  %02X %02X %02X\n", __func__, rgb->rgb.r, rgb->rgb.g, rgb->rgb.b);
			BrushOutTest(_note, rgb->rgb.r, rgb->rgb.g, rgb->rgb.b);
#endif
		}
}


