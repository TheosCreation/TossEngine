/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <streambuf>
#include <vector>
#include <sampleutil/sampleutil_common.h>

namespace sce {	namespace SampleUtil { namespace Helper {
	struct MemoryStreambuf : public std::streambuf
	{
		const size_t kMaxTempBufferSize = 4 * 1024 * 1024;

		std::vector<uint8_t> 	&m_buffer;

		MemoryStreambuf(std::vector<uint8_t> &buffer)
			: m_buffer	(buffer)
		{
			m_buffer.resize(kMaxTempBufferSize);
			char *pBuffer = reinterpret_cast<char *>(m_buffer.data());
			setp(pBuffer, &pBuffer[kMaxTempBufferSize]);
			setg(pBuffer, pBuffer, &pBuffer[kMaxTempBufferSize]);
		}

		void	close()
		{
			m_buffer.resize(pptr() - reinterpret_cast<char *>(m_buffer.data()));
		}

		~MemoryStreambuf()
		{
			close();
		}

		std::ios::pos_type	seekoff(std::ios::off_type __off, std::ios_base::seekdir __way, std::ios_base::openmode __which = std::ios_base::out) override
		{
			SCE_SAMPLE_UTIL_ASSERT_MSG(__off  < (std::ios::off_type)kMaxTempBufferSize, "run short of stream buffer temp memory.");
			std::ios::pos_type result;

			if ((__which & std::ios_base::out) != std::ios_base::out)
			{
				result = std::ios::pos_type(std::ios::off_type(-1));
			} else {
				char* targetOut = nullptr;
				switch (__way)
				{
				case std::ios_base::beg:
					targetOut = this->pbase() + __off;
					break;
				case std::ios_base::end:
					targetOut = this->epptr() + __off - 1;
					break;
				case std::ios_base::cur:
					targetOut = this->pptr() + __off;
					break;
				}

				if (this->pbase() <= targetOut && targetOut < this->epptr())
				{
					result = targetOut - this->pbase();

					if (pptr() != targetOut)
					{
						this->pbump(static_cast<int>(targetOut - this->pptr()));
					}
				} else {
					result = std::ios::pos_type(std::ios::off_type(-1));
				}
			}

			return result;
		}

		std::ios::pos_type	seekpos(std::ios::pos_type __sp, std::ios_base::openmode __which = std::ios_base::in | std::ios_base::out) override
		{
			return seekoff(off_type(__sp), std::ios_base::beg, __which);
		}
	}; // struct MemoryStreambuf

	struct StreamOutToMem : public std::ostream
	{
		StreamOutToMem(MemoryStreambuf &streambuf) : std::ostream(&streambuf, std::ios_base::binary) {}
	};

	struct StreamInFromMem : public std::istream
	{
		StreamInFromMem(MemoryStreambuf &streambuf) : std::istream(&streambuf, std::ios_base::binary) {}
	};
} } } // namespace sce::sampleUtil::Helper
