/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2016

*************************************************************************/

#ifndef _Network_H_
#define _Network_H_

#include <QObject>
#include <QSslSocket>
#include <QCoreApplication>

class CNetwork : public QObject
{
	Q_OBJECT
public:
	explicit CNetwork(QObject *parent = 0);
public slots:
	void onConnectedToServer();
	void onReadyRead();
	void onBytesWritten(qint64 bytes);
	void onDisconnected();
private:
	bool connected;
	QSslSocket* m_socket;
public:
	void ConnectToServer();
	void SendQuery(QByteArray data);
};

#endif
