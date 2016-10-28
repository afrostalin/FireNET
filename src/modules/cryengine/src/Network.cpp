// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "StdAfx.h"
#include "Global.h"
#include <QCoreApplication>
#include <thread>

void TimeoutCounter()
{
	float timeout = gEnv->pConsole->GetCVar("firenet_timeout")->GetFVal();

	float start_time = gEnv->pTimer->GetCurrTime();
	float end_time = start_time + timeout;

	gEnv->pLog->LogAlways(TITLE  "Start time = %f, end time = %f", start_time, end_time);

	while (gEnv->pTimer->GetCurrTime() <= end_time)
	{
		if (gCryModule->bConnected)
		{
			return;
		}

		CrySleep(33);
	}

	// If server not connecting after timeout
	gEnv->pLog->LogError(TITLE  "Can't connect to FireNET. Connection timeout!");

	SUIArguments args;
	args.AddArgument("@connectionTimeout");
	gCryModule->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
}

CNetwork::CNetwork(QObject *parent) : QObject(parent)
{
	m_socket = nullptr;
	bInit = false;
}

void CNetwork::Init()
{
	gEnv->pLog->LogAlways(TITLE  "Init network...");

	int argc = 0;
	char* argv[1] = {};
	QCoreApplication networker(argc, argv);

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

	connect(m_socket, &QSslSocket::encrypted, this, &CNetwork::onConnectedToServer);
	connect(m_socket, &QSslSocket::readyRead, this, &CNetwork::onReadyRead);
	connect(m_socket, &QSslSocket::disconnected, this, &CNetwork::onDisconnected);
	connect(m_socket, &QSslSocket::bytesWritten, this, &CNetwork::onBytesWritten);

	bInit = true;

	gEnv->pLog->LogAlways(TITLE  "Init complete. Start connecting...");
	ConnectToServer();

	networker.exec();
}

void CNetwork::ConnectToServer()
{
	if (gCryModule->bConnected)
	{
		gEnv->pLog->LogError(TITLE  "Can't connect to FireNET because you alredy connected!");
		return;
	}

	if (bInit)
	{
		QString adrress = gEnv->pConsole->GetCVar("firenet_ip")->GetString();
		int port = gEnv->pConsole->GetCVar("firenet_port")->GetIVal();
		
		if (!adrress.isEmpty() && port != 0)
		{
			gEnv->pLog->LogAlways(TITLE  "Connecting to FireNET...");

			m_socket->connectToHostEncrypted(adrress, port);

			std::thread pTimeoutCounter(TimeoutCounter);
			pTimeoutCounter.detach();		
		}
		else
		{
			gEnv->pLog->LogError(TITLE  "Can't connect to FireNET because network settings is wrong!");
			return;
		}
	}
	else
		gEnv->pLog->LogError(TITLE  "Can't connect to FireNET because network not init!");
}

void CNetwork::SendQuery(QByteArray data)
{
	if (m_socket != nullptr)
		m_socket->write(data);
	else
	{
		gEnv->pLog->LogWarning(TITLE  "Can't send query to FireNET because you not conneted!!!");

		SUIArguments args;
		args.AddArgument("@connectionLost");
		gCryModule->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
	}
}

void CNetwork::onConnectedToServer()
{
	gEnv->pLog->LogAlways(TITLE  "Connection with FireNET establishment");
	gCryModule->bConnected = true;
	gCryModule->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnConnectionEstablishment);
}

void CNetwork::onReadyRead()
{
	if (!m_socket)
		return;

	gEnv->pLog->Log(TITLE  "Recived new message from FireNET");

	QByteArray rawmessage;
	rawmessage = m_socket->readAll();
	gCryModule->pXmlWorker->ReadXmlData(rawmessage.toStdString().c_str());
}

void CNetwork::onBytesWritten(qint64 bytes)
{
	if (!m_socket)
		return;

	gEnv->pLog->Log(TITLE  "Message to FireNET sended");
}

void CNetwork::onDisconnected()
{
	gEnv->pLog->LogWarning(TITLE  "Connection with FireNET lost!!!");
	gCryModule->bConnected = false;

	SUIArguments args;
	args.AddArgument("@connectionLost");
	gCryModule->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
}

void CNetwork::CloseConnection()
{
	m_socket->close();
}