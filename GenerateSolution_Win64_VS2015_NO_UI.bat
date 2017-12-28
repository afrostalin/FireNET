@echo off
RMDIR /s/q solution
mkdir solution && cd solution
cmake .. -G "Visual Studio 14 2015 Win64" -DNO_UI=ON
if ERRORLEVEL 1 (
	pause
) else (
	echo Starting cmake-gui...
	start cmake-gui .
)
popd