#pragma once

struct SpiBridgeMessageHeader;

class CSessionMessage
{
public:
	CSessionMessage();
	~CSessionMessage();
protected:
	void InitIo(HANDLE in, HANDLE out);
	SpiBridgeMessageHeader* ReadMessageBase();

private:
	HANDLE	in_;
	HANDLE	out_;

};

