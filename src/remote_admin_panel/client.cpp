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

void RemoteClient::SendMessage(QByteArray &data)
{
    if(m_socket)
    {
        m_socket->write(data);
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
        QByteArray data = m_socket->readAll();
        QString rawData = data;
        QStringList clearData = rawData.split("\n");

        //qDebug() << "FireNET answer :";

        for(int i=0; i < clearData.size(); i++)
        {
            QString line = clearData[i];

            if(!line.isEmpty())
                qDebug() << line;
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
