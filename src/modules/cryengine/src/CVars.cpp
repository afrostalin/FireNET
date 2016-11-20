// Copyright © 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include "CVars.h"
#include "Global.h"

void SimpleFunctionForRegister(IConsoleCmdArgs* pArgs)
{
}

bool CModuleCVars::RegisterCVars()
{
	CryLog(TITLE "Register CVars...");

	if(gEnv->pConsole)
	{
		gEnv->pConsole->RegisterString("firenet_ip", gModuleEnv->m_firenet_ip, VF_NULL, "FireNET ip address");
		gEnv->pConsole->RegisterInt("firenet_port", gModuleEnv->m_firenet_port, VF_NULL, "FireNET port");
		gEnv->pConsole->RegisterFloat("firenet_timeout", gModuleEnv->m_timeout, VF_NULL, "FireNET timeout in seconds");

		CryLog(TITLE "CVars registered");
		return true;
	}

	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Register CVars failed");
	return false;
}


bool CModuleCVars::RegisterCComands()
{
/*	CryLog(TITLE "Register console commands...");

	if(gEnv->pConsole)
	{
		gEnv->pConsole->AddCommand("firenet_connect", ConnectToFireNET, VF_NULL, "Connect to FireNET");
		gEnv->pConsole->AddCommand("firenet_disconnect",DisconnectFromFireNET, VF_NULL, "Disconnect from FireNET");
		gEnv->pConsole->AddCommand("firenet_reg_gs", RegisterGameServer, VF_NULL, "Register game server in FireNET");
		return true;
	}

	CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE "Register console commands failed!");
	return false;*/

	return false;
}