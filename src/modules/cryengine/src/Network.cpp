// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include "Global.h"
#include <QCoreApplication>
#include <thread>

// TODD - Remove this and use PostUpdate
void TimeoutCounter()
{
	float timeout = gEnv->pConsole->GetCVar("firenet_timeout")->GetFVal();

	float start_time = gEnv->pTimer->GetCurrTime();
	float end_time = start_time + timeout;

	CryLog(TITLE  "DEPRICATED : Start time = %f, end time = %f", start_time, end_time);

	while (gEnv->pTimer->GetCurrTime() <= end_time)
	{
		if (gModuleEnv->bConnected)
		{
			return;
		}

		CrySleep(33);
	}

	// If server not connecting after timeout
	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't connect to FireNET. Connection timeout");
#ifndef DEDICATED_SERVER
	SUIArguments args;
	args.AddArgument("@connectionTimeout");
	gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
#endif
}

CNetwork::CNetwork(QObject *parent) : QObject(parent)
{
	m_socket = nullptr;
	bInit = false;
}

void CNetwork::Init()
{
	CryLog(TITLE  "Init network...");

	int argc = 0;
	char* argv[1] = {};
	QCoreApplication networker(argc, argv);

	// Update certificate when you cnahge server ip
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

	CryLog(TITLE  "Init complete. Start connecting...");

	ConnectToServer();
	networker.exec();
}

void CNetwork::ConnectToServer()
{
	if (gModuleEnv->bConnected)
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't connect to FireNET because you alredy connected");
		return;
	}

	if (bInit)
	{
		if (gEnv && gEnv->pConsole)
		{
			QString adrress = gEnv->pConsole->GetCVar("firenet_ip")->GetString();
			int port = gEnv->pConsole->GetCVar("firenet_port")->GetIVal();

			if (!adrress.isEmpty() && port > 0)
			{
				CryLogAlways(TITLE  "Connecting to FireNET...");

				m_socket->connectToHostEncrypted(adrress, port);

				// TODO : Remove this
				std::thread pTimeoutCounter(TimeoutCounter);
				pTimeoutCounter.detach();
			}
			else
			{
				CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't connect to FireNET because network settings is wrong");
				return;
			}
		}
	}
	else
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't connect to FireNET because network not init");
}

bool CNetwork::SendQuery(const char* data)
{
	gModuleEnv->bHaveNewResult = false;

	if (m_socket != nullptr)
	{
		m_socket->write(data);
		return true;
	}
	else
	{
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Can't send query to FireNET because you not conneted");

#ifndef DEDICATED_SERVER
		SUIArguments args;
		args.AddArgument("@connectionLost");
		gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
#endif
		return false;
	}
}

bool CNetwork::SendSyncQuery(const char * data, float timeout)
{
	int i = 0;

	while (i < 5)
	{
		CryLog("Test wait function");
		CrySleep(1000);
		i++;
	}

	return false;
}

void CNetwork::onConnectedToServer()
{
	CryLogAlways(TITLE  "Connection with FireNET establishment");
	gModuleEnv->bConnected = true;
#ifndef DEDICATED_SERVER
	gModuleEnv->pUIEvents->SendEmptyEvent(CModuleUIEvents::eUIGE_OnConnectionEstablishment);
#else
	RegisterGameServer();
#endif
}

void CNetwork::onReadyRead()
{
	if (!m_socket)
		return;

	CryLog(TITLE "Recived new message from FireNET");

	QByteArray rawmessage;
	rawmessage = m_socket->readAll();

	if (gModuleEnv->pXmlWorker)
		gModuleEnv->pXmlWorker->ReadXmlData(rawmessage.toStdString().c_str());
}

void CNetwork::onBytesWritten(qint64 bytes)
{
	if (!m_socket)
		return;

	CryLog(TITLE  "Message to FireNET sended. Size = %d", bytes);
}

void CNetwork::onDisconnected()
{
	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Connection with FireNET lost");
	gModuleEnv->bConnected = false;

#ifndef DEDICATED_SERVER
	SUIArguments args;
	args.AddArgument("@connectionLost");
	gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
#endif
}

void CNetwork::CloseConnection()
{
	if (m_socket)
	{
		CryLog(TITLE "Closing connection with FireNET");
		m_socket->close();
	}
}

#ifdef DEDICATED_SERVER
bool CNetwork::RegisterGameServer()
{
	CryLogAlways(TITLE "Register game server in FireNET...");

	QString ip = gEnv->pConsole->GetCVar("sv_bind")->GetString();
	int port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
	QString serverName = gEnv->pConsole->GetCVar("sv_servername")->GetString();
	int online = 0;
	int maxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers")->GetIVal();
	QString mapName = gEnv->pConsole->GetCVar("sv_map")->GetString();
	mapName.remove("Multiplayer/");
	QString gamerules = (char*)gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

	QString query = "<query type='register_game_server'><data name = '" + serverName +
		"' ip = '" + ip +
		"' port = '" + QString::number(port) +
		"' map = '" + mapName +
		"' gamerules = '" + gamerules +
		"' online = '" + QString::number(online) +
		"' maxPlayers = '" + QString::number(maxPlayers) + "'/></query>";

	if (SendQuery(query.toStdString().c_str()))
	{
		return true;
	}
	else
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't register game server because you not connected to FireNET");

	return false;
}

bool CNetwork::UpdateGameServer()
{
	CryLogAlways(TITLE "Try update game server info in FireNET");

	QString ip = gEnv->pConsole->GetCVar("sv_bind")->GetString();
	int port = gEnv->pConsole->GetCVar("sv_port")->GetIVal();
	QString serverName = gEnv->pConsole->GetCVar("sv_servername")->GetString();
	int online = 0; // TODO
	int maxPlayers = gEnv->pConsole->GetCVar("sv_maxplayers")->GetIVal();
	QString mapName = gEnv->pConsole->GetCVar("sv_map")->GetString();
	mapName.remove("Multiplayer/");
	QString gamerules = (char*)gEnv->pConsole->GetCVar("sv_gamerules")->GetString();

	QString query = "<query type='update_game_server'><data name = '" + serverName +
		"' ip = '" + ip +
		"' port = '" + QString::number(port) +
		"' map = '" + mapName +
		"' gamerules = '" + gamerules +
		"' online = '" + QString::number(online) +
		"' maxPlayers = '" + QString::number(maxPlayers) + "'/></query>";

	if (SendQuery(query.toStdString().c_str()))
	{
		return true;
	}
	else
		CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Can't update game server because you not connected to FireNET");

	return false;
}
#endif