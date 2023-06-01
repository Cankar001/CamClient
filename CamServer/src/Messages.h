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
};

struct header_t
{
	uint16 Version;
	uint16 Type;
};

struct ClientConnectionStartMessage
{
	header_t Header;

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

};

struct ServerConnectionCloseResponse
{
	header_t Header;

};

struct ServerFrameResponse
{
	header_t Header;

};

#pragma pack(pop)

