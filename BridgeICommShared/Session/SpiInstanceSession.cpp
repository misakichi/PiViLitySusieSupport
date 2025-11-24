#include "stdafx.h"
#include "SpiInstanceSession.h"

using namespace PvlIpc;
CSpiInstanceSession::CSpiInstanceSession()
	: CInstanceSession()
{
	loadPluginEvent_ = CreateEvent(NULL, TRUE, FALSE, NULL);
}	

CSpiInstanceSession::~CSpiInstanceSession()
{
}

/// <summary>
/// 
/// </summary>
/// <param name="pluginPath"></param>
/// <returns></returns>
int CSpiInstanceSession::LoadPlugin(const wchar_t* pluginPath)
{
	if(pluginStatus_!= PluginStatus::None && pluginStatus_!=PluginStatus::Failed)
	{
		//すでにロード中、またはロード済み
		return -1;
	}

	pluginStatus_ = PluginStatus::Loading;
	auto len = wcslen(pluginPath);
	auto bytes = (len + 1) * sizeof(wchar_t) + sizeof(SpiSessionMessageLoadPlugin);
	auto loadMsg = (SpiSessionMessageLoadPlugin*)alloca(bytes);
	loadMsg->Init();
	wcscpy_s(loadMsg->pluginPath, len + 1, pluginPath);
	loadMsg->pluginPath[len] = L'\0';
	loadMsg->bytes = bytes;
	
	ResetEvent(loadPluginEvent_);
	SendCommMessage(*loadMsg, L"SpiSessionMessageLoadPlugin");
	WaitForSingleObject(loadPluginEvent_, INFINITE);

	return pluginStatus_ == PluginStatus::Loaded ? 0 : -2;
}


SessionUpdateResult CSpiInstanceSession::UpdateInner(SessionMessageHeader* _msg)
{
	auto msg = (SpiSessionMessageHeader*)_msg;
	if(auto noneMsg = msg->Cast<SpiSessionMessageNone>())
	{
		//何もしないメッセージ
		return SessionUpdateResult::Enum::Succcess;
	}
	else if(auto loadPluginMsg = msg->Cast<SpiSessionMessageLoadPlugin>())
	{
		//プラグインロード応答		
		if(auto module = LoadLibraryW(loadPluginMsg->pluginPath))
		{
			pluginModule_ = module;
			pluginStatus_ = PluginStatus::Loaded;
		}
		else
		{
			pluginStatus_ = PluginStatus::Failed;
		}
		wprintf(L"Load Plugin Result: %s -> %p\n", loadPluginMsg->pluginPath, pluginModule_);
		SetEvent(loadPluginEvent_);
	}
	return Base::UpdateInner(msg);
}
