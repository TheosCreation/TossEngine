/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#include "audio_paraser.h"

#include <string.h>

namespace sce { namespace SampleUtil { namespace Audio { namespace Parser
{
	ResultCode parseMpegAudio( MpegAudioHeader & outHeader, const void * data, const size_t dataSizeInBytes )
	{
		constexpr size_t MPEG_AUDIO_HEADER_SIZE = 4;

		constexpr uint32_t bitRate[ 4 ][ 16 ] =
		{
			  { 0,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160 }	// MPEG2.5
			, { 0,  0,  0,  0,  0,  0,  0,  0,   0,   0,   0,   0,   0,   0,   0 }	// reserved
			, { 0,  8, 16, 24, 32, 40, 48, 56,  64,  80,  96, 112, 128, 144, 160 }	// MPEG2
			, { 0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320 }	// MPEG1
		};
		constexpr uint32_t samplingFrequency[ 4 ][ 3 ] =
		{
			  { 11025, 12000,  8000 }	// MPEG2.5
			, {     0,     0,     0 }	// reserved
			, { 22050, 24000, 16000 }	// MPEG2
			, { 44100, 48000, 32000 }	// MPEG1
		};
		constexpr uint32_t numChannels[ 4 ] =
		{
			2, 2, 2, 1
		};

		ResultCode resultCode;

		MpegAudioHeader tempHeader = { };

		do
		{
			if ( nullptr == data )
			{
				resultCode = ResultCode::kError_InvalidParam_Data;
				break; // Exit
			}

			if ( MPEG_AUDIO_HEADER_SIZE > dataSizeInBytes )
			{
				resultCode = ResultCode::kError_InvalidParam_DataSize;
				break; // Exit
			}

			// Parse Mpeg Header.
			const uint8_t * dataAsUint8 = static_cast < const uint8_t * > ( data );

			tempHeader.syncWord               = ( dataAsUint8[ 0 ] & 0xFF ) << 4 | ( dataAsUint8[ 1 ] & 0xE0 ) >> 4;
			tempHeader.version                = ( dataAsUint8[ 1 ] & 0x18 ) >> 3;
			tempHeader.layer                  = ( dataAsUint8[ 1 ] & 0x06 ) >> 1;
			tempHeader.protectionBit          = ( dataAsUint8[ 1 ] & 0x01 ) >> 0;
			tempHeader.bitRateIndex           = ( dataAsUint8[ 2 ] & 0xF0 ) >> 4;
			tempHeader.samplingFrequencyIndex = ( dataAsUint8[ 2 ] & 0x06 ) >> 2;
			tempHeader.paddingBit             = ( dataAsUint8[ 2 ] & 0x02 ) >> 1;
			tempHeader.privateBit             = ( dataAsUint8[ 2 ] & 0x01 ) >> 0;
			tempHeader.chMode                 = ( dataAsUint8[ 3 ] & 0xC0 ) >> 6;
			tempHeader.modeExtension          = ( dataAsUint8[ 3 ] & 0x30 ) >> 4;
			tempHeader.copyrightBit           = ( dataAsUint8[ 3 ] & 0x08 ) >> 3;
			tempHeader.originalBit            = ( dataAsUint8[ 3 ] & 0x04 ) >> 2;
			tempHeader.emphasis               = ( dataAsUint8[ 3 ] & 0x03 ) >> 0;

			if ( 0xFFE != tempHeader.syncWord )
			{
				resultCode = ResultCode::kError_BadData_Mp3;
				break; // Exit
			}

			tempHeader.bitRate				= bitRate[ tempHeader.version ][ tempHeader.bitRateIndex ];
			tempHeader.samplingFrequency	= samplingFrequency[ tempHeader.version ][ tempHeader.samplingFrequencyIndex ];
			tempHeader.numChannels			= numChannels[ tempHeader.chMode ];

			// Succeeded
			resultCode = ResultCode::kOK;

			outHeader = tempHeader;
		}
		while ( false );

		return resultCode;
	} // ParseMpegAudio

