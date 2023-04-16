1.00A(110203) Amplify
Implanted libprism into FIFO.

1.00B(110205) Blizzard
bootMoonlight() is working for MS2.

1.00C(110206) Crow
Passing firmware info scheme done.
Now Return to NDS Firmware is working.

1.00D(110206) Diamond
All functions other than sound are usable.

1.00E(110207) Elfin
Now playing music is availbale via MaxMod (with glitches... why?).
Applied very special workaround for bootMoonlight(). Now it can launch both MoonShell2 and menudo.

1.00.110209
fatx.c supports libelm.
Got out of beta.

1.00a.110215
Updated libnds to 1.5.0.

1.01.110221
ARM7 is now written in thumb.
Fixed Rudolph/Moonlight hybrid loader.
Added MoonShell2 Loader (thx to MoonCalc src).

1.01a.110226
Capsulized Frontend API.
Removed m3dscover; nds.m3loader.nds can now be used as loader_m3.nds.

1.01b.110227
Some fixes.
Now ARGV[1] IO is supported.

1.01c.110304
SCDS_SetSDHCForDSTT() is called before Main().

1.02.110315
XenoFile can now convert NO$GBA saves.

1.02a.110315
Fixed XenoFile msp handling.

1.03.110325 beta
Now returnDSMenu() is GPL free.

1.03.110327
returnDSMenu() is implemented to libvalkyria rather than XenoFile, which enabled die() extention.
Now this can be used as RAM unlocker.
Fixed disc_unmount() timing in some loaders.

1.03a.110403
11MB of DSi RAM is allocated for extram, rather than 8MB.

1.03b.110425
XenoFile can do openpatch according to /GameList.txt
Please note GameList has to be with Game ID (you can generate using GameListV2Builder.zip in XenoBox).
This is experimental; GameList.txt is in XenoBox package.

1.03c.110508
Fixed XenoFile openpatch_single.

1.03c1.110605
Fixed some warnings.

1.03d.110606
Now XenoFile can treat extmem for msp.

1.04.110627
Rebuilded using devkitARM r33.

1.04a.110704
Updated libnds to 1.5.1.
Chessmaster issue was fixed for AK2i (not for DSONEi).
I don't know about side-effects though.

1.05.110706
Rebuilded using devkitARM r34.

1.05a.110711
Updated libnds to 1.5.2.

1.05b.110713
XenoFile can decrypt any R4 dats (on R4).
XenoFile can show DS card info (aww, only on SCDS/TTDS).

1.05c.110817
Updated libnds to 1.5.3.

1.05d.110823
Updated libnds to 1.5.4.

1.05e.110824
Fixed no sound issue in 1.05d.

1.10.110829
Now libprism receives ARGV in proper way.
mshl2tools treats all paths using driver letters.
(Perhaps) now mshl2tools support sudokuhax.

1.11.110830
Now moonshell simply loader patches NDS header to use 0x02ff... for DSi mode.

1.11a.110912 Schlaf
Version bump...

1.12.110917 Schlaf
Fixed a fatal bug in _FAT_directory_lfnLength() (thanks to desmume's lovely bug).

1.12a.111005 Schlaf
Fixed 95% warnings.

1.13.111116 Schlaf
Fixed "Change DLDI" feature (forgot to clear cache :p).

1.13a.111124 Schlaf
ARM7 bios isn't dumped if not required.
Outputs nocashMessage().

1.13b.111125 Phase:Rebirth
Cleaned up ram.c.

1.13c.111211 Phase:Rebirth
Fixed XenoFile's CRC16 calculation. (Now it shows the same value as LHA)
Fixed XenoFile's very rare case bug: if selected music file can't be opened, xenofile freezes.
