@echo off

rem Prepare environment.
set build_dir=build

mkdir %build_dir%
cd %build_dir%
cmake -G "MinGW Makefiles" -D CMAKE_C_COMPILER=gcc-9.2 -D CMAKE_CXX_COMPILER=g++-9.2 ..
cmake --build . --config Release
cd ..
pause