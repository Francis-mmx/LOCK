@ECHO OFF

:START

echo [[[ 改配置 ]]]
echo 1:5211
echo 2:5212
echo 3:5213
echo 4:5218
echo 5:5219
echo q:返回
echo.
echo 选择配置文档
choice /c 12345q /n 
set a=%errorlevel%
if %a% EQU 1 (
	if exist "isd_tools_5211.cfg" (
		del isd_tools.cfg
		copy isd_tools_5211.cfg isd_tools.cfg
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 2 (
	if exist "isd_tools_5212.cfg" (
		del isd_tools.cfg
		copy isd_tools_5212.cfg isd_tools.cfg
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 3 (
	if exist "isd_tools_5213.cfg" (
		del isd_tools.cfg
		copy isd_tools_5213.cfg isd_tools.cfg
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 4 (
	if exist "isd_tools_5218.cfg" (
		del isd_tools.cfg
		copy isd_tools_5218.cfg isd_tools.cfg
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 5 (
	if exist "isd_tools_5219.cfg" (
		del isd_tools.cfg
		copy isd_tools_5219.cfg isd_tools.cfg
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 6 (
	exit
) else (
	cls
	goto START
)

:END
choice /t 1 /c 0123456789q /n /d 0