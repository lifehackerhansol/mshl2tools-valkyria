/*---------------------------------------------------------------------------------
	$Id: console.c,v 1.4 2005/07/14 08:00:57 wntrmute Exp $

	console code -- provides basic print functionality

  Copyright (C) 2005
			Michael Noland (joat)
			Jason Rogers (dovoto)
			Dave Murphy (WinterMute)

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any
  damages arising from the use of this software.

  Permission is granted to anyone to use this software for any
  purpose, including commercial applications, and to alter it and
  redistribute it freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you
     must not claim that you wrote the original software. If you use
     this software in a product, an acknowledgment in the product
     documentation would be appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and
     must not be misrepresented as being the original software.
  3. This notice may not be removed or altered from any source
     distribution.

	$Log: console.c,v $
	Revision 1.4  2005/07/14 08:00:57  wntrmute
	resynchronise with ndslib
	

---------------------------------------------------------------------------------*/

#include <stdarg.h>
#include "libvalkyria.h"

extern const u8 _console_font_fixed6x6_bin[];

/////////////////////////////////////////
//global console variables

#define CONSOLE_USE_COLOR255 16

#define CONSOLE_MAPWIDTH (64)
#define CONSOLE_WIDTH (256/6)
#define CONSOLE_HEIGHT (192/6)
#define TAB_SIZE 4

//map to print to
static u16* fontMap;

//location of cursor
static u8 row, col;

static volatile u32 stNullChar=0;

//char console_strbuf[2048];
type_charp consolePrint2_callback;
type_charp consolePrintOnce2_callback;
type_charp_u32_u32 consolePrintProgress2_callback;
type_void consoleClear2_callback;
type_void consolePrintOnceEnd2_callback;
type_void consoleStartProgress2_callback;
type_void consoleEndProgress2_callback;

///////////////////////////////////////////////////////////
//consoleInit
// param: 
//		font: 16 color font
//		charBase: the location the font data will be loaded to
//		numCharacters: count of characters in the font
//		charStart: The ascii number of the first character in the font set
//					if you have a full set this will be zero
//		map: pointer to the map you will be printing to.
//		pal: specifies the 16 color palette to use, if > 15 it will change all non-zero
//			entries in the font to use palette index 255
void _consoleInit2562(u16* font, u16* charBase, u16 numCharacters, u16* map)
{
	int i;

	row = CONSOLE_HEIGHT-1;
	col = 0;
	
	fontMap = map;

	for (i = 0; i < numCharacters * (8*8*8/16); i++){ // 8x8x8bit (16bitBus)
		charBase[i] = font[i];
	}
}

void _consoleInitDefault2(u16* map, u16* charBase) {
	_consoleInit2562((u16*)_console_font_fixed6x6_bin, charBase, 256, map);
}

/*
void _consolePrintSet2(int x, int y) {
	if(y < CONSOLE_HEIGHT)
		row = y;
	else
		row = CONSOLE_HEIGHT - 1;

	if(x < CONSOLE_WIDTH)
		col = x;
	else
		col = CONSOLE_WIDTH - 1;
}

int _consoleGetPrintSetY2(void)
{
  return(row);
}
*/

