#ifndef PATCHREADER_H
#define PATCHREADER_H

#include <QObject>
#include <QXmlStreamReader>

class PatchReader : public QObject
{
    Q_OBJECT
public:
    explicit PatchReader(QObject *parent = 0);
public:
    void ReadPatchFile();
private:
    QXmlStreamAttributes GetAttributesFromArray(const char* data, const char* name);
signals:

public slots:
};

#endif // PATCHREADER_H
