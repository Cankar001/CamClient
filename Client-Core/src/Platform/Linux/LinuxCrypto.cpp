#include "LinuxCrypto.h"

#ifdef CAM_PLATFORM_LINUX

namespace Core
{
	LinuxCrypto::LinuxCrypto()
	{
	}

	LinuxCrypto::~LinuxCrypto()
	{
	}

	bool LinuxCrypto::GenKeys(key_t *pub, key_t *pri)
	{
		return false;
	}

	uint64 LinuxCrypto::GenToken()
	{
		return uint64();
	}

	bool LinuxCrypto::SignSignature(Byte *sig, uint32 sig_bytes, Byte const *data, uint32 data_size, Byte const *pri_key, uint32 pri_key_bytes)
	{
		return false;
	}

	bool LinuxCrypto::TestSignature(void const *sig, uint32 sig_bytes, void const *src, uint32 src_bytes, Byte const *pub_key, uint32 pub_key_bytes)
	{
		return false;
	}
}

#endif // CAM_PLATFORM_LINUX

