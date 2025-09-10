/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <cstdint>
#include <istream>
#include <string>
#include <array>
#include <vector>
#include <type_traits>
#include "sampleutil/sampleutil_common.h"

namespace sce { namespace SampleUtil { namespace Graphics {
namespace Offline
{
#if !defined(DOXYGEN_IGNORE)
	static inline void	readString(std::istream	&is, std::string	&str)
	{
		uint32_t length;
		is.read(reinterpret_cast<char *>(&length), sizeof(length));
		SCE_SAMPLE_UTIL_ASSERT(length < 0x400u);
		str.resize(length);
		if (length > 0)
		{
			is.read(&str[0], length);
		}
	}

	template<typename T>
	static inline typename std::enable_if<std::is_trivially_copyable<T>::value>::type	readValue(std::istream	&is, T	&data)
	{
		is.read(reinterpret_cast<char *>(&data), sizeof(T));
	}

	template<typename T, size_t N>
	static inline typename std::enable_if<std::is_trivially_copyable<T>::value>::type	readArray(std::istream	&is, std::array<T,N>	&data)
	{
		is.read(reinterpret_cast<char *>(data.data()), sizeof(T) * N);
	}

	template<typename T>
	static inline typename std::enable_if<std::is_trivially_copyable<T>::value>::type	readVector(std::istream	&is, std::vector<T>	&data)
	{
		uint32_t size;
		is.read(reinterpret_cast<char *>(&size), sizeof(size));
		SCE_SAMPLE_UTIL_ASSERT(size < 0x800000u);
		if (size > 0)
		{
			data.resize(size);
			is.read(reinterpret_cast<char *>(data.data()), size * sizeof(T));
		}
	}
#endif
} // namespace Offline
} } } // namespace sce::SampleUtil::Graphics