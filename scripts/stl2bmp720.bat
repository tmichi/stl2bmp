@echo off
if "%1"=="" (
 echo "Drag and drop a STL file here."
 pause
 exit /b
)
stl2bmp %1 720