@ECHO OFF

:START

echo [[[ 改UI ]]]
echo 1:LY风格 @ 旋转0
echo 2:LY风格 @ 旋转90
echo 3:LY风格 @ 旋转270
echo 4:JL02风格 @ 旋转90
echo 5:JL02_double720风格 @ 旋转90
echo q:返回
echo.
echo 选择UI风格
choice /c 1234q /n 
set a=%errorlevel%
if %a% EQU 1 (
	if exist "res_0" (
		del /q res\*
		del /q audlogo\*
		copy res_0\* res\
		copy audlogo_0\* audlogo\
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 2 (
	if exist "res_90" (
		del /q res\*
		del /q audlogo\*
		copy res_90\* res\
		copy audlogo_90\* audlogo\
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 3 (
	if exist "res_270" (
		del /q res\*
		del /q audlogo\*
		copy res_270\* res\
		copy audlogo_270\* audlogo\
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 4 (
	if exist "res_L90" (
		del /q res\*
		del /q audlogo\*
		copy res_L90\* res\
		copy audlogo_L90\* audlogo\
	) else (
		echo 没有找到备份的文件
	)
) else if %a% EQU 5 (
	if exist "res_L90_double720" (
		del /q res\*
		del /q audlogo\*
		copy res_L90_double720\* res\
		copy audlogo_L90_double720\* audlogo\
	) else (
		echo 没有找到备份的文件
	)
)else if %a% EQU 6 (
	exit
) else (
	cls
	goto START
)

:END
choice /t 1 /c 0123456789q /n /d 0