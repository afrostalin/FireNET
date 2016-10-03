#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QFile mainCPP ("main.cpp");

    bool line0Finded = false;

    if(mainCPP.exists())
    {
        qInfo() << "main.cpp finded";

        if(mainCPP.open(QIODevice::ReadOnly))
        {
            qInfo() << "main.cpp opened";

            QTextStream in (&mainCPP);
            in.setCodec(QTextCodec::codecForName("UTF-8"));

            QByteArray textBuffer;
            int lineNumber = 0;

            QString line;
            do
            {
                line = in.readLine();
                lineNumber++;

                if(line.contains("int buildNumber ="))
                {
                    line.remove("\tint buildNumber = ");
                    line.remove(";");

                    int buildVersion = line.toInt() + 1;

                    QString newLine = "\tint buildNumber = " + QString::number(buildVersion) + ";";

                    qInfo() << "Update line " << lineNumber;
                    line.clear();
                    line = newLine;
                    line0Finded = true;
                }

                textBuffer.append(line + "\n");

            } while (!line.isNull());

            if(!textBuffer.isEmpty() && line0Finded)
            {
                QFile::remove("main.cpp.bak");

                mainCPP.close();
                mainCPP.rename("main.cpp.bak");

                QFile newFile("main.cpp");
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "main.cpp updated!";
                    return 0;
                }
                else
                {
                    qCritical() << "Can't save main.cpp !!!";
                }
            }
            else
            {
                qCritical() << "Can't update main.cpp !!!";
                mainCPP.close();
            }

        }
        else
        {
            qCritical() << "Can't open main.cpp";
        }

    }
    else
    {
        qCritical() << "Can't find main.cpp!!!";
    }

    return a.exec();
}
