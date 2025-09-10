/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <ostream>
#include <string>
#include <array>
#include <vector>
#include <type_traits>

namespace sce { namespace SampleUtil { namespace Graphics {
namespace Offline
{
	static inline void	writeString(std::ostream	&os, const std::string	&str)
	{
		uint32_t length = static_cast<uint32_t>(str.size());
		os.write(reinterpret_cast<const char *>(&length), sizeof(length));
		if (length > 0)
		{
			os.write(str.data(), length);
		}
	}

	template<typename T>
	static inline typename std::enable_if<std::is_trivially_copyable<T>::value>::type	writeValue(std::ostream	&os, const T	&data)
	{
		os.write(reinterpret_cast<const char *>(&data), sizeof(T));
	}

	template<typename T, size_t N>
	static inline typename std::enable_if<std::is_trivially_copyable<T>::value>::type	writeArray(std::ostream	&os, const std::array<T,N>	&data)
	{
		os.write(reinterpret_cast<const char *>(data.data()), sizeof(T) * N);
	}

	template<typename T>
	static inline typename std::enable_if<std::is_trivially_copyable<T>::value>::type	writeVector(std::ostream	&os, const std::vector<T>	&data, int start = 0, int end = INT_MAX)
	{
		uint32_t size = static_cast<uint32_t>(std::min(end - start, (int)data.size()));
		os.write(reinterpret_cast<const char *>(&size), sizeof(size));
		if (size > 0)
		{
			os.write(reinterpret_cast<const char *>(data.data() + start), size * sizeof(T));
		}
	}
} // namespace Offline
} } } // namespace sce::SampleUtil::Graphics
