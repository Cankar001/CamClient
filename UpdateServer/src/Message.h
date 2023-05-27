#pragma once

#include <Client-Core.h>

enum MessageType : uint16
{
	NONE = 0,
	CLIENT_REQUEST_VERSION,
	CLIENT_UPDATE_BEGIN,
	CLIENT_UPDATE_PIECE,
	SERVER_RECEIVE_VERSION,
	SERVER_UPDATE_TOKEN,
	SERVER_UPDATE_BEGIN,
	SERVER_UPDATE_PIECE
};

#define SIG_BYTES 512
#define PIECE_BYTES 1024

struct Signature
{
	Byte Data[SIG_BYTES];
};

#pragma pack(push, 1)

struct header_t
{
	uint16 Version;
	uint16 Type;
};

/// <summary>
/// This message is sent, if the client wants to know the current server version.
/// Corresponds to CLIENT_REQUEST_VERSION message type.
/// </summary>
struct ClientWantsVersionMessage
{
	header_t Header;
	uint32 LocalVersion;
	uint32 ClientVersion;
};

struct ClientUpdateBeginMessage
{
	header_t Header;
	uint64 ClientToken;
	uint64 ServerToken;
	uint32 ClientVersion;
};

struct ClientUpdatePieceMessage
{
	header_t Header;
	uint64 ClientToken;
	uint64 ServerToken;
	uint32 ServerVersion;
	uint32 PiecePos;
};

struct ServerVersionInfoMessage
{
	header_t Header;
	uint32 Version;
};

struct ServerUpdateBeginMessage
{
	header_t Header;
	uint64 ClientToken;
	uint64 ServerToken;
	uint32 ServerVersion;
	uint32 UpdateSize;
	Signature UpdateSignature;
};

struct ServerUpdateTokenMessage
{
	header_t Header;
	uint64 ClientToken;
	uint64 ServerToken;
};

struct ServerUpdatePieceMessage
{
	header_t Header;
	uint64 ClientToken;
	uint64 ServerToken;
	uint32 ServerVersion;
	uint32 PiecePos;
	uint16 PieceSize;
};

#pragma pack(pop)

