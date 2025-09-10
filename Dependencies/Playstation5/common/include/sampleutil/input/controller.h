/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc. 
* 
*/

#ifndef _SCE_SAMPLE_UTIL_INPUT_CONTROLLER_H
#define _SCE_SAMPLE_UTIL_INPUT_CONTROLLER_H

#include <scebase_common.h>
#include <sampleutil/sampleutil_common.h>
#include <vectormath.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Input
		{
			/*!
			 * @~English
			 * 
			 * @brief The enumeration type to be used for specifying a matching pattern of buttons 
			 * @details Used for specifying for the 2nd argument "pattern" of the PadContext member functions isButtonDown(), isButtonUp(), isButtonPressed(), and isButtonReleased(). 
			 * @~Japanese
			 * 
			 * @brief ボタンの一致パターンを指定するために利用する列挙型 
			 * @details PadContextのメンバ関数isButtonDown()、isButtonUp()、isButtonPressed()、isButtonReleased() の第2引数patternに指定して利用します。 
			 */
			enum ButtonEventPattern
			{
				/*!
				 * @~English
				 * @brief If any one of the specified buttons matches, the function will return true as the return value. 
				 * @~Japanese
				 * @brief 指定されたボタンがどれか一つでも一致していたら関数が返り値にtrueを返す挙動となる 
				 */
				kButtonEventPatternAny = 0,
				/*!
				 * @~English
				 * @brief If all of the specified buttons matches, the function will return true as the return value. 
				 * @~Japanese
				 * @brief 指定されたボタンが全て一致していたら関数が返り値にtrueを返す挙動となる 
				 */
				kButtonEventPatternAll
			};

			/*!
			 * @~English
			 * 
			 * @brief The enum value of a button 
			 * @details Used for specifying for the 1st argument "buttons" of the PadContext member functions isButtonDown(), isButtonUp(), isButtonPressed(), and isButtonReleased(). 
			 * @~Japanese
			 * 
			 * @brief ボタンのenum値 
			 * @details PadContextのメンバ関数isButtonDown()、isButtonUp()、isButtonPressed()、isButtonReleased()の第1引数buttonsに指定して利用します。 
			 */
			enum Button
			{
				/*!
				 * @~English
				 * @brief L3 button 
				 * @~Japanese
				 * @brief L3ボタン 
				 */
				kButtonL3		=		(1<<1),
				/*!
				 * @~English
				 * @brief R3 button 
				 * @~Japanese
				 * @brief R3ボタン 
				 */
				kButtonR3		=		(1<<2),
				/*!
				 * @~English
				 * @brief OPTIONS button 
				 * @~Japanese
				 * @brief OPTIONSボタン 
				 */
				kButtonOptions	=		(1<<3),
				/*!
				 * @~English
				 * @brief Up button 
				 * @~Japanese
				 * @brief 方向キー上 
				 */
				kButtonUp		=		(1<<4),
				/*!
				 * @~English
				 * @brief Right button 
				 * @~Japanese
				 * @brief 方向キー右 
				 */
				kButtonRight	=		(1<<5),
				/*!
				 * @~English
				 * @brief Down button 
				 * @~Japanese
				 * @brief 方向キー下 
				 */
				kButtonDown		=		(1<<6),
				/*!
				 * @~English
				 * @brief Left button 
				 * @~Japanese
				 * @brief 方向キー左 
				 */
				kButtonLeft		=		(1<<7),
				/*!
				 * @~English
				 * @brief L2 button 
				 * @~Japanese
				 * @brief L2ボタン 
				 */
				kButtonL2		=		(1<<8),
				/*!
				 * @~English
				 * @brief R2 button 
				 * @~Japanese
				 * @brief R2ボタン 
				 */
				kButtonR2		=		(1<<9),
				/*!
				 * @~English
				 * @brief L1 button 
				 * @~Japanese
				 * @brief L1ボタン 
				 */
				kButtonL1		=		(1<<10),
				/*!
				 * @~English
				 * @brief R1 button 
				 * @~Japanese
				 * @brief R1ボタン 
				 */
				kButtonR1		=		(1<<11),
				/*!
				 * @~English
				 * @brief Triangle button 
				 * @~Japanese
				 * @brief △ボタン 
				 */
				kButtonTriangle	=		(1<<12),
				/*!
				 * @~English
				 * @brief Circle button 
				 * @~Japanese
				 * @brief ○ボタン 
				 */
				kButtonCircle	=		(1<<13),
				/*!
				 * @~English
				 * @brief Cross button 
				 * @~Japanese
				 * @brief ×ボタン 
				 */
				kButtonCross	=		(1<<14),
				/*!
				 * @~English
				 * @brief Square button 
				 * @~Japanese
				 * @brief □ボタン 
				 */
				kButtonSquare	=		(1<<15),
				/*!
				 * @~English
				 * @brief Touchpad 
				 * @~Japanese
				 * @brief タッチパッド 
				 */
				kButtonTouchPad	=		(1<<20)
			};


			struct ControllerData 
			{
				uint64_t	timeStamp;	/*E time stamp (usec)            */
				uint32_t	buttons;	/*E pressed digital button       */
				uint8_t		lx;			/*E L analog controller (X-axis) */
				uint8_t		ly;			/*E L analog controller (Y-axis) */
				uint8_t		rx;			/*E R analog controller (X-axis) */
				uint8_t		ry;			/*E R analog controller (Y-axis) */
			};
		}
	}
}



#endif /* _SCE_SAMPLE_UTIL_INPUT_CONTROLLER_H */

