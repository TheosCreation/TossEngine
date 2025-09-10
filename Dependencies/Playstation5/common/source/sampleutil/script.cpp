/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */

#include <cstdio>
#include <cstdlib>
#include <strings.h>
#include <string>
#include <algorithm>
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core.h>
#include <agc_gpu_address.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gpu_address.h>
#endif
#include <video_out.h>
#include <sampleutil/input.h>
#include <sampleutil/skeleton.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/script.h>

namespace
{
	// Helper function to save 32bpp bmp file.
	int	saveTextureToBMP(uint32_t	width, uint32_t	height, const void	*pData, const std::string	&bmpFileName)
	{
		FILE *pBmpFile = fopen(bmpFileName.c_str(), "w+b");
		SCE_SAMPLE_UTIL_ASSERT(pBmpFile != nullptr);
		if (pBmpFile == nullptr)
		{
			return	SCE_SAMPLE_UTIL_ERROR_FILE_OPEN;
		}

		const uint32_t numberOfBytesInDIBHeader	= 40;
		const uint32_t pixelArrayOffset			= numberOfBytesInDIBHeader + 14;

		// Allocate a buffer and grab the screen + 2 to make sure saves to uint32_t are aligned (first 2 stores to the header are of type char)
		uint32_t screenBMPSize = width * height * 4 + pixelArrayOffset;
		auto *pScreenBMP = reinterpret_cast<char *>(malloc(screenBMPSize));
		SCE_SAMPLE_UTIL_ASSERT(pScreenBMP != nullptr);
		if (pScreenBMP == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}

		// Fill i != nulltprn the BMP header.
		pScreenBMP[0] = 'B';									// ID Field
		pScreenBMP[1] = 'M';									// ID Field
		memcpy(pScreenBMP + 2, &screenBMPSize, 4);				// Size of the BMP file
		*(int16_t *)(pScreenBMP + 6) = 0x0;						// Application specific
		*(int16_t *)(pScreenBMP + 8) = 0x0;						// Application specific
		memcpy(pScreenBMP + 10, &pixelArrayOffset, 4);			// Offset where pixel array (bitmap data) can be found
		memcpy(pScreenBMP + 14, &numberOfBytesInDIBHeader, 4);	// Number of bytes in the DIB header(from this point)
		memcpy(pScreenBMP + 18, &width, 4);						// Width of the bitmap in pixel
		memcpy(pScreenBMP + 22, &height, 4);					// Height of the bitmap in pixel
		*(int16_t *)(pScreenBMP + 26) = (int16_t)0x1;			// Number of color planes being used
		*(int16_t *)(pScreenBMP + 28) = (int16_t)32;			// Number of bits per pixel
		memset(pScreenBMP + 30, 0, 24);

		char *pt = pScreenBMP + pixelArrayOffset;
		uint32_t scanlineBytes = width * 4;

		auto	*pSrc = reinterpret_cast<const char *>(pData);
		auto	*pDst = pt + scanlineBytes * (height - 1);
		for (int h = 0; h < height; ++h) 
		{
			for (uint32_t i = 0; i < scanlineBytes / 4; ++i)
			{
				pDst[i * 4 + 0] = pSrc[i * 4 + 2];
				pDst[i * 4 + 1] = pSrc[i * 4 + 1];
				pDst[i * 4 + 2] = pSrc[i * 4 + 0];
				pDst[i * 4 + 3] = pSrc[i * 4 + 3];
			}
			pDst -= scanlineBytes;
			pSrc += scanlineBytes;
		}

		fwrite(pScreenBMP, 1, screenBMPSize, pBmpFile);
		fclose(pBmpFile);

		free(pScreenBMP);
				
		return	SCE_OK;
	}

	uint32_t	convertToButtonMask(const char	*pParam)
	{
		uint32_t buttonMask = 0;

		if (strcasecmp(pParam, "Circle") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonCircle;
		} else if (strcasecmp(pParam, "Cross") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonCross;
		} else if (strcasecmp(pParam, "Triangle") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonTriangle;
		} else if (strcasecmp(pParam, "Square") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonSquare;
		} else if (strcasecmp(pParam, "Options") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonOptions;
		} else if (strcasecmp(pParam, "Up") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonUp;
		} else if (strcasecmp(pParam, "Down") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonDown;
		} else if (strcasecmp(pParam, "Left") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonLeft;
		} else if (strcasecmp(pParam, "Right") == 0)
		{
			buttonMask |= sce::SampleUtil::Input::kButtonRight;
		} else {
			SCE_SAMPLE_UTIL_ASSERT_MSG(false, "Unrecognised argument '%s' passed to Buttonpress()\n", pParam);
		}

		return	buttonMask;
	}
} // anonymous namespace

