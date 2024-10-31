apu_make.exe -addr 0x1000000 -infile sdram.bin  -ofile sdram.apu -compress


isd_download.exe -cfg isd_tools.cfg -input uboot.boot ver.bin sdram.apu -resource res audlogo 32 -disk norflash -div 2 -dev dv15 -boot 0xf02000 -aline 4096 -reboot



