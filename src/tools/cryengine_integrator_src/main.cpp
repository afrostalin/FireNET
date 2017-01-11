// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

QString codeFolder = "/Code/GameSDK/GameDll";
QString pluginIncludesFolder = "tools_src/cryengine_integrator_src/PluginLoader";

bool UpdateIncludeFileList(QString cryEngineFolder)
{
    qInfo() << "Updating wscript ...";

    QFile oldFile(cryEngineFolder + codeFolder + "/wscript");
    bool line0Finded = false;

    if(oldFile.exists() && !QFile::exists(cryEngineFolder + codeFolder + "/wscript.bak"))
    {
        qInfo() << "wscript finded";
        if(oldFile.open(QIODevice::ReadWrite))
        {
            qInfo() << "wscript opened";

            QTextStream in (&oldFile);
            in.setCodec(QTextCodec::codecForName("UTF-8"));

            QByteArray textBuffer;
            int lineNumber = 0;

            QString paste0 = ",\n"
                             "\t\t\"PluginLoader\":\n"
                             "\t\t[\n"
                             "\t\t\t\"PluginLoader/PluginLoader.h\",\n"
                             "\t\t\t\"PluginLoader/PluginLoader.cpp\"\n"
                             "\t\t]";

            QString line;
            do
            {
                line = in.readLine();
                textBuffer.append(line);
                lineNumber++;

                if(lineNumber == 12)
                {
                    qInfo() << "Update line " << lineNumber;
                    textBuffer.append(paste0);
                    line0Finded = true;
                }
                if(!line.isNull())
                    textBuffer.append("\n");


            } while (!line.isNull());


            if(!textBuffer.isEmpty() && line0Finded)
            {
                oldFile.close();
                oldFile.rename(cryEngineFolder + codeFolder + "/gamedllsdk.waf_files.bak");

                QFile newFile(cryEngineFolder + codeFolder + "/gamedllsdk.waf_files");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "gamedllsdk.waf_files updated!";
                    return true;
                }
                else
                {
                    qCritical() << "Can't save gamedllsdk.waf_files !!!";
                    return false;
                }
            }
            else
            {
                qCritical() << "Can't update gamedllsdk.waf_files !!!";
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
        qCritical() << "Can't find gamedllsdk.waf_files or gamedllsdk.waf_files alredy updated (Try remove gamedllsdk.waf_files.bak)!!!";
        return false;
    }
}

bool UpdateGameCpp(QString cryEngineFolder)
{
    qInfo() << "Updating Game.cpp ...";

    QFile oldFile(cryEngineFolder + codeFolder + "/Game.cpp");
    bool line0Finded = false;
    bool line1Finded = false;
    bool line2Finded = false;

    if(oldFile.exists() && !QFile::exists(cryEngineFolder + codeFolder + "/Game.cpp.bak"))
    {
        qInfo() << "Game.cpp finded";
        if(oldFile.open(QIODevice::ReadOnly))
        {
            qInfo() << "Game.cpp opened";

            QTextStream in (&oldFile);
            in.setCodec(QTextCodec::codecForName("UTF-8"));

            QByteArray textBuffer;
            int lineNumber = 0;

            QString paste0 = "// FireNET module loader\n"
                             "#include <FireNET_impl.h>\n";

            QString paste1 = "\t// FireNET loader\n"
                             "\tFireNET::InitFireNETModule();\n";

            QString paste2 = "\t\t// FireNET loader\n"
                             "\t\tFireNET::RegisterFlowNodes();\n";

            QString line;
            do
            {
                line = in.readLine();
                if(!line.isNull())
                    textBuffer.append(line + "\n");
                lineNumber++;

                if(line.contains("#include <CrySystem/Profilers/FrameProfiler/FrameProfiler.h>"))
                {
                    qInfo() << "Update line " << lineNumber;
                    textBuffer.append(paste0);
                    line0Finded = true;
                }

                if(line.contains("InlineInitializationProcessing(\"CGame::Init\");"))
                {
                    qInfo() << "Update line " << lineNumber;
                    textBuffer.append(paste1);
                    line1Finded = true;

                }

                if(line.contains("CAutoRegFlowNodeBase* pFactory = CAutoRegFlowNodeBase::m_pFirst;"))
                {
                    qInfo() << "Update line " << lineNumber;
                    textBuffer.append(paste2);
                    line2Finded = true;
                }


            } while (!line.isNull());

            if(!textBuffer.isEmpty() && line0Finded && line1Finded && line2Finded)
            {
                oldFile.close();
                oldFile.rename(cryEngineFolder + codeFolder + "/Game.cpp.bak");

                QFile newFile(cryEngineFolder + codeFolder + "/Game.cpp");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "Game.cpp updated!";
                    return true;
                }
                else
                {
                    qCritical() << "Can't save Game.cpp !!!";
                    return false;
                }
            }
            else
            {
                qCritical() << "Can't update Game.cpp !!!";
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
        qCritical() << "Can't find Game.cpp or Game.cpp alredy updated (Try remove Game.cpp.bak)!!!";
        return false;
    }
}

bool UpdateGameStartupCpp(QString cryEngineFolder)
{
    qInfo() << "Updating GameStartup.cpp ...";

    QFile oldFile(cryEngineFolder + codeFolder + "/GameStartup.cpp");
    bool line0Finded = false;
    bool line1Finded = false;

    if(oldFile.exists() && !QFile::exists(cryEngineFolder + codeFolder + "/GameStartup.cpp.bak"))
    {
        qInfo() << "GameStartup.cpp finded";
        if(oldFile.open(QIODevice::ReadOnly))
        {
            qInfo() << "GameStartup.cpp opened";

            QTextStream in (&oldFile);
            in.setCodec(QTextCodec::codecForName("UTF-8"));

            QByteArray textBuffer;
            int lineNumber = 0;

            QString paste0 = "#if defined(DEDICATED_SERVER)\n"
                    "\tgEnv->pConsole->ExecuteString(\"exec server.cfg\");\n"
                    "#else\n"
                    "\tgEnv->pConsole->ExecuteString(\"exec client.cfg\");\n"
                    "#endif\n";

            QString paste1 = "// FireNET module loader\n"
                             "#include <FireNET_impl.h>\n";

            QString line;
            do
            {
                line = in.readLine();
                lineNumber++;

                if(line.contains("gEnv->pConsole->ExecuteString( \"exec autoexec.cfg\" );"))
                {
                    qInfo() << "Update line " << lineNumber;
                    line.clear();
                    line = paste0;
                    line0Finded = true;
                }

                if(line.contains("#include <CryThreading/IThreadConfigManager.h>"))
                {
                    qInfo() << "Update line " << lineNumber;
                    line.clear();
                    line = paste1;
                    line1Finded = true;
                }

                if(!line.isNull())
                    textBuffer.append(line + "\n");

            } while (!line.isNull());

            if(!textBuffer.isEmpty() && line0Finded)
            {
                oldFile.close();
                oldFile.rename(cryEngineFolder + codeFolder + "/GameStartup.cpp.bak");

                QFile newFile(cryEngineFolder + codeFolder + "/GameStartup.cpp");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "GameStartup.cpp updated!";
                    return true;
                }
                else
                {
                    qCritical() << "Can't save GameStartup.cpp !!!";
                    return false;
                }
            }
            else
            {
                qCritical() << "Can't update GameStartup.cpp !!!";
                oldFile.close();
                return false;
            }
        }
        else
        {
            qCritical() << "Can't open GameStartup.cpp !!!";
            return false;
        }
    }
    else
    {
        qCritical() << "Can't find GameStartup.cpp or GameStartup.cpp alredy updated (Try remove GameStartup.cpp.bak)!!!";
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

    QFile file7 (cryEngineFolder + "/bin/win_x64/platforms/qminimal.dll");
    QFile file8 (cryEngineFolder + "/bin/win_x64/platforms/qwindows.dll");

    // Files for Dedicated Server
    QFile file9 (cryEngineFolder + "/bin/win_x64_dedicated/Qt5Core.dll");
    QFile file10 (cryEngineFolder + "/bin/win_x64_dedicated/Qt5Network.dll");
    QFile file11 (cryEngineFolder + "/bin/win_x64_dedicated/libeay32.dll");
    QFile file12 (cryEngineFolder + "/bin/win_x64_dedicated/ssleay32.dll");

    qInfo() << "Removing old libraries...";

    file0.remove();
    file1.remove();
    file2.remove();
    file3.remove();
    file4.remove();
    file5.remove();
    file6.remove();

    file7.remove();
    file8.remove();

    // Files for Dedicated Server
    file9.remove();
    file10.remove();
    file11.remove();
    file12.remove();

    qInfo() << "Copying new libraries...";

    if(QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/libeay32.dll", cryEngineFolder + "/bin/win_x64/libeay32.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5Core.dll", cryEngineFolder + "/bin/win_x64/Qt5Core.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5Gui.dll", cryEngineFolder + "/bin/win_x64/Qt5Gui.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5Network.dll", cryEngineFolder + "/bin/win_x64/Qt5Network.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5OpenGL.dll", cryEngineFolder + "/bin/win_x64/Qt5OpenGL.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5Widgets.dll", cryEngineFolder + "/bin/win_x64/Qt5Widgets.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/ssleay32.dll", cryEngineFolder + "/bin/win_x64/ssleay32.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/platforms/qminimal.dll", cryEngineFolder + "/bin/win_x64/platforms/qminimal.dll") &&
            QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/platforms/qwindows.dll", cryEngineFolder + "/bin/win_x64/platforms/qwindows.dll") &&

             QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5Core.dll", cryEngineFolder + "/bin/win_x64_dedicated/Qt5Core.dll") &&
             QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/Qt5Network.dll", cryEngineFolder + "/bin/win_x64_dedicated/Qt5Network.dll") &&
             QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/libeay32.dll", cryEngineFolder + "/bin/win_x64_dedicated/libeay32.dll") &&
             QFile::copy("tools_src/cryengine_integrator_src/CryEngineFix/ssleay32.dll", cryEngineFolder + "/bin/win_x64_dedicated/ssleay32.dll"))
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

bool CopyFireNetLibraries(QString cryEngineFolder)
{
    qInfo() << "Coping FireNET libraries...";

    QFile file0 (cryEngineFolder + "/bin/win_x64/FireNET.dll");
    QFile file1 (cryEngineFolder + "/bin/win_x64_dedicated/FireNET_Dedicated.dll");

    qInfo() << "Removing old FireNET libraries...";

    file0.remove();
    file1.remove();

    qInfo() << "Copying new FireNET libraries...";

    if(QFile::copy("../bin/WIN64/_modules/cryengine/release/FireNET.dll", cryEngineFolder + "/bin/win_x64/FireNET.dll") &&
            QFile::copy("../bin/WIN64/_modules/cryengine/release/FireNET_Dedicated.dll", cryEngineFolder + "/bin/win_x64_dedicated/FireNET_Dedicated.dll"))
    {
        qInfo() << "FireNET libraries copyed!";
        return true;
    }
    else
    {
        qCritical() << "Failed copying FireNET libraries !!!";
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

    qInfo() << "Cheking code folder exists...";

    if(!QDir().exists(cryEngineFolder + codeFolder))
    {
        qCritical() << "Integration failed! Code/GameSDK folder not found! Check you root folder!!!";
        errors = true;
    }
    else
    {
        qInfo() << "Code folder finded!";
        qInfo() << "Creating 'FireNET' sdk folder...";

        if(!QDir().exists(cryEngineFolder + "/SDKs/FireNET"))
        {
            if(!QDir().mkdir(cryEngineFolder + "/SDKs/FireNET"))
            {
                qCritical() << "Integration failed! Failed create folder!";
                errors = true;
            }
            else
                qInfo() << "'FireNET' sdk folder created!";
        }
        else
            qInfo() << "Folder alredy exists";

        qInfo() << "Copying files...";

        QString pluginFolder =  cryEngineFolder + "/SDKs/FireNET";

        QFile pluginFile0(pluginFolder + "/FireNET_impl.h");
        QFile pluginFile1(pluginFolder + "/FireNET_Base.h");

        if(pluginFile0.exists() || pluginFile1.exists())
        {
            qInfo() << "Files alredy exists. Removing...";

            pluginFile0.remove();
            pluginFile1.remove();
        }

        if(QFile::copy(pluginIncludesFolder + "/FireNET_impl.cpp", pluginFolder + "/FireNET_impl.cpp") &&
                QFile::copy(pluginIncludesFolder + "/FireNET_Base.h", pluginFolder + "/FireNET_Base.h"))
        {
            qInfo() << "Files copyed!";
        }
        else
        {
            qCritical() << "Integration failed! Failed copy files!";
            errors = true;
        }


        if(!UpdateIncludeFileList(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't update gamesdk.waf_files !!!";
            errors = true;
        }

        if(!UpdateGameCpp(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't update Game.cpp !!!";
            errors = true;
        }

        if(!UpdateGameStartupCpp(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't update GameStartup.cpp !!!";
            errors = true;
        }


        if(!CopyCryEngineFix(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't copying CryEngine fix !!!";
            errors = true;
        }

        /*if(!CopyFireNetLibraries(cryEngineFolder))
        {
            qCritical() << "Integration failed! Can't copying FireNET libraries !!!";
            errors = true;
        }*/

        if(!errors)
        {
            qInfo() << "Integration finished!!!";
            qInfo() << "Run cry_waf.exe and regenerate uber files and solution!!!";
        }
        else
            qInfo() << "INTEGRATION FAILED !!! SEE LOGS TO MORE INFORMATION !!!";
    }
    return a.exec();
}
