// Copyright (C) 2014-2017 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: https://github.com/afrostalin/FireNET/blob/master/LICENSE

#include <QDebug>
#include "tcpclient.h"


tcpclient::tcpclient(QString ip, int port, QObject *parent) : QObject(parent),
    m_socket(nullptr),
    bConnected(false),
    bLastMsgSended(true),
	m_ip(ip),
	m_port(port)
{

}

void tcpclient::update()
{
    if(bConnected && bLastMsgSended && m_packets.size() > 0)
    {
        NetPacket packet = m_packets.front();
		m_packets.pop();
		bLastMsgSended = false;

		m_socket->write(packet.toString());      
    }

    if(m_packets.size() <= 0 && bLastMsgSended)
    {
       CreateTestList();
    }
}

void tcpclient::SendMsg(NetPacket &packet)
{
    m_packets.push(packet);
}

void tcpclient::CreateClient()
{
    qDebug() << this << "Creating TcpClient";

    m_socket = new QSslSocket(this);
	
    connect(m_socket, &QSslSocket::encrypted, this, &tcpclient::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &tcpclient::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &tcpclient::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &tcpclient::onBytesWritten);

	m_socket->addCaCertificates("key.pem");
    m_socket->connectToHostEncrypted(m_ip.toStdString().c_str(), m_port);
}

void tcpclient::CreateTestList()
{
    //qDebug() << this << "Creating test list...";

	NetPacket reg_packet(net_Query);
	reg_packet.WriteInt(net_query_register);
	reg_packet.WriteString("test@test");
	reg_packet.WriteString("testtest");

    NetPacket auth_packet(net_Query);
    auth_packet.WriteInt(net_query_auth);
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

void tcpclient::onReadyRead()
{
    //qDebug() << this << "Server answer :" << m_socket->readAll();
}

void tcpclient::onBytesWritten(qint64 bytes)
{
    //qDebug() << this << "Message sended" << bytes;
    bLastMsgSended = true;
}

void tcpclient::onDisconnected()
{
    qDebug() << this << "Connection with server lost";
    bConnected = false;
}
