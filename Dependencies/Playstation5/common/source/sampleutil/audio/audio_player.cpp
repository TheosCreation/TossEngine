/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */
#include <sampleutil/audio/audio_player.h>

#include "audio_internal.h"
#include "audio_decoder.h"
#include "audio_paraser.h"

#include <sampleutil/thread/thread.h>
#include <queue>
#include <vector>

namespace sce { namespace SampleUtil { namespace Audio
{
	void getFormatFromAudioOutFormat(uint32_t &outbyteWidth, uint32_t &outNumChannels, const Format &format)
	{
		switch (format)
		{
		case Format::kS16_Mono:
			outbyteWidth	= 2;
			outNumChannels	= 1;
			break;
		case Format::kS16_Stereo:
			outbyteWidth	= 2;
			outNumChannels	= 2;
			break;
		case Format::kS16_8Ch:
			outbyteWidth	= 2;
			outNumChannels	= 8;
			break;
		case Format::kFloat_Mono:
			outbyteWidth	= 4;
			outNumChannels	= 1;
			break;
		case Format::kFloat_Stereo:
			outbyteWidth	= 4;
			outNumChannels	= 2;
			break;
		case Format::kFloat_8Ch:
			outbyteWidth	= 4;
			outNumChannels	= 8;
			break;
		case Format::kS16_8chStd:
			outbyteWidth	= 2;
			outNumChannels	= 8;
			break;
		case Format::kFloat_8ChStd:
			outbyteWidth	= 4;
			outNumChannels	= 8;
			break;
		default:
			SCE_SAMPLE_UTIL_ASSERT(false);
			break;
		}
	}

	class DecodeThreadFunction;

	class AlignedRingBuffer
	{
	private:
		class Pointer;
	
		class Point
		{
			friend Pointer;
	
		private:
			uint64_t				position_;
			bool					same_;
	
		public:
			Point( )
			 : position_( 0 )
			 , same_( false )
			{
			}
	
			Point( const uint64_t & position )
			 : position_( position )
			 , same_( false )
			{
			}
	
			Point( const Point & position )
			 : position_( position.position_ )
			 , same_( position.same_ )
			{
			}
	
			Point( const Point & position, const bool & same )
			 : position_( position.position_ )
			 , same_( same )
			{
			}
	
			operator uint64_t( ) const
			{
				return this->position_;
			}
	
			Point operator + ( const Point & magnitude ) const
			{
				return Point( this->position_ + magnitude.position_ );
			}
			Point operator - ( const Point & magnitude ) const
			{
				return Point( this->position_ - magnitude.position_ );
			}
			Point operator % ( const Point & magnitude ) const
			{
				return Point( this->position_ % magnitude.position_ );
			} 
			const Point & operator += ( const Point & magnitude )
			{
				this->position_ += magnitude.position_;
				return *this;
			}
			bool operator == ( const Point & point ) const
			{
				return this->position_ == point.position_;
			}
			bool operator != ( const Point & point ) const
			{
				return this->position_ != point.position_;
			}
			bool operator < ( const Point & point ) const
			{
				return this->position_ < point.position_;
			}
			bool operator > ( const Point & point ) const
			{
				return this->position_ > point.position_;
			}
			bool operator <= ( const Point & point ) const
			{
				return this->position_ <= point.position_;
			}
			bool operator >= ( const Point & point ) const
			{
				return this->position_ >= point.position_;
			}
		};
	
		class Pointer
		{
		public:
			using AdvanceCallbackFunction = void ( const Point & point, uint64_t length, const uint64_t advacedMagnitude, void * param );
	
		private:
			Point			&		current_;
			Point			&		tail_;
			const Point				limit_;
	
		public:
			Pointer( Point & current, Point & tail, const Point & limit )
			 : current_( current )
			 , tail_( tail )
			 , limit_( limit )
			{
			}
	
			operator uint64_t( ) const
			{
				return this->current_.position_;
			}

			Point advance( const Point & magnitude, AdvanceCallbackFunction * callbackFunction = nullptr, void * userData = nullptr )
			{
				Point advacedMagnitude( 0 );
	
				Point tempMagnitude = magnitude;
	
				const Point advanceableMagnitude = this->advanceableMagnitudeInBytes( );
				if ( 0 < advanceableMagnitude )
				{
					if ( ( this->current_ >= this->tail_ ) && ( false == this->tail_.same_ ) )
					{
						const uint64_t length = ( this->limit_ - this->current_ ) >= tempMagnitude ? tempMagnitude : ( this->limit_ - this->current_ );
						if ( nullptr != callbackFunction )
						{
							callbackFunction( this->current_, length, advacedMagnitude, userData );
						}
	
						tempMagnitude = length >= tempMagnitude ? Point( 0 ) : tempMagnitude - Point( length );	// residue.
	
						advacedMagnitude += length;
						this->current_ += length;
	
						SCE_SAMPLE_UTIL_ASSERT( this->current_ <= this->limit_ );
						if ( this->current_ == this->limit_ )
						{
							this->current_ = 0;
	
							this->tail_.same_ = ( this->current_ == this->tail_ ? true : false );
						}
					}
	
					if ( this->current_ < this->tail_ )
					{
						const uint64_t length = ( this->tail_ - this->current_ ) >= tempMagnitude ? tempMagnitude : ( this->tail_ - this->current_ );
						if ( nullptr != callbackFunction )
						{
							callbackFunction( this->current_, length, advacedMagnitude, userData );
						}
	
						advacedMagnitude += length;
						this->current_ += length;
	
						this->tail_.same_ = ( this->current_ == this->tail_ ? true : false );
					}
				}
	
				if ( 0 < advacedMagnitude )
				{
					this->current_.same_ = false;
				}
	
				return advacedMagnitude;
				
			}
	
