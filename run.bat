@echo off

rem Prepare environment.
set exe_path=build/onyx.exe

rem Build & run the executable.
call build.bat
%exe_path%
