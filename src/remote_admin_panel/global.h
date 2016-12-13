#ifndef GLOBAL_H
#define GLOBAL_H

class RemoteClient;
class InputListener;

struct SGlobalEnvironment
{
    SGlobalEnvironment()
    {
        magicKey = 2016207;
        bUsePacketDebug = false;
    }
    RemoteClient* pClient;
    InputListener* pListener;
    int magicKey;
    bool bUsePacketDebug;
};

extern SGlobalEnvironment* gEnv;

#endif // GLOBAL_H
