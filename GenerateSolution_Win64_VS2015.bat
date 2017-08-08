@echo off
mkdir build && cd build
cmake .. -G "Visual Studio 14 2015 Win64"
if ERRORLEVEL 1 (
	pause
) else (
	echo Starting cmake-gui...
	start cmake-gui .
)
popd