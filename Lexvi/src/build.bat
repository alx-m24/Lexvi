@echo off
setlocal enabledelayedexpansion

set "OUTPUT_FOLDER=%~1" REM Optional argument for output folder

echo Building to output folder: %OUTPUT_FOLDER%

echo Building Lexvi...

REM Finding boot.asm file starting from the current directory
set "BOOT_FILE="
for /r "%~dp0" %%f in (boot.asm) do (
	set "BOOT_FILE=%%f"
)
if not defined BOOT_FILE (
	echo Error: boot.asm file not found.
	exit /b 1
)

echo Found boot.asm at %BOOT_FILE%

echo Assembling boot.asm...

REM using NASM to assemble the boot file to .img format in project output folder
nasm -f bin %BOOT_FILE% -o %OUTPUT_FOLDER%\boot.img

endlocal

endlocal