

REM dv10_app_make.exe -addr 0x4000000 -infile sdram.bin  -compression
apu_make.exe -addr 0x1000000 -infile sdram.bin  -ofile sdram.apu

dv16_isd_sdr.exe isd_tools_flash-new.cfg -f uboot.boot sdram.apu   -tonorflash -runaddr 0x0f50000   -dev dv16 -boot 0xf02000  -reboot -go 0xf50000  -erase -aline 4096
pause

rem 

