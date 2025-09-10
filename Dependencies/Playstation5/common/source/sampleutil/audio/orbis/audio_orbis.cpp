/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2020 Sony Interactive Entertainment Inc. 
 * 
 */

#include "../audio_internal.h"

#if ( 0 < _SCE_TARGET_OS_ORBIS )

#pragma comment( lib, "libSceAudioOut_stub_weak.a" )

#include <audioout.h>

static constexpr int		HANDLE_AUDIO_INVALID = -1;

//#define USE_SULPHA
#if defined ( USE_SULPHA )

#include <sulpha.h>

#pragma comment( lib, "libSceSulpha_stub_weak.a" )

static int			Global_sulphaInitializedCount	= 0;
static void		*	Global_sulphaWorkingMemory		= nullptr;

#endif /* #if defined ( USE_SULPAHA ) */

namespace sce { namespace SampleUtil { namespace Audio {
namespace Platform
{
	struct AttributePcmData
	{
		int32_t					nativePortHandle;
		const void				*data;
	};

	struct ContextHandleInstance
	{
		bool							enabled;
		uint32_t						grains;
		std::vector<AttributePcmData>	attributePcmData;
	};
	struct UserHandleInstance
	{
		SceUserServiceUserId	userId;
	};
	struct PortHandleInstance
	{
		int32_t						nativePortHandle;
		ContextHandleInstance		*contextHandleInstance;
	};

	ResultCode initialize()
	{
		ResultCode resultCode = ResultCode::Invalid;

		do
		{
			// Initialize library
			static bool s_isInitialized = false;
			if (false == s_isInitialized)
			{
				const int32_t nativeResult = sceAudioOutInit();
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
				if (SCE_OK != nativeResult)
				{
					switch (nativeResult)
					{
					case SCE_AUDIO_OUT_ERROR_ALREADY_INIT   : resultCode = ResultCode::Error_Initialize_NativeAPI_Fatal        ; break;
					case SCE_AUDIO_OUT_ERROR_MEMORY         : resultCode = ResultCode::Error_Initialize_NativeAPI_OutOfMemory  ; break;
					case SCE_AUDIO_OUT_ERROR_SYSTEM_RESOURCE: resultCode = ResultCode::Error_Initialize_NativeAPI_OutOfResource; break;
					case SCE_AUDIO_OUT_ERROR_TRANS_EVENT    : resultCode = ResultCode::Error_Initialize_NativeAPI_Fatal        ; break;
					default                                 : resultCode = ResultCode::Error_Initialize_NativeAPI_Unknown      ; break;
					}

					break;	// Exit
				}
				s_isInitialized = true;
			}

#if defined ( USE_SULPHA )
			if ( 0 == Global_sulphaInitializedCount )
			{
				SceSulphaConfig sulphaConfig;
				const int result_sulphaGetDefaultConfig = sceSulphaGetDefaultConfig( &sulphaConfig );
				SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_sulphaGetDefaultConfig ); ( void ) result_sulphaGetDefaultConfig;

				size_t sulphaNeededMemorySizeInBytes;
				const int result_sulphaGetNeededMemory = sceSulphaGetNeededMemory( &sulphaConfig, &sulphaNeededMemorySizeInBytes );
				SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_sulphaGetNeededMemory ); ( void ) result_sulphaGetNeededMemory;

				Global_sulphaWorkingMemory = malloc( sulphaNeededMemorySizeInBytes );
				SCE_SAMPLE_UTIL_ASSERT( nullptr != Global_sulphaWorkingMemory );

				const int result_sulphaInit = sceSulphaInit( &sulphaConfig, Global_sulphaWorkingMemory, sulphaNeededMemorySizeInBytes );
				SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_sulphaInit ); ( void ) result_sulphaInit;

				Global_sulphaInitializedCount++;
			}
			else
			{
				Global_sulphaInitializedCount++;
			}
#endif /* #if defined ( USE_SULPHA ) */

			// Succeeded
			resultCode = ResultCode::OK;
		}
		while ( false );

		return	resultCode;
	}

