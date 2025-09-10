/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include <sampleutil/input/controller.h>
#include <sampleutil/sampleutil_error.h>

#include <sampleutil/input.h>
#include <sampleutil/input/keyboard.h>

/* ------------------------------------------------------------------------- */
namespace ssi  = sce::SampleUtil::Input;

/* ------------------------------------------------------------------------- */
ssi::KeyboardContext::KeyboardContext(SampleUtil::System::UserId userId)
{
	int ret = initialize(userId);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
}

/* ------------------------------------------------------------------------- */
ssi::KeyboardContext::~KeyboardContext()
{
	finalize();
}

bool ssi::KeyboardContext::isKeyDown(ssi::Key key) const
{
	return (m_keyStateArray[key] & kKeyStateDown) == kKeyStateDown;
}

bool ssi::KeyboardContext::isKeyUp(ssi::Key key) const
{
	return (m_keyStateArray[key] & kKeyStateUp) == kKeyStateUp;
}

bool ssi::KeyboardContext::isKeyPressed(ssi::Key key) const
{
	return (m_keyStateArray[key] & kKeyStatePressed) == kKeyStatePressed;
}

bool ssi::KeyboardContext::isKeyRepeated(ssi::Key key) const
{
	return (m_keyStateArray[key] & kKeyStateRepeated) == kKeyStateRepeated;
}

bool ssi::KeyboardContext::isKeyReleased(ssi::Key key) const
{
	return (m_keyStateArray[key] & kKeyStateReleased) == kKeyStateReleased;
}
