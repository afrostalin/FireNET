#include "global.h"
#include "client.h"

#include <QTcpServer>

RemoteClient::RemoteClient(QObject *parent) : QObject(parent)
{
    m_socket = nullptr;
    bConnected = false;
}

void RemoteClient::ConnectToServer(QString ip, int port)
{
    m_socket = new QSslSocket(this);
    connect(m_socket, &QSslSocket::encrypted, this, &RemoteClient::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &RemoteClient::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &RemoteClient::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &RemoteClient::onBytesWritten);

    m_socket->addCaCertificates("key.pem");
    m_socket->connectToHostEncrypted(ip ,port);
    if(!m_socket->waitForEncrypted(3000))
    {
        qDebug() << "Connection timeout";
        return;
    }
}

void RemoteClient::SendMessage(NetPacket &packet)
{
    if(m_socket)
    {
        m_socket->write(packet.toString());
        m_socket->waitForBytesWritten(10);
    }
    else
        qDebug() << "You cannot send command to FireNET, you need connect before.";
}

void RemoteClient::onConnectedToServer()
{
    qDebug() << "You connected to FireNET";
    bConnected = true;
}

void RemoteClient::onReadyRead()
{
    if(m_socket)
    {
       NetPacket packet(m_socket->readAll());

       if(packet.getType() == net_Result)
       {
           switch (packet.ReadInt())
           {
           case net_result_remote_admin_login_complete:
           {
               qInfo() << "You successfully log in!";
               break;
           }
           case net_result_remote_command_complete:
           {
               qInfo() << "Command successfully executed on server :";

               int type = packet.ReadInt();

               if(type == 0)
               {
                   qInfo() << "Server status";
                   qInfo() << "Version " << packet.ReadString();
                   qInfo() << "IP " << packet.ReadString();
                   qInfo() << "Port " << packet.ReadInt();
                   qInfo() << "Tick rate " << packet.ReadInt() << " per/sec.";
                   qInfo() << "Database mode " << packet.ReadString();
                   qInfo() << "Authorization mode " << packet.ReadString();
                   qInfo() << "Players amount " << packet.ReadInt();
                   qInfo() << "Game servers amount " << packet.ReadInt();
               } else if(type == 1)
               {
                   qInfo() << "Message sended";
               } else if(type == 2)
               {
                   qInfo() << "Command sended";
               } else if(type == 3)
               {
                   qInfo() << "Players : " << packet.ReadString();
               } else if(type == 4)
               {
                   qInfo() << "Servers : " << packet.ReadString();
               }

               break;
           }
           default:
           {
               qCritical() << "Unknown result type";
               break;
           }
           }
       } else if(packet.getType() == net_Error)
       {
           switch (packet.ReadInt())
           {
           case net_error_remote_admin_login_fail:
           {
               int reason = packet.ReadInt();

               if(reason == 0)
               {
                   qWarning() << "Can't log in in FireNet! Login not found!";
               } else if (reason == 1)
               {
                   qWarning() << "Can't log in in FireNet! Incorrect password!";
               } else if (reason == 2)
               {
                   qWarning() << "Can't log in in FireNet! Aministator alredy log in!";
               }

               break;
           }
           case net_error_remote_command_fail:
           {
               qWarning() << "Command not found in FireNET!";
               break;
           }
           default:
           {
               qCritical() << "Unknown error type";
               break;
           }
           }
       }
       else
       {
           qCritical() << "Unknown packet type!";
       }
    }
}

void RemoteClient::onBytesWritten(qint64 bytes)
{
    //qDebug() << "Message to FireNET sended. Size =" << bytes;
}

void RemoteClient::onDisconnected()
{
    qDebug() << "Connection with FireNET lost!";
    bConnected = false;
}
