@echo off
apu_make.exe -addr 0x1000000 -infile sdram.bin  -ofile sdram.apu -compress

echo 代码文件  -bfumode 1
echo 资源文件  -bfumode 2
echo 代码+资源 -bfumode 3

isd_download.exe  -cfg isd_tools.cfg -input uboot.boot ver.bin sdram.apu -resource res audlogo 32 -disk file -aline 4096 -bfumode 3 -dev upgrade_bfu

del upgrade\*.bfu upgrade\*.bin

copy jl_isd.bfu upgrade\JL_AC5X.bfu
del jl_isd.bfu 
del sdram.apu
echo.
echo 升级文件在upgrade目录下，将upgrade目录下的所有文件copy到SD卡的根目录，插卡上电即可升级，2秒钟后自动关闭窗口
echo.
choice /t 2 /d y /n >nul 
::pause