/*
 * File:   LEDMatrix.cpp
 * Author: Nilesh Mundphan
 * Created on FEB 27, 2021, 11:03 PM
 * Private Use Only
 */

#include "Arduino.h"
#include "LEDMatrix.h"

volatile uint8_t buffer[DISPLAY_WIDTH*DISPLAY_HEIGHT /8];
volatile uint8_t tbuffer[8096] = {0};
char font[10];

LEDMatrix::LEDMatrix(){}

LEDMatrix::LEDMatrix(uint8_t en,uint8_t a,uint8_t b,uint8_t sh,uint8_t st,uint8_t ds)
{
	p10_pins[0]=en;
	p10_pins[1]=a;
	p10_pins[2]=b;
	p10_pins[3]=sh;
	p10_pins[4]=st;
	p10_pins[5]=ds;
	brightnessval=50;
}

void LEDMatrix::init()
{
	#if defined (__AVR_ATmega328P__)
		HC595_DDR|=((1<<HC595_SH_CP_POS)|(1<<HC595_ST_CP_POS)|(1<<HC595_DS_POS));
		P10_CH_DDR|=((1<<P10_CH_A_POS)|(1<<P10_CH_B_POS));
		P10_EN_DDR|=(1<<P10_EN_POS);
	#else
		uint8_t i;
		for (i=1;i<6;i++)
		{
			pinMode(p10_pins[i],OUTPUT);
		}
	#endif
	DisplayClear();
	EMGfxInit();
}

void LEDMatrix::clear(void)
{

}


 void LEDMatrix::HC595Pulse(void)
 {
	#if defined (__AVR_ATmega328P__)
		HC595_PORT|=(1<<HC595_SH_CP_POS);//HIGH
		HC595_PORT&=(~(1<<HC595_SH_CP_POS));//LOW
	#else
		digitalWrite(p10_pins[3],HIGH);
		digitalWrite(p10_pins[3],LOW);
	#endif
 }

 //Sends a clock pulse on ST_CP line
 void LEDMatrix::HC595Latch(void)
 {
	#if defined (__AVR_ATmega328P__)
		HC595_PORT|=(1<<HC595_ST_CP_POS);//HIGH
		_delay_loop_1(1);
		HC595_PORT&=(~(1<<HC595_ST_CP_POS));//LOW
		_delay_loop_1(1);
	#else
		digitalWrite(p10_pins[4],LOW);
		digitalWrite(p10_pins[4],HIGH);
 	    //delayMicroseconds(1);
		digitalWrite(p10_pins[4],LOW);
		//delayMicroseconds(1);
	#endif
 }

 void LEDMatrix::HC595Write(uint8_t data)
 {
	 uint8_t i;
	 //Send each 8 bits serially
	 //Order is MSB first
	 for(i=0;i<8;i++)
	 {
		 //Output the data on DS line according to the
		 //Value of MSB
		 digitalWrite(p10_pins[3],LOW);
		 
         if(data & 0x80)
		 {
			 //MSB is 1 so output high
			 //HC595DataHigh();
			#if defined (__AVR_ATmega328P__)
				HC595DataHigh();
			#else
				digitalWrite(p10_pins[5],HIGH);
			#endif
		}
		 else
		 {
			 //MSB is 0 so output high
			#if defined (__AVR_ATmega328P__)
				HC595DataLow();
			#else
				digitalWrite(p10_pins[5],LOW);
			#endif
			 //HC595DataLow();
			 //digitalWrite(p10_pins[5],LOW);
		 }
		 digitalWrite(p10_pins[3],HIGH);
		 //HC595Pulse();  //Pulse the Clock line
		 data=data<<1;  //Now bring next bit at MSB position

	 }

	 //Now all 8 bits have been transferred to shift register
	 //Move them to output latch at one
	 //HC595Latch();
 }


void LEDMatrix::EMGfxInit(void)
{
    EMGfxSelCh(3);
}

