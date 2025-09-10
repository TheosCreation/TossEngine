/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <scetypes.h>

namespace sce { namespace SampleUtil { namespace Audio {
	namespace Parser
	{
		enum class ResultCode
		{
			  kInvalid
			, kOK
			, kError_InvalidParam_Data
			, kError_InvalidParam_DataSize
			, kError_BadData_Mp3
			, kError_BadData_Atrac9
			, kError_Format_RIFF
			, kError_Format_Tag
			, kError_Format_Unknown
			, kError_Chunk_Unknown
		};
		
		struct MpegAudioHeader
		{
			uint32_t	syncWord;
			uint32_t	version;
			uint32_t	layer;
			uint32_t	protectionBit;
			uint32_t	bitRateIndex;
			uint32_t	samplingFrequencyIndex;
			uint32_t	paddingBit;
			uint32_t	privateBit;
			uint32_t	chMode;
			uint32_t	modeExtension;
			uint32_t	copyrightBit;
			uint32_t	originalBit;
			uint32_t	emphasis;
			uint32_t	bitRate;
			uint32_t	samplingFrequency;
			uint32_t	numChannels;
		};

		struct RiffWaveHeader
		{
			uint32_t	chunkId;
			uint32_t	chunkDataSize;
			uint32_t	typeId;
		};

		struct ChunkHeader
		{
			uint32_t	chunkId;
			uint32_t	chunkDataSize;
		};

		struct FmtChunk
		{
			uint16_t	formatTag;			// format ID
			uint16_t	numChannels;		// number of channels, 1 = mono, 2 = stereo
			uint32_t	samplesPerSec;		// sampling rate
			uint32_t	avgBytesPerSec;		// average bite rate
			uint16_t	blockAlign;			// audio block size, ((mono) 1 = 8bit, 2 = 16bit), ((stereo) 2 = 8bit, 4 = 16bit)
			uint16_t	bitsPerSample;		// quantization bit rate, 8, 12, 16
			uint16_t	cbSize;				// extension size, 34
			uint16_t	samplesPerBlock;	// number of output samples of audio block
			uint32_t	chennelMask;		// mapping of channels to spatial location
			uint8_t		subFormat[ 16 ];	// codec ID
			uint32_t	versionInfo;		// version information, 0
			uint8_t		configData[ 4 ];	// ATRAC9 setting information
			uint32_t	reserved;			// reserved, 0
		};

		struct FactChunk
		{
			uint32_t	totalSamples;						// total samples per chennel
			uint32_t	delaySamplesInputOverlap;			// samples of input and overlap delay
			uint32_t	delaySamplesInputOverlapEncoder;	// samples of input and overlap and encoder delay
		};

		struct SampleLoop
		{
			uint32_t	identifier;
			uint32_t	type;
			uint32_t	start;
			uint32_t	end;
			uint32_t	fraction;
			uint32_t	playCount;
		};

		struct SmplChunk
		{
			uint32_t	manufacturer;
			uint32_t	product;
			uint32_t	samplePeriod;
			uint32_t	midiUnityNote;
			uint32_t	midiPitchFraction;
			uint32_t	smpteFormat;
			uint32_t	smpteOffset;
			uint32_t	sampleLoops;
			uint32_t	samplerData;
			SampleLoop	sampleLoop;
		};

		struct ATRAC9AudioHeader
		{
			RiffWaveHeader	riffWaveHeader;
			ChunkHeader		fmtChunkHeader;
			FmtChunk		fmtChunk;
			ChunkHeader		factChunkHeader;
			FactChunk		factChunk;
			ChunkHeader		smplChunkHeader;
			SmplChunk		smplChunk;
			ChunkHeader		dataChunkHeader;
		};

		ResultCode parseMpegAudio( MpegAudioHeader & outHeader, const void * data, const size_t dataSizeInBytes );
		ResultCode parseATRAC9Audio( ATRAC9AudioHeader & outHeader, size_t & outHeaderSizeInBytes, const void * data, const size_t dataSizeInBytes );

	}/* namespace Parser */
} /* namespace Audio */ } /* namespace SampleUtil */ } /* namespace sce */
