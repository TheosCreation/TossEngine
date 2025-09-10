/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc. 
* 
*/

#include "stdafx.h"

#if _SCE_TARGET_OS_WINDOWS || _SCE_TARGET_OS_PSP2

#include <sampleutil/input/controller.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil_internal.h>
#include "controller_common.h"

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{
			namespace Impl
			{

				class ControllerContextImpl : public ControllerContext, public sce::SampleUtil::Impl::ResourceImpl
				{
				public:

					ControllerContextImpl(void){}
					~ControllerContextImpl(void)
					{
						finalize();
					}

					int initialize(const ControllerContextOption* option=NULL);

					int finalize(void);

					void update(void); 

					int getData(uint32_t port, ControllerData* data, uint32_t length) const;


					bool isButtonDown(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
					bool isButtonUp(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
					bool isButtonPressed(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;
					bool isButtonReleased(uint32_t buttons, ButtonEventPattern pattern=kButtonEventPatternAll) const;

					const sce::Vectormath::Simd::Aos::Vector2& getLeftStick(void) const;

					const sce::Vectormath::Simd::Aos::Vector2& getRightStick(void) const;
					void setDeadZone(float deadZone);

					std::string getResourceType(void) const
					{
						return "Input::ControllerContext";
					}

				private:
					ControllerData*	m_currentCtrlData;
					ControllerData*	m_previousCtrlData;
					sce::Vectormath::Simd::Aos::Vector2			m_leftStickXY;
					sce::Vectormath::Simd::Aos::Vector2			m_rightStickXY;

					float			m_deadZone;
					uint32_t		m_numBufs;
					uint32_t        m_numAvailableData;
					uint32_t		m_pressedButtonData;
					uint32_t		m_releasedButtonData;

					static const float		m_defaultDeadZone;
					static const uint32_t	m_defaultNumBufs			= 64;
					static const float		m_recipMaxByteAsFloat;

					ControllerBase m_controllerBase;
				};
			}
		}
	}
}

namespace ss = sce::SampleUtil;
namespace ssi = sce::SampleUtil::Input;
namespace ssii = sce::SampleUtil::Input::Impl;
namespace vm = sce::Vectormath::Simd::Aos;

const float ssii::ControllerContextImpl::m_defaultDeadZone 	= 0.25;
const float ssii::ControllerContextImpl::m_recipMaxByteAsFloat		= 1.0f / 255.0f;


int ssii::ControllerContextImpl::initialize(const ssi::ControllerContextOption* option)
{
	if (option == NULL) {
		m_numBufs = m_defaultNumBufs;
	} else {
		if (option->numBufs < 1 || kMaxNumControllerDataBufs < option->numBufs) {
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		m_numBufs = option->numBufs;
	}
	m_deadZone = m_defaultDeadZone;

	m_currentCtrlData = (ControllerData*)malloc(sizeof(ControllerData) * m_numBufs);
	if (m_currentCtrlData == NULL) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	m_previousCtrlData = (ControllerData*)malloc(sizeof(ControllerData) * m_numBufs);
	if (m_previousCtrlData == NULL ) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}

	for (uint32_t i = 0; i < m_numBufs; i++) {
		memset((void*)&m_currentCtrlData[i], 0, sizeof(m_currentCtrlData[i]));
		memset((void*)&m_previousCtrlData[i], 0, sizeof(m_previousCtrlData[i]));
	}

	m_pressedButtonData		= 0;
	m_releasedButtonData	= 0;
	m_leftStickXY.setX(0.0f);
	m_leftStickXY.setY(0.0f);
	m_rightStickXY.setX(0.0f);
	m_rightStickXY.setY(0.0f);

	m_numAvailableData = 0;

	int ret = m_controllerBase.initialize();
	if (ret < 0) {
		free(m_currentCtrlData);
		free(m_previousCtrlData);
		return ret;
	}

	return SCE_OK;
}

int ssii::ControllerContextImpl::finalize(void)
{
	if (m_currentCtrlData) {
		free(m_currentCtrlData);
	}

	if (m_previousCtrlData) {
		free(m_previousCtrlData);
	}

	m_controllerBase.finalize();

	return SCE_OK;
}

void ssii::ControllerContextImpl::update(void)
{
	if (m_currentCtrlData == NULL || m_previousCtrlData == NULL) {
		return;
	}

	for (uint32_t i = 0; i < m_numBufs; i++) {
		m_previousCtrlData[i] = m_currentCtrlData[i];
	}

	// Read the controller state
	m_numAvailableData = m_controllerBase.readBufferPositive(m_currentCtrlData, m_numBufs);

	// ascertain button pressed / released events from the current & previous state
	m_pressedButtonData = (m_currentCtrlData[0].buttons & ~m_previousCtrlData[0].buttons);		///< pressed button event data
	m_releasedButtonData = (~m_currentCtrlData[0].buttons & m_previousCtrlData[0].buttons);		///< released button event data

	// Get the analogue stick values
	// Remap ranges from 0-255 to -1 > +1
	float lX = (float)((int32_t)m_currentCtrlData[0].lx * 2 -255) * m_recipMaxByteAsFloat;
	float lY = (float)((int32_t)m_currentCtrlData[0].ly * 2 -255) * m_recipMaxByteAsFloat;
	float rX = (float)((int32_t)m_currentCtrlData[0].rx * 2 -255) * m_recipMaxByteAsFloat;
	float rY = (float)((int32_t)m_currentCtrlData[0].ry * 2 -255) * m_recipMaxByteAsFloat;

	// store stick values adjusting for deadzone
	m_leftStickXY.setX((fabsf(lX) < m_deadZone) ? 0.0f : lX);
	m_leftStickXY.setY((fabsf(lY) < m_deadZone) ? 0.0f : lY);
	m_rightStickXY.setX((fabsf(rX) < m_deadZone) ? 0.0f : rX);
	m_rightStickXY.setY((fabsf(rY) < m_deadZone) ? 0.0f : rY);
}

int ssii::ControllerContextImpl::getData(uint32_t port, ssi::ControllerData* data, uint32_t length) const
{
	if (data == NULL) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	length = (m_numAvailableData < length) ? m_numAvailableData : length;
	for (uint32_t i=0; i<length; i++) {
		data[i] = m_currentCtrlData[i];
	}
	return length;
}

const vm::Vector2& ssii::ControllerContextImpl::getLeftStick(void) const
{
	return m_leftStickXY;
}

const vm::Vector2& ssii::ControllerContextImpl::getRightStick(void) const
{
	return m_rightStickXY;
}

void ssii::ControllerContextImpl::setDeadZone(float deadZone)
{
	m_deadZone = deadZone;
}

bool ssii::ControllerContextImpl::isButtonDown(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	if (m_currentCtrlData == NULL) {
		return false;
	}

	if (pattern == kButtonEventPatternAny) {
		if ((m_currentCtrlData[0].buttons & buttons) != 0) {	
			return true;
		} else {
			return false;
		}
	} else if (pattern == kButtonEventPatternAll) {
		if ((m_currentCtrlData[0].buttons & buttons) == buttons) {
			return true;
		} else {
			return false;
		}
	}
	return false;
}

bool ssii::ControllerContextImpl::isButtonUp(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	if (m_currentCtrlData == NULL) {
		return false;
	}

	if (pattern == kButtonEventPatternAny) {
		if ((m_currentCtrlData[0].buttons & buttons) != 0) {	
			return false;
		} else {
			return true;
		}
	} else if (pattern == kButtonEventPatternAll) {
		if ((m_currentCtrlData[0].buttons & buttons) == buttons) {
			return false;
		} else {
			return true;
		}
	}
	return false;
}

bool ssii::ControllerContextImpl::isButtonPressed(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	if (pattern == kButtonEventPatternAny) {
		if ((m_pressedButtonData & buttons) != 0) {	
			return true;
		} else {
			return false;
		}
	} else if (pattern == kButtonEventPatternAll) {
		if ((m_pressedButtonData & buttons) == buttons) {
			return true;
		} else {
			return false;
		}
	}
	return false;
}

bool ssii::ControllerContextImpl::isButtonReleased(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	if (pattern == kButtonEventPatternAny) {
		if ((m_releasedButtonData & buttons) != 0) {	
			return true;
		} else {
			return false;
		}
	} else if (pattern == kButtonEventPatternAll) {
		if ((m_releasedButtonData & buttons) == buttons) {
			return true;
		} else {
			return false;
		}
	}
	return false;
}

int ssi::createControllerContext(ssi::ControllerContext **outControllerContext, const ssi::ControllerContextOption* option)
{
	if (outControllerContext == NULL) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	ssii::ControllerContextImpl *t = new ssii::ControllerContextImpl;
	int ret = t->initialize(option);
	if (ret != SCE_OK) {
		delete t;
		return ret;
	}
	*outControllerContext = t;
	return ret;
}

#endif /*_SCE_TARGET_OS_WINDOWS || _SCE_TARGET_OS_PSP2*/
