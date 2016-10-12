# Description
FireNET - multi-threaded master-server for creating and managing multiplayer games based on CryEngine. 
It is the independent platform which currently works on Windows and Linux systems. 
It includes systems such as: 
* registration / authentication by login and password
* creating a player profile, indicating nickname and model character
* in-game store system
* inventory system
* the system of friends and private messages

The server is based on the QT 5.7 framework, uses the NoSql Redis database, also ssl encryption with open and private keys.
The server is distributed under the MIT license.

# Building
For building project and modules you need have installed QT 5.7

[Building on Windows (QT-Creator)] (https://github.com/afrostalin/FireNET/wiki/%5BFireNET%5D-Building-on-Windows-(QT-Creator))

[Building on Windows (VS2015)] (https://github.com/afrostalin/FireNET/wiki/%5BFireNET%5D-Building-on-Windows-(Visual-Studio-2015))

[Building on Linux (QT-Creator)] (https://github.com/afrostalin/FireNET/wiki/%5BFireNET%5D-Building-on-Linux-(QT-Creator))

# Using

On Windows :

* Go to 3rd/bin/windows/redis/
* Start Redis-Server.exe
* Go to bin/win64/release/
* Start FireNET.exe
* Start SimpleClient.exe

# WIKI

Please see [FireNET WIKI](https://github.com/afrostalin/FireNET/wiki)

# TODO List

* Add MongoDB
* Add game server synchronization with master-server
* Add compression for big packets 