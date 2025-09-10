/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#include "audio_decoder.h"

#include <stdlib.h>

namespace sce { namespace SampleUtil { namespace Audio {
	namespace Decoder
	{
		namespace Platform
		{
			struct Context;

			ResultCode initialize( Context *& outContext, const Codec & codec, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes );
			ResultCode finalize( Context *& outContext );
			ResultCode decode( Context & context, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes, void *& outOutputPcmBuffer, const size_t outputPcmBufferSizeInBytes, size_t & outOutOutputPcmBufferInBytes );

		} /* namespace Platform */

		struct Context
		{
			Platform::Context * platformContext;
		};

		ResultCode initialize( Context *& outContext, const Codec & codec, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes )
		{
			ResultCode retval = ResultCode::kInvalid;

			Context * tempContext = nullptr;

			do
			{
				tempContext = static_cast < Context * > ( malloc( sizeof( *tempContext ) ) );
				if ( nullptr == tempContext )
				{
					retval = ResultCode::kError_OutOfMemory;
					break; // Exit
				}
				tempContext->platformContext = nullptr;

				const ResultCode result_PlatformInitialize = Platform::initialize( tempContext->platformContext, codec, inputData, inputDataSizeInBytes, outConsumedInputDataSizeInBytes );
				if ( ResultCode::kOK != result_PlatformInitialize )
				{
					retval = result_PlatformInitialize;
					break; // Exit
				}

				// Succeeded.
				retval = ResultCode::kOK;

				outContext = tempContext;
				tempContext = nullptr;
			}
			while ( false );

			if ( nullptr != tempContext )
			{
				finalize( tempContext );
			}

			return retval;
		}

		ResultCode finalize( Context *& inoutContext )
		{
			ResultCode retval = ResultCode::kInvalid;

			do
			{
				if ( nullptr != inoutContext )
				{
					const ResultCode result_PlatformFinalize = Platform::finalize( inoutContext->platformContext );
					if ( ResultCode::kOK != result_PlatformFinalize )
					{
						retval = result_PlatformFinalize;
						break;	// Exit
					}

					free( inoutContext );
					inoutContext = nullptr;
				}

				// Succeeded.
				retval = ResultCode::kOK;
			}
			while ( false );

			return retval;
		}

		ResultCode decode( Context & context, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes, void *& outOutputPcmBuffer, const size_t outputPcmBufferSizeInBytes, size_t & outOutOutputPcmBufferInBytes )
		{
			ResultCode retval = ResultCode::kInvalid;

			do
			{
				if ( nullptr == context.platformContext )
				{
					retval = ResultCode::kError_NotInitializeContext;
					break; // Exit
				}

				const ResultCode result_PlatformDeocde = Platform::decode( *context.platformContext, inputData, inputDataSizeInBytes, outConsumedInputDataSizeInBytes, outOutputPcmBuffer, outputPcmBufferSizeInBytes,  outOutOutputPcmBufferInBytes );
				if ( ResultCode::kOK != result_PlatformDeocde )
				{
					retval = result_PlatformDeocde;
					break; // Exit
				}

				// Succeeded.
				retval = ResultCode::kOK;
			}
			while ( false );

			return retval;
		}

	} /* namespace Decoder */
} /* namespace Audio */ } /* namespace SampleUtil */ } /* namespace sce */
