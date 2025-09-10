/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include <sampleutil/audio/audio_out.h>
#include "audio_internal.h"

#include <array>
#include <map>
#include <queue>

#define MY_LOCK(o)								\
{												\
	const int result= o.lock();					\
	SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);	\
	(void) result;								\
}

#define MY_UNLOCK(o)							\
{												\
	const int result = o.unlock();				\
	SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);	\
	(void) result;								\
}

#define MY_WAIT(o)								\
{												\
	const int result = o.wait();				\
	SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);	\
	(void) result;								\
}

#define MY_SIGNAL(o)							\
{												\
	const int result = o.signal();				\
	SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);	\
	(void) result;								\
}

namespace sce { namespace SampleUtil { namespace Audio {

	static constexpr uint32_t	GRAINS			= 256;
	static constexpr uint32_t	QUEUE_DEPTH		= 1;
	static constexpr uint32_t	MAX_PORTS		= 8;

	static constexpr int		PORT_ID_START	= 0x20000000;
	static constexpr int		PORT_ID_END		= 0x30000000;
	static constexpr int		PORT_ID_INVALID	= 0x00000010;

	enum class UserState
	{
		  Disabled	= 0
		, Enabled
	};


	static int convertPlatformResultCodeToSampleUtilResultCode(const Platform::ResultCode & plfResultCode)
	{
		int retval = SCE_OK;

		switch (plfResultCode)
		{
		case Platform::ResultCode::OK                                                            : retval = SCE_OK                                         ; break;
		case Platform::ResultCode::Error_Initialize_NativeAPI_Unknown                            : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_Initialize_NativeAPI_OutOfResource                      : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_RESOURCE          ; break;
		case Platform::ResultCode::Error_Initialize_NativeAPI_OutOfMemory                        : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY            ; break;
		case Platform::ResultCode::Error_Initialize_NativeAPI_Fatal                              : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_Initialize_NativeAPI_NotReady                           : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;
		case Platform::ResultCode::Error_Initialize_NativeAPI_InvalidParam                       : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreateContext_AllocateMemory                            : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY            ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_Unknown                         : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_InvalidParam                    : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_OutOfMemory                     : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY            ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_OutOfResource                   : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_RESOURCE          ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_NotReady                        : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_NotInitialized                  : retval = SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED          ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_Fatal                           : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_InvalidPortType                 : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_InvalidUser                     : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreateContext_NativeAPI_AlreadyInitialize               : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;

		case Platform::ResultCode::Caution_DestroyContext_InvalidContext                         : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Caution_DestroyContext_ContradictoryParameters                : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;

		case Platform::ResultCode::Error_CreateUser_AllocateMemory                               : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY            ; break;
		case Platform::ResultCode::Error_CreateUser_NativeAPI_Unknown                            : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_CreateUser_NativeAPI_InvalidParam                       : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreateUser_NativeAPI_OutOfResource                      : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_RESOURCE          ; break;

		case Platform::ResultCode::Caution_DestroyUser_ContradictoryParameters                   : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Caution_DestroyUser_InvalidUser                               : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;

		case Platform::ResultCode::Error_CreatePort_AllocateMemory                               : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY            ; break;
		case Platform::ResultCode::Error_CreatePort_InvalidContext                               : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_Unknown                            : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_InvalidParam                       : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_OutOfMemory                        : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY            ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_OutOfResource                      : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_RESOURCE          ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_NotReady                           : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_InvalidPortType                    : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_InvalidFormat                      : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_InvalidSampleFreq                  : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_CreatePort_NativeAPI_InvalidPort                        : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;

		case Platform::ResultCode::Caution_DestroyPort_InvalidPort                               : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Caution_DestroyPort_ContradictoryParameters                   : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;

		case Platform::ResultCode::Error_GetNumSpaceAvailableDeviceQueues_InvalidContext         : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_Unknown      : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_NotReady     : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;
		case Platform::ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_InvalidParam : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_GetNumSpaceAvailableDeviceQueues_NativeAPI_OutOfResource: retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_RESOURCE          ; break;

		case Platform::ResultCode::Error_SetPortAttributes_NativeAPI_Unknown                     : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_SetPortAttributes_BadAttributeParams                    : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_SetPortAttributes_NativeAPI_Fatal                       : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_SetPortAttributes_NativeAPI_InvalidPort                 : retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_PORT_NOT_EXISTS; break;
		case Platform::ResultCode::Error_SetPortAttributes_NativeAPI_InvalidParam                : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_SetPortAttributes_NativeAPI_NotReady                    : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;
		case Platform::ResultCode::Error_SetPortAttributes_NativeAPI_NotInitialized              : retval = SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED          ; break;

		case Platform::ResultCode::Error_Advance_NativeAPI_Unknown                               : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_Advance_NativeAPI_InvalidPort                           : retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_PORT_NOT_EXISTS; break;
		case Platform::ResultCode::Error_Advance_NativeAPI_NotReady                              : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;

		case Platform::ResultCode::Error_Push_Fatal                                              : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_Push_NativeAPI_Unknown                                  : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_Push_NativeAPI_InvalidParam                             : retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM            ; break;
		case Platform::ResultCode::Error_Push_NativeAPI_Fatal                                    : retval = SCE_SAMPLE_UTIL_ERROR_FATAL                    ; break;
		case Platform::ResultCode::Error_Push_NativeAPI_NotInitialized                           : retval = SCE_SAMPLE_UTIL_ERROR_NOT_INITIALIZED          ; break;
		case Platform::ResultCode::Error_Push_NativeAPI_NotReady                                 : retval = SCE_SAMPLE_UTIL_ERROR_NOT_READY                ; break;
		case Platform::ResultCode::Error_Push_NativeAPI_OutOfResource                            : retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_RESOURCE          ; break;

		case Platform::ResultCode::Invalid:
		/* [[fallthrough]]; */
		default:
			SCE_SAMPLE_UTIL_ASSERT(false);
			retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
			break;
		}

		return retval;
	}

