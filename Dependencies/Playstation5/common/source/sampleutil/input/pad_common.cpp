/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#include <sampleutil/input/controller.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/input.h>


/* ------------------------------------------------------------------------- */

namespace ssi = sce::SampleUtil::Input;

const float ssi::PadContext::m_recipMaxByteAsFloat	= 1.0f / 255.0f;

/* ------------------------------------------------------------------------- */
ssi::PadContext::PadContext(const sce::SampleUtil::System::UserId userId, const int32_t type, const int32_t index, const PadContextOption* option)
{
	int ret = initialize(userId, type, index, option);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

/* ------------------------------------------------------------------------- */
ssi::PadContext::~PadContext()
{
	int ret = finalize();
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

/* ------------------------------------------------------------------------- */
int ssi::PadContext::getData(ssi::PadData *data, uint32_t length) const
{
	if (data == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	length = (this->m_numCurrentPadData) < length ? this->m_numCurrentPadData : length;
	for (uint32_t i=0; i<length; i++) {
		data[i] = this->m_currentPadData[i];
	}

	return length;
}

int ssi::PadContext::getPreviousUpdateData(ssi::PadData *data, uint32_t length) const
{
	if (data == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	length = (this->m_numPreviousPadData) < length ? this->m_numPreviousPadData : length;
	for (uint32_t i=0; i<length; i++) {
		data[i] = this->m_previousPadData[i];
	}

	return length;
}


int ssi::PadContext::getPressedButtons(ssi::Button *pressedButton)
{
	if (pressedButton == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	*pressedButton = (ssi::Button)this->m_pressedButtonData;

	return SCE_OK;
}

int ssi::PadContext::getReleasedButtons(ssi::Button *releasedButton)
{
	if (releasedButton == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}

	*releasedButton = (ssi::Button)this->m_releasedButtonData;

	return SCE_OK;
}

/* ------------------------------------------------------------------------- */
static bool _buttonBitPatternMatch(uint32_t buttonData, uint32_t bits, ssi::ButtonEventPattern pattern)
{
	if (pattern == ssi::kButtonEventPatternAny) {
		if ((buttonData & bits) != 0) {	
			return true;
		} else {
			return false;
		}
	} else if (pattern == ssi::kButtonEventPatternAll) {
		if ((buttonData & bits) == bits) {
			return true;
		} else {
			return false;
		}
	}

	return false;
}

/* ------------------------------------------------------------------------- */
bool ssi::PadContext::isButtonDown(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	return _buttonBitPatternMatch(this->m_currentPadData[0].buttons, buttons, pattern);
}

/* ------------------------------------------------------------------------- */
bool ssi::PadContext::isButtonUp(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	return !_buttonBitPatternMatch(this->m_currentPadData[0].buttons, buttons, pattern);
}

/* ------------------------------------------------------------------------- */
bool ssi::PadContext::isButtonPressed(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	return _buttonBitPatternMatch(this->m_pressedButtonData, buttons, pattern);
}

/* ------------------------------------------------------------------------- */
bool ssi::PadContext::isButtonReleased(uint32_t buttons, ssi::ButtonEventPattern pattern) const
{
	return _buttonBitPatternMatch(this->m_releasedButtonData, buttons, pattern);
}

/* ------------------------------------------------------------------------- */
const sce::Vectormath::Simd::Aos::Vector2& ssi::PadContext::getLeftStick() const
{
	return this->m_leftStickValue;
}

/* ------------------------------------------------------------------------- */
const sce::Vectormath::Simd::Aos::Vector2& ssi::PadContext::getRightStick() const
{
	return this->m_rightStickValue;
}

/* ------------------------------------------------------------------------- */
void ssi::PadContext::setRightAnalogStickDeadZone(float rightAnalogStickDeadZone)
{
	this->m_rightAnalogStickDeadZone = rightAnalogStickDeadZone;
	this->m_isRightAnalogStickDeadZoneUpdated = true;
}

/* ------------------------------------------------------------------------- */
void ssi::PadContext::setLeftAnalogStickDeadZone(float leftAnalogStickDeadZone)
{
	this->m_leftAnalogStickDeadZone = leftAnalogStickDeadZone;
	this->m_isLeftAnalogStickDeadZoneUpdated = true;
}

/* ------------------------------------------------------------------------- */
float ssi::PadContext::getDefaultLeftAnalogStickDeadZone()
{
	return this->m_defaultLeftAnalogStickDeadZone;
}

/* ------------------------------------------------------------------------- */
float ssi::PadContext::getDefaultRightAnalogStickDeadZone()
{
	return this->m_defaultRightAnalogStickDeadZone;
}

/* ------------------------------------------------------------------------- */
sce::SampleUtil::System::UserId ssi::PadContext::getUserId()
{
	return this->m_userId;
}

/* ------------------------------------------------------------------------- */
int32_t ssi::PadContext::getType()
{
	return this->m_type;
}

/* ------------------------------------------------------------------------- */
int32_t ssi::PadContext::getIndex()
{
	return this->m_index;
}
