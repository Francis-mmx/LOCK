

apu_make.exe -addr 0x1000000 -infile sdram.bin  -ofile sdram.apu
apu_make.exe  -infile ui.bin  -ofile ui.apu
copy sdram.bin sdram.app /y 

dv16_isd_sdr.exe isd_tools_flash-new.cfg -f uboot.boot ui.bin sdram.app   -tonorflash -runaddr 0x0f50000   -dev dv16 -boot 0xf02000  -reboot -go 0xf50000   -erase -aline 4096 -app_check_sum 
rem 
pause
rem 