	class AudioOutputController;
	
	class InternalOutputParams
	{
		friend AudioOutputController;
	public:
		struct Param
		{
			int					portId;
			const uint8_t		*data;
			size_t				dataSizeInBytes;

			Param()
			{
			}

			Param(const int in_portId, const uint8_t *in_data, const size_t in_dataSizeInBytes)
			 : portId(in_portId)
			 , data(in_data)
			 , dataSizeInBytes(in_dataSizeInBytes)
			{
			}

		};

	private:
		std::vector<Param>			params;
		Thread::CondVar<uint64_t>	*condvar;
		uint64_t					condValue;

	public:
		InternalOutputParams(Thread::CondVar<uint64_t> *in_condvar, uint64_t in_condValue)
		 : params()
		 , condvar(in_condvar)
		 , condValue(in_condValue)
		{
		}
		void push(const Param &param)
		{
			this->params.push_back(param);
		}

	}; /* class OutputParams */

	class Porter
	{
	private:
		bool								actived;

		InternalOutputParams::Param			outputParam;
		size_t								consumedOutputDataSizeInBytes;

		uint32_t							currentWorkingBufferIndex;

		uint32_t							byteWidth;
		uint32_t							numChannels;

		std::array<uint8_t*, QUEUE_DEPTH>	workingBuffers;
		size_t								workingBufferSizeInBytesPerDepth;

	public:
		Porter(const Format &format)
		 : actived(false)
		 , currentWorkingBufferIndex(0)
		 , byteWidth(0)
		 , numChannels(0)
		 , workingBufferSizeInBytesPerDepth(0)
		{
			switch (format)
			{
			case Format::kS16_Mono:
				this->byteWidth		= 2;
				this->numChannels	= 1;
				break;
			case Format::kS16_Stereo:
				this->byteWidth		= 2;
				this->numChannels	= 2;
				break;
			case Format::kS16_8Ch:
				this->byteWidth		= 2;
				this->numChannels	= 8;
				break;
			case Format::kFloat_Mono:
				this->byteWidth		= 4;
				this->numChannels	= 1;
				break;
			case Format::kFloat_Stereo:
				this->byteWidth		= 4;
				this->numChannels	= 2;
				break;
			case Format::kFloat_8Ch:
				this->byteWidth		= 4;
				this->numChannels	= 8;
				break;
			case Format::kS16_8chStd:
				this->byteWidth		= 2;
				this->numChannels	= 8;
				break;
			case Format::kFloat_8ChStd:
				this->byteWidth		= 4;
				this->numChannels	= 8;
				break;
			default:
				SCE_SAMPLE_UTIL_ASSERT(false);
				break;
			}

			this->workingBufferSizeInBytesPerDepth = this->byteWidth * this->numChannels * GRAINS;

			for (auto & workingBuffer : this->workingBuffers)
			{
				workingBuffer = static_cast<uint8_t*>(malloc(this->workingBufferSizeInBytesPerDepth));
				SCE_SAMPLE_UTIL_ASSERT(nullptr != workingBuffer);
			}
		}
		~Porter()
		{
			for (auto & workingBuffer : this->workingBuffers)
			{
				if (nullptr != workingBuffer)
				{
					free(workingBuffer);
					workingBuffer = nullptr;
				}
			}
		}

		uint32_t getChannelNum() const
		{
			return this->numChannels;
		}

		int isActived(bool & outActived)
		{
			outActived = this->actived;
			return SCE_OK;
		}

		int activate(const InternalOutputParams::Param & param)
		{
			this->outputParam = param;

			this->actived						= true;
			this->consumedOutputDataSizeInBytes = 0;
			this->currentWorkingBufferIndex = 0;

			return SCE_OK;
		}

