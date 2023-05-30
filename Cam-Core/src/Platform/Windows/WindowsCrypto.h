#pragma once

#ifdef CAM_PLATFORM_WINDOWS

#include "Core/Crypto.h"

#include <Windows.h>
#include <wincrypt.h>

namespace Core
{
	class WindowsCrypto : public Crypto
	{
	public:

		WindowsCrypto();
		~WindowsCrypto();

		virtual bool GenKeys(key_t *pub, key_t *pri) override;
		virtual uint64 GenToken() override;

		virtual bool SignSignature(Byte *sig, uint32 sig_bytes, Byte const *data, uint32 data_size, Byte const *pri_key, uint32 pri_key_bytes) override;
		virtual bool TestSignature(void const *sig, uint32 sig_bytes, void const *src, uint32 src_bytes, Byte const *pub_key, uint32 pub_key_bytes) override;

	private:

		HCRYPTPROV m_HProv;
	};
}

#endif // CAM_PLATFORM_WINDOWS

