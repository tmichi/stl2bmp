@echo off
if "%1"=="" (
 echo "Drag and drop an image folder here."
 pause
 exit /b
)


SET INPUT=%1
for /r "%INPUT%" %%F in (*.bmp) do (
  magick convert %%F %INPUT%/%%~nF.png
)
PAUSE