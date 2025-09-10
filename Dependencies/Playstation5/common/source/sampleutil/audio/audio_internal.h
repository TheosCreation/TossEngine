/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#pragma once

#include <sampleutil/audio/audio_out.h>
#include <vector>

namespace sce
{
	namespace SampleUtil
	{
		namespace Audio
		{
			enum class ResultCode : uint32_t
			{
				  Invalid = 0
				, OK
				, Error_Undefined
				, Error_CreateAudioContext
				, Error_CreateAudioContext_Option_User
				, Error_CreateAudioContext_Option_Port

				, Error_Initialize_AllocateMemory
				, Error_Initialize_CreateAudioDataQueueCond
				, Error_Initialize_CreateAudioOutputControlThread
				, Error_Initialize_CreateAudioOutputControlThreadFunction
				, Error_Initialize_StartAudioOutputControlThread

				, Error_CreateUser_OutOfMemory
				, Error_CreatePort_OutOfMemory

				, Error_Finalize_Incomplete

				, Error_OutputControlThread_Fatal
			};


			inline constexpr uint32_t convertToSamplingBitwidthInBytes(const sce::SampleUtil::Audio::Format & format)
			{
				uint32_t retval = 0;
				switch (format)
				{
				case sce::SampleUtil::Audio::Format::kS16_Mono: retval = sizeof(int16_t); break;
				case sce::SampleUtil::Audio::Format::kS16_Stereo: retval = sizeof(int16_t); break;
				case sce::SampleUtil::Audio::Format::kS16_8Ch: retval = sizeof(int16_t); break;
				case sce::SampleUtil::Audio::Format::kFloat_Mono: retval = sizeof(float); break;
				case sce::SampleUtil::Audio::Format::kFloat_Stereo: retval = sizeof(float); break;
				case sce::SampleUtil::Audio::Format::kFloat_8Ch: retval = sizeof(float); break;
				case sce::SampleUtil::Audio::Format::kS16_8chStd: retval = sizeof(int16_t); break;
				case sce::SampleUtil::Audio::Format::kFloat_8ChStd: retval = sizeof(float); break;
				default:
					SCE_SAMPLE_UTIL_ASSERT(false);
					break;
				}
				return retval;
			}

			inline constexpr uint32_t convertToNumberOfChannels(const sce::SampleUtil::Audio::Format & format)
			{
				uint32_t retval = 0;
				switch (format)
				{
				case sce::SampleUtil::Audio::Format::kS16_Mono: retval = 1; break;
				case sce::SampleUtil::Audio::Format::kS16_Stereo: retval = 2; break;
				case sce::SampleUtil::Audio::Format::kS16_8Ch: retval = 8; break;
				case sce::SampleUtil::Audio::Format::kFloat_Mono: retval = 1; break;
				case sce::SampleUtil::Audio::Format::kFloat_Stereo: retval = 2; break;
				case sce::SampleUtil::Audio::Format::kFloat_8Ch: retval = 8; break;
				case sce::SampleUtil::Audio::Format::kS16_8chStd: retval = 8; break;
				case sce::SampleUtil::Audio::Format::kFloat_8ChStd: retval = 8; break;
				default:
					SCE_SAMPLE_UTIL_ASSERT(false);
					break;
				}
				return retval;
			}

			inline int convertResultCode(const ResultCode & resultCode)
			{
				int result = SCE_SAMPLE_UTIL_ERROR_FATAL;

				switch (resultCode)
				{
				case ResultCode::OK: result = SCE_OK; break;
				case ResultCode::Error_Undefined: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;
				case ResultCode::Error_CreateAudioContext: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;
				case ResultCode::Error_Initialize_CreateAudioDataQueueCond: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;
				case ResultCode::Error_Initialize_CreateAudioOutputControlThread: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;
				case ResultCode::Error_Initialize_CreateAudioOutputControlThreadFunction: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;
				case ResultCode::Error_Initialize_StartAudioOutputControlThread: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;

				case ResultCode::Error_Finalize_Incomplete: result = SCE_OK; break;

				case ResultCode::Error_OutputControlThread_Fatal: result = SCE_SAMPLE_UTIL_ERROR_FATAL; break;



				default:
					/* nothing */
					break;
				}
				return result;
			}

			namespace Platform
			{
				constexpr auto HANDLE_INVAILD = nullptr;

				struct ContextHandleInstance;
				struct UserHandleInstance;
				struct PortHandleInstance;

				using ContextHandle	= ContextHandleInstance*;
				using UserHandle	= UserHandleInstance*;
				using PortHandle	= PortHandleInstance*;

				enum class ResultCode : uint32_t
				{
					  Invalid	= 0
					, OK

					, Error_Initialize_NativeAPI_Unknown
					, Error_Initialize_NativeAPI_OutOfResource
					, Error_Initialize_NativeAPI_OutOfMemory
					, Error_Initialize_NativeAPI_Fatal
					, Error_Initialize_NativeAPI_NotReady
					, Error_Initialize_NativeAPI_InvalidParam

