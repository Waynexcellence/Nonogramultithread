@echo off
setlocal enabledelayedexpansion

set "count=0"
set "src="
set "obj="
set "Src_Dir=src"
set "Header_Dir=./header"
set "Object_Dir=object"
set "Public_Header_Dir=C:/Users/wayne/Desktop/C/public/header"
set "Public_Object_Dir=C:/Users/wayne/Desktop/C/public/object"
set "STD99=-std=c99"

rem 找 src 資料夾底下的所有 .c 檔
for %%f in ("%Src_Dir%\*.c") do (
    set /a count+=1
    echo Found !count!'th file: %%f
    set "src=!src! %%f"
)

echo.
echo Found !count! .c files in %Src_Dir%

pause

if not exist "%Object_Dir%" (
    mkdir "%Object_Dir%"
)

rem 尋找public/object底下所有.o檔案
for %%f in ("%Public_Object_Dir%\*.o") do (
	set "obj=!obj! %%f"
)

rem 編譯./src底下的所有.c檔案
for %%f in (!src!) do (
    echo Compiling %%f...
    gcc %STD99% -I%Header_Dir% -I%Public_Header_Dir% -c %%f -pthread -o %Object_Dir%\%%~nf.o
    if errorlevel 1 (
        echo Compilation failed for %%f
        exit /b 1
    )
    set "obj=!obj! %Object_Dir%\%%~nf.o"
)

echo.
echo Linking object files...
rem 編譯當前資料夾底下的所有.c檔案
for %%f in ("*.c") do (
	echo Compiling %%f...
	gcc %STD99% -I%Header_Dir% -I%Public_Header_Dir% -c %%f -pthread -o %Object_Dir%\%%~nf.o
	gcc %Object_Dir%\%%~nf.o !obj! -o %%~nf -lz
)

pause
