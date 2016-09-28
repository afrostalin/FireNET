// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>

QString codeFolder = "/Code/GameSDK/GameDll";
QString pluginIncludesFolder = "PluginLoader";

bool UpdateUberFileList(QString cryEngineFolder)
{
    qInfo() << "Updating gamesdk.waf_files ...";
    QFile oldFile(cryEngineFolder + codeFolder + "/gamedllsdk.waf_files");
    if(oldFile.exists())
    {
        qInfo() << "gamedllsdk.waf_files finded";
        if(oldFile.open(QIODevice::ReadOnly))
        {
            qInfo() << "gamedllsdk.waf_files opened";
            QTextStream in (&oldFile);
            QByteArray buffer = in.readAll().toStdString().c_str();

            if( !buffer.isEmpty())
            {
                QString paste0 = "],\n"
                                 "\t\t\"PluginLoader\":\n"
                                 "\t\t[\n"
                                 "\t\t\t\"PluginLoader/PluginLoader.h\",\n"
                                 "\t\t\t\"PluginLoader/PluginLoader.cpp\",\n"
                                 "\t\t" ;

                int index0 = buffer.indexOf("]\r\n\t},\r\n   \"CryGameSDKDLL_uber_0.cpp\"");

                if(!buffer.contains(paste0.toStdString().c_str()))
                    buffer.insert(index0, paste0);

                // Remove old file

                oldFile.close();
                oldFile.remove();

                QFile newFile(cryEngineFolder + codeFolder + "/gamedllsdk.waf_files");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(buffer);
                }
                newFile.close();

                qInfo() << "gamedllsdk.waf_files update!";
                return true;
            }
            else
            {
                qCritical() << "Can't read gamedllsdk.waf_files !!!";
                oldFile.close();
                return false;
            }
        }
        else
        {
            qCritical() << "Can't open gamedllsdk.waf_files !!!";
            return false;
        }
    }
    else
    {
        qCritical() << "Can't find gamedllsdk.waf_files !!!";
        return false;
    }
}

bool UpdateGameCpp(QString cryEngineFolder)
{
    qInfo() << "Updating Game.cpp ...";
    QFile oldFile(cryEngineFolder + codeFolder + "/Game.cpp");
    if(oldFile.exists())
    {
        qInfo() << "Game.cpp finded";
        if(oldFile.open(QIODevice::ReadOnly))
        {
            qInfo() << "Game.cpp opened";
            QTextStream in (&oldFile);
            QByteArray buffer = in.readAll().toStdString().c_str();

            if( !buffer.isEmpty())
            {
                QString paste0 = "// Plugin loader\n"
                                 "#include \"PluginLoader/PluginLoader.h\"\n";
                QString paste1 = "\n\t// Plugin loader\n"
                                 "\tpPluginLoader->LoadPlugin();\n\n";
                QString paste2 = "\n\t\t\t// Plugin loader\n"
                                 "\t\t\tpPluginLoader->pRegisterFlowNodes();";

                int index0 = buffer.indexOf("//#define GAME_DEBUG_MEM  // debug memory usage");
                int index1 = buffer.indexOf("m_pConsole = gEnv->pConsole;");
                int index2 = buffer.indexOf("pFactory->m_pNext;");

                if(!buffer.contains(paste0.toStdString().c_str()))
                    buffer.insert(index0, paste0);
                if(!buffer.contains(paste1.toStdString().c_str()))
                    buffer.insert(index1, paste1);
                if(!buffer.contains(paste2.toStdString().c_str()))
                    buffer.insert(index2 + 18, paste2);

                // Remove old file
                oldFile.close();
                oldFile.remove();

                QFile newFile(cryEngineFolder + codeFolder + "/Game.cpp");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(buffer);
                }
                newFile.close();

                qInfo() << "Game.cpp updated!";
                return true;
            }
            else
            {
                qCritical() << "Can't read Game.cpp !!!";
                oldFile.close();
                return false;
            }
        }
        else
        {
            qCritical() << "Can't open Game.cpp !!!";
            return false;
        }
    }
    else
    {
        qCritical() << "Can't find Game.cpp !!!";
        return false;
    }
}