			Point advanceableMagnitudeInBytes( ) const
			{
				Point advanceableMagnitude( 0 );
	
				if ( this->tail_ < this->current_ )
				{	//++T---C+++L
					advanceableMagnitude = ( this->limit_ - this->current_ ) + this->tail_;
				}
				else if ( this->tail_ > this->current_ )
				{	//--C+++++T-L
					advanceableMagnitude = this->tail_ - this->current_;
				}
				else /* if ( this->current_ == this->tail_ ) */
				{	//C---------L or C+++++++++L
					advanceableMagnitude = ( true == this->tail_.same_ ? Point( 0 ) : this->limit_ );
				}
	
				return advanceableMagnitude;
			}
		};
	
	
	private:
				void		*	const		memory_;
		const	uint64_t					memorySizeInBytes_;
		const	uint64_t					alignmentSizeInBytes_;
	
				Point						writingPoint;
				Point						readingPoint;
				Point						recyclingPoint;
	
				Pointer						writingPointer;
				Pointer						readingPointer;
				Pointer						recyclingPointer;
	
	public:
		AlignedRingBuffer( void * memory, const uint64_t memorySizeInBytes, const uint64_t alignmentSizeInBytes )
		 : memory_( memory )
		 , memorySizeInBytes_( memorySizeInBytes )
		 , alignmentSizeInBytes_( alignmentSizeInBytes )
		 , writingPoint( 0, true )
		 , readingPoint( 0, true )
		 , recyclingPoint( 0, false )
		 , writingPointer( writingPoint, recyclingPoint, Point( memorySizeInBytes_ ) )
		 , readingPointer( readingPoint, writingPoint, Point( memorySizeInBytes_ ) )
		 , recyclingPointer( recyclingPoint, readingPoint, Point( memorySizeInBytes_ ) )
		{
			SCE_SAMPLE_UTIL_ASSERT( memorySizeInBytes >= alignmentSizeInBytes );
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( memorySizeInBytes % alignmentSizeInBytes ) );
		}
	
		virtual ~AlignedRingBuffer( )
		{
		}
	
		void reset( )
		{
			this->writingPoint		= Point( 0, true );
			this->readingPoint		= Point( 0, true );
			this->recyclingPoint	= Point( 0, false );
		}
	
		uint64_t getAlignmentSizeInBytes( ) const
		{
			return this->alignmentSizeInBytes_;
		}
	
		bool writable( ) const
		{
			return ( 0 < this->writableSizeInBytes( ) ? true : false );
		}
	
		uint64_t writableSizeInBytes( ) const
		{
			return this->writingPointer.advanceableMagnitudeInBytes( );
		}
	
		uint64_t write( const void * src, const uint64_t size )
		{
			struct UserData
			{
				AlignedRingBuffer	*	alignedRingBuffer;
				const void			*	srcBuffer;
					
			} userData;
	
			auto ctrlFunc = [ ] ( const Point & point, uint64_t length, const uint64_t advacedMagnitude, void * param )
			{
				UserData * userData = static_cast < UserData * > ( param );
	
				AlignedRingBuffer * alignedRingBuffer	= userData->alignedRingBuffer;
				const void		*	srcBuffer			= userData->srcBuffer;
	
						void * dst = static_cast < uint8_t * > ( alignedRingBuffer->memory_ ) + point;
				const	void * src = static_cast < const uint8_t * > ( srcBuffer ) + advacedMagnitude;
	
				memcpy( dst, src, length );
			};
	
	
			userData.alignedRingBuffer	= this;
			userData.srcBuffer			= src;
	
			return this->writingPointer.advance( size, ctrlFunc, &userData );
		}
	
		uint64_t readableNumberInUnitOfAlignment( ) const
		{
			return this->readingPointer.advanceableMagnitudeInBytes( ) / this->alignmentSizeInBytes_;
		}
	
		uint64_t readbleSizeInBytes( ) const
		{
			return this->readingPointer.advanceableMagnitudeInBytes( );
		}
	
