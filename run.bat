@echo off

rem Prepare environment.
set exe_path=build\ilc.exe

rem Build & run the executable.
call build.bat
echo --- Running program ---
%exe_path%
