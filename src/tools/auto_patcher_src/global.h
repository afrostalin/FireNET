#ifndef GLOBAL_H
#define GLOBAL_H

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QXmlStreamReader>

#include "worker.h"
#include "patchreader.h"
#include "variables.h"

struct SGlobalEnv
{
    SGlobalEnv()
    {
        m_patchList = "";
        pWorker = nullptr;
        pPatchReader = nullptr;
        pVariables = nullptr;
    }

    QString m_patchList;

    Worker* pWorker;
    PatchReader* pPatchReader;
    Variables* pVariables;
};

extern SGlobalEnv* gEnv;

#endif // GLOBAL_H
