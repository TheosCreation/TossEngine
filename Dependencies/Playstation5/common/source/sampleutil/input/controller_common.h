/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc. 
* 
*/

#include "stdafx.h"

#ifndef _SAMPLE_UTILITIES_CONTROLLER_COMMON_H
#define _SAMPLE_UTILITIES_CONTROLLER_COMMON_H

#if _SCE_TARGET_OS_WINDOWS || _SCE_TARGET_OS_PSP2

#include <sampleutil/input/controller.h>

#if _SCE_TARGET_OS_PSP2

#include <ctrl.h>
#pragma comment(lib, "SceCtrl_stub")
#include <alloca.h>

#endif // _SCE_TARGET_OS_PSP2

#if _SCE_TARGET_OS_WINDOWS 
#define DIRECTINPUT_VERSION 0x0800 /* DIRECTINPUT_HEADER_VERSION */
#include <windows.h>
#include <dinput.h>
#include <dinputd.h>
#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "DXGUID.LIB")

#endif // _SCE_TARGET_OS_WINDOWS 

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{
			namespace Impl
			{

				class ControllerBase
				{
				public:
#if _SCE_TARGET_OS_WINDOWS 
					LPDIRECTINPUT8			m_pDI;
					LPDIRECTINPUTDEVICE8	m_pJoystick;
					static BOOL CALLBACK enumJoysticksCallback( const DIDEVICEINSTANCE* pdidInstance, VOID* pContext );
					static BOOL CALLBACK enumObjectsCallback( const DIDEVICEOBJECTINSTANCE* pdidoi, VOID* pContext );
					int readBufferPositive(ControllerData *ctrlData, uint32_t numBufs);

					int initialize(void);

					int finalize(void);

#endif // _SCE_TARGET_OS_WINDOWS

#if _SCE_TARGET_OS_PSP2
					int readBufferPositive(ControllerData *ctrlData, uint32_t numBufs);

					int initialize(void);

					int finalize(void);
#endif // _SCE_TARGET_OS_PSP2

				};
			}
		}
	}
}

#endif /*_SCE_TARGET_OS_WINDOWS || _SCE_TARGET_OS_PSP2*/

#endif /*_SAMPLE_UTILITIES_CONTROLLER_COMMON_H*/
