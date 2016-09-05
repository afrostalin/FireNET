/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2016

*************************************************************************/

#ifndef _Xml_Worker_H_
#define _Xml_Worker_H_

class CXmlWorker
{
public:
	CXmlWorker() {}
	~CXmlWorker() {}

public:
	void ReadXmlData(const char* data);
private:
	void onError(const char* data);
	void onRegisterComplete(const char* data);
	void onAuthComplete(const char* data);
	void onProfileDataRecived(const char* data);
	void onShopItemsRecived(const char* data);
	void onChatMessageRecived(const char* data);
	void onInvite(const char* data);
	void onFriendStatusUpdated(const char* data);
private:
	void UpdateFriendList();
};

#endif // _Xml_Worker_H_

