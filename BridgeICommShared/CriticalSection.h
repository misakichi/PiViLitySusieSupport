#pragma once

namespace PvlIpc
{

	struct SessionMessageHeader;


	class CCriticalSection
	{
	public:
		CCriticalSection();
		~CCriticalSection();
		void Enter();
		void Leave();

		//std::lock_guard—p
		inline void lock() { Enter(); }
		inline void unlock() { Leave(); }
	private:
		CRITICAL_SECTION cs_;
	};

	class CCriticalSectionLock
	{
	public:
		CCriticalSectionLock(CCriticalSection& cs) : cs_(cs)
		{
			cs_.Enter();
		}
		~CCriticalSectionLock()
		{
			cs_.Leave();
		}
	private:
		CCriticalSection& cs_;
	};

}
