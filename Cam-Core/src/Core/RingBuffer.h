#pragma once

#include "Core.h"

namespace Core
{
    template <class T>
    class RingBuffer
    {
    public:

        RingBuffer(uint32 capacity)
        {
            m_ReadPos = 0;
            m_WritePos = 0;
            m_Size = 0;
            m_Capacity = capacity;
            m_Data = ((T *)operator new(m_Capacity * sizeof(T)));
        }

        ~RingBuffer()
        {
            // first destroy any content
            while (m_Size != 0)
                Pop();

            // then release the buffer.
            operator delete(m_Data);
        }

        void Push(T const &t)
        {
            // ensure there's room in buffer:
            if (m_Size == m_Capacity)
                Pop();

            // construct copy of object in-place into buffer
            new(&m_Data[m_WritePos++]) T(t);
            // keep pointer in bounds.
            m_WritePos %= m_Capacity;
            ++m_Size;
        }

        // return oldest object in queue:
        T Front()
        {
            return m_Data[m_ReadPos];
        }

        // remove oldest object from queue:
        void Pop()
        {
            // destroy the object:
            m_Data[m_ReadPos++].~T();

            // keep pointer in bounds.
            m_ReadPos %= m_Capacity;
            --m_Size;
        }

        uint32 Size() const
        {
            return m_Size;
        }

        void Clear()
        {
            // first destroy any content
            while (m_Size != 0)
                Pop();
        }

    private:

        T *m_Data = nullptr;
        uint32 m_ReadPos = 0;
        uint32 m_WritePos = 0;
        uint32 m_Size = 0;
        uint32 m_Capacity = 0;
    };
}

