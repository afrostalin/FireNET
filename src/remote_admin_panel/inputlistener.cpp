#include "inputlistener.h"
#include "global.h"
#include "client.h"

#include <QTextStream>
#include <QDebug>
#include <QEventLoop>
#include <QTimer>

InputListener::InputListener(QObject *parent) : QObject(parent)
{ 
}

void InputListener::Run()
{
    gEnv->pClient = new RemoteClient();
    QTextStream s(stdin);
    QString input;

    while(true)
    {
        input = s.readLine();

        if(input == "list")
        {
            qDebug() << "connect <ip> <port> - connecting to FireNET with specific address";
            qDebug() << "connect - connecting to FireNET with default address (127.0.0.1:64000)";
            qDebug() << "login <login> <password> - authorization in FireNET by login and password";
            qDebug() << "status - get full FireNET status";
            qDebug() << "send_global_message <message> - Send message to all connected players";
            qDebug() << "send_console_command <command> <arguments> - Send console command to all connected players";
            qDebug() << "players - Get player list";
            qDebug() << "servers - Get game server list";
        }

        if(input.contains("connect"))
        {
            QStringList list = input.split(" ");
            if(list.size() == 3)
            {
                QString ip = list[1];
                QString port = list[2];

                qDebug() << "Start connection to FireNET (" << ip << ":" << port.toInt() << ")";

                gEnv->pClient->ConnectToServer(ip, port.toInt());
            }
            else
            {
                qDebug() << "Start connection to FireNET (127.0.0.1:64000)";
                gEnv->pClient-> ConnectToServer("127.0.0.1", 64000);
            }
        }

        if(input.contains("login"))
        {
            QStringList list = input.split(" ");
            if(list.size() == 3)
            {
                QString login = list[1];
                QString password = list[2];

                qDebug() << "Start loggining in FireNET";

                QByteArray query;
                query.append("<query type = 'admin_auth'><data login='" + login + "' password = '" + password + "'/></query>");

                gEnv->pClient->SendMessage(query);
            }
            else
            {
                qDebug() << "Syntax error! Use login <login> <password>";
            }
        }

        if(input == "status")
        {
            QByteArray query;
            query.append("<query type = 'console_command'><data command='status' value=''/></query>");

            gEnv->pClient->SendMessage(query);
        }

        if(input.contains("send_global_message"))
        {
            QString message = input.remove("send_global_message");

            if(!message.isEmpty())
            {
                QByteArray query;
                query.append("<query type = 'console_command'><data command='send_global_message' value='" + message + "'/></query>");

                gEnv->pClient->SendMessage(query);
            }
            else
                qDebug() << "Syntax error! Use send_global_message <message>";
        }

        if(input.contains("send_console_command"))
        {
            QString message = input.remove("send_console_command");

            if(!message.isEmpty())
            {
                QByteArray query;
                query.append("<query type = 'console_command'><data command='send_console_command' value='" + message + "'/></query>");

                gEnv->pClient->SendMessage(query);
            }
            else
                qDebug() << "Syntax error! Use send_console_command <command> <arguments>";
        }

        if(input == "players")
        {
            QByteArray query;
            query.append("<query type = 'console_command'><data command='players' value=''/></query>");

            gEnv->pClient->SendMessage(query);
        }

        if(input == "servers")
        {
            QByteArray query;
            query.append("<query type = 'console_command'><data command='servers' value=''/></query>");

            gEnv->pClient->SendMessage(query);
        }

        QEventLoop loop;
        QTimer::singleShot(33, &loop, SLOT(quit()));
        loop.exec();
    }
}