namespace sce { namespace SampleUtil {
	ApplicationScript::ApplicationScript()
	{
		// Register all our commands
		m_commands.insert(std::make_pair("Frames"		, frames));
		m_commands.insert(std::make_pair("Screenshot"	, screenshot));
		m_commands.insert(std::make_pair("Framerate"	, framerate));
		m_commands.insert(std::make_pair("Buttonpress"	, buttonpress));
		m_commands.insert(std::make_pair("ButtonDown"	, buttonDown));
		m_commands.insert(std::make_pair("ButtonUp"		, buttonUp));
		m_commands.insert(std::make_pair("Quit"			, quit));
	}

	int ApplicationScript::executeNextCommand(std::istream	&script, SampleSkeleton	&app)
	{
		int ret = SCE_OK;

		// Get command from the script
		std::string line;
		while (app.m_isScriptRunning && (app.m_scriptWaitFrames == 0) && getline(script >> std::ws, line))
		{
			std::size_t openBracket		= line.find('(');
			std::size_t closeBracket	= line.find(')');
			
			SCE_SAMPLE_UTIL_ASSERT_MSG((openBracket != std::string::npos) && (closeBracket != std::string::npos) &&
										(openBracket > 0) && (closeBracket > openBracket),
										"Badly formed command %s\n", line.c_str());
			// This is a well formed command, Tidy up and return it.
			std::string command	= line.substr(0, openBracket);
			std::string params	= line.substr(openBracket + 1, closeBracket - openBracket - 1); // Params after the open bracket.

			if (m_commands.find(command) == m_commands.end())
			{
				// Didn't find the command
				SCE_SAMPLE_UTIL_ASSERT_MSG(false, "Unrecognized command %s(%s)\n", command.c_str(), params.c_str());

				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			// Match it to our list and execute it
			ret = (*m_commands[command])(app, params);
			SCE_SAMPLE_UTIL_ASSERT_MSG(ret == SCE_OK, "Error while executing %s(%s)\n", command.c_str(), params.c_str());
			if (ret != SCE_OK)
			{
				return ret;
			}
		}

		// No more commands
		return SCE_OK;
	}

	int	ApplicationScript::framerate(sce::SampleUtil::SampleSkeleton	&app, const std::string	&params)
	{
		auto	context = app.getGraphicsContext();
		if (context != nullptr)
		{
			printf("## Sample Application: Framerate:%s: %f %d ##\n", params.c_str(), 1.0f / app.m_deltaTime, context->m_flipCountMain);
		} else {
			printf("## Sample Application: Framerate:%s: %f ##\n", params.c_str(), 1.0f / app.m_deltaTime);
		}

		return	SCE_OK;
	}

	int	ApplicationScript::frames(sce::SampleUtil::SampleSkeleton	&app, const std::string	&params)
	{
		int waitFrames;
		sscanf(params.c_str(), "%d", &waitFrames);
		app.m_scriptWaitFrames = std::max(waitFrames - 1, 0);

		return	SCE_OK;
	}

	int	ApplicationScript::screenshot(sce::SampleUtil::SampleSkeleton	&app, const std::string	&params)
	{
		int ret = SCE_OK; (void)ret;

		if (params != "")
		{
			auto context = app.getGraphicsContext();
			if (context != nullptr)
			{
				uint32_t    numFrameBuffer;
#if _SCE_TARGET_OS_PROSPERO
				sce::Agc::Label *pFlipLabels;
#endif
#if _SCE_TARGET_OS_ORBIS
				volatile uint64_t *pFlipLabels;
#endif
				uint32_t *pFlipCount;
				SceVideoOutBusType bus = SCE_VIDEO_OUT_BUS_TYPE_MAIN;

				std::string filename;
				std::string busname;
				std::string arg(params);

				auto comma = arg.find(',');

				if (comma != std::string::npos)
				{
					filename	= arg.substr(0, comma);
					busname		= arg.substr(comma + 1, arg.length());
				} else {
					filename	= arg;
				}
#if _SCE_TARGET_OS_PROSPERO
				if (strcasecmp(busname.c_str(), "overlay") == 0)
				{
					pFlipCount		= &context->m_flipCountOverlay;
					numFrameBuffer	= context->m_option.numFrameBufferOverlay;
					pFlipLabels		= context->m_flipLabelsOverlay.get();
					bus				= SCE_VIDEO_OUT_BUS_TYPE_OVERLAY;
				} else
#endif
				{
					pFlipCount		= &context->m_flipCountMain;
					numFrameBuffer	= context->m_option.numFrameBufferMain;
					pFlipLabels		= context->m_flipLabelsMain.get();
					bus				= SCE_VIDEO_OUT_BUS_TYPE_MAIN;
				}

				SCE_SAMPLE_UTIL_ASSERT_MSG((pFlipLabels != nullptr) && (pFlipCount != nullptr), "Can not take screenshot because requested render target is not supported by sample.\n");
				if ((pFlipLabels == nullptr) || (pFlipCount == nullptr))
				{
					return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
				}

				const int frameBufferIndex = *pFlipCount % numFrameBuffer;
#if _SCE_TARGET_OS_PROSPERO
				while (pFlipLabels[frameBufferIndex].m_value != 1)
#endif
#if _SCE_TARGET_OS_ORBIS
				while (pFlipLabels[frameBufferIndex] != 1)
#endif
				{
					sceKernelUsleep(1000);
				}

				uint32_t	targetWidth = 0;
				uint32_t	targetHeight = 0;
#if _SCE_TARGET_OS_PROSPERO
				sce::Agc::Core::RenderTargetSpec rts;
				ret = sce::Agc::Core::translate(&rts, context->getNextRenderTarget(bus));
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				targetWidth		= rts.getWidth();
				targetHeight	= rts.getHeight();

				sce::AgcGpuAddress::SurfaceSummary summary;
				ret = sce::Agc::Core::translate(&summary, &rts);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

				std::vector<uint32_t> detiled(summary.m_totalSizeInBytes);
				ret = sce::AgcGpuAddress::detileSurface(detiled.data(), summary.m_totalSizeInBytes, context->getNextRenderTarget(bus)->getDataAddress(),
														summary.m_totalSizeInBytes, &summary, 0, 0);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#endif
#if _SCE_TARGET_OS_ORBIS
				sce::GpuAddress::TilingParameters tp;
				ret = tp.initFromRenderTarget(context->getNextRenderTarget(bus), 0);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				uint64_t	untiledSize = 0;
				sce::Gnm::AlignmentType align;
				ret = sce::GpuAddress::computeUntiledSurfaceSize(&untiledSize, &align, &tp);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
				std::vector<uint32_t> detiled(untiledSize);
				sce::GpuAddress::detileSurface(detiled.data(), context->getNextRenderTarget(bus)->getBaseAddress(), &tp);
				SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
#endif

				saveTextureToBMP(targetWidth, targetHeight, detiled.data(), filename);
			}
		}

		return	SCE_OK;
	}

	int	ApplicationScript::buttonpress(sce::SampleUtil::SampleSkeleton	&app, const std::string	&params)
	{
		uint32_t buttonMask = convertToButtonMask(params.c_str());
		app.m_scriptPadData.buttons		|= buttonMask;
		app.m_scriptPadButtonClearMask	|= buttonMask;

		return	SCE_OK;
	}

	int	ApplicationScript::buttonDown(sce::SampleUtil::SampleSkeleton	&app, const std::string	&params)
	{
		uint32_t buttonMask = convertToButtonMask(params.c_str());
		app.m_scriptPadData.buttons		|= buttonMask;
		app.m_scriptPadButtonClearMask	&= ~buttonMask;

		return	SCE_OK;
	}

	int	ApplicationScript::buttonUp(sce::SampleUtil::SampleSkeleton	&app, const std::string &params)
	{
		uint32_t buttonMask = convertToButtonMask(params.c_str());
		app.m_scriptPadData.buttons		&= ~buttonMask;
		app.m_scriptPadButtonClearMask	|= buttonMask;

		return	SCE_OK;
	}

	int	ApplicationScript::quit(sce::SampleUtil::SampleSkeleton	&app, const std::string	&params)
	{
		(void)params;

		app.m_isScriptRunning = false;

		return SCE_OK;
	}
}} // namespace sce::SampleUtil
