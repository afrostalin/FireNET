#include "patchreader.h"
#include "global.h"

#define TITLE "[PatchReader] "

PatchReader::PatchReader(QObject *parent) : QObject(parent)
{
    gEnv->pVariables = new Variables;
}

void PatchReader::ReadPatchFile()
{
    if(QFile::exists(gEnv->m_patchList))
    {
        qDebug() << TITLE "Patch list found. Reading...";

        QFile patchList(gEnv->m_patchList);

        if(patchList.open(QIODevice::ReadWrite))
        {
            QByteArray data = patchList.readAll();


        }
        else
            qCritical() << TITLE "Can't open patch list";
    }
    else
    {
        qCritical() << TITLE "Patch list not found!";
    }
}

QXmlStreamAttributes PatchReader::GetAttributesFromArray(const char *data, const char *name)
{
    QXmlStreamReader xml(data);
    QXmlStreamAttributes attributes;

    xml.readNext();
    while (!xml.atEnd() && !xml.hasError())
    {
        xml.readNext();

        if (xml.name() == name)
        {
            return xml.attributes();
        }
    }

    return attributes;
}
