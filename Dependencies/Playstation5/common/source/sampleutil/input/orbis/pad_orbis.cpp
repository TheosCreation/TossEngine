/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc. 
 * 
 */

#include <scebase_common.h>
#if _SCE_TARGET_OS_ORBIS

#include <sampleutil/input.h>
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/input/controller.h>
#include <sampleutil/debug/api_capture.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/event.h>
#include <kernel.h>
#include <user_service.h>

#pragma comment(lib,"libScePad_stub_weak.a")


namespace ssi = sce::SampleUtil::Input;
using namespace sce::Vectormath::Simd::Aos;

static bool	m_usedPadInit = false;

static ssi::PadData convert(const ScePadData &pad, int touchPadWidth, int touchPadHeight)
{
	ssi::PadData ctrl;
	memset(&ctrl, 0, sizeof(ctrl));
	ctrl.connected = pad.connected;

	if (!pad.connected) {
		ctrl.lx = 0x80;
		ctrl.ly = 0x80;
		ctrl.rx = 0x80;
		ctrl.ry = 0x80;

		ctrl.l2 = 0;
		ctrl.r2 = 0;
		return ctrl;
	}
	ctrl.timeStamp = pad.timestamp;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_L3    ) ? ssi::kButtonL3       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_R3    ) ? ssi::kButtonR3       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_OPTIONS ) ? ssi::kButtonOptions  : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_UP    ) ? ssi::kButtonUp       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_RIGHT ) ? ssi::kButtonRight    : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_DOWN  ) ? ssi::kButtonDown     : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_LEFT  ) ? ssi::kButtonLeft     : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_L2    ) ? ssi::kButtonL2       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_R2    ) ? ssi::kButtonR2       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_L1    ) ? ssi::kButtonL1       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_R1    ) ? ssi::kButtonR1       : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_TRIANGLE ) ? ssi::kButtonTriangle : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_CIRCLE   ) ? ssi::kButtonCircle   : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_CROSS    ) ? ssi::kButtonCross    : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_SQUARE   ) ? ssi::kButtonSquare   : 0;
	ctrl.buttons |= (pad.buttons & SCE_PAD_BUTTON_TOUCH_PAD   ) ? ssi::kButtonTouchPad   : 0;
	ctrl.lx = pad.leftStick.x;
	ctrl.ly	= pad.leftStick.y;
	ctrl.rx	= pad.rightStick.x;
	ctrl.ry = pad.rightStick.y;

	ctrl.l2	= pad.analogButtons.l2;
	ctrl.r2 = pad.analogButtons.r2;

	ctrl.motionData.acceleration.setX(pad.acceleration.x);
	ctrl.motionData.acceleration.setY(pad.acceleration.y);
	ctrl.motionData.acceleration.setZ(pad.acceleration.z);
	ctrl.motionData.angularVelocity.setX(pad.angularVelocity.x);
	ctrl.motionData.angularVelocity.setY(pad.angularVelocity.y);
	ctrl.motionData.angularVelocity.setZ(pad.angularVelocity.z);
	ctrl.motionData.orientation.setW(pad.orientation.w);
	ctrl.motionData.orientation.setX(pad.orientation.x);
	ctrl.motionData.orientation.setY(pad.orientation.y);
	ctrl.motionData.orientation.setZ(pad.orientation.z);

	ctrl.touchNumber = pad.touchData.touchNum;

	for (int j=0;j<SCE_PAD_MAX_TOUCH_NUM;j++) {
		ctrl.touchPadData[j].id = pad.touchData.touch[j].id;
		ctrl.touchPadData[j].x  = (float)pad.touchData.touch[j].x / touchPadWidth;
		ctrl.touchPadData[j].y  = (float)pad.touchData.touch[j].y / touchPadHeight;
	}

	ctrl.intercepted = (pad.buttons & SCE_PAD_BUTTON_INTERCEPTED) != 0;
	ctrl.connectedCount = pad.connectedCount;

	return ctrl;
}


