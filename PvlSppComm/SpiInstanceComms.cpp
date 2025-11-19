#include "stdafx.h"
#include "SpiInstanceComms.h"
#include "Session/BridgeMessage.h"
#include "Session/StandardSession.h"
#include "Session/SpiInstanceSession.h"
#include "Pipe.h"
#include <thread>
#include <vcclr.h>
using namespace System;
using namespace System::Threading::Tasks;
using namespace PvlSppComm;
using namespace System::Runtime::InteropServices;

/// <summary>
/// 指定プラグインのロード
/// </summary>
/// <param name="pluginPath"></param>
/// <returns></returns>
Task<bool>^ SpiInstanceComms::LoadPlugin(String^ pluginPath)
{
	TaskFactory<bool>^ factory = gcnew TaskFactory<bool>();
	return factory->StartNew(gcnew Func<Object^,bool>(this, &SpiInstanceComms::LoadPluginInner), pluginPath);
}


/// <summary>
/// プラグインロード内部処理
/// </summary>
/// <param name="pluginPath_"></param>
/// <returns></returns>
bool SpiInstanceComms::LoadPluginInner(Object^ pluginPath_)
{
	String^ pluginPath = safe_cast<String^>(pluginPath_);
	pin_ptr<const wchar_t> wPluginPath = PtrToStringChars(pluginPath);
	instance_->LoadPlugin(wPluginPath);
    
    return true;
}