void _consolePrintChar2(char c) {
	int rowup=0;
	if(c==8){
		if(col==0)row--,col=CONSOLE_WIDTH;
		col--;

		u32 ofs=(col/2) + (row * (CONSOLE_MAPWIDTH/2));
		u16 data=fontMap[ofs];
		u16 dst=' ';
		if((col&1)==0){
			data=(data&0xff00) | (dst << 0);
		}else{
			data=(data&0x00ff) | (dst << 8);
		}
		fontMap[ofs]=data;
		return;
	}

	if(col >= CONSOLE_WIDTH) {
		col = 0;

		row++;rowup=1;
	}
	
	if(row >= CONSOLE_HEIGHT) {
		if(rowup&&c==10)return;
		row--;

//		for(int i = CONSOLE_MAPWIDTH; i < CONSOLE_HEIGHT * (CONSOLE_MAPWIDTH/2); i++) fontMap[i - (CONSOLE_MAPWIDTH/2)] = fontMap[i];
		
		{
		  u16 *dmasrc=&fontMap[1*(CONSOLE_MAPWIDTH/2)];
		  u16 *dmadst=&fontMap[0*(CONSOLE_MAPWIDTH/2)];
		  u16 dmasize=(CONSOLE_HEIGHT-1)*CONSOLE_MAPWIDTH;
		  DMA3_SRC = (uint32)dmasrc;
		  DMA3_DEST = (uint32)dmadst;
		  DMA3_CR = DMA_ENABLE | DMA_SRC_INC | DMA_DST_INC | DMA_32_BIT | (dmasize>>2);
		  while(DMA3_CR & DMA_BUSY);
		}
		
//		for(int i = 0; i < (CONSOLE_MAPWIDTH/2); i++) fontMap[i + (CONSOLE_HEIGHT-1)*(CONSOLE_MAPWIDTH/2)] = 0;
		
		{
		  u16 *dmadst=&fontMap[(CONSOLE_HEIGHT-1)*(CONSOLE_MAPWIDTH/2)];
		  u16 dmasize=1*CONSOLE_MAPWIDTH;
		  DMA3_SRC = (uint32)&stNullChar;
		  DMA3_DEST = (uint32)dmadst;
		  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (dmasize>>2);
		  while(DMA3_CR & DMA_BUSY);
		}

	}
	
	switch(c){
	  case 10:
	    if(!rowup)row++;
	  case 13:
	    col = 0;
	    break;
	  case 9:
	    col += TAB_SIZE;
	    break;
	  default: {
	    u32 ofs=(col/2) + (row * (CONSOLE_MAPWIDTH/2));
	    u16 data=fontMap[ofs];
	    u16 dst=(u16)c;
	    if((col&1)==0){
	      data=(data&0xff00) | (dst << 0);
	      }else{
	      data=(data&0x00ff) | (dst << 8);
	    }
	    fontMap[ofs]=data;
	    
	    col++;
	  } break;
	}
}

void _consolePrint2(const char* s)
{
  if(nocashMessageSub)nocashMessageSafe(s);
  if(consolePrint2_callback){consolePrint2_callback(s);return;}
  while(*s!=0){
    _consolePrintChar2(*s);
    s++;
  }
}

void _consolePrintf2(const char* format, ...)
{
  va_list args;
  
  va_start( args, format );
  vsprintf( console_strbuf, format, args );
  _consolePrint2(console_strbuf);
  va_end(args);
}

void _consolePrintOnce2(const char* s)
{
  if(consolePrintOnce2_callback){consolePrintOnce2_callback(s);return;}
  while(*s!=0){
    _consolePrintChar2(*s);
    s++;
  }
  _consolePrintChar2('\r');
}

void _consolePrintfOnce2(const char* format, ...)
{
  va_list args;
  
  va_start( args, format );
  vsprintf( console_strbuf, format, args );
  _consolePrintOnce2(console_strbuf);
  va_end(args);
}

void _consolePrintOnceEnd2()
{
	if(consolePrintOnceEnd2_callback){consolePrintOnceEnd2_callback();}
	for(col=0;col<CONSOLE_WIDTH;)_consolePrintChar2(' ');
	col=0;
}

void _consolePrintProgress2(const char* s, u32 v1, u32 v2)
{
  if(consolePrintProgress2_callback){consolePrintProgress2_callback(s,v1,v2);return;}
  _consolePrintf2("%s %8d / %8d\r",s,v1,v2); //99MB is enough I think...
}

void _consoleStartProgress2()
{
	if(consoleStartProgress2_callback)consoleStartProgress2_callback();
}

void _consoleEndProgress2()
{
	if(consoleEndProgress2_callback){consoleEndProgress2_callback();}
	for(col=0;col<CONSOLE_WIDTH;)_consolePrintChar2(' ');
	col=0;
}

void _consoleClear2(void)
{
	if(consoleClear2_callback){consoleClear2_callback();return;}
//	for(int i = 0; i < CONSOLE_HEIGHT * (CONSOLE_MAPWIDTH/2); i++) fontMap[i] = 0;
	
	{
	  u16 *dmadst=&fontMap[0*(CONSOLE_MAPWIDTH/2)];
	  u16 dmasize=CONSOLE_HEIGHT*CONSOLE_MAPWIDTH;
	  DMA3_SRC = (uint32)&stNullChar;
	  DMA3_DEST = (uint32)dmadst;
	  DMA3_CR = DMA_ENABLE | DMA_SRC_FIX | DMA_DST_INC | DMA_32_BIT | (dmasize>>2);
	  while(DMA3_CR & DMA_BUSY);
	}
	
	row=col=0; //_consolePrintSet2(0,0);
}
/*
void _consolePrintOne2(char *str,u32 v)
{
  _consolePrintf2(str,v);
}
*/
