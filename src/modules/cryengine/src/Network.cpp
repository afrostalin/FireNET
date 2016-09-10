// Copyright � 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "StdAfx.h"
#include "Global.h"
#include <QCoreApplication>

CNetwork::CNetwork(QObject *parent) : QObject(parent)
{
	connected = false;
	m_socket = nullptr;
}

void CNetwork::ConnectToServer()
{
	gEnv->pLog->LogAlways(TITLE ONLINE_TITLE "Starting network service...");

	int argc = 0;
	char* argv[1] = {};
	QCoreApplication networker(argc, argv);

	QString adrress = gEnv->pConsole->GetCVar("online_ms_address")->GetString();
	int port = gEnv->pConsole->GetCVar("online_ms_port")->GetIVal();

	gEnv->pLog->LogAlways(TITLE ONLINE_TITLE "Connecting to server...");

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
	QSslCertificate cert;
	QList<QSslCertificate> SslCertificate = cert.fromData(certificate);

	m_socket = new QSslSocket(this);
	m_socket->addCaCertificates(SslCertificate);
	m_socket->connectToHostEncrypted(adrress, port);

	connect(m_socket, &QSslSocket::encrypted, this, &CNetwork::onConnectedToServer);
	connect(m_socket, &QSslSocket::readyRead, this, &CNetwork::onReadyRead);
	connect(m_socket, &QSslSocket::disconnected, this, &CNetwork::onDisconnected);
	connect(m_socket, &QSslSocket::bytesWritten, this, &CNetwork::onBytesWritten);

	networker.exec();
}

void CNetwork::SendQuery(QByteArray data)
{
	if (!m_socket)
		return;

	if (connected)
		m_socket->write(data);
	else
		gEnv->pLog->LogWarning(TITLE ONLINE_TITLE "Client not connected to online server!");
}

void CNetwork::onConnectedToServer()
{
	gEnv->pLog->LogWarning(TITLE ONLINE_TITLE "Client connected to online server!");
	connected = true;
	gCryModule->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnConnectionEstablishment);
}

void CNetwork::onReadyRead()
{
	if (!m_socket)
		return;

	gEnv->pLog->LogAlways(TITLE ONLINE_TITLE "Client recived new message");

	QByteArray rawmessage;
	rawmessage = m_socket->readAll();
	gCryModule->pXmlWorker->ReadXmlData(rawmessage.toStdString().c_str());
}

void CNetwork::onBytesWritten(qint64 bytes)
{
	if (!m_socket)
		return;

	gEnv->pLog->LogAlways(TITLE ONLINE_TITLE "Client sended message to server!");
}

void CNetwork::onDisconnected()
{
	gEnv->pLog->LogWarning(TITLE ONLINE_TITLE "Online server disconnected!");
	connected = false;

	SUIArguments args;
	args.AddArgument("@connectionLost");
	gCryModule->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
}