int ssi::PadContext::initialize(System::UserId userId, int32_t type, int32_t index, const ssi::PadContextOption* option)
{
	int ret = SCE_OK;

	if (option == nullptr)	{
		m_numPadData = kDefaultNumPadDataBufs;
	} else {
		if (option->numBufs < 1 || kMaxNumPadDataBufs < option->numBufs) {
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}
		m_numPadData = option->numBufs;
	}
	m_numCurrentPadData = 0;
	m_numPreviousPadData = 0;

	m_userId = userId;
	m_type = type;
	m_index = index;

	m_pressedButtonData = 0;
	m_releasedButtonData = 0;

	m_isTimeSpecifiedVibing = false;

	if (m_usedPadInit == false) {
		ret = scePadInit();
		m_usedPadInit = true;
	}
	if (ret != SCE_OK) {
		return ret;
	}

	m_handle = scePadOpen(userId, type, index, (option != nullptr)?option->padOpenParam:nullptr);
	if (m_handle < 0) {
		return m_handle;
	}

	m_defaultRightAnalogStickDeadZone = 0.06666667;
	m_defaultLeftAnalogStickDeadZone  = 0.06666667;
	m_touchPadWidth        = 0x00000780;
	m_touchPadHeight       = 0x000002f2;
	m_touchPadPixelDensity = 42.2000008;
	m_connectionType       = -1;

	m_touchPadAspectRatio  = (float)m_touchPadWidth / m_touchPadHeight;

	m_currentConnectedCount = 0;

	m_rightAnalogStickDeadZone = m_defaultRightAnalogStickDeadZone;
	m_leftAnalogStickDeadZone  = m_defaultLeftAnalogStickDeadZone;

	m_currentPadData = (PadData*)malloc(sizeof(PadData) * m_numPadData);
	if (m_currentPadData == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}
	for (uint32_t i = 0; i < m_numPadData; i++) {
		memset((void*)&m_currentPadData[i], 0, sizeof(m_currentPadData[i]));
	}

	m_previousPadData = (PadData*)malloc(sizeof(PadData) * m_numPadData);
	if (m_previousPadData == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}
	for (uint32_t i = 0; i < m_numPadData; i++) {
		memset((void*)&m_previousPadData[i], 0, sizeof(m_previousPadData[i]));
	}

	m_currentData = (ScePadData*)malloc(sizeof(ScePadData) * m_numPadData);
	if (m_currentData == nullptr) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}
	memset(m_currentData, 0, sizeof(ScePadData) * m_numPadData);

	m_leftStickValue.setX(0.0f);
	m_leftStickValue.setY(0.0f);
	m_rightStickValue.setX(0.0f);
	m_rightStickValue.setY(0.0f);

	if (Debug::ApiCapture::RenderHudContext::get().m_pPad == nullptr)
	{
		Debug::ApiCapture::RenderHudContext::get().m_pPad = this; // steal pad context
	}

	return ret;
}

int ssi::PadContext::finalize()
{
	if (Debug::ApiCapture::RenderHudContext::get().m_pPad == this)
	{
		Debug::ApiCapture::RenderHudContext::get().m_pPad = nullptr; // reset stolen pad
	}

	if (m_handle > 0) {
		int ret = scePadClose(m_handle);
		if (ret < 0) {
			return ret;
		}
	}
	if (m_currentPadData) {
		free(m_currentPadData);
	}
	if (m_previousPadData) {
		free(m_previousPadData);
	}
	if (m_currentData) {
		free(m_currentData);
	}

	setVibration(0.0f, 0.0f, 0);	

	return SCE_OK;
}