		ResultCode	finalize()
		{
#if defined ( USE_SULPHA )
		SCE_SAMPLE_UTIL_ASSERT( 0 < Global_sulphaInitializedCount );
		Global_sulphaInitializedCount--;
		if ( Global_sulphaInitializedCount == 0 )
		{
			const int result_sulphaShutdown = sceSulphaShutdown( );
			SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_sulphaShutdown ); ( void ) result_sulphaShutdown;

			free( Global_sulphaWorkingMemory );
			Global_sulphaWorkingMemory = nullptr;
		}
#endif /* #if defined ( USE_SULPHA ) */
			return ResultCode::OK;
		}

		ResultCode createContext(ContextHandle & outContextHandle, const ContextParams & contextParams)
		{
			ResultCode resultCode = ResultCode::Invalid;

			ContextHandleInstance		*		contextHandleInstance = nullptr;

			outContextHandle = HANDLE_INVAILD;

			do
			{
				// Create 'Internal Context Handle'
				{
					contextHandleInstance = new ContextHandleInstance();
					SCE_SAMPLE_UTIL_ASSERT(nullptr != contextHandleInstance);
					if (nullptr == contextHandleInstance)
					{
						resultCode = ResultCode::Error_CreateContext_AllocateMemory;
						break;	// Exit
					}
				}

				contextHandleInstance->attributePcmData.clear();
				contextHandleInstance->grains = contextParams.numGrains;

				// Succeeded
				resultCode = ResultCode::OK;

				contextHandleInstance->enabled = true;

				outContextHandle = contextHandleInstance;
				contextHandleInstance = nullptr;

			} while (false);

			if (nullptr != contextHandleInstance)
			{
				delete contextHandleInstance;
			}

			return resultCode;
		}

		ResultCode destroyContext(ContextHandle contextHandle)
		{
			ResultCode resultCode = ResultCode::Invalid;

			if (HANDLE_INVAILD != contextHandle)
			{
				contextHandle->enabled = false;

				delete contextHandle;
	
				resultCode = ResultCode::OK;
			}
			else
			{
				resultCode = ResultCode::Caution_DestroyContext_InvalidContext;
			}

			return resultCode;
		}

		ResultCode createUser(UserHandle &outUserHandle, const UserParams &userParams)
		{
			ResultCode resultCode = ResultCode::Invalid;

			UserHandleInstance *userHandleInstance = nullptr;

			SceUserServiceUserId nativeUserHandle = SCE_USER_SERVICE_USER_ID_INVALID;

			outUserHandle = HANDLE_INVAILD;

			do
			{
				// Create 'Internal User Handle'
				{
					userHandleInstance = static_cast < UserHandleInstance* > ( malloc( sizeof( *userHandleInstance ) ) );
					SCE_SAMPLE_UTIL_ASSERT(nullptr != userHandleInstance );
					if ( nullptr == userHandleInstance )
					{
						resultCode = ResultCode::Error_CreateUser_AllocateMemory;

						break; // Exit
					}
				}

				// Create 'Native User'
				SCE_SAMPLE_UTIL_ASSERT(SCE_USER_SERVICE_USER_ID_INVALID != userParams.userId);
				if (!(SCE_USER_SERVICE_USER_ID_INVALID != userParams.userId))
				{
					resultCode = ResultCode::Error_CreateUser_NativeAPI_InvalidParam;
					break; // Exit
				}
				nativeUserHandle = userParams.userId;

				// Succeeded
				resultCode = ResultCode::OK;

				userHandleInstance->userId = nativeUserHandle;

				outUserHandle = userHandleInstance;
				userHandleInstance = nullptr;
			} while (false);

			if (nullptr != userHandleInstance)
			{
				free( userHandleInstance );
			}

			return resultCode;
		}

		ResultCode destroyUser(UserHandle userHandle)
		{
			ResultCode resultCode = ResultCode::Invalid;

			if ( HANDLE_INVAILD != userHandle )
			{
				SCE_SAMPLE_UTIL_ASSERT(SCE_USER_SERVICE_USER_ID_INVALID != userHandle->userId);
				free( userHandle );
				resultCode = ResultCode::OK;
			}
			else
			{
				resultCode = ResultCode::Caution_DestroyUser_InvalidUser;
			}

			return resultCode;
		}

		ResultCode createPort(PortHandle &outPortHandle, ContextHandle contextHandle, const PortParams &portParams)
		{
			ResultCode resultCode = ResultCode::Invalid;

			PortHandleInstance * portHandleInstance = nullptr;

			int nativePortHandle = HANDLE_AUDIO_INVALID;

			outPortHandle = HANDLE_INVAILD;

			do
			{
				SCE_SAMPLE_UTIL_ASSERT( ( HANDLE_INVAILD != contextHandle ) && ( true == contextHandle->enabled ) );
				if ( !( ( HANDLE_INVAILD != contextHandle ) && ( true == contextHandle->enabled ) ) )
				{
					resultCode = ResultCode::Error_CreatePort_InvalidContext;

					break; // Exit
				}

				SCE_SAMPLE_UTIL_ASSERT( ( nullptr != portParams.userHandle ) && ( SCE_USER_SERVICE_USER_ID_INVALID != portParams.userHandle->userId ) );
				if ( !(( nullptr != portParams.userHandle ) && ( SCE_USER_SERVICE_USER_ID_INVALID != portParams.userHandle->userId ) ) )
				{
					resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidParam;
					break; // Exit
				}


				// Create 'Internal Port Handle'
				{
					portHandleInstance = static_cast < PortHandleInstance * > ( malloc( sizeof( *portHandleInstance ) ) );
					SCE_SAMPLE_UTIL_ASSERT( nullptr != portHandleInstance );
					if ( nullptr == portHandleInstance )
					{
						resultCode = ResultCode::Error_CreatePort_AllocateMemory;

						break;	// Exit
					}

					portHandleInstance->nativePortHandle = HANDLE_AUDIO_INVALID;
					portHandleInstance->contextHandleInstance	= HANDLE_INVAILD;
				}

				// Setup AudioOutPort Parameters
				struct AudioOutOpenParam
				{
					SceUserServiceUserId	userId;
					int32_t					portType;
					int32_t					index;
					uint32_t				grains;
					uint32_t				freq;
					uint32_t				param;
				} nativePortparam = {};
				{

					nativePortparam.userId		= portParams.userHandle->userId;
					nativePortparam.portType	= [](const OutPortType & portType)
					{
						uint16_t nativePortType = 0;

						switch (portType)
						{
						case OutPortType::kMain    : nativePortType = SCE_AUDIO_OUT_PORT_TYPE_MAIN    ; break;
						case OutPortType::kBgm     : nativePortType = SCE_AUDIO_OUT_PORT_TYPE_BGM     ; break;
						case OutPortType::kVoice   : nativePortType = SCE_AUDIO_OUT_PORT_TYPE_VOICE   ; break;
						case OutPortType::kPersonal: nativePortType = SCE_AUDIO_OUT_PORT_TYPE_PERSONAL; break;
						case OutPortType::kPadspk  : nativePortType = SCE_AUDIO_OUT_PORT_TYPE_PADSPK  ; break;
						case OutPortType::kAux     : nativePortType = SCE_AUDIO_OUT_PORT_TYPE_AUX     ; break;
						default: SCE_SAMPLE_UTIL_ASSERT(false); break;
						}

						return nativePortType;
					} (portParams.portType);

					nativePortparam.index	= 0;	// unsed
					nativePortparam.grains	= contextHandle->grains;
					nativePortparam.freq	= portParams.samplingFreq;

					nativePortparam.param	= [](const Format & format)
					{
						uint32_t nativeFormat = 0;

						switch (format)
						{
						case Format::kS16_Mono    : nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_S16_MONO     ; break;
						case Format::kS16_Stereo  : nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_S16_STEREO   ; break;
						case Format::kS16_8Ch     : nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_S16_8CH      ; break;
						case Format::kFloat_Mono  : nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_MONO   ; break;
						case Format::kFloat_Stereo: nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_STEREO ; break;
						case Format::kFloat_8Ch   : nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_8CH    ; break;
						case Format::kS16_8chStd  : nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_S16_8CH_STD  ; break;
						case Format::kFloat_8ChStd: nativeFormat = SCE_AUDIO_OUT_PARAM_FORMAT_FLOAT_8CH_STD; break;
						default: SCE_SAMPLE_UTIL_ASSERT(false); break;
						}

						return nativeFormat;
					} (portParams.dataFormat);
					/* audioOutPortparam.reserved[ 16 ]; */
				}

				// Create AudioOutPort
				{
					const int32_t nativeResult = sceAudioOutOpen(nativePortparam.userId, nativePortparam.portType, nativePortparam.index, nativePortparam.grains, nativePortparam.freq, nativePortparam.param);
					SCE_SAMPLE_UTIL_ASSERT(0 < nativeResult);
					if (!(0 < nativeResult))
					{
						switch (nativeResult)
						{
						case SCE_AUDIO_OUT_ERROR_NOT_INIT           : resultCode = ResultCode::Error_CreatePort_NativeAPI_NotReady         ; break;
						case SCE_AUDIO_OUT_ERROR_INVALID_SIZE       : resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidParam     ; break;
						case SCE_AUDIO_OUT_ERROR_INVALID_FORMAT     : resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidFormat    ; break;
						case SCE_AUDIO_OUT_ERROR_INVALID_SAMPLE_FREQ: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidSampleFreq; break;
						case SCE_AUDIO_OUT_ERROR_INVALID_PORT_TYPE  : resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidPortType  ; break;
						case SCE_AUDIO_OUT_ERROR_PORT_FULL          : resultCode = ResultCode::Error_CreatePort_NativeAPI_OutOfResource    ; break;
						case SCE_AUDIO_OUT_ERROR_SYSTEM_RESOURCE    : resultCode = ResultCode::Error_CreatePort_NativeAPI_OutOfResource    ; break;
						case SCE_AUDIO_OUT_ERROR_MEMORY             : resultCode = ResultCode::Error_CreatePort_NativeAPI_OutOfMemory      ; break;
						case SCE_AUDIO_OUT_ERROR_TRANS_EVENT        : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown          ; break;
						default                                     : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown          ; break;
						}

						break; // Exit
					}
					nativePortHandle = nativeResult;
				}

				// Set volume
				int32_t volumes[] =
				{
					  SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
					, SCE_AUDIO_OUT_VOLUME_0DB
				};

				const int32_t result_setVolume = sceAudioOutSetVolume(nativePortHandle, 0xf, volumes);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_setVolume);
				if (!(SCE_OK == result_setVolume))
				{
					switch (result_setVolume)
					{
					case SCE_AUDIO_OUT_ERROR_INVALID_PORT     : resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidPort    ; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_PORT_TYPE: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidPortType; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_POINTER  : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown        ; break;
					case SCE_AUDIO_OUT_ERROR_NOT_OPENED       : resultCode = ResultCode::Error_Advance_NativeAPI_InvalidPort       ; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_FLAG     : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown        ; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_VOLUME   : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown        ; break;
					case SCE_AUDIO_OUT_ERROR_NOT_INIT         : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown        ; break;
					default                                   : resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown        ; break;
					}

					break; // Exit
				}

				// Succeeded
				resultCode = ResultCode::OK;

				portHandleInstance->nativePortHandle		= nativePortHandle;
				portHandleInstance->contextHandleInstance	= contextHandle;
				nativePortHandle							= HANDLE_AUDIO_INVALID;

				outPortHandle		= portHandleInstance;
				portHandleInstance	= nullptr;

			} while (false);

			if (nullptr != portHandleInstance)
			{
				free(portHandleInstance);
			}

			if (HANDLE_AUDIO_INVALID != nativePortHandle)
			{
				const int32_t result_destroyPort = sceAudioOutClose(nativePortHandle);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_destroyPort);
				(void)result_destroyPort;
			}

			return resultCode;
		}

		ResultCode	destroyPort(PortHandle portHandle)
		{
			ResultCode resultCode = ResultCode::Invalid;

			if ( HANDLE_INVAILD != portHandle )
			{
				SCE_SAMPLE_UTIL_ASSERT( HANDLE_AUDIO_INVALID != portHandle->nativePortHandle );
				if (HANDLE_AUDIO_INVALID != portHandle->nativePortHandle)
				{
					SceAudioOutOutputParam audioOutOutputParam;
					audioOutOutputParam.handle	= portHandle->nativePortHandle;
					audioOutOutputParam.ptr		= nullptr;
					const int32_t result_shutdown		= sceAudioOutOutputs(&audioOutOutputParam, 1);
					SCE_SAMPLE_UTIL_ASSERT(0 < result_shutdown);
					const int32_t result_destroyPort	= sceAudioOutClose(portHandle->nativePortHandle);
					SCE_SAMPLE_UTIL_ASSERT((SCE_OK == result_destroyPort) || (SCE_AUDIO_OUT_ERROR_NOT_OPENED == result_destroyPort));
					(void)result_destroyPort;

					free( portHandle );

					resultCode = ResultCode::OK;
				}
				else
				{
					resultCode = ResultCode::Caution_DestroyPort_ContradictoryParameters;
				}
			}
			else
			{
				resultCode = ResultCode::Caution_DestroyPort_InvalidPort;
			}

			return resultCode;
		}

		ResultCode getQueueLevel(ContextHandle contextHandle, uint32_t &outQueueLevel, uint32_t &outAvailableQueues)
		{
			(void) contextHandle;

			outQueueLevel		= 0;
			outAvailableQueues	= 1;
			return ResultCode::OK;
		}

		ResultCode setPortAttributes(PortHandle portHandle, const std::vector<AttributeParam> &attributeParams)
		{
			for ( const auto & attributeParam : attributeParams )
			{
				switch (attributeParam.type)
				{
				case AttributeParam::Type::Gain:
					/* nothing */
					break;
				case AttributeParam::Type::Pcm:
					{
						AttributePcmData attributePcmData;
						attributePcmData.nativePortHandle	= portHandle->nativePortHandle;
						attributePcmData.data				= attributeParam.value;

						portHandle->contextHandleInstance->attributePcmData.push_back(attributePcmData);
					}
					break;
				default:
					SCE_SAMPLE_UTIL_ASSERT(false);
					break;
				}
			}
			return ResultCode::OK;
		}

		ResultCode	advance(ContextHandle contextHandle)
		{
			(void) contextHandle;
			return ResultCode::OK;
		}

		ResultCode	push(ContextHandle contextHandle, const BlockingMode &blockingMode)
		{
			(void) blockingMode;

			ResultCode resultCode = ResultCode::Invalid;
			
			do
			{
				SceAudioOutOutputParam *outputParams = static_cast<SceAudioOutOutputParam*>(alloca(sizeof(*outputParams) * contextHandle->attributePcmData.size()));
				SCE_SAMPLE_UTIL_ASSERT(nullptr != outputParams);
				if (!(nullptr != outputParams))
				{
					resultCode = ResultCode::Error_Push_Fatal;
					break;	// Exit
				}

				for ( int iii  = 0 ; iii < contextHandle->attributePcmData.size() ; iii++ )
				{
					outputParams[iii].handle	= contextHandle->attributePcmData[iii].nativePortHandle;
					outputParams[iii].ptr		= contextHandle->attributePcmData[iii].data;
				}

				const int result_output = sceAudioOutOutputs(outputParams, contextHandle->attributePcmData.size());
				SCE_SAMPLE_UTIL_ASSERT(0 < result_output);
				if (!(0 < result_output))
				{
					switch (result_output)
					{
					case SCE_AUDIO_OUT_ERROR_NOT_INIT       : resultCode = ResultCode::Error_Push_NativeAPI_NotInitialized; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_PORT   : resultCode = ResultCode::Error_Push_NativeAPI_InvalidParam  ; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_SIZE   : resultCode = ResultCode::Error_Push_NativeAPI_InvalidParam  ; break;
					case SCE_AUDIO_OUT_ERROR_PORT_FULL      : resultCode = ResultCode::Error_Push_NativeAPI_NotReady      ; break;
					case SCE_AUDIO_OUT_ERROR_INVALID_POINTER: resultCode = ResultCode::Error_Push_NativeAPI_Fatal         ; break;
					case SCE_AUDIO_OUT_ERROR_NOT_OPENED     : resultCode = ResultCode::Error_Push_NativeAPI_InvalidParam  ; break;
					case SCE_AUDIO_OUT_ERROR_BUSY           : resultCode = ResultCode::Error_Push_NativeAPI_Fatal         ; break;
 					case SCE_AUDIO_OUT_ERROR_TRANS_EVENT    : resultCode = ResultCode::Error_Push_NativeAPI_Fatal         ; break;
					default                                 : resultCode = ResultCode::Error_Push_NativeAPI_Fatal         ; break;
					}

					break; // Exit
				}

				contextHandle->attributePcmData.clear();

				// Succeeded
				resultCode = ResultCode::OK;
			}
			while (false);

			return resultCode;
		}

	}/* namespace Platform */
}}}	/* namespace sce::SampleUtil::Audio */

#endif /* #if ( 0 < _SCE_TARGET_OS_ORBIS ) */
