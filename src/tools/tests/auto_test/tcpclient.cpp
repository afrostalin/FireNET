#include <QDebug>
#include "tcpclient.h"


tcpclient::tcpclient(QObject *parent) : QObject(parent),
    m_socket(nullptr),
    bConnected(false),
    bLastMsgSended(false)
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
    if(m_packets.size() <= 0)
        bLastMsgSended = true;

    m_packets.push(packet);
}

void tcpclient::CreateClient()
{
    qDebug() << this << "Creating TcpClient";

    QByteArray certificate = "-----BEGIN CERTIFICATE-----\n"
                             "MIIDCDCCAnGgAwIBAgIJAPSN9sMotF78MA0GCSqGSIb3DQEBCwUAMIGcMQswCQYD\n"
                             "VQQGEwJSVTEbMBkGA1UECAwSVGFtYm92c2theWEgb2JsYXN0MQ8wDQYDVQQHDAZU\n"
                             "YW1ib3YxEjAQBgNVBAoMCUJlYXRHYW1lczESMBAGA1UECwwJQmVhdEdhbWVzMRIw\n"
                             "EAYDVQQDDAkxMjcuMC4wLjExIzAhBgkqhkiG9w0BCQEWFGNoZXJuZWNvZmZAZ21h\n"
                             "aWwuY29tMB4XDTE2MDgxNjE1MzIzNVoXDTE3MDgxNjE1MzIzNVowgZwxCzAJBgNV\n"
                             "BAYTAlJVMRswGQYDVQQIDBJUYW1ib3Zza2F5YSBvYmxhc3QxDzANBgNVBAcMBlRh\n"
                             "bWJvdjESMBAGA1UECgwJQmVhdEdhbWVzMRIwEAYDVQQLDAlCZWF0R2FtZXMxEjAQ\n"
                             "BgNVBAMMCTEyNy4wLjAuMTEjMCEGCSqGSIb3DQEJARYUY2hlcm5lY29mZkBnbWFp\n"
                             "bC5jb20wgZ8wDQYJKoZIhvcNAQEBBQADgY0AMIGJAoGBAMp4/l2eJF1HKYH128vx\n"
                             "Vp8k6+3D3UpI/3yopuYfDr9uppEIdCCps+pCNQuKEjely+e0CrLN4CTlpZUXhI2B\n"
                             "ItgR7/OesDWn9ZtiUst7zGbf/NxshVm+UhNweWPeLmDfAv/r2xMLKw1t2x0ywpa8\n"
                             "A6+GMl0XhpGzoAVOJD0K2Z1vAgMBAAGjUDBOMB0GA1UdDgQWBBTw6qaXrwwJb3lq\n"
                             "X4zB6qOhwK0qDjAfBgNVHSMEGDAWgBTw6qaXrwwJb3lqX4zB6qOhwK0qDjAMBgNV\n"
                             "HRMEBTADAQH/MA0GCSqGSIb3DQEBCwUAA4GBAH6xbBlq9HT6nSSfCFaSWDE6ejX6\n"
                             "48v6BBW2o/q8OiRr1hmd3gCO4g1bW5DVl60CLOt4j3ARM7k5CZnrR8bln+/a7jug\n"
                             "lw8HKEzR62YmM2xmNJwi3nNTBMaXh9bJMzwQfyIO1HIUno9S6Wn1WHpfGc/zU5pJ\n"
                             "Y5e33lAhe2n21Uul\n"
                             "-----END CERTIFICATE-----\n";

    QSslCertificate cert(certificate);

    m_socket = new QSslSocket(this);

    m_socket->addCaCertificate(cert);

    connect(m_socket, &QSslSocket::encrypted, this, &tcpclient::onConnectedToServer);
    connect(m_socket, &QSslSocket::readyRead, this, &tcpclient::onReadyRead);
    connect(m_socket, &QSslSocket::disconnected, this, &tcpclient::onDisconnected);
    connect(m_socket, &QSslSocket::bytesWritten, this, &tcpclient::onBytesWritten);


    m_socket->connectToHostEncrypted("127.0.0.1", 3322);
}

void tcpclient::CreateTestList()
{
    qDebug() << this << "Creating test list...";

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
    qDebug() << this << "Server answer :" << m_socket->readAll();
}

void tcpclient::onBytesWritten(qint64 bytes)
{
    qDebug() << this << "Message sended" << bytes;
    bLastMsgSended = true;
}

void tcpclient::onDisconnected()
{
    qDebug() << this << "Connection with server lost";
    bConnected = false;
}
