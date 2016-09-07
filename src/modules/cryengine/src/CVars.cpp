/*************************************************************************
Copyright (C), chernecoff@gmail.com, 2016

*************************************************************************/

#include "StdAfx.h"
#include "CVars.h"
#include "Global.h"


bool CModuleCVars::RegisterCVars()
{
	gEnv->pLog->Log(TITLE "Register CVars");

	if(gEnv->pConsole)
	{
		gEnv->pConsole->RegisterString("online_ms_address", gCryModule->online_ms_address, VF_NULL, "Master server address");
		gEnv->pConsole->RegisterInt("online_ms_port", gCryModule->online_ms_port, VF_NULL, "Master server port");
		return true;
	}

	gEnv->pLog->LogError(TITLE "Register CVars failed!");
	return false;
}


bool CModuleCVars::RegistevCComands()
{
	gEnv->pLog->Log(TITLE "Register console commands");

	if(gEnv->pConsole)
	{
		//gEnv->pConsole->AddCommand("command_name", Function, VF_NULL,"Description");
		return true;
	}

	gEnv->pLog->LogError(TITLE "Register console commands failed!");
	return false;
}