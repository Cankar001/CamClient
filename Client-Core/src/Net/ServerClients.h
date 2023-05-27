#pragma once

#include "Core/Core.h"
#include "Core/Crypto.h"

#include "Socket.h"
#include "IPTable.h"

namespace Core
{
	class Clients
	{
	public:

		class Node
		{
		public:
			Node *Next;
			addr_t Addr;
			uint64 ServerToken;
			int64 TimeMS;
			uint32 Bandwidth;

			// Updates current bandwidth limits. Returns true if node has bandwidth available to it.
			bool IsBandwidthAvailable(int64 now_ms);

		protected:
			// Rate to decrease bandwidth usage, bytes/ms.
			// Best set to a fraction of your maximum expected speed and to increase max_bandwidth instead.
			static constexpr int64 DECREASE_RATE = 100;

			// Maximum expected bandwidth to transfer, bytes.
			static constexpr uint32 MAX_BANDWIDTH = 2000000;
		};

		// Initializes the clients table.
		Clients();

		Clients(Crypto *crypto, IPTable *iptable);

		// Resets the clients table.
		void Reset();

		// Inserts or returns the client for the given address.
		Node *Insert(addr_t addr, int64 now_ms);

		// Removes the client associated with the given address.
		void Remove(addr_t addr);

	protected:

		Crypto *m_Crypto;
		IPTable *m_IPTable;
		uint32 m_Num;
		Node m_Data[65536];
		Node *m_Table[32768];
	};
}

