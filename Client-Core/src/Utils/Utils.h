#pragma once

namespace Core::utils
{
	// Returns the number of elements in a fixed sized array.
	template <typename T, size_t n>
	constexpr size_t Count(T const (&x)[n])
	{
		return n;
	}

	// Returns the maximum of x or y for a value type.
	template <typename type_t>
	constexpr type_t Max(type_t x, type_t y)
	{
		return (x < y) ? y : x;
	}

	// Returns the mininum of x or y for a value type.
	template <typename type_t>
	constexpr type_t Min(type_t x, type_t y)
	{
		return (y < x) ? y : x;
	}
}