void LEDMatrix::EMGfxSelCh(uint8_t ch)
{
	#if defined (__AVR_ATmega328P__)
		P10_CH_PORT&=(~((1<<P10_CH_A_POS)|(1<<P10_CH_B_POS)));
	#else
		digitalWrite(p10_pins[1],LOW);
		digitalWrite(p10_pins[2],LOW);
	#endif
	switch(ch)
	{
		case 0:
			break;
		case 1:
			#if defined (__AVR_ATmega328P__)
				P10_CH_PORT|=(1<<P10_CH_A_POS);
			#else
				digitalWrite(p10_pins[1],HIGH);
			#endif
			break;
		case 2:
			#if defined (__AVR_ATmega328P__)
				P10_CH_PORT|=(1<<P10_CH_B_POS);
			#else
				digitalWrite(p10_pins[2],HIGH);
			#endif
			break;
		case 3:
			#if defined (__AVR_ATmega328P__)
				P10_CH_PORT|=((1<<P10_CH_A_POS)|(1<<P10_CH_B_POS));
			#else
				digitalWrite(p10_pins[1],HIGH);
				digitalWrite(p10_pins[2],HIGH);
			#endif
			break;
	}
}

void LEDMatrix::brightness(int16_t br){
	brightnessval=map(br,0,100,0,1023);
	brval=br;
}

void LEDMatrix::EMGfxDispOff(void)
{
    #if defined (__AVR_ATmega328P__)
        P10_EN_PORT&=~(1<<P10_EN_POS);
    #else
        pinMode(p10_pins[0], OUTPUT);
        digitalWrite(p10_pins[0],LOW);
        //analogWrite(p10_pins[0], 0);
    #endif
}

void LEDMatrix::EMGfxDispOn(void)
{
	#if defined (__AVR_ATmega328P__)
		P10_EN_PORT|=(1<<P10_EN_POS);
	#else
		//digitalWrite(p10_pins[0],HIGH);
	    #if defined(__arm__)
		    pinMode(p10_pins[0], PWM);
            pwmWrite(p10_pins[0], brightnessval);
	    #else
            //analogWrite(p10_pins[0], brightnessval);
            digitalWrite(p10_pins[0],HIGH);
        #endif        
    #endif
}

void LEDMatrix::writePixel(int16_t x, int16_t y, uint16_t color)
{
	if ((x < 0) || (y < 0) || (y >= GFX_SCREEN_HEIGHT))
    return;

    tbuffer[x*GFX_SCREEN_HEIGHT/8+ (y/8)] |=  (1 << (y%8));
}

void LEDMatrix::update_tbuff(uint8_t *buf,size_t len)
{
        memset((char*)tbuffer, 0x00, sizeof(tbuffer));
    
	for(int i=0;i<len;i++)
	{
		tbuffer[i]=buf[i];
	}
}

void LEDMatrix::cpy_buff(int mov)
{
        for(int y=0;y<GFX_SCREEN_HEIGHT;y++){
                for(int x=0;x<(GFX_SCREEN_WIDTH+mov);x++){
                        uint8_t temp=tbuffer[(x + mov)*GFX_SCREEN_HEIGHT/8+ (y/8)];
                        if(temp & (1 << (y%8)))
                        {
                                writePixel1(x,y,1);
                                //printf("X: %d Y: %d XT: %d\n",x,y,x+mov);     
                        }
                }
        }
}


void LEDMatrix::writePixel1(int16_t x, int16_t y, uint16_t color)
{
	uint8_t xx;
	uint8_t d;
	uint8_t dd;

	if( x<0 || x>=DISPLAY_WIDTH || y<0 || y>=DISPLAY_HEIGHT) return;

	x=DISPLAY_WIDTH-x-1;

	xx=x/8;
	d=buffer[xx*16+y];
	dd=x%8;

	dd=7-dd;
	d &=~(1<<dd);
	buffer[xx*16+y]=d;
}


void LEDMatrix::EMGfxClear(void)
{
	uint8_t i;
	for(i=0;i<BUFF_SIZE;i++)
		buffer[i]=0xff;
}

//Timer Setup
void LEDMatrix::update_disp(void)
{
	static uint8_t ch=0;
  	uint8_t i;
	for(i=0; i < (DISPLAY_WIDTH/2) ; i++)
	{
		HC595Write(buffer[i*4+ch]);
	}

	EMGfxDispOff();
	
	HC595Latch();
	
	EMGfxSelCh(3-ch);
	EMGfxDispOn();
	ch++;
	if(ch==4)
	  ch=0;
}

void LEDMatrix::DisplayClear()
{
    memset((char*)buffer, 0xff, sizeof(buffer));
}

