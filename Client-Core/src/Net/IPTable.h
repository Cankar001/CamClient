#pragma once

#include "Core/Core.h"

namespace Core
{
	class IPTable
	{
	public:

		struct Node
		{
			Node *Next;
			uint32 Host;
			uint32 Count;
			int64 TimeMS;
		};

		IPTable();

		// Resets the data for the table.
		void Reset();

		// Inserts a host into the table.
		void Insert(uint32 host, int64 now_ms);

		// Removes a host from the table.
		void Remove(uint32 host);

		// Returns true if the host should be blocked from connecting.
		bool Blocked(uint32 host);

	private:

		// Timeout value.
		static constexpr uint32 const TIMEOUT_MS = 300000;

		// Maximum number of connections per IP before the time-out period.
		static constexpr uint32 const MAX_CONNECTIONS_PER_IP = 16;

		uint32 m_Num;
		Node m_Data[65536];
		Node *m_Table[16384];
	};
}

