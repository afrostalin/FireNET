# Description
FireNET - multi-threaded master-server for creating and managing multiplayer games based on CryEngine. 
It is the independent platform which currently works on Windows and Linux systems. 
It includes systems such as: 
* registration / authentication by login and password
* creating a player profile, indicating nickname and model character
* in-game store system
* inventory system
* the system of friends and private messages
* simple matchmaking system

The server is based on the QT 5.7 framework, can use Redis and MySql databases, also ssl encryption with open and private keys.

# Building

[Building on Windows (VS2015)] (https://github.com/afrostalin/FireNET/wiki/%5BFireNET%5D-Building-on-Windows-(Visual-Studio-2015))

[Building on Windows (QT-Creator)] (https://github.com/afrostalin/FireNET/wiki/%5BFireNET%5D-Building-on-Windows-(QT-Creator))

[Building on Linux (QT-Creator)] (https://github.com/afrostalin/FireNET/wiki/%5BFireNET%5D-Building-on-Linux-(QT-Creator))

# Using

On Windows :

* Go to tools/databases/windows/redis/
* Start Redis-Server.exe
* Go to bin/win64/release/
* Start FireNET.exe
* Start SimpleClient.exe

On Linux :

* Instal redis-server `$ sudo apt-get install redis-server`
* Start redis server `$ redis-server`
* Go to bin/linux/release folder `$ cd path_to_firenet/bin/linux/release`
* Start FireNET.sh `$ ./FireNET.sh`

# WIKI

Please see [FireNET WIKI](https://github.com/afrostalin/FireNET/wiki)

# TODO List

* Add game server synchronization with master-server
* Add compression for big packets 
