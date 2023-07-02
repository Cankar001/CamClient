#pragma once

#include <Cam-Core.h>

enum MessageType : uint16
{
	NONE = 0,
	CLIENT_CONNECTION_START,
	CLIENT_CONNECTION_CLOSE,
	CLIENT_FRAME,
	SERVER_CONNECTION_START,
	SERVER_CONNECTION_CLOSE,
	SERVER_FRAME
};

#pragma pack(push, 1)

struct FrameData
{
	Byte *Frame;
	uint32 FrameSize;
	uint32 FrameWidth;
	uint32 FrameHeight;
	int32 Format;
};

struct header_t
{
	uint16 Version;
	uint16 Type;
};

struct ClientConnectionStartMessage
{
	header_t Header;
	std::string FrameName;
	uint32 FPS;
};

struct ClientConnectionCloseMessage
{
	header_t Header;

};

struct ClientFrameMessage
{
	header_t Header;
	FrameData Frame;
};

struct ServerConnectionStartResponse
{
	header_t Header;
	bool ConnectionAccepted;
};

struct ServerConnectionCloseResponse
{
	header_t Header;
	bool ConnectionClosed;
};

struct ServerFrameResponse
{
	header_t Header;
	bool FrameStored;
	uint32 StoredFrameCount;
};

#pragma pack(pop)

