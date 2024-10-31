
copy sdram.bin sdram.app /y 

isd_download.exe -cfg isd_tools.cfg -input uboot.boot ver.bin sdram.app  -resource res audlogo 32 -disk norflash -div 2 -dev dv15 -boot 0xf02000 -reboot -aline 4096

pause
rem 

