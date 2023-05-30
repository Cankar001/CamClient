#pragma once

#include "Core.h"

namespace Core
{
	template<typename T>
	class Singleton
	{
	public:

		static T *Get()
		{
			static T _instance;
			return &_instance;
		}

	protected:

		Singleton()
		{
		}

		~Singleton()
		{
		}

	private:

		Singleton(Singleton const &)
		{
		}

		Singleton &operator=(Singleton const &)
		{
			return *this;
		}
	};
}

