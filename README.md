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

On Linux (QT Creator) :

* Open FireNET.pro
* Configure project
* Select Debug or Release build type
* Build project


On Windows (QT Creator) :

* Open FireNET.pro
* Configure project
* Select Debug or Release build type
* Build project

On Windows (Visual Studio 2015) :

* Install Qt5Package.vsix (see 3rd folder)
* Open FireNET.sln
* Go to Qt plugin settings and configure him
* Set paths to QT libs and includes in project settings
* Choose build type
* Build solution

See [WIKI](https://github.com/afrostalin/FireNET/wiki#building) for more instructions.

# Using

On Windows :

* Go to 3rd/bin/windows/redis/
* Start Redis-Server.exe
* Go to bin/win64/release/
* Start FireNET.exe
* Start SimpleClient.exe

On Linux :

* `$ cd .../3rd/bin/linux/redis/`
* `$ ./redis-server`
* `$ cd .../bin/linux/release`
* `$ ./firenet`
* `$ ./simpleclient`

# Examples

* CryEngine simple shooter
* UnrealEngine simple shooter
* Unity3D simple shooter 

# TODO List

* Add MongoDB
* Add matchmaking system
* Add game server synchronization with master-server
* Add compression for big packets 

# Plans

* The database "Redis" has a huge speed because all the data is stored in ram memory, but this advantage follows its main drawback - the possibility of loss of piece of data in case of crash of the server. 
There is a mechanism called "snapshots" in this database, which periodically asynchronously saves data to disk. 
It is planned to use a bunch of MongoDB and Redis where Redis will serve as a "hot" cache, providing the speed and MongoDB will be the main database.
