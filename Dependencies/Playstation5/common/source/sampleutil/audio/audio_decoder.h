/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <scetypes.h>

namespace sce { namespace SampleUtil { namespace Audio {
	namespace Decoder
	{
		enum class Codec
		{
			  kMP3
			, kATRAC9
		};

		enum class ResultCode
		{
			  kInvalid	= 0
			, kOK
			, kError_Unknown
			, kError_Internal
			, kError_InvalidParameter
			, kError_OutOfMemory
			, kError_OutOfResources
			, kError_NotInitializeContext
			, kError_InvalidFormat
			, kError_NotSupportedCodec
			, kError_NotSupportedFormat
			, kError_Parser_Frame
			, kError_Native_Unknown
			, kError_Native_Invernal
			, kError_Native_InvalidParameter
			, kError_Native_OutOfMemory
			, kError_Native_OutOfResources
			, kError_Native_InvalidFormat
			, kError_Native_RegsterCodec
			, kError_Native_CreateDecoderInstance
			, kError_Native_InitializeBatch
			, kError_Native_InitializeBatchJob
			, kError_Native_BindBatchJob
			, kError_Native_DecodeSetup
			, kError_Native_StartBatch
			, kError_Native_WaitBatch
			, kError_Native_Decode
		};

		struct Context;

		ResultCode initialize( Context *& outContext, const Codec & codec, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes );
		ResultCode finalize( Context *& inoutContext );
		ResultCode decode( Context & context, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes, void *& outOutputPcmBuffer, const size_t outputPcmBufferSizeInBytes, size_t & outOutOutputPcmBufferInBytes );
	} /* namespace Decoder */
} /* namespace Audio */ } /* namespace SampleUtil */ } /* namespace sce */
