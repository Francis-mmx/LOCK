


::！！！！！更改摄像头效果文件请改此处, 注意路径信息！！！！

..\isp_cfg.exe -i isp_test_cfg.J3L -o ..\res\isp_cfg_0.bin
..\isp_cfg.exe -i isp_test_cfg.J3L -o 	..\res\isp_cfg_1.bin
..\isp_cfg.exe -i isp_test_cfg.J3L -o ..\res\isp_cfg_2.bin
..\isp_cfg.exe -i isp_test_cfg.J3L -o ..\res\isp_cfg_3.bin

set /p  run="是否启动下载？(y / n)
echo %run%
if %run%==y  (
    cd ..
    call 下载代码.bat
)





