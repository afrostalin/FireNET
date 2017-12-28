// Copyright (C) 2014-2018 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QDebug>

#include "global.h"
#include "tcpclient.h"


tcpclient::tcpclient(QString ip, int port, QObject *parent)
	: QObject(parent)
	, m_socket(nullptr)
	, bConnected(false)
	, bLastMsgSended(true)
	, m_ip(ip)
	, m_port(port)
{

}

void tcpclient::update()
{
    if(bConnected && bLastMsgSended && m_packets.size() > 0)
    {
        CTcpPacket packet = m_packets.front();
		m_packets.pop();
		bLastMsgSended = false;

		m_socket->write(packet.toString());      
    }

    if(m_packets.size() <= 0 && bLastMsgSended)
    {
       CreateTestList();
    }
}

void tcpclient::SendMsg(CTcpPacket &packet)
{
    m_packets.push(packet);
}

void tcpclient::CreateClient()
{
    m_socket = new QSslSocket(this);
	
    connect(m_socket, &QSslSocket::encrypted, this, &tcpclient::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &tcpclient::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &tcpclient::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &tcpclient::onBytesWritten);

	m_socket->addCaCertificates("key.pem");
    m_socket->connectToHostEncrypted(m_ip.toStdString().c_str(), m_port);
	if (!m_socket->waitForEncrypted(3000))
	{
		LogError("%p - Connection timeout", this);
		return;
	}
}

void tcpclient::CreateTestList()
{
    //qDebug() << this << "Creating test list...";

	CTcpPacket reg_packet(EFireNetTcpPacketType::Query);
	reg_packet.WriteQuery(EFireNetTcpQuery::Register);
	reg_packet.WriteString("test@test");
	reg_packet.WriteString("testtest");

	CTcpPacket auth_packet(EFireNetTcpPacketType::Query);
	auth_packet.WriteQuery(EFireNetTcpQuery::Login);
    auth_packet.WriteString("test@test");
    auth_packet.WriteString("testtest");

	SendMsg(reg_packet);
    SendMsg(auth_packet);   
}

void tcpclient::onConnectedToServer()
{
    qDebug() << this << "Connection with server established";
    bConnected = true;
}

void tcpclient::onReadyRead() const
{
}

void tcpclient::onBytesWritten(qint64 bytes)
{
    bLastMsgSended = true;
}

void tcpclient::onDisconnected()
{
    qDebug() << this << "Connection with server lost";
    bConnected = false;
}
