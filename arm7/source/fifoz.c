#include <nds.h>
#include "../fifoz.h"

#define myPM_LED_ON    (0<<4)
#define myPM_LED_SLEEP (1<<4)
#define myPM_LED_BLINK (3<<4)

extern FWINFO fwinfo;
extern u32 dstt_sdhc;
extern u8 userdata;

extern __attribute__((noreturn)) void _menu7_Gen_s();
extern __attribute__((noreturn)) void reboot();

void bootMoonlight(u32 bootAddress, int isPrivateRAM){
				u32 i;
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				REG_IME = IME_DISABLE;	// Disable interrupts
				REG_IF = REG_IF;	// Acknowledge interrupt

				//while(!IPCZ->bootaddress)swiDelay(4);
					//for(w=0;w<0x100;w++);
				REG_IME = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;
#ifdef _LIBNDS_MAJOR_
				REG_SOUNDCNT = 0;
#else
				SOUND_CR = 0;
#endif

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;
#if 0
				{ //switch to user mode
					//u32 r0=0;
					//__asm {
						asm("mov r0, #0x1F");
						asm("msr cpsr, r0");
					//}
				}
#endif
				SwitchUserMode();

				REG_IE = 0;
				REG_IF = ~0;
				(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
				(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
				REG_POWERCNT = 1;  //turn off power to stuffs

				// Reload DS Firmware settings
				// already done in load_PersonalData()

				*(vu32*)0x02fFFE34 = bootAddress;	// Bootloader start address
				if(isPrivateRAM){
					*(vu32*)0x02fFFE04 = 0xE59FF018;
					resetARM9(0x02fFFE04);
				}else{
					while(*((vu32*)0x02fFFE24) != (u32)0x02fFFE04 && *((vu32*)0x02fFFE24) != (u32)0x0cfFFE04);
				}
				swiSoftReset();
				while(1);
}

//Write Flash from DSOrganize (not copied, as usual...)
static void Write_Flash(int address, void *destination){ //length is always 256
	int i=0;
	u8 *dst = (u8*)destination;
	SerialWaitBusy();

	//Enable write
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0x6;
	SerialWaitBusy();
	REG_SPICNT = 0;

	//Write start
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0x5;
	SerialWaitBusy();
	do{
		REG_SPIDATA = 0x0;
		SerialWaitBusy();
	}while(!(REG_SPIDATA&0x02));
	REG_SPICNT = 0;

	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0xa;
	SerialWaitBusy();

	REG_SPIDATA = (address >> 16) & 0xFF;
	SerialWaitBusy();

	REG_SPIDATA = (address >> 8) & 0xFF;
	SerialWaitBusy();

	REG_SPIDATA = (address) & 0xFF;
	SerialWaitBusy();

	for(; i < 256; i++) {
		REG_SPIDATA = dst[i];
		SerialWaitBusy();
	}
	REG_SPICNT = 0;

	//Write end
	REG_SPICNT = SPI_ENABLE|SPI_CONTINUOUS|SPI_DEVICE_NVRAM;
	REG_SPIDATA = 0x5;
	SerialWaitBusy();
	do{
		REG_SPIDATA = 0x0;
		SerialWaitBusy();
	}while(REG_SPIDATA&0x01);
	REG_SPICNT = 0;
}

u32 Queue[QueueMax];
void ReadNotify(u32 argc){
	int i=0;
	for(;i<argc;){
		if(fifoCheckValue32(FIFO_USER_01)){
			Queue[i] = fifoGetValue32(FIFO_USER_01);
			i++;
		}
	}
	fifoSendValue32(FIFO_USER_01,Queue[0]);
	switch(Queue[0]){
			case ResetRudolph:{
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)_menu7_Gen_s;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}break;
/*
			//Use resetARM7(0x06000000)
			case ResetBootlib:{
				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				while(*((vu32*)0x02fFFE24) != (u32)0x02fFFE04 && *((vu32*)0x02fFFE24) != (u32)0x0cfFFE04)swiWaitForVBlank(); 
				irqDisable(IRQ_ALL);
				*((vu32*)0x02fFFE34) = (u32)0x06000000; //BG_BMP_RAM(0)
				swiSoftReset();
				while(1);
			}break;
*/
			case ResetMoonlight:{
				bootMoonlight(Queue[1],Queue[2]);
			}break;
			case ResetRudolphM:{
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);
				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;
#ifdef _LIBNDS_MAJOR_
				REG_SOUNDCNT = 0;
#else
				SOUND_CR = 0;
#endif

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;
#if 0
				{ //switch to user mode
					//u32 r0=0;
					//__asm {
						asm("mov r0, #0x1F");
						asm("msr cpsr, r0");
					//}
				}
#endif
				SwitchUserMode();

				REG_IE = 0;
				REG_IF = ~0;
				(*(vu32*)(0x04000000-4)) = 0;  //IRQ_HANDLER ARM7 version
				(*(vu32*)(0x04000000-8)) = ~0; //VBLANK_INTR_WAIT_FLAGS, ARM7 version
				REG_POWERCNT = 1;  //turn off power to stuffs

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)_menu7_Gen_s;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}break;
			case ResetMoonShell2:{
				#define ARM7_PROG (0x03810000 - 0xA00)
				void (*_menu7_Gen)();
				u32	*adr;
				u32	*buf;
				u32	i;

				*(vu32*)0x2fffc24=dstt_sdhc;
				//writePowerManagement(0, readPowerManagement(0) | myPM_LED_BLINK);

				REG_IME = 0;
				REG_IE = 0;
				REG_IF = REG_IF;

				REG_IPC_SYNC = 0;
				DMA0_CR = 0;
				DMA1_CR = 0;
				DMA2_CR = 0;
				DMA3_CR = 0;

				for(i=0x04000400; i<0x04000500; i+=4)
					*((u32*)i)=0;
#ifdef _LIBNDS_MAJOR_
				REG_SOUNDCNT = 0;
#else
				SOUND_CR = 0;
#endif

				//clear out ARM7 DMA channels and timers
				for(i=0x040000B0;i<(0x040000B0+0x30);i+=4)
					*((vu32*)i)=0;
				for(i=0x04000100;i<0x04000110;i+=2)
					*((u16*)i)=0;

				//copy final loader to private RAM
				adr = (u32*)ARM7_PROG;
				buf = (u32*)reboot;
				for(i = 0; i < 0x200/4; i++) {
					*adr = *buf;
					adr++;
					buf++;
				}

				//while((*(vu32*)0x02fFFDFC) != 0x02fFFDF8 && (*(vu32*)0x02fFFDFC) != 0x0cfFFDF8);	// Timing adjustment with ARM9
				_menu7_Gen = (void(*)())ARM7_PROG;
				_menu7_Gen();
				while(1);
			}break;
			//case Shutdown:{
			//Use systemShutDown() FIFO.
			//	writePowerManagement(0, PM_SYSTEM_PWR);
			//}break;
			case ReturnBootstub:{
				sys_exit();
			}break;
			//case Slot2NDS:{
			//	//Use bootMoonlight(0x080000c0)
			//}break;
			case Slot2GBA:{
				writePowerManagement(0, PM_SOUND_AMP | ((userdata>>3)&1?PM_BACKLIGHT_BOTTOM:PM_BACKLIGHT_TOP));
				swiChangeSoundBias(0,0x400);
				swiSwitchToGBAMode();
				while(1);
			}break;
			//case ReturnDSMenu:{
			//	//GetFirmware then decompress it
			//}break;
			case ReturnDSiMenu:{
				// http://devkitpro.org/viewtopic.php?f=13&t=2140#p6013
				writePowerManagement(0x10, 1);
			}break;
#if 0

			}else if(IPCZ->cmd==PlaySound){
				PCMstartSound(0, IPCZ->PCM_freq, IPCZ->PCM_L, IPCZ->PCM_size, 100,   0, IPCZ->PCM_bits);
				PCMstartSound(1, IPCZ->PCM_freq, IPCZ->PCM_R, IPCZ->PCM_size, 100, 127, IPCZ->PCM_bits);
				IPCZ->cmd=0;
			}else if(IPCZ->cmd==StopSound){
				PCMstopSound(0);
				PCMstopSound(1);
				IPCZ->cmd=0;
			}
