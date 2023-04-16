#ifndef NDS_FIFOZ_INCLUDE
#define NDS_FIFOZ_INCLUDE

#include <nds.h>

#ifdef __cplusplus
extern "C" {
#endif

#if defined(ARM9)
#define CONST const
#elif defined(ARM7)
#define CONST
#else
#error only for NDS
#endif

enum FIFOCommand{
	ResetRudolph		= 0x01,
	//ResetBootlib		= 0x02,
	ResetMoonlight	= 0x03,
	ResetRudolphM		= 0x04,
	ResetMoonShell2		= 0x05,

	Shutdown		= 0x11,
	ReturnBootstub	= 0x12,

	//Slot2NDS		= 0x18, //Use bootMoonlight(0x080000c0)
	Slot2GBA		= 0x19,
	//ReturnDSMenu	= 0x1a, //GetFirmware then decompress it to get image
	ReturnDSiMenu		= 0x1b,

	GetARM7Bios		= 0x101,
	GetFirmware		= 0x102,
	WriteFirmware		= 0x103,
	RequestBatteryLevel	= 0x111,
	RequestTemperature	= 0x112,

	EnableDSTTSDHCFlag	= 0x201,
	DisableDSTTSDHCFlag	= 0x202,

	PlaySound            = 0x301,
	StopSound            = 0x302,
};

#define QueueMax 16

enum NDSType{
	NDSPhat	=0,
	NDSLite	=1,
	NDSi		=2,	//eXtra Large hardware is completely the same.
	ThreeDS	=0x10, //reserved?
};

typedef struct{
	CONST u8  NDSType;
	CONST u8  flashmeversion;
	CONST u8  MAC[6];
	CONST u16 fwchksum;
	CONST u16 dummy1;
	CONST u32 fwsize;
}FWINFO;

#ifdef __cplusplus
}
#endif
#endif //included