		bool readInUnitOfAlignment( void *& outData )
		{
			bool succeeded = false;
	
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->readingPoint % Point( this->alignmentSizeInBytes_ ) ) );
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->recyclingPoint % Point( this->alignmentSizeInBytes_ ) ) );
	
			if ( 1 <= this->readableNumberInUnitOfAlignment( ) )
			{
				struct UserData
				{
					AlignedRingBuffer	*	alignedRingBuffer;
					void				**	outData;
				} userData;
	
				auto ctrlFunc = [ ] ( const Point & point, uint64_t length, const uint64_t advacedMagnitude, void * param )
				{
					UserData * userData = static_cast < UserData * > ( param );
	
					AlignedRingBuffer	*	alignedRingBuffer	= userData->alignedRingBuffer;
	
					void * src = static_cast < uint8_t * > ( alignedRingBuffer->memory_ ) + point;

					*userData->outData = src;
				};
	
				userData.alignedRingBuffer	= this;
				userData.outData			= &outData;
	
				const Point advancedMagnitude = this->readingPointer.advance( this->alignmentSizeInBytes_, ctrlFunc, &userData );
				SCE_SAMPLE_UTIL_ASSERT( advancedMagnitude == Point( this->alignmentSizeInBytes_ ) ); ( void ) advancedMagnitude;
	
				succeeded = true;
			}
	
			return succeeded;
		}
	
		uint64_t read( void *& outData, const uint64_t sizeInBytes )
		{
			uint64_t readSizeInBytes = 0;
	
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->readingPoint % Point( this->alignmentSizeInBytes_ ) ) );
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->recyclingPoint % Point( this->alignmentSizeInBytes_ ) ) );
	
	
			if ( 1 <= this->readbleSizeInBytes( ) )
			{
				struct UserData
				{
					AlignedRingBuffer	*	alignedRingBuffer;
					void				**	outData;
	
				} userData;
	
				auto ctrlFunc = [ ] ( const Point & point, uint64_t length, const uint64_t advacedMagnitude, void * param )
				{
					UserData * userData = static_cast < UserData * > ( param );
	
					AlignedRingBuffer	*	alignedRingBuffer	= userData->alignedRingBuffer;
	
					void * src = static_cast < uint8_t * > ( alignedRingBuffer->memory_ ) + point;
	
					*userData->outData = src;
				};
	
				userData.alignedRingBuffer	= this;
				userData.outData			= &outData;
	
				const Point advancedMagnitude = this->readingPointer.advance( sizeInBytes, ctrlFunc, &userData );
	
				readSizeInBytes = advancedMagnitude;
			}
			return readSizeInBytes;
		}
	
		uint64_t recyclableNumberInUnitOfAlignment( ) const
		{
			return this->recyclingPointer.advanceableMagnitudeInBytes( ) / this->alignmentSizeInBytes_;
		}
	
		uint64_t recyclableSizeInBytes( ) const
		{
			return this->recyclingPointer.advanceableMagnitudeInBytes( );
		}
	
		bool recycleInUnitOfAlignment( )
		{
			bool succeeded = false;
	
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->readingPoint % Point( this->alignmentSizeInBytes_ ) ) );
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->recyclingPoint % Point( this->alignmentSizeInBytes_ ) ) );
	
			if ( 1 <= this->recyclableNumberInUnitOfAlignment( ) )
			{
				const Point advancedMagnitude = this->recyclingPointer.advance( this->alignmentSizeInBytes_ );
				SCE_SAMPLE_UTIL_ASSERT( advancedMagnitude == Point( this->alignmentSizeInBytes_ ) ); ( void ) advancedMagnitude;
	
				succeeded = true;
			}
	
			return succeeded;
		}

		uint64_t recycle( const uint64_t sizeInBytes )
		{
			uint64_t recycledSizeInBytes = 0;
	
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->readingPoint % Point( this->alignmentSizeInBytes_ ) ) );
			SCE_SAMPLE_UTIL_ASSERT( 0 == ( this->recyclingPoint % Point( this->alignmentSizeInBytes_ ) ) );
	
	
			if ( 1 <= this->recyclableSizeInBytes( ) )
			{
				const Point advancedMagnitude = this->recyclingPointer.advance( sizeInBytes );
	
				recycledSizeInBytes = advancedMagnitude;
			}
			return recycledSizeInBytes;
		}
	
	};	/* AlignedRingBuffer */

	struct InternalContext
	{
		bool								isInitialized;

		AudioOutContext					*	audioOutContext;

		Thread::Thread					*	decodeThread;
		DecodeThreadFunction			*	decodeThreadFunction;

		Player::Codec						codec;

		void							*	audioDataAllocatedBySelf;

		uint8_t	const					*	inputData;
		size_t								inputDataSizeInBytes;
		size_t								decodedInputDataSizeInBytes;
		uint8_t							*	decodedDataTemporaryBuffer;
		size_t								decodedDataTemporaryBufferSizeInBytes;

		Format								targetFormat;
		uint32_t							targetsamplingRate;

		void							*	ringQueueWorkingMemory;
		size_t								ringQueueWorkingMemorySizeInBytes;

		Thread::CondVar< uint64_t >		*	watchOutputCondVar;

		bool							*	isFinished;

	};

	struct Player::Context
	{
		InternalContext	* internalContext;
	};


	// class DecodeThreadFunction
	class DecodeThreadFunction : public Thread::ThreadFunction
	{
		friend class Player;

	private:
		InternalContext		*	m_internalContext;
		bool					m_exit;
		bool					m_completed;

	private:
		DecodeThreadFunction( InternalContext * internalContxt )
		 : m_internalContext( internalContxt )
		 , m_exit( false )
		 , m_completed( false )
		{

		}

	private:
		int run( ) override
		{
			int retval = SCE_OK;

			Decoder::Context			*		decoderContext			= nullptr;
			AlignedRingBuffer			*		alignedRingBuffer		= nullptr;

			struct WaitingQueueParam
			{
				uint64_t		index;
				void		*	dataAddress;
				size_t			dataSizeInBytes;
			};
			std::queue< WaitingQueueParam >		*	waitingQueue			= nullptr;
			uint64_t								waitingIndex			= 0;

			do
			{
				uint32_t outputGrains = 0;
				const int  result_getSystemGrains = this->m_internalContext->audioOutContext->getGrains( &outputGrains );
				SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_getSystemGrains );
				if ( SCE_OK != result_getSystemGrains ) break; // Exit.

				uint32_t byteWidth		= 0;
				uint32_t numChannels	= 0;
				getFormatFromAudioOutFormat(byteWidth, numChannels, this->m_internalContext->targetFormat);

				const uint32_t outputUnitSizeinBytes = outputGrains * byteWidth * numChannels;

				alignedRingBuffer = new AlignedRingBuffer(  this->m_internalContext->ringQueueWorkingMemory, this->m_internalContext->ringQueueWorkingMemorySizeInBytes, outputUnitSizeinBytes );
				if ( nullptr == alignedRingBuffer ) break;


				waitingQueue = new std::queue< WaitingQueueParam >( );
				if( nullptr == waitingQueue ) break;

				Decoder::Codec decoderCodec;
				switch ( this->m_internalContext->codec )
				{
				case Player::Codec::kMp3:
					decoderCodec = Decoder::Codec::kMP3;
					break;
				case Player::Codec::kAtrac9:
					decoderCodec = Decoder::Codec::kATRAC9;
					break;
				default:
					retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_NOT_SUPPORTED_CODEC;
					break;
				}
				if ( SCE_OK != retval ) break;

				size_t consumedInputDataSizeInBytesByInitialize = 0;
				const Decoder::ResultCode result_decoderInitialize = Decoder::initialize( decoderContext, decoderCodec, this->m_internalContext->inputData, this->m_internalContext->inputDataSizeInBytes, consumedInputDataSizeInBytesByInitialize );
				switch ( result_decoderInitialize )
				{
				case Decoder::ResultCode::kOK:
					// nothing.
					break;
				default:
					retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_DECODE;
					break;
				}
				if ( SCE_OK != retval ) break;

				void		*	pcmData				= this->m_internalContext->decodedDataTemporaryBuffer;
				size_t			pcmDataSizeInBytes	= this->m_internalContext->decodedDataTemporaryBufferSizeInBytes;
				size_t			totalConsumedInputDataSizeInBytes = consumedInputDataSizeInBytesByInitialize;

				enum class Mode
				{
					  Decode
					, WriteToPool
					, OutputToDevice
					, Recycle
					, WaitForCompletionOfOutputsAll
				} mode;

				mode = Mode::Decode;

				bool		finishedDecode		= false;
				void	*	pendingWriteData	= nullptr;
				size_t		pendingWriteDataSizeInBytes	= 0;
				while ( ( true == this->m_internalContext->isInitialized ) && ( false == this->m_exit ) && ( false == this->m_completed ) )
				{
					switch ( mode )
					{
					case Mode::Decode:
						{
							SCE_SAMPLE_UTIL_ASSERT( 0 == pendingWriteDataSizeInBytes );
							if ( 0 == pendingWriteDataSizeInBytes )
							{
								const void		*	sourceData				= ( this->m_internalContext->inputData + totalConsumedInputDataSizeInBytes );
								const size_t		sourceDataSizeInBytes	=  this->m_internalContext->inputDataSizeInBytes - totalConsumedInputDataSizeInBytes;

								const size_t		decodeRequestDataSizeInBytes	= ( 0 < ( sourceDataSizeInBytes / 2048 ) ) ? 2048 : sourceDataSizeInBytes % 2048;

								size_t	consumedInputDataSizeInBytes	= 0;
								size_t	decodedPcmDataSizeInBytes		= 0;

								const Decoder::ResultCode result_Decode = Decoder::decode( *decoderContext, sourceData, decodeRequestDataSizeInBytes, consumedInputDataSizeInBytes, pcmData, pcmDataSizeInBytes, decodedPcmDataSizeInBytes );
								if ( Decoder::ResultCode::kOK == result_Decode )
								{
									totalConsumedInputDataSizeInBytes += consumedInputDataSizeInBytes;

									finishedDecode = totalConsumedInputDataSizeInBytes == this->m_internalContext->inputDataSizeInBytes;	// became empty of the input data.

									if ( 0 < decodedPcmDataSizeInBytes )
									{
										pendingWriteData			= pcmData;
										pendingWriteDataSizeInBytes = decodedPcmDataSizeInBytes;
										mode = Mode::WriteToPool;
									}
									else
									{
										if ( true == finishedDecode )
										{
											mode = Mode::OutputToDevice;
										}
									}
								}
							}
							else
							{
								mode = Mode::WriteToPool;
							}
						}
						break;
					case Mode::WriteToPool:
						{
							SCE_SAMPLE_UTIL_ASSERT( ( 0 < pendingWriteDataSizeInBytes ) && ( nullptr != pendingWriteData ) );
							if ( 0 < pendingWriteDataSizeInBytes )
							{
								if ( true == alignedRingBuffer->writable( ) )
								{
									const size_t writtenDataSizeInBytes = alignedRingBuffer->write( pendingWriteData, pendingWriteDataSizeInBytes );

									SCE_SAMPLE_UTIL_ASSERT( 0 < writtenDataSizeInBytes );
									pendingWriteData			= static_cast < uint8_t * > ( pendingWriteData ) + writtenDataSizeInBytes;
									pendingWriteDataSizeInBytes	-= writtenDataSizeInBytes;

									mode = Mode::OutputToDevice;
								}
								else
								{
									mode = Mode::Recycle;
								}
							}
							else
							{
								mode = Mode::Decode;
							}
						}
						break;
					case Mode::OutputToDevice:
						{
							while ( 1 <= alignedRingBuffer->readableNumberInUnitOfAlignment( ) )
							{
								void	*	outputData				= nullptr;
								size_t		outputDataSizeInBytes	= 0;

								const bool result_readInUnitOfAlignment = alignedRingBuffer->readInUnitOfAlignment( outputData );
								SCE_SAMPLE_UTIL_ASSERT( true == result_readInUnitOfAlignment ); ( void ) result_readInUnitOfAlignment;

								outputDataSizeInBytes = alignedRingBuffer->getAlignmentSizeInBytes( );

								SCE_SAMPLE_UTIL_ASSERT( ( nullptr != outputData ) && ( outputUnitSizeinBytes == outputDataSizeInBytes ) );
								if ( ( nullptr != outputData ) && ( outputUnitSizeinBytes == outputDataSizeInBytes ) )
								{
									const int result_output = this->m_internalContext->audioOutContext->output( outputData, outputDataSizeInBytes, this->m_internalContext->watchOutputCondVar, waitingIndex );
									SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_output ); ( void ) result_output;
	
									WaitingQueueParam waitingQueueParam;
									waitingQueueParam.index				= waitingIndex++;
									waitingQueueParam.dataAddress		= outputData;
									waitingQueueParam.dataSizeInBytes	= outputDataSizeInBytes;
	
									waitingQueue->push( waitingQueueParam );
								}
							}

							mode = ( 0 == pendingWriteDataSizeInBytes ? Mode::Decode : Mode::WriteToPool );

							if ( ( true == finishedDecode ) && ( 0 == pendingWriteDataSizeInBytes ) )
							{
								void	*	outputData				= nullptr;
								size_t		outputDataSizeInBytes	= 0;

								const uint64_t reqestReadSizeInBytes	= alignedRingBuffer->readbleSizeInBytes( );
								if ( 0 < reqestReadSizeInBytes )
								{
									const uint64_t readSize					= alignedRingBuffer->read( outputData, reqestReadSizeInBytes );
									SCE_SAMPLE_UTIL_ASSERT( reqestReadSizeInBytes == readSize ); ( void ) reqestReadSizeInBytes; ( void ) readSize;
	
									outputDataSizeInBytes = readSize;
	
									SCE_SAMPLE_UTIL_ASSERT( nullptr != outputData );
									if ( nullptr != outputData )
									{
										const int result_output = this->m_internalContext->audioOutContext->output( outputData, outputDataSizeInBytes, this->m_internalContext->watchOutputCondVar, waitingIndex );
										SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_output ); ( void ) result_output;
		
										WaitingQueueParam waitingQueueParam;
										waitingQueueParam.index				= waitingIndex++;
										waitingQueueParam.dataAddress		= outputData;
										waitingQueueParam.dataSizeInBytes	= outputDataSizeInBytes;
		
										waitingQueue->push( waitingQueueParam );
									}
								}
								mode = Mode::WaitForCompletionOfOutputsAll;
							}
						}
						break;
					case Mode::Recycle:
						{
							SCE_SAMPLE_UTIL_ASSERT( 1 <= waitingQueue->size( ) );
							if ( 1 <= waitingQueue->size( ) )
							{
								const WaitingQueueParam waitingQueueParam = waitingQueue->front( );
	
								const int result_waitGreaterEqual = this->m_internalContext->watchOutputCondVar->waitGreaterEqual( waitingQueueParam.index );
								SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_waitGreaterEqual ); ( void ) result_waitGreaterEqual;
	
								waitingQueue->pop( );
								alignedRingBuffer->recycle( waitingQueueParam.dataSizeInBytes );

								mode = Mode::WriteToPool;
							}
							else
							{
								mode = Mode::WriteToPool;
							}
						}
						break;
					case Mode::WaitForCompletionOfOutputsAll:
						{
							while ( 1 <= waitingQueue->size( ) )
							{
								const WaitingQueueParam waitingQueueParam = waitingQueue->front( );

								const int result_waitGreaterEqual = this->m_internalContext->watchOutputCondVar->waitGreaterEqual( waitingQueueParam.index );
								SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_waitGreaterEqual ); ( void ) result_waitGreaterEqual;

								waitingQueue->pop( );
							}

							this->m_completed = true;
						}
						break;
					}
				} /* while */
			}
			while ( false );

			this->m_exit = true;
			if ( nullptr != this->m_internalContext->isFinished )
			{
				*this->m_internalContext->isFinished = true;
			}

			if ( nullptr != alignedRingBuffer )
			{
				delete alignedRingBuffer;
			}

			if ( nullptr != waitingQueue )
			{
				// wait until all decoded data is outputted
				while (waitingQueue->size() > 0)
				{
					const WaitingQueueParam	waitingQueueParam = waitingQueue->front();
					const int	result_waitGreaterEqual = m_internalContext->watchOutputCondVar->waitGreaterEqual(waitingQueueParam.index);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(result_waitGreaterEqual, SCE_OK); (void)result_waitGreaterEqual;

					waitingQueue->pop();
				}

				this->m_completed = true;
				delete waitingQueue;
			}

			( void ) Decoder::finalize( decoderContext );

			return retval;
		}

	public:
		bool isFinished( ) const
		{
			return this->m_exit;
		}

	};	// DecodeThreadFunction
	
	Player::Player( )
	 : m_context( nullptr )
	{
		this->m_context = static_cast< Context * >( malloc( sizeof( Context ) ) );
		SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context );
		this->m_context->internalContext = nullptr;

		this->m_context->internalContext = static_cast< InternalContext * >( malloc( sizeof( InternalContext ) ) );
		SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context->internalContext );
		memset( this->m_context->internalContext, 0, sizeof( *this->m_context->internalContext ) );
	}
	
	Player::~Player( )
	{
		if ( nullptr != this->m_context )
		{
			if ( nullptr != this->m_context->internalContext )
			{
				free( this->m_context->internalContext );
			}

			free( this->m_context );
		}
	}
	
	int Player::initialize( Codec codec, const void *pData, size_t dataSizeInBytes, void * workingMemory, const size_t workingMemorySizeInBytes )
	{
		int retval = SCE_OK;

		Thread::Thread	*	tempDecodeThread					= nullptr;

		constexpr size_t		tempDecodedDataTemporaryBufferSizeInBytes	= 1048 * 64;
		uint8_t				*	tempDecodedDataTemporaryBuffer				= nullptr;

		Thread::CondVar< uint64_t >		*	tempWatchOutputCondVar	= nullptr;

		do
		{
			// Check Context.
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context );
			if ( nullptr == this->m_context )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break;	// Exit
			}
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context->internalContext );
			if ( nullptr == this->m_context->internalContext )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break;	// Exit
			}

			// Paser Data.
			uint32_t	sampleRate		= 0 ;
			uint32_t	bitsPerSample	= 0;
			uint32_t	channels		= 0;

			switch ( codec )
			{
			case Codec::kMp3:
				{
					Parser::MpegAudioHeader mpegAudioHeader;
					const Parser::ResultCode result_Parse = Parser::parseMpegAudio( mpegAudioHeader, pData, dataSizeInBytes );
					switch ( result_Parse )
					{
					case Parser::ResultCode::kOK:
						sampleRate		= mpegAudioHeader.samplingFrequency;
						channels		= mpegAudioHeader.numChannels;
						bitsPerSample	= 16;
						break;
					default:
						retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_NOT_SUPPORTED_CODEC;
						break;
					}
				}
				break;
			case Codec::kAtrac9:
				{
					Parser::ATRAC9AudioHeader atrac9AudioHeader;
					size_t headerSizeInBytes;
					const Parser::ResultCode result_Parse = Parser::parseATRAC9Audio( atrac9AudioHeader, headerSizeInBytes, pData, dataSizeInBytes );
					switch ( result_Parse )
					{
					case Parser::ResultCode::kOK:
						sampleRate		= atrac9AudioHeader.fmtChunk.samplesPerSec;
						channels		= atrac9AudioHeader.fmtChunk.numChannels;
						bitsPerSample	= atrac9AudioHeader.fmtChunk.bitsPerSample;
						
						break;
					default:
						retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_NOT_SUPPORTED_CODEC;
						break;
					}
				}
				break;
			default:
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_NOT_SUPPORTED_CODEC;
				break;
			}
			if ( SCE_OK != retval ) break;	// Exit.

			// Build Format.
			const Format format = [ ] ( uint32_t bitsPerSample, uint32_t channels ) -> Format
			{
				switch ( channels )
				{
				case 1:
					switch ( bitsPerSample )
					{
					case 8 : return Format::kS16_Mono;
					case 12: return Format::kS16_Mono;
					case 16: return Format::kS16_Mono;
					}
				case 2:
					switch ( bitsPerSample )
					{
					case 8 : return Format::kS16_Stereo;
					case 12: return Format::kS16_Stereo;
					case 16: return Format::kS16_Stereo;
					default: return Format::kS16_Stereo;
					}
				default:
					return Format::kS16_Stereo;
				}
			}( bitsPerSample, channels );
			

			// Allocate resources.
			tempDecodedDataTemporaryBuffer = static_cast < uint8_t * > ( malloc( tempDecodedDataTemporaryBufferSizeInBytes ) );
			if ( nullptr == tempDecodedDataTemporaryBuffer )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
				break; // Exit
			}

			tempWatchOutputCondVar = new Thread::CondVar< uint64_t > ( "AudioPlayerWatcher" );
			if ( nullptr == tempWatchOutputCondVar )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
				break; // Exit
			}

			// Setup Decode Thread.
			tempDecodeThread = new Thread::Thread( "AudioPlayerDecode", SCE_KERNEL_PRIO_FIFO_DEFAULT, 64 * 1024, nullptr );
			if ( nullptr == tempDecodeThread )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}

			// Succeeded
			this->m_context->internalContext->decodeThread		= tempDecodeThread;
			tempDecodeThread = nullptr;

			this->m_context->internalContext->codec					= codec;

			this->m_context->internalContext->inputData					= static_cast < const uint8_t * > ( pData );
			this->m_context->internalContext->inputDataSizeInBytes		= dataSizeInBytes;

			this->m_context->internalContext->decodedDataTemporaryBuffer			= tempDecodedDataTemporaryBuffer;
			this->m_context->internalContext->decodedDataTemporaryBufferSizeInBytes	= tempDecodedDataTemporaryBufferSizeInBytes;
			tempDecodedDataTemporaryBuffer = nullptr;

			this->m_context->internalContext->decodedInputDataSizeInBytes	= 0;

			this->m_context->internalContext->targetsamplingRate	= sampleRate;
			this->m_context->internalContext->targetFormat			= format;

			this->m_context->internalContext->ringQueueWorkingMemory			= workingMemory;
			this->m_context->internalContext->ringQueueWorkingMemorySizeInBytes	= workingMemorySizeInBytes;

			this->m_context->internalContext->watchOutputCondVar	= tempWatchOutputCondVar;
			tempWatchOutputCondVar = nullptr;

			this->m_context->internalContext->isInitialized = true;
		}
		while ( false );


		SCE_SAMPLE_UTIL_SAFE_DELETE( tempDecodeThread );

		if ( nullptr != tempDecodedDataTemporaryBuffer )
		{
			free( tempDecodedDataTemporaryBuffer );
		}

		if ( nullptr != tempWatchOutputCondVar )
		{
			delete tempWatchOutputCondVar;
		}

		return retval;
	}
	
	int Player::initialize( Codec codec, const char *pFilename, void * workingMemory, const size_t workingMemorySizeInBytes )
	{
		int retval = SCE_OK;

		FILE	*	fileHandle	= nullptr;
		void	*	fileData	= nullptr;

		do
		{
			SceKernelStat stat;
			const int result_Stat = sceKernelStat( pFilename, &stat );
			if ( SCE_OK != result_Stat )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
				break; // Exit;
			}

			const size_t fileSize = stat.st_size;
			fileData = malloc( fileSize );
			if ( nullptr == fileData )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
				break; // Exit;
			}

			fileHandle = fopen( pFilename, "r" );
			if ( nullptr == fileHandle )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
				break;	// Exit
			}

			const size_t readSize = fread( fileData, 1, fileSize, fileHandle );
			if ( readSize != fileSize )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_IO;
				break; // Exit
			}

			retval = this->initialize( codec, fileData, fileSize, workingMemory, workingMemorySizeInBytes );
			if ( SCE_OK != retval )
			{
				break; // Exit
			}

			// Succeeded.
			this->m_context->internalContext->audioDataAllocatedBySelf = fileData;
			fileData = nullptr;

		}
		while ( false );

		if ( nullptr != fileData )
		{
			free( fileData );
		}

		if ( nullptr != fileHandle )
		{
			fclose( fileHandle );
		}

		return retval;
	}
	
	int Player::play( AudioOutContext &audioOut, bool *pOutIsFinished )
	{
		int retval = SCE_OK;

		DecodeThreadFunction		*	tempDecodeThreadFunction		= nullptr;

		do
		{
			// Check Context.
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context );
			if ( nullptr == this->m_context )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit;
			}
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context->internalContext );
			if ( nullptr == this->m_context->internalContext )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}
			if ( false == this->m_context->internalContext->isInitialized )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_PLAYER_NOT_INITIALIZED;
				break;	// Exit
			}

			// Create Thread Functions.
			tempDecodeThreadFunction = new DecodeThreadFunction( this->m_context->internalContext );
			SCE_SAMPLE_UTIL_ASSERT( nullptr != tempDecodeThreadFunction );
			if ( nullptr == tempDecodeThreadFunction )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break;	// Exit
			}

			// Succeeded.
			this->m_context->internalContext->audioOutContext			= &audioOut;		// Shallow copy AudioOutContext instance.

			this->m_context->internalContext->decodeThreadFunction		= tempDecodeThreadFunction;
			tempDecodeThreadFunction = nullptr;

			this->m_context->internalContext->isFinished				= pOutIsFinished;
		}
		while( false );


		if ( nullptr != tempDecodeThreadFunction )
		{
			delete tempDecodeThreadFunction;
		}

		if ( SCE_OK == retval )
		{
			// Start Threads.
			const int result_startDecodeThread = this->m_context->internalContext->decodeThread->start( this->m_context->internalContext->decodeThreadFunction );
			SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_startDecodeThread ); ( void ) result_startDecodeThread;

			if ( nullptr == this->m_context->internalContext->isFinished )
			{	// Blocking mode.
				const int result_decodeThreadJoin		= this->m_context->internalContext->decodeThread->join( );
				SCE_SAMPLE_UTIL_ASSERT( SCE_OK == result_decodeThreadJoin ); ( void ) result_decodeThreadJoin;
			}
		}

		return retval;
	}
	
	int Player::finalize( )
	{
		int retval = SCE_OK;

		do
		{
			// Check Context.
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context );
			if ( nullptr == this->m_context )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context->internalContext );
			if ( nullptr == this->m_context->internalContext )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}

			if ( false == this->m_context->internalContext->isInitialized )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_PLAYER_NOT_INITIALIZED;
				break;	// Exit
			}

			// Release resources.
			if ( nullptr != this->m_context->internalContext->decodeThread )
			{
				this->m_context->internalContext->decodeThreadFunction->m_exit = true;
				( void ) this->m_context->internalContext->decodeThread->join();
				SCE_SAMPLE_UTIL_SAFE_DELETE( this->m_context->internalContext->decodeThread );
			}

			if ( nullptr != this->m_context->internalContext->decodeThreadFunction )
			{
				delete this->m_context->internalContext->decodeThreadFunction;
				this->m_context->internalContext->decodeThreadFunction = nullptr;
			}

			if ( nullptr != this->m_context->internalContext->decodedDataTemporaryBuffer )
			{
				free( this->m_context->internalContext->decodedDataTemporaryBuffer );
				this->m_context->internalContext->decodedDataTemporaryBuffer = nullptr;
			}

			if ( nullptr != this->m_context->internalContext->watchOutputCondVar )
			{
				delete ( this->m_context->internalContext->watchOutputCondVar );
				this->m_context->internalContext->watchOutputCondVar = nullptr;
			}

			if ( nullptr != this->m_context->internalContext->audioDataAllocatedBySelf )
			{
				free( this->m_context->internalContext->audioDataAllocatedBySelf );
				this->m_context->internalContext->audioDataAllocatedBySelf = nullptr;
			}
			this->m_context->internalContext->inputData = nullptr;

			// Succeeded.
			this->m_context->internalContext->isInitialized = false;

		}
		while ( false );

		return retval;
	}

	int Player::getTargetSamplingRate( uint & outSamplingRate ) const
	{
		int retval = SCE_OK;
		do
		{
			// Check Context.
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context );
			if ( nullptr == this->m_context )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context->internalContext );
			if ( nullptr == this->m_context->internalContext )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}

			if ( false == this->m_context->internalContext->isInitialized )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_PLAYER_NOT_INITIALIZED;
				break;	// Exit
			}

			// Succeeded.
			outSamplingRate = this->m_context->internalContext->targetsamplingRate;

		}
		while ( false );

		return retval;
		
	}
	int Player::getTargetAudioFormat( Format & outAudioFormat ) const
	{
		int retval = SCE_OK;
		do
		{
			// Check Context.
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context );
			if ( nullptr == this->m_context )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}
			SCE_SAMPLE_UTIL_ASSERT( nullptr != this->m_context->internalContext );
			if ( nullptr == this->m_context->internalContext )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_INTERNAL;
				break; // Exit
			}

			if ( false == this->m_context->internalContext->isInitialized )
			{
				retval = SCE_SAMPLE_UTIL_ERROR_AUDIO_PLAYER_NOT_INITIALIZED;
				break;	// Exit
			}

			// Succeeded.
			outAudioFormat = this->m_context->internalContext->targetFormat;
		}
		while ( false );

		return retval;
	}

} /* namespace Audio */ } /* namespace SampleUtil */ } /* namespace sce */
