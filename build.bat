@echo off

rem Prepare environment.
set build_dir=build

rmdir /s /q %build_dir%
mkdir %build_dir%
cd %build_dir%
cmake ..
cd ..
