![](https://pp.userapi.com/c638128/v638128053/258f5/hflY6sGj5rA.jpg)

FireNet - a set of tools for creating and managing MMO games based on CryEngine V

Master-server includes (C++ 11, Qt 5.7) : 
* Authorization system 
* Player profile system
* In-game store/inventory system
* Invite system (friends/game/clans)
* Chat system (global/private/in-game)
* Matchmacking system (find/join/host game)

FireNet plugins includes (C++ 11, Boost.Asio 1.62, OpenSSL 1.1.0e) :
* Core plugin to work with master-server
* Client plugin to work with game-server
* Server plugin to create dedicated game-server without CryEngine network code

Main advantages :
* FireNet absolutely free and not have any royalties
* Low latency and very fast work 
* SSL encryption for all transferred  data
* Easy integration in CryEngine with plugin system
* Full functionality documentation with youtube lessons
* Excellent smoothing network for players and physics
* Supports up to 64 players on one map at a time

# Building (Windows)

## Master-server:
### Before compiling
* Go to 3rd folder and unpack 3rd.7z archive

### Compiling
The FireNET is based on `cmake` for the compilation. In order to build the executable, you have to open a terminal and follow these steps:

```bash
# Clone the project
git clone https://github.com/afrostalin/FireNET.git
# Go inside the the project directory
cd FireNET
# Create build folder and go inside it
mkdir build && cd build
# Generate the Makefile using CMake
cmake .. -G "Visual Studio 14 2015 Win64"
```

This will generate a `FireNET.sln` file (that is, a Visual Studio solution).
* Open this file with Visual Studio.
* Build the solution in the configuration (Debug, Release, ...) of your choice.

### After compiling
* Go to FireNET/tools and run "BuildDeployer"
* Set system name and build type
* After deployment close "BuildDeployer"

## Plugins :

**Warning №1 : FireNet compatible only with CryEngine v.5.3.2 +**

**Warning №2 : For building plugins you need installed CryEngine and latest source code from CryEngine repository**

### Before compiling
* Install CryEngine v.5.3.2 or later
* Download latest source code from [CryEngine repository](https://github.com/CRYTEK-CRYENGINE/CRYENGINE/releases)
* Go to plugins/3rd folder and unpack 3rd.7z archive

### Compiling
* Go to plugins folder and choose plugin (FireNetCore/FireNetClient/FireNetServer)
* Use .cryproject file to generate solution
* Open solution with Visual Studio.
* Build the solution in the configuration (Debug, Release, ...) of your choice.

# Using (Windows)

## Master-server :
* Run redis-server 
* Go to bin/Windows/Server/Release folder and run FireNet.exe

## Plugins :
* Copy plugins in bin folder
* Use cryplugin.csv to include plugin

# TODO

To see TODO list go to [this link](https://github.com/afrostalin/FireNET/projects/1)

# WIKI

Please see [FireNET WIKI](https://github.com/afrostalin/FireNET/wiki)
