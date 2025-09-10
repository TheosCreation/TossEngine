/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc. 
 * 
 */
#include <sampleutil/graphics.h>
#include <sampleutil/sampleutil_error.h>

#include <stack>

namespace
{
	sce::SampleUtil::Graphics::HdrFormat	s_hdr = sce::SampleUtil::Graphics::HdrFormat::kBT709;

	std::stack<sce::SampleUtil::Graphics::HdrFormat> s_hdrStack;

} /* anonymous namespace */

namespace sce {	namespace SampleUtil { namespace Graphics {
HdrFormat	getHdr()
{
	return	s_hdr; 
}

int pushHdr(HdrFormat	newHdr)
{
	s_hdrStack.push(s_hdr);	// Push current value.
	s_hdr = newHdr;
	return SCE_OK;
}

int	popHdr()
{
	SCE_SAMPLE_UTIL_ASSERT(0 < s_hdrStack.size());
	if (s_hdrStack.size() == 0) return SCE_SAMPLE_UTIL_ERROR_NOT_READY;
	s_hdr = s_hdrStack.top();
	s_hdrStack.pop();

	return SCE_OK;
}

} /* namespace Graphics */ } /* namespace SampleUtil */ } /* namespace sce */