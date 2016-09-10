// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#ifndef _PluginLoader_
#define _PluginLoader_

#ifndef _WINDEF_
class HINSTANCE__;
typedef HINSTANCE__* HINSTANCE;
#endif

#include "StdAfx.h"

#define PLUGIN_NAME "Online.dll"

#define PLUGIN_LOADED "[" PLUGIN_NAME "] " "Plugin loaded!"
#define PLUGIN_ERROR "[" PLUGIN_NAME "] " "Error loading plugin!"

class CPLuginLoader
{
public:
	CPLuginLoader();
	~CPLuginLoader();

	void LoadPlugin();
	void ReloadPlugins();
	void FreePlugins();
	void(*pRegisterFlowNodes)(void);

	HINSTANCE hndl;

private:
	void(*pInitModule)(SSystemGlobalEnvironment&);
};

extern CPLuginLoader* pPluginLoader;

#endif