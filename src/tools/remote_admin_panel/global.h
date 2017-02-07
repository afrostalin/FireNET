// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#pragma once

class RemoteClient;
class InputListener;

struct SGlobalEnvironment
{
    SGlobalEnvironment()
    {
		pClient = nullptr;
		pListener = nullptr;

        magicKey = 2016207;
        bUsePacketDebug = false;
    }

    RemoteClient*  pClient;
    InputListener* pListener;

    int            magicKey;
    bool           bUsePacketDebug;
};

extern SGlobalEnvironment* gEnv;