		int advance(const void *&outData, size_t &outDataSizeInBytes)
		{
			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
			if ((true == this->actived) && (this->consumedOutputDataSizeInBytes < this->outputParam.dataSizeInBytes))
			{
				auto workingBuffer = this->workingBuffers[this->currentWorkingBufferIndex];

				memset(workingBuffer, 0, this->workingBufferSizeInBytesPerDepth);

				const uint8_t	*head					= this->outputParam.data + this->consumedOutputDataSizeInBytes;
				const size_t	outputDataSizeInBytes	= [&] ( ) -> size_t
					{
						if (0 < ((this->outputParam.dataSizeInBytes - this->consumedOutputDataSizeInBytes) / this->workingBufferSizeInBytesPerDepth))
						{
							return this->workingBufferSizeInBytesPerDepth;
						}
						else
						{
							return this->outputParam.dataSizeInBytes % this->workingBufferSizeInBytesPerDepth;
						}
					}();

				memcpy(workingBuffer, head, outputDataSizeInBytes);

				this->consumedOutputDataSizeInBytes += outputDataSizeInBytes;
				this->currentWorkingBufferIndex++;
				this->currentWorkingBufferIndex %= QUEUE_DEPTH;

				outData				= workingBuffer;
				outDataSizeInBytes	= this->workingBufferSizeInBytesPerDepth;

				retval = SCE_OK;
			}
			else
			{
				// Already completed.
				outData				= nullptr;
				outDataSizeInBytes	= 0;

				retval = SCE_OK;
			}
			return retval;
		}

		void postPush()
		{
			if (true == this->actived)
			{
				if (this->consumedOutputDataSizeInBytes >= this->outputParam.dataSizeInBytes)
				{
					this->actived =  false;
				}
			}
		}


	}; /* class Porter */

	class PortHandle
	{
	private:
		Platform::PortHandle	plfPortHandle;
		Porter					porter;

	public:
		PortHandle(Platform::PortHandle &handle, const Format &format)
		 : plfPortHandle(handle)
		 , porter(format)
		{
		}
		virtual ~PortHandle()
		{
		}

		const Platform::PortHandle & referPlatformHandle() const
		{
			return this->plfPortHandle;
		}

		Platform::PortHandle & referPlatformHandle()
		{
			return this->plfPortHandle;
		}

		const Porter & referPorter() const
		{
			return this->porter;
		}

		Porter & referPorter()
		{
			return this->porter;
		}
	};

	class AudioOutputController : public Thread::ThreadFunction
	{
	private:
		using PortHandleMap		= std::map<int, PortHandle*>;
		using PortHandlePair	= std::pair<int, PortHandle*>;

	private:
		Platform::ContextHandle							contextHandle;
		Platform::UserHandle							userHandle;
		PortHandleMap									portHandles;

		Thread::Cond						controlCond;
		std::queue<InternalOutputParams>	outputRequestQueue;

		bool	exiting;
		bool	dirtyObjects;


	private:
		int run() override
		{
			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

			int result_sampleUtil;

			sce::SampleUtil::Thread::CondVar<uint64_t>	*notifier				= nullptr;
			uint64_t									notificationCondValue	= 0;

			while (false == this->exiting)
			{
				// Check Device
				//
				{
					MY_LOCK(this->controlCond);

					while (
							(false == this->exiting) &&
							((Platform::HANDLE_INVAILD == this->contextHandle) || (Platform::HANDLE_INVAILD == this->userHandle) || (0 == this->portHandles.size()))
						  )
					{
						MY_WAIT(this->controlCond);
					}

					this->dirtyObjects = false;

					if ((true == this->exiting) || (true == this->dirtyObjects))
					{
						MY_UNLOCK(this->controlCond);
						continue;	// goto [while (false == this->exiting)]
					}

					MY_UNLOCK(this->controlCond);
				} // Check Device

				// Check New Request
				//
				{
					MY_LOCK(this->controlCond);

					while (
							((false == this->exiting) && (false == this->dirtyObjects)) &&
							(0 == this->outputRequestQueue.size())
						  )
					{
						MY_WAIT(this->controlCond);
					}

					if ((true == this->exiting) || (true == this->dirtyObjects))
					{
						MY_UNLOCK(this->controlCond);
						continue;	// goto [while (false == this->exiting)]
					}

					const InternalOutputParams newOutputRequest = this->outputRequestQueue.front();
					this->outputRequestQueue.pop();

					// Get new enetry
					for (auto & param : newOutputRequest.params)
					{

						auto it = this->portHandles.find(param.portId);

						SCE_SAMPLE_UTIL_ASSERT(this->portHandles.cend() != it);
						if (this->portHandles.cend() != it)
						{
							Porter & porter = it->second->referPorter();

							(void) porter.activate(param);
						}
					}
					notifier				= newOutputRequest.condvar;
					notificationCondValue	= newOutputRequest.condValue;

					MY_UNLOCK(this->controlCond);
				} // Check New Request

				// Outpu
				//
				while (true)
				{
					MY_LOCK(this->controlCond);

					// Check whether pushable
					{
						uint32_t queueLevel			= 0;	// unused
						uint32_t availableQueues	= 0;
						while (
								((false == this->exiting) && (false == this->dirtyObjects)) &&
								(0 == availableQueues)
							  )
						{
							const Platform::ResultCode result_plfApi = Platform::getQueueLevel(this->contextHandle, queueLevel, availableQueues);
							SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == result_plfApi);
							if (0 == availableQueues)
							{
								MY_UNLOCK(this->controlCond);

								SceKernelTimespec sleepTime;
								sleepTime.tv_sec	= 0;
								sleepTime.tv_nsec	= 1;
								(void ) sceKernelNanosleep(&sleepTime, nullptr);

								MY_LOCK(this->controlCond);
							}
						}

						if ((true == this->exiting) || (true == this->dirtyObjects))
						{
							MY_UNLOCK(this->controlCond);

							break; // goto [while (false == this->exiting)]
						}
					} // Check whether pushable

					// Attribute and Push
					{
 						// Setup Port Attribute
						int attributedCount = 0;
						for ( auto portHandle : this->portHandles )
						{
							auto &porter				= portHandle.second->referPorter();
							auto &platformPortHandle	= portHandle.second->referPlatformHandle();

							bool actived;
							(void) porter.isActived(actived);
							if (true == actived)
							{
								const void	*data;
								size_t		dataSizeInBytes;
								result_sampleUtil = porter.advance(data, dataSizeInBytes);
								SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_sampleUtil);

								std::vector<Platform::AttributeParam> attributeParams;
								if (nullptr != data)
								{
									{
										static const float gainValues[8] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };

										Platform::AttributeParam gain;
										gain.type				= Platform::AttributeParam::Type::Gain;
										gain.value				= gainValues;
										gain.valueSizeInBytes	= sizeof(gainValues[0]) * porter.getChannelNum();
										attributeParams.push_back(gain);
									}
									{
										Platform::AttributeParam pcm;
										pcm.type				= Platform::AttributeParam::Type::Pcm;
										pcm.value				= data;
										pcm.valueSizeInBytes	= dataSizeInBytes;
										attributeParams.push_back(pcm);
									}
	
									const Platform::ResultCode result_plfApi = Platform::setPortAttributes(platformPortHandle, attributeParams);
									SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == result_plfApi);

									attributedCount++;
								}
							}
						}
						
