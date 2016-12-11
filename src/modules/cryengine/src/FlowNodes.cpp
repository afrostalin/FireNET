// Copyright � 2016 Ilya Chernetsov. All rights reserved. Contacts: <chernecoff@gmail.com>
// License: http://opensource.org/licenses/MIT

#include <StdAfx.h>
#include <CryFlowGraph/IFlowBaseNode.h>
#include "Global.h"
#include "NetPacket.h"

#include "Shellapi.h"

#ifndef DEDICATED_SERVER

namespace FireNET_FlowNodes
{
	// Send login request
	class CFlowNode_Login : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Send = 0,
			EIP_Login,
			EIP_Password,
		};

	public:
		CFlowNode_Login(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_Login()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_Login(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Send", _HELP("Send login request")),
				InputPortConfig<string>("Login", _HELP("Email")),
				InputPortConfig<string>("Password", _HELP("Password")),
				{0}
			};
			static const SOutputPortConfig out_ports[] =
			{
				{0}
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send login request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Send))
				{
					if (GetPortString(pActInfo, EIP_Login).size() < 5)
					{
						SUIArguments args;
						args.AddArgument("@shortLogin");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{
						if (GetPortString(pActInfo, EIP_Password).size() < 6)
						{
							SUIArguments args;
							args.AddArgument("@shortPassword");
							gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
						}
						else
						{
							string login = GetPortString(pActInfo, EIP_Login);
							string password = GetPortString(pActInfo, EIP_Password);

							NetPacket packet(net_Query);
							packet.WriteInt(net_query_auth);
							packet.WriteString(login.c_str());
							packet.WriteString(password.c_str());

							if (gModuleEnv->pNetwork)
								gModuleEnv->pNetwork->SendQuery(packet);
						}
					}
				}
			}
			break;
			}
		}

	protected:
		SActivationInfo m_actInfo;
	};

	// Send register request
	class CFlowNode_Register : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Send = 0,
			EIP_Login,
			EIP_Password,
		};

	public:
		CFlowNode_Register(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_Register()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_Register(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Send", _HELP("Send register request")),
				InputPortConfig<string>("Login", _HELP("Login")),
				InputPortConfig<string>("Password", _HELP("Password")),
				{0}
			};
			static const SOutputPortConfig out_ports[] =
			{
				{0}
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send register request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Send))
				{

					if (GetPortString(pActInfo, EIP_Login).size() < 5)
					{
						SUIArguments args;
						args.AddArgument("@shortLogin");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						if (GetPortString(pActInfo, EIP_Password).size() < 6)
						{
							SUIArguments args;
							args.AddArgument("@shortPassword");
							gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
						}
						else
						{
							string login = GetPortString(pActInfo, 1);
							string password = GetPortString(pActInfo, 2);

							NetPacket packet(net_Query);
							packet.WriteInt(net_query_register);
							packet.WriteString(login.c_str());
							packet.WriteString(password.c_str());

							if (gModuleEnv->pNetwork)
								gModuleEnv->pNetwork->SendQuery(packet);
						}
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Create profile
	class CFlowNode_CreateProfile : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Send = 0,
			EIP_Nickname,
			EIP_Model,
		};

	public:
		CFlowNode_CreateProfile(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_CreateProfile()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_CreateProfile(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Send", _HELP("Send create profile request")),
				InputPortConfig<string>("Nickname", _HELP("Character nickname")),
				InputPortConfig<string>("Model", _HELP("Character model")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send create profile request to online server");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Send))
				{
					if (GetPortString(pActInfo, EIP_Nickname).size() < 4)
					{
						SUIArguments args;
						args.AddArgument("@shortNickname");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						string nickname = GetPortString(pActInfo, EIP_Nickname);
						string model = GetPortString(pActInfo, EIP_Model);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_create_profile);
						packet.WriteString(nickname.c_str());
						packet.WriteString(model.c_str());

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Get profile
	class CFlowNode_GetProfile : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Get = 0,
		};

	public:
		CFlowNode_GetProfile(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_GetProfile()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_GetProfile(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Get", _HELP("Get profile from FireNET")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send get profile request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Get))
				{
					NetPacket packet(net_Query);
					packet.WriteInt(net_query_get_profile);

					if (gModuleEnv->pNetwork)
						gModuleEnv->pNetwork->SendQuery(packet);
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Get shop items
	class CFlowNode_GetShopItems : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Get = 0,
		};

	public:
		CFlowNode_GetShopItems(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_GetShopItems()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_GetShopItems(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Get", _HELP("Get items in shop from FireNET")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Get items in shop from FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Get))
				{
					NetPacket packet(net_Query);
					packet.WriteInt(net_query_get_shop);

					if (gModuleEnv->pNetwork)
						gModuleEnv->pNetwork->SendQuery(packet);
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Buy item
	class CFlowNode_BuyItem : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Send = 0,
			EIP_Item,
		};

	public:
		CFlowNode_BuyItem(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_BuyItem()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_BuyItem(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Send", _HELP("Send buy item request")),
				InputPortConfig<string>("ItemName", _HELP("Item name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send buy item request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Send))
				{
					if (GetPortString(pActInfo, EIP_Item).size() < 3)
					{
						SUIArguments args;
						args.AddArgument("@shortItemName");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						string itemName = GetPortString(pActInfo, EIP_Item);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_buy_item);
						packet.WriteString(itemName.c_str());

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Remove item
	class CFlowNode_RemoveItem : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Remove = 0,
			EIP_Item,
		};

	public:
		CFlowNode_RemoveItem(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_RemoveItem()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_RemoveItem(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Remove", _HELP("Send remove item request")),
				InputPortConfig<string>("ItemName", _HELP("Item name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send remove item request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Remove))
				{
					if (GetPortString(pActInfo, EIP_Item).size() < 3)
					{
						SUIArguments args;
						args.AddArgument("@shortItemName");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						string itemName = GetPortString(pActInfo, EIP_Item);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_remove_item);
						packet.WriteString(itemName.c_str());

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Send invite
	class CFlowNode_SendInvite : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_SendFriendInvite = 0,
			EIP_SendGameInvite,
			EIP_SendClanInvite,
			EIP_UserName,
			EIP_ServerIp,
			EIP_MapName,
			EIP_ClanName,
		};

	public:
		CFlowNode_SendInvite(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_SendInvite()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_SendInvite(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("SendFriendInvite", _HELP("Send friend invite")),
				InputPortConfig_Void("SendGameInvite", _HELP("Send game invite")),
				InputPortConfig_Void("SendClanInvite", _HELP("Send clan invite")),
				InputPortConfig<string>("UserName", _HELP("User name")),
				InputPortConfig<string>("ServerIP", _HELP("Server ip")),
				InputPortConfig<string>("MapName", _HELP("Server map name")),
				InputPortConfig<string>("ClanName", _HELP("Clan name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send invite");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_SendFriendInvite))
				{
					if (GetPortString(pActInfo, EIP_UserName) == gModuleEnv->m_profile->nickname)
					{
						SUIArguments args;
						args.AddArgument("@youCannotSendInviteYourself");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
						break;
					}

					if (GetPortString(pActInfo, EIP_UserName).size() < 4)
					{
						SUIArguments args;
						args.AddArgument("@shortUserName");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{
						bool userAdded = false;
						string userName = GetPortString(pActInfo, EIP_UserName);

						for (auto it = gModuleEnv->m_friends.begin(); it != gModuleEnv->m_friends.end(); ++it)
						{
							if (it->nickname == userName)
							{
								SUIArguments args;
								args.AddArgument("@thisUserAlredyAddedToYouFriends");
								gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
								userAdded = true;
								break;
							}
						}

						if (!userAdded)
						{
							NetPacket packet(net_Query);
							packet.WriteInt(net_query_send_invite);
							packet.WriteString(userName.c_str());

							if (gModuleEnv->pNetwork)
								gModuleEnv->pNetwork->SendQuery(packet);
						}
					}
				}

				if (IsPortActive(pActInfo, EIP_SendGameInvite))
				{
				}
				if (IsPortActive(pActInfo, EIP_SendClanInvite))
				{
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Decline invite
	class CFlowNode_DeclineInvite : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Decline = 0,
			EIP_UserName,
		};

	public:
		CFlowNode_DeclineInvite(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_DeclineInvite()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_DeclineInvite(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("DeclineInvite", _HELP("Decline invite from user")),
				InputPortConfig<string>("UserName", _HELP("User name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Decline invite from user");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Decline))
				{
					if (GetPortString(pActInfo, EIP_UserName).size() < 4)
					{
						SUIArguments args;
						args.AddArgument("@shortUserName");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{
						string userName = GetPortString(pActInfo, EIP_UserName);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_decline_invite);
						packet.WriteString(userName.c_str());

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Add friend (Need execute after accept invite)
	class CFlowNode_AddFriend : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Add = 0,
			EIP_Friend,
		};

	public:
		CFlowNode_AddFriend(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_AddFriend()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_AddFriend(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Add", _HELP("Send add friend request")),
				InputPortConfig<string>("FriendName", _HELP("Friend name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send add friend request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Add))
				{
					if (GetPortString(pActInfo, EIP_Friend).size() < 4)
					{
						SUIArguments args;
						args.AddArgument("@shortFriendName");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						string friendName = GetPortString(pActInfo, EIP_Friend);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_add_friend);
						packet.WriteString(friendName.c_str());

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Remove friend
	class CFlowNode_RemoveFriend : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Remove = 0,
			EIP_Friend,
		};

	public:
		CFlowNode_RemoveFriend(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_RemoveFriend()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_RemoveFriend(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Remove", _HELP("Send remove friend request")),
				InputPortConfig<string>("FriendName", _HELP("Friend name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send remove friend request to FireNET");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Remove))
				{
					if (GetPortString(pActInfo, EIP_Friend).size() < 4)
					{
						SUIArguments args;
						args.AddArgument("@shortFriendName");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						string friendName = GetPortString(pActInfo, EIP_Friend);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_remove_friend);
						packet.WriteString(friendName.c_str());

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Get friend list
	class CFlowNode_GetFriends : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Get = 0,
		};

	public:
		CFlowNode_GetFriends(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_GetFriends()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_GetFriends(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Get", _HELP("Get friend list")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Get friend list");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Get))
				{
					for (auto it = gModuleEnv->m_friends.begin(); it != gModuleEnv->m_friends.end(); ++it)
					{
						SUIArguments args;
						args.AddArgument(it->nickname);
						args.AddArgument(it->uid);

						if (it->status == 0)
							args.AddArgument("offline");
						if (it->status == 1)
							args.AddArgument("online");
						if (it->status == 3)
							args.AddArgument("ingame");
						if (it->status == 4)
							args.AddArgument("afk");

						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnFriendRecived, args);

						CryLog(TITLE  "Add friend from friend list buffer");
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Send global chat message
	class CFlowNode_SendGlobalChatMessage : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_SendGlobal = 0,
			EIP_Message,
		};

	public:
		CFlowNode_SendGlobalChatMessage(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_SendGlobalChatMessage()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_SendGlobalChatMessage(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("SendGlobalChatMessagw", _HELP("Send message to global chat")),
				InputPortConfig<string>("Message", _HELP("Message")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send chat message to global chat");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_SendGlobal))
				{
					if (GetPortString(pActInfo, EIP_Message).size() == 0)
					{
						SUIArguments args;
						args.AddArgument("@shortMessage");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{

						string message = GetPortString(pActInfo, EIP_Message);

						NetPacket packet(net_Query);
						packet.WriteInt(net_query_send_chat_msg);
						packet.WriteString(message.c_str());
						packet.WriteString("all");

						if (gModuleEnv->pNetwork)
							gModuleEnv->pNetwork->SendQuery(packet);
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Send private message
	class CFlowNode_SendPrivateChatMessage : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Send,
			EIP_Message,
			EIP_Reciver,
		};

	public:
		CFlowNode_SendPrivateChatMessage(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_SendPrivateChatMessage()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_SendPrivateChatMessage(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Send", _HELP("Send private chat message")),
				InputPortConfig<string>("Message", _HELP("Message")),
				InputPortConfig<string>("Reciver", _HELP("Message reciver")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send private chat message");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Send))
				{
					if (GetPortString(pActInfo, EIP_Message).size() == 0)
					{
						SUIArguments args;
						args.AddArgument("@shortMessage");
						gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnServerResultRecived, args);
					}
					else
					{
						string message = GetPortString(pActInfo, EIP_Message);
						string reciver = GetPortString(pActInfo, EIP_Reciver);

						if (reciver != "")
						{
							NetPacket packet(net_Query);
							packet.WriteInt(net_query_send_chat_msg);
							packet.WriteString(message.c_str());
							packet.WriteString(reciver.c_str());

							if (gModuleEnv->pNetwork)
								gModuleEnv->pNetwork->SendQuery(packet);
						}
						else
						{
							SUIArguments args;
							args.AddArgument("@emptyReciver");
							gModuleEnv->pUIEvents->SendEvent(CModuleUIEvents::eUIGE_OnError, args);
						}
					}
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Spawn player model
	class CFlowNode_SpawnPlayerModel : public CFlowBaseNode<eNCT_Singleton>
	{
		enum EInputPorts
		{
			EIP_Spawn,
			EIP_ArchetypeName,
			EIP_Name,
			EIP_Pos,
			EIP_Rot,
			EIP_Scale,
			EIP_Model,
		};

		enum EOutputPorts
		{
			EOP_Done,
			EOP_Succeeded,
			EOP_Failed,
		};

	public:
		////////////////////////////////////////////////////
		CFlowNode_SpawnPlayerModel(SActivationInfo* pActInfo)
		{

		}

		////////////////////////////////////////////////////
		virtual ~CFlowNode_SpawnPlayerModel(void)
		{

		}

		////////////////////////////////////////////////////
		virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
		{

		}

		////////////////////////////////////////////////////
		virtual void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig inputs[] =
			{
				InputPortConfig_Void("Spawn", _HELP("Spawn an entity using the values below")),
				InputPortConfig<string>("Archetype", "", _HELP("Entity archetype name"), 0, _UICONFIG("enum_global:entity_archetypes")),
				InputPortConfig<string>("Name",      "", _HELP("Entity's name"),  0,  0),
				InputPortConfig<Vec3>("Pos", _HELP("Initial position")),
				InputPortConfig<Vec3>("Rot", _HELP("Initial rotation")),
				InputPortConfig<Vec3>("Scale", Vec3(1,1,1), _HELP("Initial scale")),
				InputPortConfig<string>("Model","",_HELP("Model name"), 0, 0),
				{ 0 }
			};

			static const SOutputPortConfig outputs[] =
			{
				OutputPortConfig_Void("Done", _HELP("Called when job is done")),
				OutputPortConfig<EntityId>("Succeeded", _HELP("Called when entity is spawned")),
				OutputPortConfig_Void("Failed", _HELP("Called when entity fails to spawn")),
				{ 0 }
			};

			config.pInputPorts = inputs;
			config.pOutputPorts = outputs;
			config.sDescription = _HELP("Spawns an archetype entity with the specified properties");
			config.SetCategory(EFLN_ADVANCED);
		}

		////////////////////////////////////////////////////
		virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
		{
			switch (event)
			{
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Spawn))
				{
					// Get properties
					string archName(GetPortString(pActInfo, EIP_ArchetypeName));
					string name(GetPortString(pActInfo, EIP_Name));
					Vec3 pos(GetPortVec3(pActInfo, EIP_Pos));
					Vec3 rot(GetPortVec3(pActInfo, EIP_Rot));
					Vec3 scale(GetPortVec3(pActInfo, EIP_Scale));

					// Define
					IEntity* pEntity = NULL;
					SEntitySpawnParams params;
					IEntityArchetype* pArchetype = gEnv->pEntitySystem->LoadEntityArchetype(archName.c_str());
					if (NULL != pArchetype)
					{
						params.nFlags = ENTITY_FLAG_SPAWNED;
						params.pArchetype = pArchetype;
						params.sName = name.empty() ? pArchetype->GetName() : name.c_str();
						params.vPosition = pos;
						params.vScale = scale;

						Matrix33 mat;
						Ang3 ang(DEG2RAD(rot.x), DEG2RAD(rot.y), DEG2RAD(rot.z));
						mat.SetRotationXYZ(ang);
						params.qRotation = Quat(mat);

						// Create
						int nCastShadowMinSpec;
						if (XmlNodeRef objectVars = pArchetype->GetObjectVars())
						{
							objectVars->getAttr("CastShadowMinSpec", nCastShadowMinSpec);

							static ICVar* pObjShadowCastSpec = gEnv->pConsole->GetCVar("e_ObjShadowCastSpec");
							if (nCastShadowMinSpec <= pObjShadowCastSpec->GetIVal())
								params.nFlags |= ENTITY_FLAG_CASTSHADOW;

							bool bRecvWind;
							objectVars->getAttr("RecvWind", bRecvWind);
							if (bRecvWind)
								params.nFlags |= ENTITY_FLAG_RECVWIND;

							bool bOutdoorOnly;
							objectVars->getAttr("OutdoorOnly", bOutdoorOnly);
							if (bRecvWind)
								params.nFlags |= ENTITY_FLAG_OUTDOORONLY;

							bool bNoStaticDecals;
							objectVars->getAttr("NoStaticDecals", bNoStaticDecals);
							if (bNoStaticDecals)
								params.nFlags |= ENTITY_FLAG_NO_DECALNODE_DECALS;
						}

						pEntity = gEnv->pEntitySystem->SpawnEntity(params);
						pEntity->EnablePhysics(false);

						if (pEntity)
						{
							IEntityRenderProxy* pRenderProx = (IEntityRenderProxy*)pEntity->GetProxy(ENTITY_PROXY_RENDER);
							if (pRenderProx)
							{
								if (XmlNodeRef objectVars = pArchetype->GetObjectVars())
								{
									int nViewDistRatio = 100;
									objectVars->getAttr("ViewDistRatio", nViewDistRatio);
									pRenderProx->SetViewDistRatio(nViewDistRatio);

									int nLodRatio = 100;
									objectVars->getAttr("lodRatio", nLodRatio);
									pRenderProx->SetLodRatio(nLodRatio);
								}
							}

							if (XmlNodeRef objectVars = pArchetype->GetObjectVars())
							{
								bool bHiddenInGame = false;
								objectVars->getAttr("HiddenInGame", bHiddenInGame);
								if (bHiddenInGame)
									pEntity->Hide(true);
							}

							if (pEntity->LoadCharacter(0, GetPortString(pActInfo, EIP_Model), 0) != -1)
								CryLog(TITLE  "Player model loaded");
							else
								CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Failed load player model");
						}
					}

					if (pEntity == NULL)
					{
						ActivateOutput(pActInfo, EOP_Failed, true);
						CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Failed spawn player model");
					}
					else
					{
						ActivateOutput(pActInfo, EOP_Succeeded, pEntity->GetId());
						CryLog(TITLE  "Player model spawned on %.f, %.f, %.f", pos.x, pos.y, pos.z);
					}

					ActivateOutput(pActInfo, EOP_Done, true);
				}
			}
			break;
			}
		}

		////////////////////////////////////////////////////
		virtual void GetMemoryUsage(ICrySizer* s) const
		{
			s->Add(*this);
		}
	};

	// Remove entity
	class CFlowNode_RemoveEntity : public CFlowBaseNode<eNCT_Singleton>
	{
		enum EInputPorts
		{
			EIP_Remove,
			EIP_EntityID,
		};

		enum EOutputPorts
		{
			EOP_Done,
			EOP_Failed,
		};

	public:
		////////////////////////////////////////////////////
		CFlowNode_RemoveEntity(SActivationInfo* pActInfo)
		{

		}

		////////////////////////////////////////////////////
		virtual ~CFlowNode_RemoveEntity(void)
		{

		}

		////////////////////////////////////////////////////
		virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
		{

		}

		////////////////////////////////////////////////////
		virtual void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig inputs[] =
			{
				InputPortConfig_Void("Remove", _HELP("Remove entity")),
				InputPortConfig<EntityId>("EntityID", _HELP("Entity id")),
				{ 0 }
			};

			static const SOutputPortConfig outputs[] =
			{
				OutputPortConfig_Void("Done", _HELP("Called when entity removed")),
				OutputPortConfig_Void("Failed",_HELP("Called when entity fails to remove")),
				{ 0 }
			};

			config.pInputPorts = inputs;
			config.pOutputPorts = outputs;
			config.sDescription = _HELP("Remove enity by EntityID");
			config.SetCategory(EFLN_ADVANCED);
		}

		////////////////////////////////////////////////////
		virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
		{
			switch (event)
			{
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Remove))
				{
					EntityId entityID  = GetPortEntityId(pActInfo, EIP_EntityID);

					IEntity* pEntity = gEnv->pEntitySystem->GetEntity(entityID);

					if (pEntity)
					{
						gEnv->pEntitySystem->RemoveEntity(entityID, true);
						ActivateOutput(pActInfo, EOP_Done, true);
					}
					else
					{
						ActivateOutput(pActInfo, EOP_Failed, true);
						CryWarning(VALIDATOR_MODULE_GAME, VALIDATOR_ERROR, TITLE  "Failed remove entity, because entity not found!");
					}
					
				}
			}
			break;
			}
		}

		////////////////////////////////////////////////////
		virtual void GetMemoryUsage(ICrySizer* s) const
		{
			s->Add(*this);
		}
	};

	// Get game server
	class CFlowNode_GetGameServer : public CFlowBaseNode<eNCT_Instanced>
	{
		enum INPUTS
		{
			EIP_Get = 0,
			EIP_Map,
			EIP_Gamerules,
			EIP_Name,
		};

	public:
		CFlowNode_GetGameServer(SActivationInfo * pActInfo)
		{
		}

		~CFlowNode_GetGameServer()
		{
		}

		IFlowNodePtr Clone(SActivationInfo* pActInfo)
		{
			return new CFlowNode_GetGameServer(pActInfo);
		}

		virtual void GetMemoryUsage(ICrySizer * s) const
		{
			s->Add(*this);
		}

		void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig in_ports[] =
			{
				InputPortConfig_Void("Get", _HELP("Get game server from online server by map/gamerules/name")),
				InputPortConfig<string>("MapName", _HELP("Get game server by map name")),
				InputPortConfig<string>("Gamerules", _HELP("Get game server by gamerules")),
				InputPortConfig<string>("ServerName", _HELP("Get game server by name")),
				{ 0 }
			};
			static const SOutputPortConfig out_ports[] =
			{
				{ 0 }
			};
			config.pInputPorts = in_ports;
			config.pOutputPorts = out_ports;
			config.sDescription = _HELP("Send get game server request to online server");
			config.SetCategory(EFLN_APPROVED);
		}

		void ProcessEvent(EFlowEvent event, SActivationInfo *pActInfo)
		{
			switch (event)
			{
			case eFE_Initialize:
			{
				m_actInfo = *pActInfo;
			}
			break;
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Get))
				{
					string mapName = GetPortString(pActInfo, EIP_Map);
					string gamerules = GetPortString(pActInfo, EIP_Gamerules);
					string serverName = GetPortString(pActInfo, EIP_Name);

					NetPacket packet(net_Query);
					packet.WriteInt(net_query_get_server);
					packet.WriteString(mapName.c_str());
					packet.WriteString(gamerules.c_str());
					packet.WriteString(serverName.c_str());

					if (gModuleEnv->pNetwork)
						gModuleEnv->pNetwork->SendQuery(packet);
				}
			}
			break;
			}
		}
	protected:
		SActivationInfo m_actInfo;
	};

	// Open url in default browser
	class CFlowNode_OpenURL : public CFlowBaseNode<eNCT_Singleton>
	{
		enum EInputPorts
		{
			EIP_Open,
			EIP_Url,
		};

		enum EOutputPorts
		{
		};

	public:
		////////////////////////////////////////////////////
		CFlowNode_OpenURL(SActivationInfo* pActInfo)
		{

		}

		////////////////////////////////////////////////////
		virtual ~CFlowNode_OpenURL(void)
		{

		}

		////////////////////////////////////////////////////
		virtual void Serialize(SActivationInfo* pActInfo, TSerialize ser)
		{

		}

		////////////////////////////////////////////////////
		virtual void GetConfiguration(SFlowNodeConfig& config)
		{
			static const SInputPortConfig inputs[] =
			{
				InputPortConfig_Void("Open", _HELP("Open url in browser")),
				InputPortConfig<string>("URL", _HELP("URL for open")),
				{ 0 }
			};

			static const SOutputPortConfig outputs[] =
			{
				{ 0 }
			};

			config.pInputPorts = inputs;
			config.pOutputPorts = outputs;
			config.sDescription = _HELP("Open url in browser");
			config.SetCategory(EFLN_ADVANCED);
		}

		////////////////////////////////////////////////////
		virtual void ProcessEvent(EFlowEvent event, SActivationInfo* pActInfo)
		{
			switch (event)
			{
			case eFE_Activate:
			{
				if (IsPortActive(pActInfo, EIP_Open))
				{
					string url = GetPortString(pActInfo, EIP_Url);
					CryLog(TITLE "Open url = %s", url);
					ShellExecute(0, "OPEN", url.c_str(), NULL, NULL, SW_SHOW);
				}
			}
			break;
			}
		}

		////////////////////////////////////////////////////
		virtual void GetMemoryUsage(ICrySizer* s) const
		{
			s->Add(*this);
		}
	};
}


REGISTER_FLOW_NODE_EX("FireNET:AuthSystem:Login", FireNET_FlowNodes::CFlowNode_Login, CFlowNode_Login);
REGISTER_FLOW_NODE_EX("FireNET:AuthSystem:Register", FireNET_FlowNodes::CFlowNode_Register, CFlowNode_Register);

REGISTER_FLOW_NODE_EX("FireNET:Profile:CreateProfile", FireNET_FlowNodes::CFlowNode_CreateProfile, CFlowNode_CreateProfile);
REGISTER_FLOW_NODE_EX("FireNET:Profile:GetProfile", FireNET_FlowNodes::CFlowNode_GetProfile, CFlowNode_GetProfile);

REGISTER_FLOW_NODE_EX("FireNET:Shop:GetShopItems", FireNET_FlowNodes::CFlowNode_GetShopItems, CFlowNode_GetShopItems);
REGISTER_FLOW_NODE_EX("FireNET:Shop:BuyItem", FireNET_FlowNodes::CFlowNode_BuyItem, CFlowNode_BuyItem);
REGISTER_FLOW_NODE_EX("FireNET:Invenrory:RemoveItem", FireNET_FlowNodes::CFlowNode_RemoveItem, CFlowNode_RemoveItem);

REGISTER_FLOW_NODE_EX("FireNET:Friends:AddFriend", FireNET_FlowNodes::CFlowNode_AddFriend, CFlowNode_AddFriend);
REGISTER_FLOW_NODE_EX("FireNET:Friends:RemoveFriend", FireNET_FlowNodes::CFlowNode_RemoveFriend, CFlowNode_RemoveFriend);
REGISTER_FLOW_NODE_EX("FireNET:Friends:GetFriendList", FireNET_FlowNodes::CFlowNode_GetFriends, CFlowNode_GetFriends);

REGISTER_FLOW_NODE_EX("FireNET:Chat:SendPrivateMessage", FireNET_FlowNodes::CFlowNode_SendPrivateChatMessage, CFlowNode_SendPrivateChatMessage);
REGISTER_FLOW_NODE_EX("FireNET:Chat:SendMessageToGlobalChat", FireNET_FlowNodes::CFlowNode_SendGlobalChatMessage, CFlowNode_SendGlobalChatMessage);

// This need only for creating 3D menu with character selection
REGISTER_FLOW_NODE_EX("FireNET:Other:SpawnPlayerAI", FireNET_FlowNodes::CFlowNode_SpawnPlayerModel, CFlowNode_SpawnPlayerModel);
REGISTER_FLOW_NODE_EX("FireNET:Other:RemoveEntity", FireNET_FlowNodes::CFlowNode_RemoveEntity, CFlowNode_RemoveEntity);
// ************************************************************
REGISTER_FLOW_NODE_EX("FireNET:Other:OpenURL", FireNET_FlowNodes::CFlowNode_OpenURL, CFlowNode_OpenURL);

REGISTER_FLOW_NODE_EX("FireNET:Invites:SendInvite", FireNET_FlowNodes::CFlowNode_SendInvite, CFlowNode_SendInvite);
REGISTER_FLOW_NODE_EX("FireNET:Invites:DeclineInvite", FireNET_FlowNodes::CFlowNode_DeclineInvite, CFlowNode_DeclineInvite);

REGISTER_FLOW_NODE_EX("FireNET:Matchmaking:GetGameServer", FireNET_FlowNodes::CFlowNode_GetGameServer, CFlowNode_GetGameServer);

#endif