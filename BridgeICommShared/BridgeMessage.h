#pragma once
#include <type_traits>

// {68803E18-6AB1-4C4E-8F71-F62C9006FB89}
static constexpr GUID s_named =
{ 0x68803e18, 0x6ab1, 0x4c4e, { 0x8f, 0x71, 0xf6, 0x2c, 0x90, 0x6, 0xfb, 0x89 } };

constexpr size_t MessageSizeMax = 2048;
/// <summary>
/// �W�����b�Z�[�W�R�}���h�^�C�v
/// </summary>
enum class SpiBridgeMessageType : int32_t
{
	None = 0,
	Standard = 1,
};

/// <summary>
/// ��{���Ƃ胁�b�Z�[�W
/// </summary>
#pragma pack(push, 4) 
struct SpiBridgeMessageHeader
{
	SpiBridgeMessageType msgType;
	int32_t bytes;

	template<typename T>
	T* Cast();
};

/// <summary>
/// �W�����b�Z�[�W�R�}���h
/// </summary>
enum class SpiBridgeStdMessage : int32_t
{
	None = 0,

	// �V�K�Z�b�V�����v��
	NewSession = 100,
	// �V�K�Z�b�V��������
	CreatedSession = 110,
	// �V�K�Z�b�V��������(���s)
	FailedNewSession = 120,

	// �Z�b�V��������v��
	FreeSession = 200,

	// �I���v��
	Exit = 1000,
};

/// <summary>
/// �W�����b�Z�[�W�w�b�_�[
/// </summary>
struct SpiBridgeStandardMessageHeader : SpiBridgeMessageHeader
{
	static constexpr SpiBridgeMessageType MsgType = SpiBridgeMessageType::Standard;
	static constexpr SpiBridgeStdMessage Message = SpiBridgeStdMessage::None;
	void Init()
	{
		bytes = sizeof(*this);
		msgType = MsgType;
		message = Message;
	}
	SpiBridgeStdMessage message;

};
using SpiBridgeStandardMessageNone = SpiBridgeStandardMessageHeader;

/// <summary>
/// �V�K�Z�b�V�����v�����b�Z�[�W
/// </summary>
struct SpiBridgeStandardMessageNewSession : SpiBridgeStandardMessageHeader
{
	static constexpr SpiBridgeMessageType MsgType = SpiBridgeMessageType::Standard;
	static constexpr SpiBridgeStdMessage Message = SpiBridgeStdMessage::NewSession;
	void Init()
	{
		bytes = sizeof(*this);
		msgType = MsgType;
		message = Message;
	}
};

struct SpiBridgeStandardMessageCreatedSession : SpiBridgeStandardMessageHeader
{
	static constexpr SpiBridgeMessageType MsgType = SpiBridgeMessageType::Standard;
	static constexpr SpiBridgeStdMessage Message = SpiBridgeStdMessage::CreatedSession;
	void Init()
	{
		bytes = sizeof(*this);
		msgType = MsgType;
		message = Message;
	}
	int32_t sessionId;
	wchar_t namedSuffix[256];

};

struct SpiBridgeStandardMessageFailedNewSession : SpiBridgeStandardMessageHeader
{
	static constexpr SpiBridgeMessageType MsgType = SpiBridgeMessageType::Standard;
	static constexpr SpiBridgeStdMessage Message = SpiBridgeStdMessage::FailedNewSession;
	void Init()
	{
		bytes = sizeof(*this);
		msgType = MsgType;
		message = Message;
	}
	int32_t errorCode;
};

struct SpiBridgeStandardMessageFreeSession : SpiBridgeStandardMessageHeader
{
	static constexpr SpiBridgeMessageType MsgType = SpiBridgeMessageType::Standard;
	static constexpr SpiBridgeStdMessage Message = SpiBridgeStdMessage::FreeSession;
	void Init()
	{
		bytes = sizeof(*this);
		msgType = MsgType;
		message = Message;
	}
	int32_t processId;
	int32_t sessionId;
};

struct SpiBridgeStandardMessageExit : SpiBridgeStandardMessageHeader
{
	static constexpr SpiBridgeMessageType MsgType = SpiBridgeMessageType::Standard;
	static constexpr SpiBridgeStdMessage Message = SpiBridgeStdMessage::Exit;
	void Init()
	{
		bytes = sizeof(*this);
		msgType = MsgType;
		message = Message;
	}
};
#pragma pack(pop) 

template<typename T>
inline T* SpiBridgeMessageHeader::Cast()
{
	static_assert(sizeof(T) < MessageSizeMax, "Message Structure is over the tls buffer size.");
	auto tThis = reinterpret_cast<T*>(this);
	bool isOk = T::MsgType == tThis->msgType;
	if constexpr (!std::is_same_v < SpiBridgeStandardMessageHeader, T>)
	{
		isOk = T::Message == tThis->message ? isOk : false;
	}
	return isOk ? tThis : nullptr;
}