						if (0 < attributedCount)
						{
							// Device Push
							Platform::advance(this->contextHandle);
							Platform::push(this->contextHandle,Platform::BlockingMode::Sync);

							for ( auto portHandle : this->portHandles )
							{
								auto &porter = portHandle.second->referPorter();
								(void) porter.postPush();
							}
						}
						else
						{
							if ( nullptr != notifier )
							{
								result_sampleUtil = notifier->set(notificationCondValue);
								SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_sampleUtil);
							}

							MY_UNLOCK(this->controlCond);
							break; // Goto while (false == this->exiting)
						}
					} // Attribute and Push

					MY_UNLOCK(this->controlCond);
				} // Output
			}

			return retval;
		}

		int clearOutputRequestQueue()
		{
			std::queue<InternalOutputParams>().swap(this->outputRequestQueue);

			return SCE_OK;
		}

	public:
		AudioOutputController()
		 : contextHandle(Platform::HANDLE_INVAILD)
		 , userHandle(Platform::HANDLE_INVAILD)
		 , portHandles()
		 , controlCond("ControlCond")
		 , exiting(false)
		 , dirtyObjects(true)
		{
			Platform::ResultCode plfResult = Platform::initialize();
			SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResult);
			(void) plfResult;
		}
		virtual ~AudioOutputController()
		{
			MY_LOCK(this->controlCond);
			this->exiting = true;
			MY_SIGNAL(this->controlCond);
			MY_UNLOCK(this->controlCond);

			if (0 != portHandles.size())
			{
				const int result = this->destroyPortAll();
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);
				(void) result;
			}

			if (Platform::HANDLE_INVAILD != userHandle)
			{
				const int result = this->destroyUser();
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);
				(void) result;
			}

			if (Platform::HANDLE_INVAILD != contextHandle)
			{
				const int result = this->destroyContext();
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);
				(void) result;
			}

			Platform::ResultCode plfResult = Platform::finalize();
			SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResult);
			(void) plfResult;
		}

		int enumeratePortId(std::vector<int> &outPortIds) const
		{
			outPortIds.clear();
			for ( auto iterator = this->portHandles.begin() ; iterator != this->portHandles.cend() ; iterator++ )
			{
				outPortIds.push_back(iterator->first);
			}

			return SCE_OK;
		}

		int createContext(const AudioOutContextOption & contextOption)
		{
			MY_LOCK(this->controlCond);

			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

			Platform::ContextHandle tmpPlfContextHandle = Platform::HANDLE_INVAILD;

			do
			{
				Platform::ContextParams plfContextParams;
				memset(&plfContextParams, 0, sizeof(plfContextParams));

				plfContextParams.maxPorts				= MAX_PORTS;
				plfContextParams.maxObjectPorts			= 0;
				plfContextParams.guaranteeObjectPorts	= 0;
				plfContextParams.queueDepth				= QUEUE_DEPTH;
				plfContextParams.numGrains				= GRAINS;
				plfContextParams.flags					= 1;

				const Platform::ResultCode plfResultCode = Platform::createContext(tmpPlfContextHandle,plfContextParams);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				if (!(Platform::ResultCode::OK == plfResultCode))
				{
					retval = convertPlatformResultCodeToSampleUtilResultCode(plfResultCode);
					break; // Exit
				}

				// Succeeded
				retval = SCE_OK;

				this->contextHandle = tmpPlfContextHandle;
				tmpPlfContextHandle = Platform::HANDLE_INVAILD;

				this->clearOutputRequestQueue();

				this->dirtyObjects = true;
				MY_SIGNAL(this->controlCond);
			}
			while (false);

			if (Platform::HANDLE_INVAILD != tmpPlfContextHandle)
			{
				const Platform::ResultCode plfResultCode = Platform::destroyContext(this->contextHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				(void)plfResultCode;
			}

			MY_UNLOCK(this->controlCond);
			return retval;
		}
		
		int destroyContext()
		{
			MY_LOCK(this->controlCond);

			int  retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

			do
			{
				const Platform::ResultCode plfResultCode = Platform::destroyContext(this->contextHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				if (!(Platform::ResultCode::OK == plfResultCode))
				{
					retval = convertPlatformResultCodeToSampleUtilResultCode(plfResultCode);
					break; // Exit
				}

				// Succeeded
				retval = SCE_OK;

				this->contextHandle = Platform::HANDLE_INVAILD;

				this->clearOutputRequestQueue();

				this->dirtyObjects = true;

				MY_SIGNAL(this->controlCond)
			}
			while (false);

			MY_UNLOCK(this->controlCond);

			return retval;
		}

		int createUser(const AudioOutUserOption & audioOutUserOption)
		{
			MY_LOCK(this->controlCond);

			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

			Platform::UserHandle  tmpPlfUserHandle = Platform::HANDLE_INVAILD;

			do
			{
				if (Platform::HANDLE_INVAILD != this->userHandle)
				{
					retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_USER_ALREADY_EXISTS;
					break; // Exit
				}

				Platform::UserParams plfUserParams;
				memset(&plfUserParams, 0, sizeof(plfUserParams));

				plfUserParams.userId = audioOutUserOption.m_userId;

				Platform::ResultCode plfResultCode = Platform::createUser(tmpPlfUserHandle, plfUserParams);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				if (!(Platform::ResultCode::OK == plfResultCode))
				{
					retval = convertPlatformResultCodeToSampleUtilResultCode(plfResultCode);
					break; // Exit
				}

				// Succeeded
				retval = SCE_OK;

				this->userHandle = tmpPlfUserHandle;
				tmpPlfUserHandle = Platform::HANDLE_INVAILD;

				this->clearOutputRequestQueue();

				this->dirtyObjects = true;
				MY_SIGNAL(this->controlCond);
			}
			while (false);

			if ( Platform::HANDLE_INVAILD != tmpPlfUserHandle)
			{
				const Platform::ResultCode plfResultCode = Platform::destroyUser(tmpPlfUserHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				(void) plfResultCode;
			}

			MY_UNLOCK(this->controlCond);
			return retval;
		}

		int destroyUser()
		{
			MY_LOCK(this->controlCond);

			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
			do
			{
				const Platform::ResultCode plfResultCode = Platform::destroyUser(this->userHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				if (!(Platform::ResultCode::OK == plfResultCode))
				{
					retval = convertPlatformResultCodeToSampleUtilResultCode(plfResultCode);
					break; // Exit
				}

				// Succeeded
				retval = SCE_OK;

				this->userHandle = Platform::HANDLE_INVAILD;

				this->clearOutputRequestQueue();

				this->dirtyObjects = true;
				MY_SIGNAL(this->controlCond);
			}
			while (false);

			MY_UNLOCK(this->controlCond);

			return retval;
		}

		int createPort(int & outPortId, const AudioOutPortOption & audioOutPortOption)
		{
			MY_LOCK(this->controlCond);

			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

			Platform::PortHandle		tmpPlfPortHandle	= Platform::HANDLE_INVAILD;
			PortHandle					*tmpPorthandle		= nullptr;

			do
			{
				SCE_SAMPLE_UTIL_ASSERT(Platform::HANDLE_INVAILD != this->contextHandle);
				if(!(Platform::HANDLE_INVAILD != this->contextHandle))
				{
					retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
					break; // Exit
				}

				SCE_SAMPLE_UTIL_ASSERT(Platform::HANDLE_INVAILD != this->contextHandle);
				if(!(Platform::HANDLE_INVAILD != this->userHandle))
				{
					retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
					break; // Exit
				}

				Platform::PortParams plfPortParams;
				memset(&plfPortParams, 0, sizeof(plfPortParams));

				plfPortParams.portType		= audioOutPortOption.m_type;
//				plfPortParams.pad;			// ignore
				plfPortParams.dataFormat	= audioOutPortOption.m_format;
				plfPortParams.samplingFreq	= audioOutPortOption.m_sampleRateInHz;
//				plfPortParams.flags;		// ignore
				plfPortParams.userHandle	= this->userHandle;

				Platform::ResultCode plfResultCode = Platform::createPort(tmpPlfPortHandle, this->contextHandle, plfPortParams);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				if (!(Platform::ResultCode::OK == plfResultCode))
				{
					retval = convertPlatformResultCodeToSampleUtilResultCode(plfResultCode);
					break; // Exit
				}

				tmpPorthandle = new PortHandle(tmpPlfPortHandle, audioOutPortOption.m_format);
				SCE_SAMPLE_UTIL_ASSERT(nullptr != tmpPorthandle);
				if (!(nullptr != tmpPorthandle))
				{
					retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
					break; //  Exit
				}

				static int portIdGen = PORT_ID_START;
				const auto result_insert = this->portHandles.insert(PortHandlePair(portIdGen, tmpPorthandle));
				SCE_SAMPLE_UTIL_ASSERT(true == result_insert.second);
				if (!(true == result_insert.second))
				{
					retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
					break; // Exit
				}

				// Succeeded
				retval = SCE_OK;

				tmpPorthandle		= nullptr;
				tmpPlfPortHandle	= Platform::HANDLE_INVAILD;

				outPortId = portIdGen;
				portIdGen++;
				portIdGen = (PORT_ID_END <= portIdGen ? PORT_ID_START : portIdGen);

				this->clearOutputRequestQueue();

				this->dirtyObjects = true;
				MY_SIGNAL(this->controlCond);
			}
			while (false);

			if (nullptr != tmpPorthandle)
			{
				delete tmpPorthandle;
			}

			if (Platform::HANDLE_INVAILD != tmpPlfPortHandle)
			{
				const Platform::ResultCode plfResultCode = Platform::destroyPort(tmpPlfPortHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				(void) plfResultCode;
			}

			MY_UNLOCK(this->controlCond);

			return retval;
		}

		int destroyPort(int portId)
		{
			MY_LOCK(this->controlCond);

			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
			do
			{
				auto result_find = this->portHandles.find(portId);
				SCE_SAMPLE_UTIL_ASSERT(this->portHandles.cend() != result_find);
				if (!(this->portHandles.cend() != result_find))
				{
					retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_PORT_NOT_EXISTS;
					break; // Exit
				}

				auto portHandle		= result_find->second;
				auto plfPortHandle	= portHandle->referPlatformHandle();

				delete portHandle;

				this->portHandles.erase(result_find);

				const Platform::ResultCode plfResultCode = Platform::destroyPort(plfPortHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				if (!(Platform::ResultCode::OK == plfResultCode))
				{
					retval = convertPlatformResultCodeToSampleUtilResultCode(plfResultCode);
					break; // Exit
				}

				// Succeeded
				retval = SCE_OK;

				(void) this->clearOutputRequestQueue();

				this->dirtyObjects = true;
				MY_SIGNAL(this->controlCond);
			}
			while (false);

			MY_UNLOCK(this->controlCond);

			return retval;
		}

		int destroyPortAll()
		{
			MY_LOCK(this->controlCond);

			auto head = this->portHandles.begin();
			while ( head != this->portHandles.cend() )
			{
				auto portHandle		= head->second;
				auto plfPortHandle	= portHandle->referPlatformHandle();

				delete portHandle;

				const Platform::ResultCode plfResultCode = Platform::destroyPort(plfPortHandle);
				SCE_SAMPLE_UTIL_ASSERT(Platform::ResultCode::OK == plfResultCode);
				(void) plfResultCode;

				head = this->portHandles.erase(head);
			}

			this->clearOutputRequestQueue();

			this->dirtyObjects = true;

			MY_SIGNAL(this->controlCond);


			MY_UNLOCK(this->controlCond);

			return SCE_OK;
		}

		int checkUserState(UserState & outUserState) const
		{
			outUserState = (Platform::HANDLE_INVAILD != this->userHandle ? UserState::Enabled : UserState::Disabled);

			return SCE_OK;
		}

		int getGrains(uint32_t & outGrains)
		{
			outGrains = GRAINS;

			return SCE_OK;
		}

		int output(const InternalOutputParams &params)
		{
			int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
			
			MY_LOCK(this->controlCond);

			do
			{
				this->outputRequestQueue.push(params);

				// Succeeded
				retval = SCE_OK;

				MY_SIGNAL(this->controlCond);
			}
			while(false);

			MY_UNLOCK(this->controlCond);

			return retval;
		}

		void exit()
		{
			MY_LOCK(this->controlCond);
			this->exiting = true;
			MY_SIGNAL(this->controlCond);
			MY_UNLOCK(this->controlCond);
		}

	}; /* class AudioOutputController */

	class InternalAudioContext
	{
	private:
		int							representPortId;

		Thread::Thread				audioOutputControlThread;
		AudioOutputController		audioOutputController;

	public:
		InternalAudioContext()
		 : representPortId(PORT_ID_INVALID)
		 , audioOutputControlThread("AudioOutputControlThread", SCE_KERNEL_PRIO_FIFO_DEFAULT, 64U * 1024U, nullptr)
		 , audioOutputController()
		{
			const int result = this->audioOutputControlThread.start(&this->audioOutputController);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);
			(void) result;
		}
		virtual ~InternalAudioContext()
		{
			this->audioOutputController.exit();

			const int result = this->audioOutputControlThread.join();
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result);
			(void) result;
		}
		
		int getRepresentPortId() const
		{
			return this->representPortId;
		}

		void setRepresentPortId(const int portId)
		{
			this->representPortId = portId;
		}

		AudioOutputController & referController()
		{
			return this->audioOutputController;
		}

		const AudioOutputController & referController() const
		{
			return this->audioOutputController;
		}
	};

	AudioOutContext::AudioOutContext(const AudioOutContextOption *contextOption)
	 : internalAudioContext(nullptr)
	{
		(void) initialize(contextOption);
	}

	AudioOutContext::~AudioOutContext()
	{
		if (nullptr != this->internalAudioContext)
		{
			delete reinterpret_cast<InternalAudioContext*>(this->internalAudioContext);
		}
	}

	int	AudioOutContext::initialize(const AudioOutContextOption *contextOption)
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		InternalAudioContext * tmpInternalAudioContext = nullptr;

		do
		{
			if (nullptr != this->internalAudioContext)
			{
				delete reinterpret_cast<InternalAudioContext*>(this->internalAudioContext);
				this->internalAudioContext = nullptr;
			}

			// Create 'Internal Context'
			{
				tmpInternalAudioContext = new InternalAudioContext();
				SCE_SAMPLE_UTIL_ASSERT(nullptr != tmpInternalAudioContext);
				if (!(nullptr != tmpInternalAudioContext))
				{
					retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
					break; // Exit
				}
			}

			// Create 'Platform Context'
			AudioOutContextOption dummyAudioOutContextOption;
			memset(&dummyAudioOutContextOption, 0, sizeof(dummyAudioOutContextOption));
			dummyAudioOutContextOption.m_audioOutUserOption = nullptr;
			dummyAudioOutContextOption.m_audioOutPortOption = nullptr;
			const AudioOutContextOption * presentContextOption = (nullptr != contextOption ?  contextOption : &dummyAudioOutContextOption);
			{
				const int result_plfApi = tmpInternalAudioContext->referController().createContext(*presentContextOption);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_plfApi);
				if (!(SCE_OK == result_plfApi))
				{
					retval = result_plfApi;
					break;	// Exit
				}
			}

			// Create 'Platform User'
			if (nullptr != presentContextOption->m_audioOutUserOption)
			{
				const int result_plfApi = tmpInternalAudioContext->referController().createUser(*presentContextOption->m_audioOutUserOption);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_plfApi);
				if (!(SCE_OK == result_plfApi))
				{
					retval = result_plfApi;
					break;	// Exit
				}
			}

			// Create 'Platform Port'
			if (nullptr != presentContextOption->m_audioOutPortOption)
			{
				int portId;
				const int result_plfApi = tmpInternalAudioContext->referController().createPort(portId, *presentContextOption->m_audioOutPortOption);
				SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_plfApi);
				if (!(SCE_OK == result_plfApi))
				{
					retval = result_plfApi;
					break;	// Exit
				}
				tmpInternalAudioContext->setRepresentPortId(portId);
			}

			// Succeded
			retval = SCE_OK;

			this->internalAudioContext = tmpInternalAudioContext;
			tmpInternalAudioContext = nullptr;
		}
		while (false);

		if (nullptr != tmpInternalAudioContext)
		{
			delete tmpInternalAudioContext;
		}

		return retval;
	}

	int AudioOutContext::createAudioOutUser(const AudioOutUserOption *audioOutUserOption)
	{
		return reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().createUser(*audioOutUserOption);
	}

	int AudioOutContext::destroyAudioOutUser()
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			UserState userState;
			const int result_checkUserState = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().checkUserState(userState);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_checkUserState);
			if (!(SCE_OK == result_checkUserState))
			{
				retval = result_checkUserState;
				break; // Exit
			}

			SCE_SAMPLE_UTIL_ASSERT(UserState::Enabled == userState);
			if (!(UserState::Enabled == userState))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_USER_NOT_EXISTS;
				break;	// Exit
			}

			const int result_destroyPortAll = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().destroyPortAll();
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_destroyPortAll);
			if (!(SCE_OK == result_destroyPortAll))
			{
				retval = result_destroyPortAll;
				break;	// Exit
			}

			const int result_destroyUser = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().destroyUser();
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_destroyUser);
			if (!(SCE_OK == result_destroyUser))
			{
				retval = result_destroyUser;
				break;	// Exit
			}

			// Succeeded
			retval = SCE_OK;
		}
		while (false);

		return retval;
	}

	int AudioOutContext::isAudioOutUserEnabled(bool * outEnabled) const
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
		do
		{
			UserState userState;
			const int result_Internal = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().checkUserState(userState);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_Internal);
			if (!(SCE_OK == result_Internal))
			{
				retval = result_Internal;
				break;	// Exit
			}

			bool enabled = false;
			switch (userState)
			{
			case UserState::Enabled : enabled = true ; break;
			case UserState::Disabled: enabled = false; break;
			default:
				SCE_SAMPLE_UTIL_ASSERT(false);
				goto Exit;
			}
			// Succeeded
			retval = SCE_OK;

			*outEnabled = enabled;
		}
		while (false);
