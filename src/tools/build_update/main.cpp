// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

	QString folderToFile = "../src/server/";
	QString fileName = folderToFile + "main.cpp";
    QFile mainCPP (fileName);

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

                if(!line.isNull())
                    textBuffer.append(line + "\n");

            } while (!line.isNull());

            if(!textBuffer.isEmpty() && line0Finded)
            {
                QFile::remove(fileName + ".bak");

                mainCPP.close();
                mainCPP.rename(fileName + ".bak");

                QFile newFile(fileName);
                if(newFile.open(QIODevice::Append))
                {
                    newFile.write(textBuffer);
                    newFile.close();
                    qInfo() << "main.cpp updated!";
                    return 0;
                }
                else
                {
                    qDebug() << "Can't save main.cpp !!!";
					return -2;
                }
            }
            else
            {
				qDebug() << "Can't update main.cpp !!!";
                mainCPP.close();
				return -3;
            }

        }
        else
        {
			qDebug() << "Can't open main.cpp";
			return -4;
        }

    }
    else
    {
		qDebug() << "Can't find main.cpp!!!";
		return -5;
    }

    return a.exec();
}