bool CopyCryEngineFix(QString cryEngineFolder)
{
    qInfo() << "Coping CryEngine fix...";

    QFile file0 (cryEngineFolder + "/bin/win_x64/libeay32.dll");
    QFile file1 (cryEngineFolder + "/bin/win_x64/Qt5Core.dll");
    QFile file2 (cryEngineFolder + "/bin/win_x64/Qt5Gui.dll");
    QFile file3 (cryEngineFolder + "/bin/win_x64/Qt5Network.dll");
    QFile file4 (cryEngineFolder + "/bin/win_x64/Qt5OpenGL.dll");
    QFile file5 (cryEngineFolder + "/bin/win_x64/Qt5Widgets.dll");
    QFile file6 (cryEngineFolder + "/bin/win_x64/ssleay32.dll");

    qInfo() << "Removing old libraries...";

    file0.remove();
    file1.remove();
    file2.remove();
    file3.remove();
    file4.remove();
    file5.remove();
    file6.remove();

    qInfo() << "Copying new libraries...";

    if(QFile::copy("CryEngineFix/libeay32.dll", cryEngineFolder + "/bin/win_x64/libeay32.dll") &&
            QFile::copy("CryEngineFix/Qt5Core.dll", cryEngineFolder + "/bin/win_x64/Qt5Core.dll") &&
            QFile::copy("CryEngineFix/Qt5Gui.dll", cryEngineFolder + "/bin/win_x64/Qt5Gui.dll") &&
            QFile::copy("CryEngineFix/Qt5Network.dll", cryEngineFolder + "/bin/win_x64/Qt5Network.dll") &&
            QFile::copy("CryEngineFix/Qt5OpenGL.dll", cryEngineFolder + "/bin/win_x64/Qt5OpenGL.dll") &&
            QFile::copy("CryEngineFix/Qt5Widgets.dll", cryEngineFolder + "/bin/win_x64/Qt5Widgets.dll") &&
            QFile::copy("CryEngineFix/ssleay32.dll", cryEngineFolder + "/bin/win_x64/ssleay32.dll"))
    {
        qInfo() << "Libraries copyed!";
        return true;
    }
    else
    {
        qCritical() << "Failed copying libraries !!!";
        return false;
    }

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    qInfo() << "Integrator FireNET to CryEngine v.1.0";
    qInfo() << "Copyright (c) 2016 Ilya Chernetsov. All rights reserved";

    bool errors = false;

    qInfo() << "Set CryEngine root direcrory : ";
    QTextStream stream(stdin);
    QString cryEngineFolder = stream.readLine();
    //QString cryEngineFolder = "F:/_Development/CustomCryEngine/Crytek/CRYENGINE_5.2";

    qInfo() << "Cheking code folder exists...";

    if(!QDir().exists(cryEngineFolder + codeFolder))
    {
        qCritical() << "Integration failed! Code/GameSDK folder not found! Check you root folder!!!";
        errors = true;
    }
    else
    {
        qInfo() << "Code folder finded!";
        qInfo() << "Creating 'PluginLoader' folder...";

        if(!QDir().exists(cryEngineFolder + codeFolder + "/PluginLoader"))
        {
            if(!QDir().mkdir(cryEngineFolder + codeFolder + "/PluginLoader"))
            {
                qCritical() << "Integration failed! Failed create folder!";
                errors = true;
            }
            else
                qInfo() << "'PluginLoader' folder created!";
        }
        else
            qInfo() << "Folder alredy exists";

        qInfo() << "Copying files...";

        QString pluginFolder =  cryEngineFolder + codeFolder + "/PluginLoader/";

        QFile pluginLoaderCpp(pluginFolder + "/PluginLoader.cpp");
        QFile pluginLoaderH(pluginFolder + "/PluginLoader.h");

        if(pluginLoaderCpp.exists() || pluginLoaderH.exists())
        {
            qInfo() << "Files alredy exists. Removing...";

            pluginLoaderCpp.remove();
            pluginLoaderH.remove();
        }

        if(QFile::copy(pluginIncludesFolder + "/PluginLoader.cpp", pluginFolder + "/PluginLoader.cpp") &&
                QFile::copy(pluginIncludesFolder + "/PluginLoader.h", pluginFolder + "/PluginLoader.h"))
        {
            qInfo() << "Files copyed!";
        }
        else
        {
            qCritical() << "Integration failed! Failed copy files!";
            errors = true;
        }


        if(!UpdateUberFileList(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't update gamesdk.waf_files !!!";
            errors = true;
        }

        if(!UpdateGameCpp(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't update Game.cpp !!!";
            errors = true;
        }


        if(!CopyCryEngineFix(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't copying CryEngine fix !!!";
            errors = true;
        }

        if(!errors)
        {
            qInfo() << "Integration finished!!!";
            qInfo() << "Run cry_waf.exe and regenerate uber files and solution!!!";
        }
        else
            qInfo() << "INTEGRATION FAILED !!! SEE LOGS TO MORE INFORMATION !!!";
    }

    while(1){}

    return a.exec();
}
