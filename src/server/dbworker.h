#ifndef DBWORKER_H
#define DBWORKER_H

#include <QObject>
#include "global.h"

class DBWorker : public QObject
{
    Q_OBJECT
public:
    explicit DBWorker(QObject *parent = 0);
public:
	// Check if login is there in database
	bool UserExists(QString &login);
	// Check if profile is there in database
	bool ProfileExists(QString &nickname);
	// Get unique id for new user
	int GetFreeUID();
	// Get user data
	QString GetUserData(QString &login);
	// Get user profile
	QString GetUserProfile(int uid);
	// Get uid by nickname
	int GetUIDbyNickname(QString nickname);
	// Create new user
	bool CreateUser(int uid, QString &login, QString &password);
	// Create new profile
	bool CreateProfile(QString &stringProfile, SProfile &profile);
	// Update profile
	bool UpdateProfile(QString &stringProfile, SProfile &profile);
};

#endif // DBWORKER_H