					, Error_CreateContext_AllocateMemory
					, Error_CreateContext_NativeAPI_Unknown
					, Error_CreateContext_NativeAPI_InvalidParam
					, Error_CreateContext_NativeAPI_OutOfMemory
					, Error_CreateContext_NativeAPI_OutOfResource
					, Error_CreateContext_NativeAPI_NotReady
					, Error_CreateContext_NativeAPI_NotInitialized
					, Error_CreateContext_NativeAPI_Fatal
					, Error_CreateContext_NativeAPI_InvalidPortType
					, Error_CreateContext_NativeAPI_InvalidUser
					, Error_CreateContext_NativeAPI_AlreadyInitialize

					, Caution_DestroyContext_InvalidContext
					, Caution_DestroyContext_ContradictoryParameters

					, Error_CreateUser_AllocateMemory
					, Error_CreateUser_NativeAPI_Unknown
					, Error_CreateUser_NativeAPI_InvalidParam
					, Error_CreateUser_NativeAPI_OutOfResource

					, Caution_DestroyUser_InvalidUser
					, Caution_DestroyUser_ContradictoryParameters

					, Error_CreatePort_AllocateMemory
					, Error_CreatePort_InvalidContext
					, Error_CreatePort_NativeAPI_Unknown
					, Error_CreatePort_NativeAPI_InvalidParam
					, Error_CreatePort_NativeAPI_OutOfMemory
					, Error_CreatePort_NativeAPI_OutOfResource
					, Error_CreatePort_NativeAPI_NotReady
					, Error_CreatePort_NativeAPI_InvalidPortType
					, Error_CreatePort_NativeAPI_InvalidFormat
					, Error_CreatePort_NativeAPI_InvalidSampleFreq
					, Error_CreatePort_NativeAPI_InvalidPort

					, Caution_DestroyPort_InvalidPort
					, Caution_DestroyPort_ContradictoryParameters

					, Error_GetNumSpaceAvailableDeviceQueues_InvalidContext
					, Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_Unknown
					, Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_NotReady
					, Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_InvalidParam
					, Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_OutOfResource

					, Error_SetPortAttributes_BadAttributeParams
					, Error_SetPortAttributes_NativeAPI_Unknown
					, Error_SetPortAttributes_NativeAPI_Fatal
					, Error_SetPortAttributes_NativeAPI_InvalidPort
					, Error_SetPortAttributes_NativeAPI_InvalidParam
					, Error_SetPortAttributes_NativeAPI_NotReady
					, Error_SetPortAttributes_NativeAPI_NotInitialized

					, Error_Advance_NativeAPI_Unknown
					, Error_Advance_NativeAPI_InvalidPort
					, Error_Advance_NativeAPI_NotReady

					, Error_Push_Fatal
					, Error_Push_NativeAPI_Unknown
					, Error_Push_NativeAPI_InvalidParam
					, Error_Push_NativeAPI_Fatal
					, Error_Push_NativeAPI_NotInitialized
					, Error_Push_NativeAPI_NotReady
					, Error_Push_NativeAPI_OutOfResource
				};

				struct ContextParams
				{
				    uint32_t	maxPorts;
				    uint32_t	maxObjectPorts;
				    uint32_t	guaranteeObjectPorts;
				    uint32_t	queueDepth;
				    uint32_t	numGrains;
				    uint32_t	flags;
				};

				struct UserParams
				{
					uint32_t	userId;
				};

				struct PortParams
				{
				    OutPortType	portType;
				    uint16_t	pad;
				    Format		dataFormat;
				    uint32_t	samplingFreq;
				    uint32_t	flags;
				    UserHandle	userHandle;
				};
				
				struct AttributeParam
				{
					enum class Type : uint32_t
					{
						  Invalid = 0
						, Gain
						, Pcm
					};

					Type		type;
					const void	*value;
					size_t		valueSizeInBytes;
				};

				enum class BlockingMode
				{
					  Invalid = 0
					, Sync
					, Async
				};

				ResultCode	initialize();
				ResultCode	finalize();
				ResultCode	createContext(ContextHandle &outContextHandle, const ContextParams &contextParams);
				ResultCode	destroyContext(ContextHandle contextHandle);
				ResultCode	createUser(UserHandle &outUserHandle, const UserParams &userParams);
				ResultCode	destroyUser(UserHandle userHandle);
				ResultCode	createPort(PortHandle &outPortHandle, ContextHandle contextHandle, const PortParams &portParams);
				ResultCode	destroyPort(PortHandle portHandle);
				ResultCode	getQueueLevel(ContextHandle contextHandle, uint32_t &outQueueLevel, uint32_t &outAvailableQueues);
				ResultCode	setPortAttributes(PortHandle portHandle, const std::vector<AttributeParam> &attributeParams);
				ResultCode	advance(ContextHandle contextHandle);
				ResultCode	push(ContextHandle contextHandle, const BlockingMode &blockingMode);
			} /* namespace Platform */

		} /* namespace Audio */
	} /* namespace SampleUtil */
} /* namespace sce */