void LEDMatrix::disp_init()
{
    _width    = 2048;
    _height   = GFX_SCREEN_HEIGHT;
    rotation  = 0;
    cursor_y  = cursor_x    = 0;
    textsize  = 1;
    textcolor = textbgcolor = 0xFFFF;
    wrap      = true;
    _cp437    = false;
    gfxFont   = NULL;

}



void LEDMatrix::writeLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1,uint16_t color) {
    int16_t steep = abs(y1 - y0) > abs(x1 - x0);
    if (steep) {
        _swap_int16_t(x0, y0);
        _swap_int16_t(x1, y1);
    }

    if (x0 > x1) {
        _swap_int16_t(x0, x1);
        _swap_int16_t(y0, y1);
    }

    int16_t dx, dy;
    dx = x1 - x0;
    dy = abs(y1 - y0);

    int16_t err = dx / 2;
    int16_t ystep;

    if (y0 < y1) {
        ystep = 1;
    } else {
        ystep = -1;
    }

    for (; x0<=x1; x0++) {
        if (steep) {
            writePixel(y0, x0, color);
        } else {
            writePixel(x0, y0, color);
        }
        err -= dy;
        if (err < 0) {
            y0 += ystep;
            err += dx;
        }
    }
}

void LEDMatrix::writeFastVLine(int16_t x, int16_t y,int16_t h, uint16_t color) {
	writeLine(x, y, x, y+h-1, color);
}

void LEDMatrix::writeFillRect(int16_t x, int16_t y, int16_t w, int16_t h,uint16_t color) {
    int16_t i;
    for (x; i<x+w; i++) {
        writeFastVLine(i, y, h, color);
    }
}


void LEDMatrix::setFont(const GFXfont *f) {
    if(f) {            // Font struct pointer passed in?
        if(!gfxFont) { // And no current font struct?
            // Switching from classic to new font behavior.
            // Move cursor pos down 6 pixels so it's on baseline.
            cursor_y += 6;
        }
    } else if(gfxFont) { // NULL passed.  Current font struct defined?
        // Switching from new to classic font behavior.
        // Move cursor pos up 6 pixels so it's at top-left of char.
        cursor_y -= 6;
    }
    gfxFont = (GFXfont *)f;
}

int LEDMatrix::drawChar(int16_t x, int16_t y, unsigned char c,uint16_t color, uint16_t bg, uint8_t size) {
int wc=0;
    if(!gfxFont) { // 'Classic' built-in font

        if((x >= _width)            || // Clip right
           (y >= _height)           || // Clip bottom
           ((x + 6 * size - 1) < 0) || // Clip left
           ((y + 8 * size - 1) < 0))   // Clip top
            return 0;

        if(!_cp437 && (c >= 176)) c++; // Handle 'classic' charset behavior

        //startWrite();
        for(int8_t i=0; i<5; i++ ) { // Char bitmap = 5 columns
            uint8_t line = pgm_read_byte(&font[c * 5 + i]);
            for(int8_t j=0; j<8; j++, line >>= 1) {
                if(line & 1) {
                    if(size == 1)
                        writePixel(x+i, y+j, color);
                    else
                        writeFillRect(x+i*size, y+j*size, size, size, color);
                } else if(bg != color) {
                    if(size == 1)
                        writePixel(x+i, y+j, bg);
                    else
                        writeFillRect(x+i*size, y+j*size, size, size, bg);
                }
            }
        }
        if(bg != color) { // If opaque, draw vertical line for last column
            if(size == 1) writeFastVLine(x+5, y, 8, bg);
            else          writeFillRect(x+5*size, y, size, 8*size, bg);
        }
        //endWrite();

    } else { // Custom font

        // Character is assumed previously filtered by write() to eliminate
        // newlines, returns, non-printable characters, etc.  Calling
        // drawChar() directly with 'bad' characters of font may cause mayhem!

        c -= (uint8_t)pgm_read_byte(&gfxFont->first);
        GFXglyph *glyph  = &(((GFXglyph *)pgm_read_pointer(&gfxFont->glyph))[c]);
        uint8_t  *bitmap = (uint8_t *)pgm_read_pointer(&gfxFont->bitmap);

        uint16_t bo = pgm_read_word(&glyph->bitmapOffset);
        uint8_t  w  = pgm_read_byte(&glyph->width),
                 h  = pgm_read_byte(&glyph->height);
        int8_t   xo = pgm_read_byte(&glyph->xOffset),
                 yo = pgm_read_byte(&glyph->yOffset);
        uint8_t  xx, yy, bits = 0, bit = 0;
        int16_t  xo16 = 0, yo16 = 0;
	wc=w;
        if(size > 1) {
            xo16 = xo;
            yo16 = yo;
        }

        // Todo: Add character clipping here

        // NOTE: THERE IS NO 'BACKGROUND' COLOR OPTION ON CUSTOM FONTS.
        // THIS IS ON PURPOSE AND BY DESIGN.  The background color feature
        // has typically been used with the 'classic' font to overwrite old
        // screen contents with new data.  This ONLY works because the
        // characters are a uniform size; it's not a sensible thing to do with
        // proportionally-spaced fonts with glyphs of varying sizes (and that
        // may overlap).  To replace previously-drawn text when using a custom
        // font, use the getTextBounds() function to determine the smallest
        // rectangle encompassing a string, erase the area with fillRect(),
        // then draw new text.  This WILL infortunately 'blink' the text, but
        // is unavoidable.  Drawing 'background' pixels will NOT fix this,
        // only creates a new set of problems.  Have an idea to work around
        // this (a canvas object type for MCUs that can afford the RAM and
        // displays supporting setAddrWindow() and pushColors()), but haven't
        // implemented this yet.

        //startWrite();
        for(yy=0; yy<h; yy++) {
            for(xx=0; xx<w; xx++) {
                if(!(bit++ & 7)) {
                    bits = pgm_read_byte(&bitmap[bo++]);
                }
                if(bits & 0x80) {
                    if(size == 1) {
                        writePixel(x+xo+xx, y+yo+yy, color);
                    } else {
                        writeFillRect(x+(xo16+xx)*size, y+(yo16+yy)*size,
                          size, size, color);
                    }
                }
                bits <<= 1;
            }
        }
        //endWrite();

    } // End classic vs custom font
return wc;
}

