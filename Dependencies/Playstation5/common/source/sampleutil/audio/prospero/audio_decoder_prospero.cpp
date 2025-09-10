/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>

#if ( 0 < _SCE_TARGET_OS_PROSPERO )

#pragma comment( lib, "libSceAjm_stub_weak.a" )

#include <ajm.h>
#include <stdlib.h>
#include <string.h>

#include <sampleutil/sampleutil_common.h>

#include "../audio_decoder.h"
#include "../audio_paraser.h"



namespace sce {	namespace SampleUtil { namespace Audio { namespace Decoder
{
	namespace Platform
	{
		template < typename Type >
		struct Item
		{
			Item( )
			 : initialized( false )
			{

			}

			bool	initialized;
			Type	content;
		};

		struct Context
		{
			Item< SceAjmContextId >		ajmContext;
			Item< SceAjmCodecType >		ajmCodec;
			Item< SceAjmInstanceId >	ajmDecoderInstance;
		};

		ResultCode initialize( Context *& outContext, const Codec & codec, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes );
		ResultCode finalize( Context *& outContext ); 
		ResultCode decode( Context & context, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes, void *& outOutputPcmBuffer, const size_t outputPcmBufferSizeInBytes, size_t & outOutOutputPcmBufferInBytes );

		ResultCode initialize( Context *& outContext, const Codec & codec, const void * inputData, const size_t inputDataSizeInBytes, size_t & outConsumedInputDataSizeInBytes )
		{
			ResultCode retval = ResultCode::kInvalid;

			Context						*	tempContext	= nullptr;
			Item< SceAjmContextId >			tempAjmContext;
			Item< SceAjmCodecType >			tempAjmCodec;
			Item< SceAjmInstanceId >		tempAjmDecoderInstance;
			
			unsigned char					ajmBatchSystemBuffer[ SCE_AJM_JOB_INITIALIZE_SIZE + SCE_AJM_JOB_DECODE_SIZE ];
			SceAjmBatchInfo					ajmBatchInfo		= { };
			SceAjmBatchId					ajmBatchId				= { };
			SceAjmBatchError				ajmBatchStartError	= { };

			union
			{
				void								*	addressPointer;
				void								*	mp3InitializeParameters;
				SceAjmDecAt9InitializeParameters	*	at9InitializeParameters;
			} codecParameters;

			size_t codecParametersSizeInBytes	= 0;


			do
			{
				// Initialize context.
				tempContext = static_cast < Context * > ( malloc( sizeof( *tempContext ) ) );
				if ( nullptr == tempContext )
				{
					retval = ResultCode::kError_Native_OutOfMemory;
					break; // Exit
				}
				memset( tempContext, 0, sizeof( *tempContext ) );


				// Setup codec.
				switch ( codec )
				{
				case Codec::kMP3   : tempAjmCodec.content = SCE_AJM_CODEC_MP3_DEC;
					{
						codecParameters.mp3InitializeParameters = nullptr;
						codecParametersSizeInBytes				= 0;

						outConsumedInputDataSizeInBytes	= 0;
					}
					break;
				case Codec::kATRAC9: tempAjmCodec.content = SCE_AJM_CODEC_AT9_DEC;
					{
						Parser::ATRAC9AudioHeader atrac9AudioHeader;
						size_t headerSizeInBytes;
						const Parser::ResultCode result_Paser = Parser::parseATRAC9Audio( atrac9AudioHeader, headerSizeInBytes, inputData, inputDataSizeInBytes );
						if ( Parser::ResultCode::kOK != result_Paser )
						{
							retval = ResultCode::kError_NotSupportedFormat;
							break;	// Exit
						}

						codecParameters.at9InitializeParameters = static_cast< SceAjmDecAt9InitializeParameters * >( alloca( sizeof( SceAjmDecAt9InitializeParameters ) ) );
						memset( codecParameters.at9InitializeParameters, 0, sizeof( *codecParameters.at9InitializeParameters ) );
						memcpy( codecParameters.at9InitializeParameters->uiConfigData, atrac9AudioHeader.fmtChunk.configData, sizeof( codecParameters.at9InitializeParameters->uiConfigData ) );
						codecParametersSizeInBytes				= sizeof( *codecParameters.at9InitializeParameters );

						outConsumedInputDataSizeInBytes	= headerSizeInBytes;
					}
					break;
				default:
					retval = ResultCode::kError_NotSupportedCodec;
					outConsumedInputDataSizeInBytes = 0;
					break;
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Initialize Ajm Context.
				{
					const int nativeResult = sceAjmInitialize( /* reserved */ 0, &tempAjmContext.content );
					switch ( nativeResult )
					{
					case SCE_OK:
						tempAjmContext.initialized	= true;
						tempContext->ajmContext		= tempAjmContext;
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER: retval = ResultCode::kError_Native_InvalidParameter; break;
					case SCE_AJM_ERROR_OUT_OF_MEMORY    : retval = ResultCode::kError_Native_OutOfMemory     ; break;
					case SCE_AJM_ERROR_OUT_OF_RESOURCES : retval = ResultCode::kError_Native_OutOfResources  ; break;
					default                             : retval = ResultCode::kError_Native_Unknown         ; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Register Codec.
				{
					const int nativeResult = sceAjmModuleRegister( tempAjmContext.content, tempAjmCodec.content, /* reserved */ 0 );
					switch ( nativeResult )
					{
					case SCE_OK:
						tempAjmCodec.initialized	= true;
						tempContext->ajmCodec		= tempAjmCodec;
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER       : retval = ResultCode::kError_Native_RegsterCodec; break;
					case SCE_AJM_ERROR_INVALID_CONTEXT         : retval = ResultCode::kError_Native_RegsterCodec; break;
					case SCE_AJM_ERROR_OUT_OF_MEMORY           : retval = ResultCode::kError_Native_OutOfMemory ; break;
					case SCE_AJM_ERROR_CODEC_ALREADY_REGISTERED: retval = ResultCode::kError_Native_RegsterCodec; break;
					case SCE_AJM_ERROR_CODEC_NOT_SUPPORTED     : retval = ResultCode::kError_Native_RegsterCodec; break;
					case SCE_AJM_ERROR_UNKNOWN                 : retval = ResultCode::kError_Native_RegsterCodec; break;
					default                                    : retval = ResultCode::kError_Native_RegsterCodec; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Create Decoder Instance.
				{
					constexpr uint64_t flag = SCE_AJM_INSTANCE_FLAG_MAX_CHANNEL( SceAjmFormatChannel::SCE_AJM_FORMAT_CHANNEL_DEFAULT ) | SCE_AJM_INSTANCE_FLAG_FORMAT( SceAjmFormatEncoding::SCE_AJM_FORMAT_ENCODING_S16 );
					const int natieResult = sceAjmInstanceCreate( tempAjmContext.content, tempAjmCodec.content, flag, &tempAjmDecoderInstance.content );
					switch ( natieResult )
					{
					case SCE_OK:
						tempAjmDecoderInstance.initialized	= true;
						tempContext->ajmDecoderInstance		= tempAjmDecoderInstance;
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER   : retval = ResultCode::kError_Native_CreateDecoderInstance; break;
					case SCE_AJM_ERROR_WRONG_REVISION_FLAG : retval = ResultCode::kError_Native_CreateDecoderInstance; break;
					case SCE_AJM_ERROR_FLAG_NOT_SUPPORTED  : retval = ResultCode::kError_Native_CreateDecoderInstance; break;
					case SCE_AJM_ERROR_INVALID_CONTEXT     : retval = ResultCode::kError_Native_CreateDecoderInstance; break;
					case SCE_AJM_ERROR_CODEC_NOT_REGISTERED: retval = ResultCode::kError_Native_CreateDecoderInstance; break;
					case SCE_AJM_ERROR_OUT_OF_MEMORY       : retval = ResultCode::kError_Native_OutOfMemory          ; break;
					case SCE_AJM_ERROR_OUT_OF_RESOURCES    : retval = ResultCode::kError_Native_OutOfResources       ; break;
					default                                : retval = ResultCode::kError_Native_CreateDecoderInstance; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Initialize Batch System.
				{
					const int nativeResult = sceAjmBatchInitialize( ajmBatchSystemBuffer, sizeof( ajmBatchSystemBuffer ), &ajmBatchInfo );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					default:
						retval = ResultCode::kError_Native_InitializeBatch;
						break;;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Initialize Batch Job.
				{
					SceAjmInitializeResult		initializeResult;

					const int nativeResult = sceAjmBatchJobInitialize( &ajmBatchInfo, tempAjmDecoderInstance.content, codecParameters.addressPointer, codecParametersSizeInBytes, &initializeResult );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER: retval = ResultCode::kError_Native_InitializeBatchJob; break;
					case SCE_AJM_ERROR_JOB_CREATION     : retval = ResultCode::kError_Native_InitializeBatchJob; break;
					default                             : retval = ResultCode::kError_Native_InitializeBatchJob; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Start Batch.
				{
					const int nativeResult = sceAjmBatchStart( tempAjmContext.content, &ajmBatchInfo ,SCE_AJM_PRIORITY_GAME_DEFAULT, &ajmBatchStartError, &ajmBatchId );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER: retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_MALFORMED_BATCH  : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_INVALID_ADDRESS  : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_INVALID_CONTEXT  : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_OUT_OF_MEMORY    : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_BAD_PRIORITY     : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_OUT_OF_RESOURCES : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_RETRY            : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_JOB_CREATION     : retval = ResultCode::kError_Native_StartBatch; break;
					default                             : retval = ResultCode::kError_Native_StartBatch; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Wait for completion the batch.
				{
					const int nativeResult = sceAjmBatchWait( tempAjmContext.content, ajmBatchId, SCE_AJM_WAIT_INFINITE, nullptr );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					case SCE_AJM_ERROR_INVALID_CONTEXT: retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_INVALID_BATCH  : retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_BUSY           : retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_MALFORMED_BATCH: retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_IN_PROGRESS    : retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_CANCELLED      : retval = ResultCode::kError_Native_WaitBatch; break;
					default                           : retval = ResultCode::kError_Native_WaitBatch; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Succeeded.
				retval = ResultCode::kOK;

				outContext = tempContext;
				tempContext = nullptr;
			}
			while ( false );

			if( nullptr != tempContext )
			{
				( void ) finalize( tempContext );
			}

			return retval;
		}

		ResultCode finalize( Context *& inoutContext )
		{
			ResultCode retval = ResultCode::kInvalid;

			do
			{
				// Destroy Context.
				if ( nullptr != inoutContext )
				{
					if ( true == inoutContext->ajmContext.initialized )
					{
						if ( true == inoutContext->ajmDecoderInstance.initialized )
						{
							// Destroy Decoder Instance.
							const int nativeResult = sceAjmInstanceDestroy( inoutContext->ajmContext.content, inoutContext->ajmDecoderInstance.content );
							switch ( nativeResult )
							{
							case SCE_OK:
								/* nothing */
								break;
							case SCE_AJM_ERROR_INVALID_CONTEXT : retval = ResultCode::kError_Native_Invernal; break;
							case SCE_AJM_ERROR_INVALID_INSTANCE: retval = ResultCode::kError_Native_Invernal; break;
							case SCE_AJM_ERROR_BUSY            : retval = ResultCode::kError_Native_Invernal; break;
							default                            : retval = ResultCode::kError_Native_Unknown ; break;
							}
						}

						if ( true == inoutContext->ajmCodec.initialized )
						{
							// Unregister Codec.
							const int nativeResult = sceAjmModuleUnregister( inoutContext->ajmContext.content, inoutContext->ajmCodec.content );
							switch ( nativeResult )
							{
							case SCE_OK:
								inoutContext->ajmCodec.initialized = false;
								break;
							case SCE_AJM_ERROR_INVALID_PARAMETER   : retval = ResultCode::kError_Native_Invernal; break;
							case SCE_AJM_ERROR_INVALID_CONTEXT     : retval = ResultCode::kError_Native_Invernal; break;
							case SCE_AJM_ERROR_CODEC_NOT_REGISTERED: retval = ResultCode::kError_Native_Invernal; break;
							case SCE_AJM_ERROR_BUSY                : retval = ResultCode::kError_Native_Invernal; break;
							default                                : retval = ResultCode::kError_Native_Unknown ; break;
							}
						}

						// Finalize Ajm Context.
						const int nativeResult = sceAjmFinalize( inoutContext->ajmContext.content );
						switch ( nativeResult )
						{
						case SCE_OK:
							inoutContext->ajmContext.initialized = false;
							break;
						case SCE_AJM_ERROR_INVALID_CONTEXT: retval = ResultCode::kError_Native_Invernal; break;
						default                           : retval = ResultCode::kError_Native_Unknown ; break;
						}
					}
					if ( ResultCode::kInvalid != retval ) break;	// Exit

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
				uint8_t						ajmBatchBuffer[ SCE_AJM_JOB_DECODE_SIZE ];
				SceAjmBatchInfo				ajmBatchInfo				= { };
				SceAjmBatchId				ajmBatchId					= { };
				SceAjmDecodeSingleResult	ajmDecodeSingleResult		= { };
				SceAjmBatchError			ajmBatchStartError			= { };

				// Initialize Batch System.
				{
					const int nativeResult = sceAjmBatchInitialize( ajmBatchBuffer, sizeof( ajmBatchBuffer ), &ajmBatchInfo );;
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					default:
						retval = ResultCode::kError_Native_InitializeBatch;
						break;;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Decode.
				{
					const int nativeResult = sceAjmBatchJobDecodeSingle( &ajmBatchInfo, context.ajmDecoderInstance.content, inputData, inputDataSizeInBytes, outOutputPcmBuffer, outputPcmBufferSizeInBytes, &ajmDecodeSingleResult );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER: retval = ResultCode::kError_Native_DecodeSetup; break;
					case SCE_AJM_ERROR_JOB_CREATION     : retval = ResultCode::kError_Native_DecodeSetup; break;
					default                             : retval = ResultCode::kError_Native_DecodeSetup; break;
					}
				}
				// Start Batch.
				{
					const int nativeResult = sceAjmBatchStart( context.ajmContext.content, &ajmBatchInfo ,SCE_AJM_PRIORITY_GAME_DEFAULT, &ajmBatchStartError, &ajmBatchId );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					case SCE_AJM_ERROR_INVALID_PARAMETER: retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_MALFORMED_BATCH  : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_INVALID_ADDRESS  : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_INVALID_CONTEXT  : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_OUT_OF_MEMORY    : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_BAD_PRIORITY     : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_OUT_OF_RESOURCES : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_RETRY            : retval = ResultCode::kError_Native_StartBatch; break;
					case SCE_AJM_ERROR_JOB_CREATION     : retval = ResultCode::kError_Native_StartBatch; break;
					default                             : retval = ResultCode::kError_Native_StartBatch; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Wait for completion the batch.
				{
					const int nativeResult = sceAjmBatchWait( context.ajmContext.content, ajmBatchId, SCE_AJM_WAIT_INFINITE, nullptr );
					switch ( nativeResult )
					{
					case SCE_OK:
						/* nothing */
						break;
					case SCE_AJM_ERROR_INVALID_CONTEXT: retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_INVALID_BATCH  : retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_BUSY           : retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_MALFORMED_BATCH: retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_IN_PROGRESS    : retval = ResultCode::kError_Native_WaitBatch; break;
					case SCE_AJM_ERROR_CANCELLED      : retval = ResultCode::kError_Native_WaitBatch; break;
					default                           : retval = ResultCode::kError_Native_WaitBatch; break;
					}
				}
				if ( ResultCode::kInvalid != retval ) break;	// Exit

				// Check Result.
				{
					SCE_SAMPLE_UTIL_ASSERT_MSG( 0 == ajmDecodeSingleResult.sResult.iResult, "Decode Result[0x%08X] Internal Code[0x%08X]\n", ajmDecodeSingleResult.sResult.iResult, ajmDecodeSingleResult.sResult.iInternalResult );
					if ( 0 != ajmDecodeSingleResult.sResult.iResult )
					{
						retval = ResultCode::kError_Native_Decode;
						break;	// Exit
					}
				}

				// Succeded.
				retval = ResultCode::kOK;

				outConsumedInputDataSizeInBytes = ajmDecodeSingleResult.sStream.iSizeConsumed;
				outOutOutputPcmBufferInBytes	= ajmDecodeSingleResult.sStream.iSizeProduced;
			}
			while ( false );

			return retval;
		}


	} /* namespace Platform */
} /* namespace Decoder */ } /* namespace Audio */ } /* namespace SampleUtil */ } /* namespace sce */


#endif /* #if ( 0 < _SCE_TARGET_OS_PROSPERO ) */
