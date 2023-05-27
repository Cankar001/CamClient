#include "IPTable.h"

#include <string.h>
#include "Core/Hash.h"
#include "Utils/Utils.h"

namespace Core
{
	IPTable::IPTable()
	{
		Reset();
	}
	
	void IPTable::Reset()
	{
		m_Num = 0;
		memset(m_Table, 0, sizeof(m_Table));
	}
	
	void IPTable::Insert(uint32 host, int64 now_ms)
	{
		auto slot = Core::Mix32(host) % utils::Count(m_Table);
		auto node = m_Table[slot];

		while (node)
		{
			if (node->Host == host)
			{
				break;
			}

			node = node->Next;
		}

		if (!node)
		{
			if (m_Num < utils::Count(m_Data))
			{
				node = &m_Data[m_Num++];
			}
			else
			{
				for (auto &iter : m_Data)
				{
					if (now_ms - iter.TimeMS > TIMEOUT_MS)
					{
						node = &iter;
						break;
					}
				}

				if (!node)
				{
					return;
				}

				Remove(node->Host);
			}

			node->Host = host;
			node->Count = 1;
			node->TimeMS = now_ms;
			node->Next = m_Table[slot];
			m_Table[slot] = node;

			return;
		}

		if (now_ms - node->TimeMS > TIMEOUT_MS)
		{
			node->Count = 1;
			node->TimeMS = now_ms;
		}
		else
		{
			node->Count += 1;
		}
	}
	
	void IPTable::Remove(uint32 host)
	{
		auto slot = Core::Mix32(host) % utils::Count(m_Table);
		Node *node = m_Table[slot];
		Node *prev = nullptr;

		while (node) {
			if (node->Host == host) {
				if (prev) {
					prev->Next = node->Next;
				}
				else {
					m_Table[slot] = node->Next;
				}

				return;
			}

			prev = node;
			node = node->Next;
		}
	}

	bool IPTable::Blocked(uint32 host)
	{
		auto node = m_Table[Core::Mix32(host) % utils::Count(m_Table)];
		while (node)
		{
			if (node->Host == host)
			{
				return node->Count >= MAX_CONNECTIONS_PER_IP;
			}

			node = node->Next;
		}

		return false;
	}
}

