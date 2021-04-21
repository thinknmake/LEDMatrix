/*
 * File:   LEDMatrix.h
 * Author: Nilesh Mundphan
 * Created on FEB 27, 2021, 11:03 PM
 * Private Use Only
 */

#ifndef EM_GFX_H
#define EM_GFX_H

#include "Arduino.h"

#if defined (__AVR_ATmega328P__)
/*
	#define HC595_PORT		PORTB
	#define HC595_DDR		DDRB

	#define HC595_DS_POS	PB5      //Data pin (DS) pin location

	#define HC595_SH_CP_POS PB3      //Shift Clock (SH_CP) pin location
	#define HC595_ST_CP_POS PB4      //Store Clock (ST_CP) pin location

	#define P10_CH_PORT		PORTB
	#define P10_CH_DDR		DDRB

	#define P10_CH_A_POS	PB1
	#define P10_CH_B_POS	PB2

	#define P10_EN_PORT		PORTB
	#define P10_EN_DDR		DDRB
	#define P10_EN_POS		PB0
*/
	#define HC595_PORT		PORTC
	#define HC595_DDR		DDRC

	#define HC595_DS_POS	PC5      //Data pin (DS) pin location

	#define HC595_SH_CP_POS PC3      //Shift Clock (SH_CP) pin location
	#define HC595_ST_CP_POS PC4      //Store Clock (ST_CP) pin location

	#define P10_CH_PORT		PORTC
	#define P10_CH_DDR		DDRC

	#define P10_CH_A_POS	PC1
	#define P10_CH_B_POS	PC2

	#define P10_EN_PORT		PORTC
	#define P10_EN_DDR		DDRC
	#define P10_EN_POS		PC0

	//Low level macros to change data (DS)lines
	#define HC595DataHigh() (HC595_PORT|=(1<<HC595_DS_POS))
	#define HC595DataLow() (HC595_PORT&=(~(1<<HC595_DS_POS)))
#endif

#if !defined(__INT_MAX__) || (__INT_MAX__ > 0xFFFF)
 #define pgm_read_pointer(addr) ((void *)pgm_read_dword(addr))
#else
 #define pgm_read_pointer(addr) ((void *)pgm_read_word(addr))
#endif

#define _swap_int16_t(a, b) { int16_t t = a; a = b; b = t; }
//#define ssd1306_swap(a, b) { int16_t t = a; a = b; b = t; }

#define GFX_SCREEN_WIDTH    (32*3)
#define GFX_SCREEN_HEIGHT 	16
#define GFX_CHAR_SPACING 	2

#define DISPLAY_WIDTH		(32*3)
#define DISPLAY_HEIGHT		16

#define BUFF_SIZE 			256

#define BLACK 0
#define WHITE 1
#define INVERSE 2

typedef struct { // Data stored PER GLYPH
        uint16_t bitmapOffset;     // Pointer into GFXfont->bitmap
        uint8_t  width, height;    // Bitmap dimensions in pixels
        uint8_t  xAdvance;         // Distance to advance cursor (x axis)
        int8_t   xOffset, yOffset; // Dist from cursor pos to UL corner
} GFXglyph;

typedef struct { // Data stored for FONT AS A WHOLE:
        uint8_t  *bitmap;      // Glyph bitmaps, concatenated
        GFXglyph *glyph;       // Glyph array
        uint8_t   first, last; // ASCII extents
        uint8_t   yAdvance;    // Newline distance (y axis)
} GFXfont;

class LEDMatrix
{
	public:
		LEDMatrix();
		LEDMatrix(uint8_t en,uint8_t a,uint8_t b,uint8_t sh,uint8_t st,uint8_t ds);
		void init();
		void clear(void);
		void HC595Pulse(void);
		void HC595Latch(void);
		void HC595Write(uint8_t data);


		void EMGfxInit(void);
		void EMGfxSelCh(uint8_t ch);
		void EMGfxDispOff(void);
		void EMGfxDispOn(void);
		void EMGfxClear(void);
		void writePixel(int16_t x, int16_t y, uint16_t color);
		void writePixel1(int16_t x, int16_t y, uint16_t color);
		void brightness(int16_t br);		

		void update_disp(void);
		void update_tbuff(uint8_t *buf ,size_t len);
		
		void setCursor(int16_t x, int16_t y);
		size_t write(uint8_t c);
		int drawChar(int16_t x, int16_t y, unsigned char c,uint16_t color, uint16_t bg, uint8_t size);
		void setFont(const GFXfont *f);
		void writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color);
		void writeFastVLine(int16_t x, int16_t y,int16_t h, uint16_t color);
		void writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color);
		void disp_init();
		void DisplayClear();
		void cpy_buff(int mov);
		void print_str(char* str);
		void print_line(char *str);
		int16_t   _width, _height, cursor_x, cursor_y;
		uint16_t brightnessval;
		uint16_t brval; 
	private:
		uint16_t textcolor, textbgcolor;
		uint8_t textsize,rotation;
		boolean wrap,_cp437; // If set, use correct CP437 charset (default is off)
		GFXfont   *gfxFont;
		uint8_t p10_pins[6]={0,0,0,0,0,0};
};

#endif
