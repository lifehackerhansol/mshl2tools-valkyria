/*---------------------------------------------------------------------------------

	default ARM7 core

		Copyright (C) 2005 - 2010
		Michael Noland (joat)
		Jason Rogers (dovoto)
		Dave Murphy (WinterMute)

	This software is provided 'as-is', without any express or implied
	warranty.  In no event will the authors be held liable for any
	damages arising from the use of this software.

	Permission is granted to anyone to use this software for any
	purpose, including commercial applications, and to alter it and
	redistribute it freely, subject to the following restrictions:

	1.	The origin of this software must not be misrepresented; you
		must not claim that you wrote the original software. If you use
		this software in a product, an acknowledgment in the product
		documentation would be appreciated but is not required.

	2.	Altered source versions must be plainly marked as such, and
		must not be misrepresented as being the original software.

	3.	This notice may not be removed or altered from any source
		distribution.

---------------------------------------------------------------------------------*/
#include <nds.h>
#include <dswifi7.h>
#include <maxmod7.h>
#include "../fifoz.h"

//#include <stdio.h>
#include <string.h>

//---------------------------------------------------------------------------------
void VblankHandler(void) {
//---------------------------------------------------------------------------------
	Wifi_Update();
}


//---------------------------------------------------------------------------------
void VcountHandler() {
//---------------------------------------------------------------------------------
	inputGetAndSend();
}

///
void nocashMessageSafe(const char *s){
	const int LENGTH=112;
	int i=0,c;
	for(;i+LENGTH<strlen(s);i+=LENGTH){
		c=s[i+LENGTH];
		((char*)s)[i+LENGTH]=0;
		nocashMessage(s+i);
		((char*)s)[i+LENGTH]=c;
	}
	nocashMessage(s+i);
}

#define myPM_LED_ON    (0<<4)
#define myPM_LED_SLEEP (1<<4)
#define myPM_LED_BLINK (3<<4)

FWINFO fwinfo;
u8 userdata;
u32 dstt_sdhc;

u8 *bootstub;
typedef void (*type_void)();
type_void bootstub_arm7;
void sys_exit(){
	if(!bootstub_arm7){
		if(fwinfo.NDSType>=NDSi)writePowerManagement(0x10, 1);
		else writePowerManagement(0, PM_SYSTEM_PWR);
	}
	bootstub_arm7();
}
///

#if defined(_LIBNDS_MINOR_)
#if _LIBNDS_MINOR_ > 4
//extern bool __dsimode;
#else
static void i2cIRQHandler(){
	int cause = (i2cReadRegister(I2C_PM, 0x10) & 0x3) | (i2cReadRegister(I2C_UNK4, 0x02)<<2);
	if(cause&1)sys_exit();
}
#endif
#endif

//---------------------------------------------------------------------------------
int main() {
//---------------------------------------------------------------------------------
	{int i=0;for(i=0;i<4;i++){
		DMA_CR(i) = 0;
		DMA_SRC(i) = 0;
		DMA_DEST(i) = 0;
		TIMER_CR(i) = 0;
		TIMER_DATA(i) = 0;
	}}

	readUserSettings();

	irqInit();
	// Start the RTC tracking IRQ
	initClockIRQ();
	fifoInit();

	mmInstall(FIFO_MAXMOD);

	SetYtrigger(80);

	installWifiFIFO();
	installSoundFIFO();

	installSystemFIFO();

	///insert this clause to default arm7.
	bootstub=(u8*)0x02ff4000;
	bootstub_arm7=(*(u64*)bootstub==0x62757473746F6F62ULL)?(*(type_void*)(bootstub+0x0c)):NULL;

	readFirmware(0x20, &fwinfo.fwsize, 2);
	fwinfo.fwsize <<=3; //*= 8;
	fwinfo.fwsize += 512;
	readFirmware(0x36, (u8*)fwinfo.MAC, 6);

	writePowerManagement(0, readPowerManagement(0) | PM_BACKLIGHT_BOTTOM | PM_BACKLIGHT_TOP | PM_SOUND_AMP);
	writePowerManagement(0, readPowerManagement(0)&~(myPM_LED_BLINK));
	REG_SOUNDCNT = SOUND_ENABLE | SOUND_VOL(0x7F);
	swiChangeSoundBias(1,0x400);

	dstt_sdhc=*(vu32*)0x2fffc24;

	//because some libnds doesn't have "struct _user_data" thing. Universal source among devkitARM r23/r31.
	userdata=((u8*)(&PersonalData->calY2px))[1];

	{
		u32 myself = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
		if(myself & (1<<6))
			fwinfo.NDSType=(myself==readPowerManagement(5))?NDSLite:NDSi;
	}

	{
		u16 flashme;
		readFirmware(0x6, (u16*)&fwinfo.fwchksum, 2); //un-volatilize
		readFirmware(0x17c, &flashme, 2);
		if(flashme!=0xffff){
			fwinfo.flashmeversion=(u8)flashme;
			if(flashme>=2)readFirmware(0x3f7fc, (u8*)&fwinfo.flashmeversion, 1);
		}
	}
	///

	irqSet(IRQ_VCOUNT, VcountHandler);
	irqSet(IRQ_VBLANK, VblankHandler);
#if _LIBNDS_MINOR_ > 4
	setPowerButtonCB(sys_exit);
#else
	irqSetAUX(IRQ_I2C, i2cIRQHandler);
	irqEnableAUX(IRQ_I2C);
#endif

	irqEnable( IRQ_VBLANK | IRQ_VCOUNT | IRQ_NETWORK);

	// Keep the ARM7 mostly idle
	for(;;){
		if ( 0 == (REG_KEYINPUT & (KEY_SELECT | KEY_START | KEY_L | KEY_R))) {
			sys_exit();
		}

		///insert this clause to default arm7.
		if(fifoCheckValue32(FIFO_USER_01)){ //NotifyARM7
			u32 argc=fifoGetValue32(FIFO_USER_01);
			extern void ReadNotify(u32 argc);
			ReadNotify(argc);
		}
		if(fifoCheckValue32(FIFO_USER_02)){ //CallARM7
			u32 argc=fifoGetValue32(FIFO_USER_02);
			extern void ReadCall(u32 argc);
			ReadCall(argc);
		}
		if(fifoCheckValue32(FIFO_USER_03)){ //GetFirmwareInfo
			u32 size=fifoGetValue32(FIFO_USER_03);
			u32 i=0;
			u32 *p=(u32*)&fwinfo;
			for(;i<size;i++)fifoSendValue32(FIFO_USER_03,p[i]);
		}
		///

		swiWaitForVBlank();
	}
	return 0;
}
