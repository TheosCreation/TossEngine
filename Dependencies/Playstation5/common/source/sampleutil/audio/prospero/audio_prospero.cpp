/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include "../audio_internal.h"

#if ( 0 < _SCE_TARGET_OS_PROSPERO )

#pragma comment( lib, "libSceAudioOut2_stub_weak.a" )

#include <audio_out2.h>

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
	struct ContextHandleInstance
	{
		SceAudioOut2ContextHandle	nativeContextHandle;
		void						*contextWorkingMemory;
	};
	struct UserHandleInstance
	{
		SceAudioOut2UserHandle		nativeUserHandle;
	};
	struct PortHandleInstance
	{
		SceAudioOut2PortHandle		nativePortHandle;
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
				const int32_t nativeResult = sceAudioOut2Initialize();
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
				if (SCE_OK != nativeResult)
				{
					switch (nativeResult)
					{
					case SCE_AUDIO_OUT2_ERROR_OUT_OF_RESOURCE: resultCode = ResultCode::Error_Initialize_NativeAPI_OutOfResource; break;
					case SCE_AUDIO_OUT2_ERROR_OUT_OF_MEMORY: resultCode = ResultCode::Error_Initialize_NativeAPI_OutOfMemory; break;
					case SCE_AUDIO_OUT2_ERROR_FATAL: resultCode = ResultCode::Error_Initialize_NativeAPI_Fatal; break;
					case SCE_AUDIO_OUT2_ERROR_NOT_READY: resultCode = ResultCode::Error_Initialize_NativeAPI_NotReady; break;
					case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_Initialize_NativeAPI_InvalidParam; break;
					default: resultCode = ResultCode::Error_Initialize_NativeAPI_Unknown; break;
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

			SceAudioOut2ContextHandle			nativeContextHandle			= SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID;
			void						*		nativeContextWorkingMemory	= nullptr;

			outContextHandle = HANDLE_INVAILD;

			do
			{
				// Create 'Internal Context Handle'
				{
					contextHandleInstance = static_cast <ContextHandleInstance*> (malloc(sizeof(*contextHandleInstance)));
					SCE_SAMPLE_UTIL_ASSERT(nullptr != contextHandleInstance);
					if (nullptr == contextHandleInstance)
					{
						resultCode = ResultCode::Error_CreateContext_AllocateMemory;
						break;	// Exit
					}

					contextHandleInstance->nativeContextHandle	= SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID;
				}

				// Setup parameter of native context
				SceAudioOut2ContextParam		nativeContextParam;
				{
					const int32_t nativeResult = sceAudioOut2ContextResetParam(&nativeContextParam);
					SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
					if (SCE_OK != nativeResult)
					{
						switch (nativeResult)
						{
						case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_CreateContext_NativeAPI_InvalidParam; break;
						default: resultCode = ResultCode::Error_CreateContext_NativeAPI_Unknown; break;
						}

						break;		// Exit
					}

					nativeContextParam.maxPorts				= contextParams.maxPorts;
					nativeContextParam.maxObjectPorts		= contextParams.maxObjectPorts;
					nativeContextParam.guaranteeObjectPorts	= contextParams.guaranteeObjectPorts;
					nativeContextParam.queueDepth			= contextParams.queueDepth;
					nativeContextParam.numGrains			= contextParams.numGrains;
					nativeContextParam.flags				= SCE_AUDIO_OUT2_CONTEXT_PARAM_FLAG_MAIN;	/* ignore contextParams.flags */
					/* audioOutContextParam.reserved[ 16 ]; */
				}

				// Calculate required memory of native context
				size_t nativeContextWorkingMemorySizeInBytes;
				{
					const int32_t nativeResult = sceAudioOut2ContextQueryMemory(&nativeContextParam, &nativeContextWorkingMemorySizeInBytes);
					SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
					if (SCE_OK != nativeResult)
					{
						switch (nativeResult)
						{
						case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_CreateContext_NativeAPI_InvalidParam; break;
						default: resultCode = ResultCode::Error_CreateContext_NativeAPI_Unknown; break;
						}

						break; // Exit
					}
				}

				// Allocate memory of native context
				{
					nativeContextWorkingMemory = malloc(nativeContextWorkingMemorySizeInBytes);
					SCE_SAMPLE_UTIL_ASSERT(nullptr != nativeContextWorkingMemory);
					if (nullptr == nativeContextWorkingMemory)
					{
						resultCode = ResultCode::Error_CreateContext_AllocateMemory;
						break;	// Exit
					}
				}

				// Create native context
				{
					const int32_t nativeResult = sceAudioOut2ContextCreate(&nativeContextParam, nativeContextWorkingMemory, nativeContextWorkingMemorySizeInBytes, &nativeContextHandle);
					SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
					if (SCE_OK != nativeResult)
					{
						switch (nativeResult)
						{
						case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_CreateContext_NativeAPI_InvalidParam; break;
						case SCE_AUDIO_OUT2_ERROR_OUT_OF_MEMORY: resultCode = ResultCode::Error_CreateContext_NativeAPI_OutOfMemory; break;
						case SCE_AUDIO_OUT2_ERROR_OUT_OF_RESOURCE: resultCode = ResultCode::Error_CreateContext_NativeAPI_OutOfResource; break;
						case SCE_AUDIO_OUT2_ERROR_NOT_READY: resultCode = ResultCode::Error_CreateContext_NativeAPI_NotReady; break;
						case SCE_AUDIO_OUT2_ERROR_NOT_INITIALIZED: resultCode = ResultCode::Error_CreateContext_NativeAPI_NotInitialized; break;
						case SCE_AUDIO_OUT2_ERROR_FATAL: resultCode = ResultCode::Error_CreateContext_NativeAPI_Fatal; break;
						case SCE_AUDIO_OUT2_ERROR_INVALID_PORT_TYPE: resultCode = ResultCode::Error_CreateContext_NativeAPI_InvalidPortType; break;
						case SCE_AUDIO_OUT2_ERROR_INVALID_USER: resultCode = ResultCode::Error_CreateContext_NativeAPI_InvalidUser; break;
						case SCE_AUDIO_OUT2_ERROR_ALREADY_INITIALIZED: resultCode = ResultCode::Error_CreateContext_NativeAPI_AlreadyInitialize; break;
						default: resultCode = ResultCode::Error_CreateContext_NativeAPI_Unknown; break;
						}
						break;	// Exit
					}
				}

				// Succeeded
				resultCode = ResultCode::OK;

				contextHandleInstance->nativeContextHandle = nativeContextHandle;
				nativeContextHandle = SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID;

				contextHandleInstance->contextWorkingMemory = nativeContextWorkingMemory;
				nativeContextWorkingMemory = nullptr;

				outContextHandle = contextHandleInstance;
				contextHandleInstance = nullptr;

			} while (false);

			if (nullptr != contextHandleInstance)
			{
				free(contextHandleInstance);
			}

			if (SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID != nativeContextHandle)
			{
				const int32_t result_destoryContext = sceAudioOut2ContextDestroy(nativeContextHandle);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK != result_destoryContext);
				(void)result_destoryContext;
			}

			if (nullptr != nativeContextWorkingMemory)
			{
				free(nativeContextWorkingMemory);
			}

			return resultCode;
		}

		ResultCode destroyContext(ContextHandle contextHandle)
		{
			ResultCode resultCode = ResultCode::Invalid;

			if (HANDLE_INVAILD != contextHandle)
			{
				SCE_SAMPLE_UTIL_ASSERT( SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID != contextHandle->nativeContextHandle );
				if (SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID != contextHandle->nativeContextHandle)
				{
					const int32_t result_destoryContext = sceAudioOut2ContextDestroy(contextHandle->nativeContextHandle);
					SCE_SAMPLE_UTIL_ASSERT((SCE_OK == result_destoryContext) || (SCE_AUDIO_OUT2_ERROR_ALREADY_DESTROYED == result_destoryContext));
					(void)result_destoryContext;


					if (nullptr != contextHandle->contextWorkingMemory)
					{
						free(contextHandle->contextWorkingMemory);
					}
	
					free(contextHandle);
	
					resultCode = ResultCode::OK;
				}
				else
				{
					resultCode = ResultCode::Caution_DestroyContext_ContradictoryParameters;
				}
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

			UserHandleInstance * userHandleInstance = nullptr;

			SceAudioOut2UserHandle nativeUserHandle = SCE_AUDIO_OUT2_USER_HANDLE_INVALID;

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
				const int32_t nativeResult = sceAudioOut2UserCreate(userParams.userId, &nativeUserHandle);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
				if (SCE_OK != nativeResult)
				{
					switch (nativeResult)
					{
					case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_CreateUser_NativeAPI_InvalidParam; break;
					case SCE_AUDIO_OUT2_ERROR_OUT_OF_RESOURCE: resultCode = ResultCode::Error_CreateUser_NativeAPI_OutOfResource; break;
					default: resultCode = ResultCode::Error_CreateUser_NativeAPI_Unknown; break;
					}
				}

				// Succeeded
				resultCode = ResultCode::OK;

				userHandleInstance->nativeUserHandle = nativeUserHandle;
				nativeUserHandle = SCE_AUDIO_OUT2_USER_HANDLE_INVALID;

				outUserHandle = userHandleInstance;
				userHandleInstance = nullptr;
			} while (false);

			if (nullptr != userHandleInstance)
			{
				free( userHandleInstance );
			}

			if (SCE_AUDIO_OUT2_USER_HANDLE_INVALID != nativeUserHandle)
			{
				const int32_t result_destroyUser = sceAudioOut2UserDestroy(nativeUserHandle);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_destroyUser);
				(void)result_destroyUser;
			}

			return resultCode;
		}

		ResultCode destroyUser(UserHandle userHandle)
		{
			ResultCode resultCode = ResultCode::Invalid;

			if ( HANDLE_INVAILD != userHandle )
			{
				SCE_SAMPLE_UTIL_ASSERT( SCE_AUDIO_OUT2_USER_HANDLE_INVALID != userHandle->nativeUserHandle );
				if ( SCE_AUDIO_OUT2_USER_HANDLE_INVALID != userHandle->nativeUserHandle )
				{
					const int32_t result_destroyUser = sceAudioOut2UserDestroy(userHandle->nativeUserHandle);
					SCE_SAMPLE_UTIL_ASSERT((SCE_OK == result_destroyUser));

					free( userHandle );

					resultCode = ResultCode::OK;
				}
				else
				{
					resultCode = ResultCode::Caution_DestroyUser_ContradictoryParameters;
				}
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

			SceAudioOut2PortHandle nativePortHandle = SCE_AUDIO_OUT2_PORT_HANDLE_INVALID;

			outPortHandle = HANDLE_INVAILD;

			do
			{
				SCE_SAMPLE_UTIL_ASSERT( ( HANDLE_INVAILD != contextHandle ) && ( SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID != contextHandle->nativeContextHandle ) );
				if ( !( ( HANDLE_INVAILD != contextHandle ) && ( SCE_AUDIO_OUT2_CONTEXT_HANDLE_INVALID != contextHandle->nativeContextHandle ) ) )
				{
					resultCode = ResultCode::Error_CreatePort_InvalidContext;

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

					portHandleInstance->nativePortHandle = SCE_AUDIO_OUT2_PORT_HANDLE_INVALID;
				}

				// Setup AudioOutPort Parameters
				SceAudioOut2PortParam nativePortparam = { };
				{
					nativePortparam.portType = [](const OutPortType & portType)
					{
						uint16_t nativePortType = 0;

						switch (portType)
						{
						case OutPortType::kMain: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_MAIN; break;
						case OutPortType::kBgm: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_BGM; break;
						case OutPortType::kVoice: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_VOICE; break;
						case OutPortType::kPersonal: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_PERSONAL; break;
						case OutPortType::kPadspk: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_PADSPK; break;
						case OutPortType::kAux: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_AUX; break;
						case OutPortType::kVibration: nativePortType = SCE_AUDIO_OUT2_PORT_TYPE_VIBRATION; break;
						default: SCE_SAMPLE_UTIL_ASSERT(false); break;
						}

						return nativePortType;
					} (portParams.portType);
					nativePortparam.pad = portParams.pad;
					nativePortparam.dataFormat = [](const Format & format)
					{
						uint32_t nativeFormat = 0;

						switch (format)
						{
						case Format::kS16_Mono: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_I16_MONO; break;
						case Format::kS16_Stereo: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_I16_STEREO; break;
						case Format::kS16_8Ch: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_I16_8CH; break;
						case Format::kFloat_Mono: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_FLOAT_MONO; break;
						case Format::kFloat_Stereo: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_FLOAT_STEREO; break;
						case Format::kFloat_8Ch: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_FLOAT_8CH; break;
						case Format::kS16_8chStd: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_I16_8CH_STD; break;
						case Format::kFloat_8ChStd: nativeFormat = SCE_AUDIO_OUT2_DATA_FORMAT_FLOAT_8CH_STD; break;
						default: SCE_SAMPLE_UTIL_ASSERT(false); break;
						}

						return nativeFormat;
					} (portParams.dataFormat);
					nativePortparam.samplingFreq	= portParams.samplingFreq;
					nativePortparam.flags			= portParams.flags;
					SCE_SAMPLE_UTIL_ASSERT( ( nullptr != portParams.userHandle ) && ( SCE_AUDIO_OUT2_USER_HANDLE_INVALID != portParams.userHandle->nativeUserHandle ) );
					nativePortparam.userHandle		= portParams.userHandle->nativeUserHandle;
					/* audioOutPortparam.reserved[ 16 ]; */
				}

				// Create AudioOutPort
				{
					const int32_t nativeResult = sceAudioOut2PortCreate(contextHandle->nativeContextHandle, &nativePortparam, &nativePortHandle);
					SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
					if (SCE_OK != nativeResult)
					{
						switch (nativeResult)
						{
						case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidParam; break;
						case SCE_AUDIO_OUT2_ERROR_OUT_OF_MEMORY: resultCode = ResultCode::Error_CreatePort_NativeAPI_OutOfMemory; break;
						case SCE_AUDIO_OUT2_ERROR_OUT_OF_RESOURCE: resultCode = ResultCode::Error_CreatePort_NativeAPI_OutOfResource; break;
						case SCE_AUDIO_OUT2_ERROR_NOT_READY: resultCode = ResultCode::Error_CreatePort_NativeAPI_NotReady; break;
						case SCE_AUDIO_OUT2_ERROR_INVALID_PORT_TYPE: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidPortType; break;
						case SCE_AUDIO_OUT2_ERROR_INVALID_FORMAT: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidFormat; break;
						case SCE_AUDIO_OUT2_ERROR_INVALID_SAMPLE_FREQ: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidSampleFreq; break;
						case SCE_AUDIO_OUT2_ERROR_INVALID_PORT: resultCode = ResultCode::Error_CreatePort_NativeAPI_InvalidPort; break;
						default: resultCode = ResultCode::Error_CreatePort_NativeAPI_Unknown; break;
						}

						break; // Exit
					}
				}

				// Succeeded
				resultCode = ResultCode::OK;

				portHandleInstance->nativePortHandle = nativePortHandle;
				nativePortHandle = SCE_AUDIO_OUT2_PORT_HANDLE_INVALID;

				outPortHandle = portHandleInstance;
				portHandleInstance = nullptr;

			} while (false);

			if (nullptr != portHandleInstance)
			{
				free(portHandleInstance);
			}

			if (SCE_AUDIO_OUT2_PORT_HANDLE_INVALID != nativePortHandle)
			{
				const int32_t result_destroyPort = sceAudioOut2PortDestroy(nativePortHandle);
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
				SCE_SAMPLE_UTIL_ASSERT( SCE_AUDIO_OUT2_PORT_HANDLE_INVALID != portHandle->nativePortHandle );
				if (SCE_AUDIO_OUT2_PORT_HANDLE_INVALID != portHandle->nativePortHandle)
				{
					const int32_t result_destroyPort = sceAudioOut2PortDestroy(portHandle->nativePortHandle);
					SCE_SAMPLE_UTIL_ASSERT((SCE_OK == result_destroyPort) || (SCE_AUDIO_OUT2_ERROR_ALREADY_DESTROYED == result_destroyPort));
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
			ResultCode resultCode = ResultCode::Invalid;

			do
			{
				SCE_SAMPLE_UTIL_ASSERT( HANDLE_INVAILD != contextHandle );
				if ( HANDLE_INVAILD == contextHandle )
				{
					resultCode = ResultCode::Error_GetNumSpaceAvailableDeviceQueues_InvalidContext;
					break; // Exit
				}

				uint32_t tmpQueueLevel;
				uint32_t tmpAvailableQueues;
				
				const int32_t nativeResult = sceAudioOut2ContextGetQueueLevel(contextHandle->nativeContextHandle, &tmpQueueLevel, &tmpAvailableQueues);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == nativeResult);
				if (SCE_OK != nativeResult)
				{
					switch (nativeResult)
					{
					case SCE_AUDIO_OUT2_ERROR_NOT_READY: resultCode = ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_NotReady; break;
					case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM: resultCode = ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_InvalidParam; break;
					case SCE_AUDIO_OUT2_ERROR_OUT_OF_RESOURCE: resultCode = ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_OutOfResource; break;
					default: resultCode = ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_Unknown; break;
					}
					break; // Exit;
				}

				// Succeeded
				resultCode = ResultCode::OK;

				outQueueLevel		= tmpQueueLevel;
				outAvailableQueues	= tmpAvailableQueues;
			} while (false);

			return resultCode;
		}

		ResultCode setPortAttributes(PortHandle portHandle, const std::vector<AttributeParam> &attributeParams)
		{
			ResultCode resultCode = ResultCode::Invalid;

			do
			{
				SceAudioOut2Attribute *nativeAttributes = reinterpret_cast<SceAudioOut2Attribute*>(alloca(sizeof(SceAudioOut2Attribute) * attributeParams.size()));
				SCE_SAMPLE_UTIL_ASSERT(nullptr != nativeAttributes);
				if (!(nullptr != nativeAttributes))
				{
					resultCode = ResultCode::Error_SetPortAttributes_BadAttributeParams;
					break; // Exit
				}

				SceAudioOut2Pcm audioOut2Pcm;
				for ( int iii = 0 ; iii < attributeParams.size() ; iii++ )
				{
					const AttributeParam		&attributeParam		= attributeParams[iii];
					auto						&nativeAttribute	= nativeAttributes[iii];

					switch (attributeParam.type)
					{
					case AttributeParam::Type::Gain:
						nativeAttribute.attributeId = SCE_AUDIO_OUT2_PORT_ATTRIBUTE_ID_GAIN;
						nativeAttribute.value		= attributeParam.value;
						nativeAttribute.valueSize	= attributeParam.valueSizeInBytes;
						break;
					case AttributeParam::Type::Pcm:
						audioOut2Pcm.data = attributeParam.value;
						nativeAttribute.attributeId = SCE_AUDIO_OUT2_PORT_ATTRIBUTE_ID_PCM;
						nativeAttribute.value		= &audioOut2Pcm;
						nativeAttribute.valueSize	= sizeof(audioOut2Pcm);
						break;
					default:
						SCE_SAMPLE_UTIL_ASSERT(false);
						continue;
					}
				}

				const int32_t result_setAttribute = sceAudioOut2PortSetAttributes(portHandle->nativePortHandle, nativeAttributes, attributeParams.size());
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_setAttribute);
				if (!(SCE_OK == result_setAttribute))
				{
					switch (result_setAttribute)
					{
					case SCE_AUDIO_OUT2_ERROR_FATAL          : resultCode = ResultCode::Error_SetPortAttributes_NativeAPI_Fatal         ; break;
					case SCE_AUDIO_OUT2_ERROR_INVALID_PORT   : resultCode = ResultCode::Error_SetPortAttributes_NativeAPI_InvalidPort   ; break;
					case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM  : resultCode = ResultCode::Error_SetPortAttributes_NativeAPI_InvalidParam  ; break;
					case SCE_AUDIO_OUT2_ERROR_NOT_READY      : resultCode = ResultCode::Error_SetPortAttributes_NativeAPI_NotReady      ; break;
					case SCE_AUDIO_OUT2_ERROR_NOT_INITIALIZED: resultCode = ResultCode::Error_SetPortAttributes_NativeAPI_NotInitialized; break;
					default:
						resultCode = ResultCode::Error_SetPortAttributes_NativeAPI_Unknown;
						break;
					}
					break; // Exit
				}

				//Succeded
				resultCode = ResultCode::OK;
			}
			while (false);
			return resultCode;
		}

		ResultCode	advance(ContextHandle contextHandle)
		{
			ResultCode resultCode = ResultCode::Invalid;
			
			do
			{
				const int32_t result_advance = sceAudioOut2ContextAdvance(contextHandle->nativeContextHandle);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_advance);
				if (!(SCE_OK == result_advance))
				{
					switch (result_advance)
					{
					case SCE_AUDIO_OUT2_ERROR_INVALID_PORT: resultCode = ResultCode::Error_Advance_NativeAPI_InvalidPort; break;
					case SCE_AUDIO_OUT2_ERROR_NOT_READY   : resultCode = ResultCode::Error_Advance_NativeAPI_NotReady   ; break;
					default:
						resultCode = ResultCode::Error_Advance_NativeAPI_Unknown;
						break;
					}
					break; // Exit
				}

				// Succeeded
				resultCode = ResultCode::OK;
			}
			while (false);

			return resultCode;
		}

		ResultCode	push(ContextHandle contextHandle, const BlockingMode &blockingMode)
		{
			ResultCode resultCode = ResultCode::Invalid;
			
			do
			{
				SceAudioOut2Blocking nativeBlockingMode;
				switch (blockingMode)
				{
				case BlockingMode::Sync : nativeBlockingMode = SceAudioOut2Blocking::SCE_AUDIO_OUT2_BLOCKING_SYNC ; break;
				case BlockingMode::Async: nativeBlockingMode = SceAudioOut2Blocking::SCE_AUDIO_OUT2_BLOCKING_ASYNC; break;
				default:
					SCE_SAMPLE_UTIL_ASSERT(false);
					nativeBlockingMode = SceAudioOut2Blocking::SCE_AUDIO_OUT2_BLOCKING_SYNC;
					break;
				}

				const int result_push = sceAudioOut2ContextPush(contextHandle->nativeContextHandle, nativeBlockingMode);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_push);
				if (!(SCE_OK == result_push))
				{
					switch (result_push)
					{
					case SCE_AUDIO_OUT2_ERROR_INVALID_PARAM  : resultCode = ResultCode::Error_Push_NativeAPI_InvalidParam  ; break;
					case SCE_AUDIO_OUT2_ERROR_FATAL          : resultCode = ResultCode::Error_Push_NativeAPI_Fatal         ; break;
					case SCE_AUDIO_OUT2_ERROR_NOT_INITIALIZED: resultCode = ResultCode::Error_Push_NativeAPI_NotInitialized; break;
					case SCE_AUDIO_OUT2_ERROR_NOT_READY      : resultCode = ResultCode::Error_Push_NativeAPI_NotReady      ; break;
					case SCE_AUDIO_OUT2_ERROR_OUT_OF_RESOURCE: resultCode = ResultCode::Error_Push_NativeAPI_OutOfResource ; break;
					default:
						resultCode = ResultCode::Error_Push_NativeAPI_Unknown;
						break;
					}
					break; // Exit
				}

				// Succeeded
				resultCode = ResultCode::OK;
			}
			while (false);

			return resultCode;
		}

	}/* namespace Platform */
}}}	/* namespace sce::SampleUtil::Audio */

#endif /* #if ( 0 < _SCE_TARGET_OS_PROSPERO ) */
