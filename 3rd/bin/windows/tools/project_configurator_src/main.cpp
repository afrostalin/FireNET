#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

bool UpdateFireNetSolution(QString qtFolder)
{
    QFile fireNet ("../../../../src/server/FireNET.vcxproj");

    bool line0Finded = false;

    if(fireNet.exists())
    {
        qInfo() << "FireNET.vcxproj finded";

        if(fireNet.open(QIODevice::ReadOnly))
        {
            qInfo() << "FireNET.vcxproj opened";

            QTextStream in (&fireNet);
            in.setCodec(QTextCodec::codecForName("UTF-8"));

            QByteArray textBuffer;
            int lineNumber = 0;

            QString paste0 = "      <AdditionalIncludeDirectories>.;..\\..\\3rd\\includes;..\\..\\3rd\\includes\\libssh2;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\include;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\include\\QtNetwork;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\include\\QtCore;"  +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\mkspecs\\win32-msvc2015;"
                             "..\\..\\temp\\moc\\server;%(AdditionalIncludeDirectories)</AdditionalIncludeDirectories>";

            QString line;
            do
            {
                line = in.readLine();
                lineNumber++;

                if(line.contains("<AdditionalIncludeDirectories>"))
                {
                    qInfo() << "Update line " << lineNumber;
                    line.clear();
                    line = paste0;
                    line0Finded = true;
                }

                textBuffer.append(line + "\n");

            } while (!line.isNull());

            if(!textBuffer.isEmpty() && line0Finded)
            {
                QFile::remove("../../../../src/server/FireNET.vcxproj.bak");

                fireNet.close();
                fireNet.rename("../../../../src/server/FireNET.vcxproj.bak");

                QFile newFile("../../../../src/server/FireNET.vcxproj");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "FireNET.vcxproj updated!";
                    return true;
                }
                else
                {
                    qCritical() << "Can't save FireNET.vcxproj !!!";
                    return false;
                }
            }
            else
            {
                qCritical() << "Can't update FireNET.vcxproj !!!";
                fireNet.close();
                return false;
            }

        }
        else
        {
            qCritical() << "Can't open FireNET.vcxproj";
            return false;
        }

    }
    else
    {
        qCritical() << "Can't find FireNET.vcxproj!!!";
        return false;
    }
}

bool UpdateCryModuleSolution(QString cryEngineFolder, QString qtFolder)
{
    QFile fireNet ("../../../../src/modules/cryengine/FireNET_DLL.vcxproj");

    bool line0Finded = false;
    bool line1Finded = false;

    if(fireNet.exists())
    {
        qInfo() << "FireNET_DLL.vcxproj finded";

        if(fireNet.open(QIODevice::ReadOnly))
        {
            qInfo() << "FireNET_DLL.vcxproj opened";

            QTextStream in (&fireNet);
            in.setCodec(QTextCodec::codecForName("UTF-8"));

            QByteArray textBuffer;
            int lineNumber = 0;

            QString paste0 = "      <AdditionalIncludeDirectories>.\\;%(AdditionalIncludeDirectories);" +
                             cryEngineFolder + "\\Code\\CryEngine\\CryCommon;" +
                             cryEngineFolder + "\\Code\\CryEngine\\CryAction;" +
                             cryEngineFolder + "\\Code\\SDKs\\boost;" +
                             cryEngineFolder + "\\Code\\SDKs\\yasli;" +
                             cryEngineFolder + "\\Code\\Libs\\yasli;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\include;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\include\\QtNetwork;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\include\\QtCore;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\mkspecs\\win32-msvc2015</AdditionalIncludeDirectories>";

            QString paste1 = "      <AdditionalLibraryDirectories>" +
                             cryEngineFolder + "\\Code\\SDKs\\Brofiler;" +
                             qtFolder + "\\Qt5.7.0\\5.7\\msvc2015_64\\lib;%(AdditionalLibraryDirectories)</AdditionalLibraryDirectories>";

            QString line;
            do
            {
                line = in.readLine();
                lineNumber++;

                if(line.contains("<AdditionalIncludeDirectories>"))
                {
                    qInfo() << "Update line " << lineNumber;
                    line.clear();
                    line = paste0;
                    line0Finded = true;
                }

                if(line.contains("<AdditionalLibraryDirectories>"))
                {
                    qInfo() << "Update line " << lineNumber;
                    line.clear();
                    line = paste1;
                    line1Finded = true;
                }

                textBuffer.append(line + "\n");

            } while (!line.isNull());

            if(!textBuffer.isEmpty() && line0Finded && line1Finded)
            {
                QFile::remove("../../../../src/modules/cryengine/FireNET_DLL.vcxproj.bak");

                fireNet.close();
                fireNet.rename("../../../../src/modules/cryengine/FireNET_DLL.vcxproj.bak");

                QFile newFile("../../../../src/modules/cryengine/FireNET_DLL.vcxproj");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "FireNET_DLL.vcxproj updated!";
                    return true;
                }
                else
                {
                    qCritical() << "Can't save FireNET_DLL.vcxproj !!!";
                    return false;
                }
            }
            else
            {
                qCritical() << "Can't update FireNET_DLL.vcxproj !!!";
                fireNet.close();
                return false;
            }

        }
        else
        {
            qCritical() << "Can't open FireNET_DLL.vcxproj";
            return false;
        }

    }
    else
    {
        qCritical() << "Can't find FireNET_DLL.vcxproj!!!";
        return false;
    }
}


int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    qInfo() << "Project configurator v.1.0";
    qInfo() << "Copyright (c) 2016 Ilya Chernetsov. All rights reserved";

    QTextStream stream(stdin);
    bool errors = false;

    qInfo() << "Set CryEngine root folder : ";

    QString cryEngineFolder = stream.readLine();

    qInfo() << "Set QT 5.7.0 root folder (eg. \"D:/QT/\") : ";

    QString qtFolder = stream.readLine();

    if(UpdateFireNetSolution(qtFolder))
        qInfo() << "FireNET solution configurated!";
    else
    {
        qCritical() << "Error configutating FireNET solution!!!";
        errors = true;
    }

    if(UpdateCryModuleSolution(cryEngineFolder, qtFolder))
        qInfo() << "CryModule solution configurated!";
    else
    {
        qCritical() << "Error configutating CryModule solution!!!";
        errors = true;
    }

    if(!errors)
        qInfo() << "All solutions configurated! Close this application!";
    else
        qCritical() << "Some solutions can't configurate! See logs!";

    return a.exec();
}
