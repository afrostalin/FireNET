#include "worker.h"
#include "global.h"

Worker::Worker(QObject *parent) : QObject(parent)
{
    gEnv->pPatchReader = new PatchReader;
}

void Worker::StartWork()
{
    gEnv->pPatchReader->ReadPatchFile();
}