Exit:

		return retval;
	}


	int AudioOutContext::createAudioOutPort(int	*outPortId, const AudioOutPortOption *audioOutPortOption)
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			SCE_SAMPLE_UTIL_ASSERT((nullptr != outPortId) && (nullptr != audioOutPortOption));
			if (!((nullptr != outPortId) && (nullptr != audioOutPortOption)))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				break; // Exit
			}

			int portId;
			const int result_internal = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().createPort(portId, *audioOutPortOption);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_internal);
			if (!(SCE_OK == result_internal))
			{
				retval = result_internal;
				break; // Exit
			}

			if (PORT_ID_INVALID == reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->getRepresentPortId())
			{
				reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->setRepresentPortId(portId);
			}

			// Succeeded
			retval = SCE_OK;

			*outPortId = portId;
		}
		while (false);

		return retval;
	}

	int AudioOutContext::destroyAudioOutPort(int portId)
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			const int result_internal = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().destroyPort(portId);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_internal);
			if (!(SCE_OK == result_internal))
			{
				retval = result_internal;
				break; // Exit
			}

			std::vector<int> portIds;
			const int result_enumeratePortId = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().enumeratePortId(portIds);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_enumeratePortId);
			if (!(SCE_OK == result_enumeratePortId))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
				break; // Exit
			}

			if (0 < portIds.size())
			{
				if(portId == reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->getRepresentPortId())
				{
					reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->setRepresentPortId(portIds[0]);
				}
			}
			else
			{
				reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->setRepresentPortId(PORT_ID_INVALID);
			}

			// Succeeded
			retval = SCE_OK;
		}
		while (false);

		return retval;
	}

	int AudioOutContext::destroyAudioOutPortsAll()
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			const int result_internal = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().destroyPortAll();
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_internal);
			if (!(SCE_OK == result_internal))
			{
				retval = result_internal;
				break; // Exit
			}

			reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->setRepresentPortId(PORT_ID_INVALID);

			// Succeeded
			retval = SCE_OK;
		}
		while (false);

		return retval;
	}

	int AudioOutContext::output(const void *audioData, const size_t audioDataSizeInBytes, Thread::CondVar<uint64_t> *condvar, const uint64_t condValue)
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			const int portId = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->getRepresentPortId();
			SCE_SAMPLE_UTIL_ASSERT(PORT_ID_INVALID != portId);
			if (!(PORT_ID_INVALID != portId))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_PORT_NOT_EXISTS;
				break; // Exit;
			}

			OutputParam outputParam;
			outputParam.m_portId				= portId;
			outputParam.m_audioData				= audioData;
			outputParam.m_audioDataSizeInBytes	= audioDataSizeInBytes;

			const int reult_output = this->outputMulti(&outputParam, 1, condvar, condValue);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == reult_output);
			if (!(SCE_OK == reult_output))
			{
				retval = reult_output;
				break; // Exit
			}

			// Succeeded
			retval = SCE_OK;
		}
		while (false);

		return retval;
	}

	int AudioOutContext::outputMulti(const OutputParam * outputParams, uint32_t numOutputParams, Thread::CondVar<uint64_t> * condvar, const uint64_t condValue)
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			SCE_SAMPLE_UTIL_ASSERT((nullptr != outputParams) && (0 < numOutputParams));
			if (!((nullptr != outputParams) && (0 < numOutputParams)))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				break;	// Exit
			}

			std::vector<int> portIds;
			const int result_enumeratePortId = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().enumeratePortId(portIds);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_enumeratePortId);
			if (!(SCE_OK == result_enumeratePortId))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_FATAL;
				break; // Exit
			}

			bool isPortCheckSucceded = true;
			for ( int iii = 0 ; iii < numOutputParams ; iii++ )
			{
				bool hit = false;
				for ( const auto &portId : portIds )
				{
					if (portId == outputParams[iii].m_portId)
					{
						hit = true;
						break;	// Hit
					}
				}
				if (true != hit)
				{
					isPortCheckSucceded = false;
					break;	// Miss
				}
			}

			SCE_SAMPLE_UTIL_ASSERT(true == isPortCheckSucceded);
			if (!(true == isPortCheckSucceded))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_OUT_PORT_NOT_EXISTS;
				break; // Exit
			}

			InternalOutputParams internalOutputParams(condvar, condValue);
			for ( int iii = 0 ; iii < numOutputParams ; iii++ )
			{
				InternalOutputParams::Param internalOutputParam(outputParams[iii].m_portId, reinterpret_cast< const uint8_t*>(outputParams[iii].m_audioData), outputParams[iii].m_audioDataSizeInBytes);

				internalOutputParams.push(internalOutputParam);
			}
			
			const int result_output = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().output(internalOutputParams);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_output);
			if (!(SCE_OK == result_output))
			{
				retval = result_output;
				break; // Exit
			}

			// Succeeded
			retval = SCE_OK;
		}
		while (false);

		return retval;
	}

	int	AudioOutContext::getGrains(uint32_t *outGrainds) const
	{
		int retval = SCE_SAMPLE_UTIL_ERROR_FATAL;

		do
		{
			SCE_SAMPLE_UTIL_ASSERT(nullptr != outGrainds);
			if (!(nullptr != outGrainds))
			{
				retval = SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;

				break; // Exit
			}


			uint32_t grains;
			int result_internal = reinterpret_cast<InternalAudioContext*>(this->internalAudioContext)->referController().getGrains(grains);
			SCE_SAMPLE_UTIL_ASSERT(SCE_OK == result_internal);
			if (!(SCE_OK == result_internal))
			{
				retval = result_internal;
				break; // Exit
			}

			// Succeeded
			retval = SCE_OK;

			*outGrainds = grains;
		}
		while (false);

		return retval;
	}
	

}}} /* namespace sce::SampleUtil::Audio */