int ssi::PadContext::update(Input::PadData	*pScriptPadData)
{
	int ret;
	
	if (m_isTimeSpecifiedVibing) {		
		if (sceKernelGetProcessTime() - m_startVibrationTime >= m_vibrationTimeInMs * 1000) {			
			setVibration(0.0f, 0.0f, 0);	
			m_isTimeSpecifiedVibing = false;			
		}
	}	

	ret = updatePadData(pScriptPadData);

	float lX=0.0f, lY=0.0f, rX=0.0f, rY=0.0f;
	m_pressedButtonData  = 0;
	m_releasedButtonData = 0;
	
	if(m_numCurrentPadData>0){
		m_pressedButtonData  = (m_currentPadData[m_numCurrentPadData-1].buttons & ~m_previousPadData[0].buttons);
		m_releasedButtonData = (~m_currentPadData[m_numCurrentPadData-1].buttons & m_previousPadData[0].buttons);		///< released button event data
		for(int i=(m_numCurrentPadData-1);i>0;i--){
			m_pressedButtonData  |= (m_currentPadData[i-1].buttons & ~m_currentPadData[i].buttons);
			m_releasedButtonData |= (~m_currentPadData[i-1].buttons & m_currentPadData[i].buttons);
		}

		// Get the analogue stick values
		// Remap ranges from 0-255 to -1 > +1
		lX = (float)((int32_t)m_currentPadData[0].lx * 2 -255) * m_recipMaxByteAsFloat;
		lY = (float)((int32_t)m_currentPadData[0].ly * 2 -255) * m_recipMaxByteAsFloat;
		rX = (float)((int32_t)m_currentPadData[0].rx * 2 -255) * m_recipMaxByteAsFloat;
		rY = (float)((int32_t)m_currentPadData[0].ry * 2 -255) * m_recipMaxByteAsFloat;
	
		// store stick values adjusting for deadzone
		float leftLength = length(Vector2(lX, lY));
		m_leftStickValue.setX((leftLength < m_leftAnalogStickDeadZone) ? 0.0f : lX);
		m_leftStickValue.setY((leftLength < m_leftAnalogStickDeadZone) ? 0.0f : lY);
		float rightLength = length(Vector2(rX, rY));
		m_rightStickValue.setX((rightLength < m_rightAnalogStickDeadZone) ? 0.0f : rX);
		m_rightStickValue.setY((rightLength < m_rightAnalogStickDeadZone) ? 0.0f : rY);

	}

	return ret;
}

