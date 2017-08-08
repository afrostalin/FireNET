@echo off
RMDIR /s/q build
mkdir build && cd build
cmake .. -G "Visual Studio 14 2015 Win64" -DNO_UI=ON
if ERRORLEVEL 1 (
	pause
) else (
	echo Starting cmake-gui...
	start cmake-gui .
)
popd