#endif
	}
}

void ReadCall(u32 argc){
	int i=0;
	u32 ret=0;
	for(;i<argc;){
		if(fifoCheckValue32(FIFO_USER_02)){
			Queue[i] = fifoGetValue32(FIFO_USER_02);
			i++;
		}
	}
	switch(Queue[0]){
		case GetARM7Bios:{
			u32 arm7bios_addr=Queue[1];
			u32 arm7bios_bufsize=Queue[2];
			ARM7_Bios(arm7bios_addr,arm7bios_bufsize-1);
			ret=1;
		}break;
		case GetFirmware:{
			void *firmware_addr=(void*)Queue[1];
			u32 firmware_bufsize=Queue[2];
			u32 size=fwinfo.fwsize,dumpsize;
			dumpsize=size<firmware_bufsize?size:firmware_bufsize;

			u32 tmp=enterCriticalSection();{
				readFirmware(0, firmware_addr, dumpsize);
			}leaveCriticalSection(tmp);
			ret=1;
		}break;
		case WriteFirmware:{
			u8 firmware_write_index=Queue[1];
			u8 *firmware_write_addr=Queue[2];
			if(firmware_write_index==0||firmware_write_index==1||firmware_write_index==2)
				*(vu16*)(firmware_write_addr+0xfe)=swiCRC16(0,firmware_write_addr,0xfe);
			else if(firmware_write_index==4||firmware_write_index==5)
				*(vu16*)(firmware_write_addr+0x72)=swiCRC16(0xffff,firmware_write_addr,0x70);
			else goto WriteFirmware_abort;

			u32 tmp=enterCriticalSection();{
				Write_Flash((fwinfo.fwsize-0x600)+(firmware_write_index<<8), firmware_write_addr);
			}leaveCriticalSection(tmp);
WriteFirmware_abort:
			ret=1;
		}break;
		case RequestTemperature:{
			int t1,t2;
			ret=touchReadTemperature(&t1, &t2);
		}break;
		//case RequestBatteryLevel:{
		//	//Use getBatteryLevel FIFO.
		//	ret=(u16)(readPowerManagement(PM_BATTERY_REG) & 1);
		//	u32 dslite = readPowerManagement(4); //(PM_BACKLIGHT_LEVEL);
		//	if(dslite & (1<<6))ret |= (dslite & (1<<3))<<12; // libnds 1.4.7 fixed with my report :)
		//}break;
		case EnableDSTTSDHCFlag:{
			*(vu32*)0x2fffc24=dstt_sdhc=1;
			ret=1;
		}break;
		case DisableDSTTSDHCFlag:{
			*(vu32*)0x2fffc24=dstt_sdhc=0;
			ret=1;
		}break;
	}
	fifoSendValue32(FIFO_USER_02,ret);
}