int ssi::PadContext::enableMotionSensor(bool bEnable)
{
	int ret = scePadSetMotionSensorState(m_handle, bEnable);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

int ssi::PadContext::enableTiltCorrection(bool bEnable)
{
	int ret = scePadSetTiltCorrectionState(m_handle, bEnable);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

int ssi::PadContext::enableAngularVelocityDeadband(bool bEnable)
{
	int ret = scePadSetAngularVelocityDeadbandState(m_handle, bEnable);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

int ssi::PadContext::resetOrientation()
{
	int ret = scePadResetOrientation(m_handle);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

int ssi::PadContext::setVibration(float largeMotorSpeed, float smallMotorSpeed, uint32_t vibrationTimeInMs)
{
	ScePadVibrationParam param;
				
	if (largeMotorSpeed < 0.0f || largeMotorSpeed > 1.0f || smallMotorSpeed < 0.0f || smallMotorSpeed > 1.0f) {
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	if (vibrationTimeInMs > 2500) {
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}
	
	if ((largeMotorSpeed == 0.0f && smallMotorSpeed == 0.0f) || vibrationTimeInMs == 0) {	
		param.largeMotor = 0;
		param.smallMotor = 0;
		m_isTimeSpecifiedVibing = false;
	} else {
		param.largeMotor = 255 * largeMotorSpeed;
		param.smallMotor = 255 * smallMotorSpeed;
		m_vibrationTimeInMs = vibrationTimeInMs;
		m_startVibrationTime = sceKernelGetProcessTime();
		m_isTimeSpecifiedVibing = true;
	}

	int ret = scePadSetVibration(m_handle, &param);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

int ssi::PadContext::setLightBar(const ScePadLightBarParam *pParam)
{
	int ret = scePadSetLightBar(m_handle, pParam);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

int ssi::PadContext::resetLightBar()
{
	int ret = scePadResetLightBar(m_handle);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		return SCE_OK;
	} else {
		return ret;
	}
}

float ssi::PadContext::getTouchPadPixelDensity()
{
	return m_touchPadPixelDensity;
}

float ssi::PadContext::getTouchPadAspectRatio()
{
	return m_touchPadAspectRatio;
}

int ssi::PadContext::getConnectionType()
{
	return m_connectionType;
}

int ssi::PadContext::getPadHandle()
{
	return m_handle;
}

int ssi::PadContext::updatePadData(Input::PadData	*pScriptPadData)
{
	int ret = 0;
	bool logout = false;

	ret = scePadRead(m_handle, (ScePadData *)m_currentData, m_numPadData);

	if (ret == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
		logout = true;
	} else {
		if (ret < SCE_OK) {
			return ret;
		}
		if (((ScePadData *)m_currentData)->connected && ((ScePadData *)m_currentData)->connectedCount > m_currentConnectedCount) {
			ScePadControllerInformation pInfo;
			int ret2 = scePadGetControllerInformation(m_handle, &pInfo);
			if (ret2 == SCE_HID_ERROR_ALREADY_LOGGED_OUT) {
				logout = true;
			} else {
				if (ret2 < SCE_OK) {
					return ret2;
				}
				m_defaultRightAnalogStickDeadZone = (float)(pInfo.stickInfo.deadZoneRight * 2) * m_recipMaxByteAsFloat;
				m_defaultLeftAnalogStickDeadZone  = (float)(pInfo.stickInfo.deadZoneLeft * 2) * m_recipMaxByteAsFloat;
				m_touchPadWidth         = pInfo.touchPadInfo.resolution.x;
				m_touchPadHeight        = pInfo.touchPadInfo.resolution.y;
				m_touchPadPixelDensity  = (float)pInfo.touchPadInfo.pixelDensity;
				m_touchPadAspectRatio   = (float)m_touchPadWidth / m_touchPadHeight;
				m_currentConnectedCount = ((ScePadData *)m_currentData)->connectedCount;
				m_connectionType        = pInfo.connectionType;
			}
		}
	}

	if (logout) {
		ret = setScePadDataForUserLogoutCase((ScePadData *)m_currentData);
	}

	m_numCurrentPadData = std::min(m_numPadData, ret);
	if (pScriptPadData != nullptr)
	{
		m_numCurrentPadData = std::max(1, m_numCurrentPadData);
	}
	if(m_numCurrentPadData > 0){
		for(int i = 0; i < m_numPreviousPadData; i++) {
			m_previousPadData[i] = m_currentPadData[i];
		}
		for(int i = 0; i < m_numCurrentPadData; i++) {
			if (pScriptPadData != nullptr)
			{
				m_currentPadData[i] = *pScriptPadData;
			} else {
				m_currentPadData[i] = convert(((ScePadData *)m_currentData)[m_numCurrentPadData - 1 - i], m_touchPadWidth, m_touchPadHeight);
			}
		}
		for(int i = m_numCurrentPadData; i < m_numPadData; i++) {
			memset((void*)&m_currentPadData[i], 0, sizeof(m_currentPadData[i]));
		}
		m_numPreviousPadData = m_numCurrentPadData;
	}

	return ret;
}

int ssi::PadContext::setScePadDataForUserLogoutCase(ScePadData* data)
{
	memset(data, 0x0, sizeof(ScePadData));
	data->connected = false;
	data->leftStick.x = 0x80;
	data->leftStick.y = 0x80;
	data->rightStick.x = 0x80;
	data->rightStick.y = 0x80;
	data->orientation.w = 1.0f;
	data->acceleration.y = 1.0f;

	return 1;
}


#endif //_SCE_TARGET_OS_ORBIS
