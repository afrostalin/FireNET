// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

enum class EFireNetUdpServerStatus : int
{
	None,
	LevelLoading,
	LevelLoaded,
	LevelLoadError,
	LevelUnloading,
	LevelUnloaded,
	GameStart,
	GameEnd,
};