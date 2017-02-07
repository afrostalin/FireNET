// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include "global.h"
#include "client.h"

#include <QTcpServer>

RemoteClient::RemoteClient(QObject *parent) : QObject(parent),
	m_socket(nullptr),
	bConnected(false),
	bLastMsgSended(true)
{
	connect(&m_Timer, &QTimer::timeout, this, &RemoteClient::Update);

	m_Timer.start(33);
}

void RemoteClient::Update()
{
	if (bConnected && bLastMsgSended && m_packets.size() > 0)
	{
		NetPacket packet = m_packets.front();
		m_packets.pop();
		bLastMsgSended = false;

		m_socket->write(packet.toString());
	}
}

void RemoteClient::ConnectToServer(const QString &ip, int port)
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
	m_packets.push(packet);
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
				   QString version = packet.ReadString();
				   QString ip = packet.ReadString();
				   int port = packet.ReadInt();
				   int tickrate = packet.ReadInt();
				   QString databaseMode = packet.ReadString();
				   QString authMode = packet.ReadString();
				   int playersCount = packet.ReadInt();
				   int maxPlayers = packet.ReadInt();
				   int gameServersCount = packet.ReadInt();
				   int maxGameServers = packet.ReadInt();

                   qInfo() << "Server status";
                   qInfo() << "Version " << version;
                   qInfo() << "IP " << ip;
                   qInfo() << "Port " << port;
                   qInfo() << "Tick rate " << tickrate << " per/sec.";
                   qInfo() << "Database mode " << databaseMode;
                   qInfo() << "Authorization mode " << authMode;
                   qInfo() << "Players amount " << playersCount << "/" << maxPlayers;
                   qInfo() << "Game servers amount " << gameServersCount << "/" << maxGameServers;
               } 
			   else if(type == 1)
               {
                   qInfo() << "Message sended";
               } 
			   else if(type == 2)
               {
                   qInfo() << "Command sended";
               } 
			   else if(type == 3)
               {
                   qInfo() << "Players : " << packet.ReadString();
               } 
			   else if(type == 4)
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
       } 
	   else if(packet.getType() == net_Error)
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
	bLastMsgSended = true;
}

void RemoteClient::onDisconnected()
{
    qDebug() << "Connection with FireNET lost!";
    bConnected = false;
}


