#ifndef GLOBAL_H
#define GLOBAL_H

class RemoteClient;
class InputListener;

struct SGlobalEnvironment
{
    RemoteClient* pClient;
    InputListener* pListener;
};

extern SGlobalEnvironment* gEnv;

#endif // GLOBAL_H
