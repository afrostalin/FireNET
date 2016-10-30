// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef HTTPWORKER_H
#define HTTPWORKER_H

#include <QObject>

class QNetworkReply;
class QUrl;
class QTimer;

class HttpWorker : public QObject
{
    Q_OBJECT
public:
    explicit HttpWorker(QObject *parent = 0);
public:
	bool Login(QString login, QString password);
	bool Register(QString login, QString password);
public:
	int GetUID();
	int GetError();
public slots:
	virtual void replyFinished(QNetworkReply* reply);
private:
	void SendPostRequest(QUrl url, QByteArray requestData);
private:
	// Login error types : -1 - no errors, 0 - login not found, 1 - account banned, 2 - wrong password, 3 - database error
	// Register error types : -1 - no errors, 0 - login alredy register, 3 - database error
	int m_lastError;
	int m_uid;
	bool bHaveResult;
	bool bSuccessAuth;
	bool bSuccessReg;
};

#endif // HTTPWORKER_H