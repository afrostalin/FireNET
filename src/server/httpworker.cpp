// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include "httpworker.h"
#include "global.h"
#include <QUrl>
#include <QNetworkRequest>
#include <QThread>
#include <QNetworkReply>
#include <QNetworkAccessManager>
#include <QTimer>
#include <QEventLoop>

HttpWorker::HttpWorker(QObject *parent) : QObject(parent)
{
	bHaveResult = false;
	bSuccessAuth = false;
	bSuccessReg = false;

	m_lastError = -1;
	m_uid = 0;
}

void HttpWorker::SendPostRequest(QUrl url, QByteArray requestData)
{
	qDebug() << "Send POST http request to " << url.toString();
	qDebug() << "Request data = " << requestData;

	// Reset
	m_lastError = -1;
	m_uid = 0;
	bHaveResult = false;
	bSuccessAuth = false;
	bSuccessReg = false;

	QNetworkRequest request;
	request.setUrl(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

	QNetworkAccessManager *manager = new QNetworkAccessManager(this);
	connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));

	manager->post(request, requestData);
}

void HttpWorker::replyFinished(QNetworkReply* reply)
{
	QString rawReply = reply->readAll();

	qDebug() << "Reply data = " << rawReply;

	if (rawReply.isEmpty())
	{
		qWarning() << "HTTP reply empty!";

		bHaveResult = true;
		m_lastError = 4;
		reply->deleteLater();
		return;
	}

	if (rawReply == "loginNotFound" || rawReply == "loginAlredyRegistered")
		m_lastError = 0;
	if (rawReply == "userBanned")
		m_lastError = 1;
	if (rawReply == "wrongPassword")
		m_lastError = 2;
	if (rawReply == "mySqlError" || rawReply == "registerFailed")
		m_lastError = 3;


	// Check success auth
	if (rawReply.contains("uid="))
	{
		QString rawUID = rawReply.remove("uid=");

		m_uid = rawUID.toInt();
		bSuccessAuth = true;

		qDebug() << "Success authorization by HTTP. UID = " << m_uid;
	}

	// Check success register
	if (rawReply == "registerComplete")
	{
		bSuccessReg = true;
		qDebug() << "Success registration by HTTP";
	}

	bHaveResult = true;
	reply->deleteLater();
}

bool HttpWorker::Login(QString login, QString password)
{
	QUrl loginUrl = gEnv->http_authPage;
	QByteArray data;
	data.append("login=" + login);
	data.append("&password=" + password);

	SendPostRequest(loginUrl, data);

	while (!bHaveResult)
	{
		QEventLoop loop;
		QTimer::singleShot(33, &loop, SLOT(quit()));
		loop.exec();
	}

	if (m_lastError == -1)
	{
		return true;
	}
	
	return false;
}

bool HttpWorker::Register(QString login, QString password)
{
	QUrl registerUrl = gEnv->http_regPage;
	QByteArray data;
	data.append("login=" + login);
	data.append("&password=" + password);

	SendPostRequest(registerUrl, data);

	while (!bHaveResult)
	{
		QEventLoop loop;
		QTimer::singleShot(33, &loop, SLOT(quit()));
		loop.exec();
	}

	if (m_lastError == -1)
	{
		return true;
	}

	return false;
}

int HttpWorker::GetUID()
{
	return m_uid;
}

int HttpWorker::GetError()
{
	return m_lastError;
}