	ResultCode parseATRAC9Audio( ATRAC9AudioHeader & outHeader, size_t & outHeaderSizeInBytes, const void * data, const size_t dataSizeInBytes )
	{
		constexpr uint16_t	WAVE_FORMAT_EXTENSIBLE = 0xFFFE;

		constexpr uint8_t	SubFormat[16] = { 0xD2, 0x42, 0xE1, 0x47, 0xBA, 0x36, 0x8D, 0x4D, 0x88, 0xFC, 0x61, 0x65, 0x4F, 0x8C, 0x83, 0x6C };		// 0x47E142D2, 0x36BA, 0x4D8D, 0x88FC61654F8C836C


		ResultCode		resultCode = ResultCode::kInvalid;

		ATRAC9AudioHeader	tempHeader = { };

	uint32_t readSize = 0;
	uint32_t chunkDataSize;

	uint32_t headerSize = 0;

		

		do
		{
			if ( nullptr == data )
			{
				resultCode = ResultCode::kError_InvalidParam_Data;
				break; // Exit
			}

			const uint8_t * dataAsUint8 = static_cast < const uint8_t * > ( data );

			// find RIFF-WAVE chunk
			for ( ; ; )
			{
				if ( dataSizeInBytes < readSize + sizeof( RiffWaveHeader ) )
				{
					resultCode = ResultCode::kError_InvalidParam_DataSize;
					break; // Exit
				}

				memcpy( &tempHeader.riffWaveHeader, dataAsUint8 + readSize, sizeof( RiffWaveHeader ) );
				readSize += sizeof( RiffWaveHeader );

				 // Check "RIFF"
				if ( 0x46464952 != tempHeader.riffWaveHeader.chunkId )
				{
					resultCode = ResultCode::kError_Format_RIFF;
					break; // Exit
				}
				chunkDataSize = tempHeader.riffWaveHeader.chunkDataSize;
				if (chunkDataSize % 2 == 1)
				{
					// if chunkDataSize is odd, add padding data length
					chunkDataSize += 1;
				}

				// Check "WAVE"
				if ( 0x45564157 == tempHeader.riffWaveHeader.typeId )
				{
					break;
				}

				if ( dataSizeInBytes < readSize + chunkDataSize - sizeof (tempHeader.riffWaveHeader.typeId ) )
				{
					resultCode = ResultCode::kError_InvalidParam_Data;
					break;	// Exit
				}
				readSize += chunkDataSize - sizeof( tempHeader.riffWaveHeader.typeId );
			}
			if ( ResultCode::kInvalid != resultCode ) break; // Exit

			while ( 0 == headerSize  )
			{
				ChunkHeader chunkHeader;
				if ( dataSizeInBytes < ( readSize + sizeof( ChunkHeader ) ) )
				{
					resultCode = ResultCode::kError_InvalidParam_DataSize;
					break; // Exit
				}

				memcpy( &chunkHeader, dataAsUint8 + readSize, sizeof( ChunkHeader ) );
				readSize += sizeof( ChunkHeader );

				chunkDataSize = chunkHeader.chunkDataSize;

				if ( ( chunkDataSize % 2 ) == 1)
				{
					// if chunkDataSize is odd, add padding data length
					chunkDataSize += 1;
				}

				switch ( chunkHeader.chunkId )
				{
				case 0x20746D66: // "fmt "
					memcpy( &tempHeader.fmtChunkHeader, &chunkHeader, sizeof( ChunkHeader ) );

					if ( dataSizeInBytes < readSize + sizeof( FmtChunk ) )
					{
						resultCode = ResultCode::kError_InvalidParam_DataSize;
						break; // Exit
					}
					memcpy( &tempHeader.fmtChunk, dataAsUint8 + readSize, sizeof( FmtChunk ) );
					readSize += sizeof( FmtChunk );

					if ( tempHeader.fmtChunk.formatTag != WAVE_FORMAT_EXTENSIBLE )
					{
						resultCode = ResultCode::kError_Format_Tag;
						break; // Exit
					}
					if ( 0 != memcmp( SubFormat, tempHeader.fmtChunk.subFormat, sizeof( SubFormat ) ) )
					{
						resultCode = ResultCode::kError_Format_Unknown;
						break; // Exit
					}
					// rest is unknown data, so just skip them
					chunkDataSize = ( chunkDataSize < sizeof( FmtChunk ) ) ? 0 : chunkDataSize - sizeof( FmtChunk );
					break;
				case 0x74636166: // "fact"
					memcpy( &tempHeader.factChunkHeader, &chunkHeader, sizeof( ChunkHeader ) );

					if ( dataSizeInBytes < readSize + sizeof( FactChunk ) )
					{
						resultCode = ResultCode::kError_InvalidParam_DataSize;
						break; // Exit
					}
					memcpy( &tempHeader.factChunk, dataAsUint8 + readSize, sizeof( FactChunk ) );
					readSize += sizeof( FactChunk );

					// rest is unknown data, so just skip them
					chunkDataSize = ( chunkDataSize < sizeof( FactChunk ) ) ? 0 : chunkDataSize - sizeof( FactChunk );
					break;
				case 0x61746164: // "data"
					memcpy( &tempHeader.dataChunkHeader, &chunkHeader, sizeof( ChunkHeader ) );

					headerSize = readSize;
					break;
				case 0x6C706D73: // "smpl"
					memcpy( &tempHeader.smplChunkHeader, &chunkHeader, sizeof( ChunkHeader ) );

					if ( dataSizeInBytes < readSize + sizeof( SmplChunk ) )
					{
						resultCode = ResultCode::kError_InvalidParam_DataSize;
						break; // Exit
					}
					memcpy( &tempHeader.smplChunk, dataAsUint8 + readSize, sizeof( SmplChunk ) );
					readSize += sizeof( SmplChunk );

					// rest is unknown data, so just skip them
					chunkDataSize = ( chunkDataSize < sizeof( SmplChunk ) ) ? 0 : chunkDataSize - sizeof( SmplChunk );
					break;
				default: // unknown chunk
					resultCode = ResultCode::kError_Chunk_Unknown;
					break;	// Exit
				}
				if ( ResultCode::kInvalid != resultCode ) break; // Exit

				if ( dataSizeInBytes < readSize + chunkDataSize )
				{
					resultCode = ResultCode::kError_InvalidParam_DataSize;
					break; // Exit
				}

				readSize += chunkDataSize;
			}
			if ( ResultCode::kInvalid != resultCode ) break; // Exit

			// Succeeded.
			resultCode = ResultCode::kOK;

			outHeader				= tempHeader;
			outHeaderSizeInBytes	= headerSize;
		}
		while ( false );

		return resultCode;
	} // ParseATRAC9Audio
} /* namespace Parser */ } /* namespace Audio */ } /* namespace SampleUtil */ } /* namespace sce */