void LEDMatrix::setCursor(int16_t x, int16_t y) {
    cursor_x = x;
    cursor_y = y;
}

size_t LEDMatrix::write(uint8_t c) {
    if(!gfxFont) { // 'Classic' built-in font

        if(c == '\n') {                        // Newline?
            cursor_x  = 0;                     // Reset x to zero,
            cursor_y += textsize * 8;          // advance y one line
        } else if(c != '\r') {                 // Ignore carriage returns
            if(wrap && ((cursor_x + textsize * 6) > _width)) { // Off right?
                cursor_x  = 0;                 // Reset x to zero,
                cursor_y += textsize * 8;      // advance y one line
            }
            drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
            cursor_x += textsize * 6;          // Advance x one char
        }

    } else { // Custom font

        if(c == '\n') {
            cursor_x  = 0;
            cursor_y += (int16_t)textsize *
                        (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        } else if(c != '\r') {
            uint8_t first = pgm_read_byte(&gfxFont->first);
            if((c >= first) && (c <= (uint8_t)pgm_read_byte(&gfxFont->last))) {
                GFXglyph *glyph = &(((GFXglyph *)pgm_read_pointer(
                  &gfxFont->glyph))[c - first]);
                uint8_t   w     = pgm_read_byte(&glyph->width),
                          h     = pgm_read_byte(&glyph->height);
                if((w > 0) && (h > 0)) { // Is there an associated bitmap?
                    int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset); // sic
                    if(wrap && ((cursor_x + textsize * (xo + w)) > _width)) {
                        cursor_x  = 0;
                        cursor_y += (int16_t)textsize *
                          (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
                    }
										//Serial.println(c);
                    //drawChar(cursor_x, cursor_y, c, textcolor, textbgcolor, textsize);
										drawChar(cursor_x, cursor_y, c, 0,0, 1);
                }
                cursor_x += (uint8_t)pgm_read_byte(&glyph->xAdvance);// * (int16_t)textsize;
            }
        }
    }
    return 1;
}

void LEDMatrix::print_line(char *str){
    memset((char*)tbuffer, 0x00, sizeof(tbuffer));
    while(*str){
        write(*str++);
	}
}

void LEDMatrix::print_str(char* str){
    int i=0,w=0;
    while(*str){
        w=drawChar(i,14,*str,0,0,1);
        if(*str==':'){
            i=i+w+4;
        }
        else{
            i=i+w+1;
        }
        str++;
    }
}
