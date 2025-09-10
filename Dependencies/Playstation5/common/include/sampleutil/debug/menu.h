/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <cstddef>
#include <cstdint>
#include <climits>
#include <vectormath/cpp/vectormath_aos.h>
#include "../source/imgui/imgui.h"

namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Debug-associated definitions 
		 * @details These are the Debug-associated definitions. 
		 * @~Japanese
		 * @brief Debug関連の定義 
		 * @details Debug関連の定義です。 
		 */
		namespace Debug
		{
			class Menu;
			class MenuItemGroup;
			class AbstractMenuItem;

			/*!
			 * @~English
			 * @brief Constant that indicates the menu item type 
			 * @details This constant indicates the menu item type. Each menu item type can be identified using the sce::SampleUtil::Debug::AbstractMenuItem::getType method. 
			 * @~Japanese
			 * @brief メニュー項目の種類を表す定数 
			 * @details メニュー項目の種類を表す定数です。sce::SampleUtil::Debug::AbstractMenuItem::getTypeメソッドを用いて、それぞれのメニュー項目の種類を識別できます。 
			 */
			typedef enum 
			{
				/*!
				 * @~English
				 * @brief Constant that indicates a single precision floating-point type menu item 
				 * @~Japanese
				 * @brief 単精度浮動小数点型のメニュー項目を表す定数 
				 */
				TYPE_FLOAT,	
				/*!
				 * @~English
				 * @brief Constant that indicates a double precision floating-point type menu item 
				 * @~Japanese
				 * @brief 倍精度浮動小数点型のメニュー項目を表す定数 
				 */
				TYPE_DOUBLE,	
				/*!
				 * @~English
				 * @brief Constant that indicates a signed 8-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号付8ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_INT8,	
				/*!
				 * @~English
				 * @brief Constant that indicates a signed 16-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号付16ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_INT16,	
				/*!
				 * @~English
				 * @brief Constant that indicates a signed 32-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号付32ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_INT32,	
				/*!
				 * @~English
				 * @brief Constant that indicates a signed 64-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号付64ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_INT64,	
				/*!
				 * @~English
				 * @brief Constant that indicates an unsigned 8-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号なし8ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_UINT8,	
				/*!
				 * @~English
				 * @brief Constant that indicates an unsigned 16-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号なし16ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_UINT16,	
				/*!
				 * @~English
				 * @brief Constant that indicates an unsigned 32-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号なし32ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_UINT32,	
				/*!
				 * @~English
				 * @brief Constant that indicates an unsigned 64-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号なし64ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_UINT64,	
				/*!
				 * @~English
				 * @brief Constant that indicates a Bool type menu item 
				 * @~Japanese
				 * @brief Bool型のメニュー項目を表す定数 
				 */
				TYPE_BOOL,
				/*!
				 * @~English
				 * @brief Constant that indicates an indexed character string set type menu item 
				 * @~Japanese
				 * @brief インデックス付文字列セット型のメニュー項目を表す定数 
				 */
				TYPE_ENUM,

				/*!
				 * @~English
				 * @brief Constant that indicates an ListBox type menu item
				 * @~Japanese
				 * @brief ListBoxのメニュー項目を表す定数
				 */
				TYPE_LISTBOX,

				/*!
				 * @~English
				 * @brief Constant that indicates int2/InputInt2 menu item
				 * @~Japanese
				 * @brief int2/InputInt2 のメニュー項目を表す定数
				 */
				TYPE_INT32_V2,

				/*!
				 * @~English
				 * @brief Constant that indicates int3/InputInt3 menu item
				 * @~Japanese
				 * @brief int3/InputInt3 のメニュー項目を表す定数
				 */
				TYPE_INT32_V3,
				/*!
				* @~English
				* @brief Constant that indicates int4/InputInt4 menu item
				* @~Japanese
				* @brief int4/InputInt4 のメニュー項目を表す定数
				*/
				TYPE_INT32_V4,
				/*!
				* @~English
				* @brief Constant that indicates float2/InputFloat2 menu item
				* @~Japanese
				* @brief float2/InputFloat2 のメニュー項目を表す定数
				*/
				TYPE_FLOAT2,
				/*!
				* @~English
				* @brief Constant that indicates float3/InputFloat3 menu item
				* @~Japanese
				* @brief float3/InputFloat3 のメニュー項目を表す定数
				*/
				TYPE_FLOAT3,
				/*!
				* @~English
				* @brief Constant that indicates float4/InputFloat4 menu item
				* @~Japanese
				* @brief float4/InputFloat4 のメニュー項目を表す定数
				*/
				TYPE_FLOAT4,

				/*!
				* @~English
				* @brief Constant that indicates SliderInt menu item
				* @~Japanese
				* @brief SliderInt のメニュー項目を表す定数
				*/
				TYPE_INT32_SLIDER,

				/*!
				* @~English
				* @brief Constant that indicates SliderInt2 menu item
				* @~Japanese
				* @brief SliderInt2 のメニュー項目を表す定数
				*/
				TYPE_INT32_V2_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderInt3 menu item
				* @~Japanese
				* @brief SliderInt3 のメニュー項目を表す定数
				*/
				TYPE_INT32_V3_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderInt4 menu item
				* @~Japanese
				* @brief SliderInt4 のメニュー項目を表す定数
				*/
				TYPE_INT32_V4_SLIDER,

				/*!
				* @~English
				* @brief Constant that indicates SliderFloat menu item
				* @~Japanese
				* @brief SliderFloat のメニュー項目を表す定数
				*/
				TYPE_FLOAT_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderFloat2 menu item
				* @~Japanese
				* @brief SliderFloat2 のメニュー項目を表す定数
				*/
				TYPE_FLOAT2_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderFloat3 menu item
				* @~Japanese
				* @brief SliderFloat3 のメニュー項目を表す定数
				*/
				TYPE_FLOAT3_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderFloat4 menu item
				* @~Japanese
				* @brief SliderFloat4 のメニュー項目を表す定数
				*/
				TYPE_FLOAT4_SLIDER,

				/*!
				* @~English
				* @brief Constant that indicates ColorEdit3 menu item
				* @~Japanese
				* @brief ColorEdit3 のメニュー項目を表す定数
				*/
				TYPE_FLOAT3_COLOR_EDIT,
				/*!
				* @~English
				* @brief Constant that indicates ColorEdit4 menu item
				* @~Japanese
				* @brief ColorEdit4 のメニュー項目を表す定数
				*/
				TYPE_FLOAT4_COLOR_EDIT,
				/*!
				* @~English
				* @brief Constant that indicates ColorPicker3 menu item
				* @~Japanese
				* @brief ColorPicker3 のメニュー項目を表す定数
				*/
				TYPE_FLOAT3_COLOR_PICKER,
				/*!
				* @~English
				* @brief Constant that indicates ColorPicker4 menu item
				* @~Japanese
				* @brief ColorPicker4 のメニュー項目を表す定数
				*/
				TYPE_FLOAT4_COLOR_PICKER,

				/*!
				* @~English
				* @brief Constant that indicates Text menu item
				* @~Japanese
				* @brief Text のメニュー項目を表す定数
				*/
				TYPE_TEXT,
				/*!
				* @~English
				* @brief Constant that indicates InputText menu item
				* @~Japanese
				* @brief InputText のメニュー項目を表す定数
				*/
				TYPE_TEXT_INPUT,
				/*!
				* @~English
				* @brief Constant that indicates LabelText menu item
				* @~Japanese
				* @brief LabelText のメニュー項目を表す定数
				*/
				TYPE_TEXT_LABEL,
				/*!
				* @~English
				* @brief Constant that indicates BulletText menu item
				* @~Japanese
				* @brief BulletText のメニュー項目を表す定数
				*/
				TYPE_TEXT_BULLET,

				/*!
				* @~English
				* @brief Constant that indicates Checkbox menu item
				* @~Japanese
				* @brief Checkbox のメニュー項目を表す定数
				*/
				TYPE_CHECKBOX,
				/*!
				* @~English
				* @brief Constant that indicates RadioButton menu item
				* @~Japanese
				* @brief RadioButton のメニュー項目を表す定数
				*/
				TYPE_RADIOBUTTON,
				/*!
				* @~English
				* @brief Constant that indicates Separator menu item
				* @~Japanese
				* @brief Separator のメニュー項目を表す定数
				*/
				TYPE_SEPARATOR,
				/*!
				* @~English
				* @brief Constant that indicates SameLine menu item
				* @~Japanese
				* @brief SameLine のメニュー項目を表す定数
				*/
				TYPE_SAMELINE,
				/*!
				* @~English
				* @brief Constant that indicates NewLine menu item
				* @~Japanese
				* @brief NewLine のメニュー項目を表す定数
				*/
				TYPE_NEWLINE,
				/*!
				* @~English
				* @brief Constant that indicates Spacing menu item
				* @~Japanese
				* @brief Spacing のメニュー項目を表す定数
				*/
				TYPE_SPACING,
				/*!
				* @~English
				* @brief Constant that indicates Dummy menu item
				* @~Japanese
				* @brief Dummy のメニュー項目を表す定数
				*/
				TYPE_DUMMY,

				/*!
				 * @~English
				 * @brief Constant that indicates an unsigned 32-bit integer type menu item 
				 * @~Japanese
				 * @brief 符号なし32ビット整数型のメニュー項目を表す定数 
				 */
				TYPE_INT = TYPE_INT32,
				/*!
				* @~English
				* @brief Constant that indicates int2/InputInt2 menu item
				* @~Japanese
				* @brief int2/InputInt2 のメニュー項目を表す定数
				*/
				TYPE_INT2 = TYPE_INT32_V2,
				/*!
				* @~English
				* @brief Constant that indicates int3/InputInt3 menu item
				* @~Japanese
				* @brief int3/InputInt3 のメニュー項目を表す定数
				*/
				TYPE_INT3 = TYPE_INT32_V3,
				/*!
				* @~English
				* @brief Constant that indicates int3/InputInt3 menu item
				* @~Japanese
				* @brief int4/InputInt4 のメニュー項目を表す定数
				*/
				TYPE_INT4 = TYPE_INT32_V4,

				/*!
				* @~English
				* @brief Constant that indicates SliderInt menu item
				* @~Japanese
				* @brief SliderInt のメニュー項目を表す定数
				*/
				TYPE_INT_SLIDER	= TYPE_INT32_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderInt2 menu item
				* @~Japanese
				* @brief SliderInt2 のメニュー項目を表す定数
				*/
				TYPE_INT2_SLIDER = TYPE_INT32_V2_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderInt3 menu item
				* @~Japanese
				* @brief SliderInt3 のメニュー項目を表す定数
				*/
				TYPE_INT3_SLIDER = TYPE_INT32_V3_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates SliderInt4 menu item
				* @~Japanese
				* @brief SliderInt4 のメニュー項目を表す定数
				*/
				TYPE_INT4_SLIDER = TYPE_INT32_V4_SLIDER,
				/*!
				* @~English
				* @brief Constant that indicates a menu item with child node
				* @~Japanese
				* @brief 子ノードを持つメニュー項目を表す定数
				*/
				TYPE_HAS_CHILD_ITEMS = 0x10000, 
	
				/*!
				 * @~English
				 * @brief Constant that indicates a container type menu item
				 * @~Japanese
				 * @brief コンテナ型のメニュー項目を表す定数
				 */
				TYPE_GROUP = TYPE_HAS_CHILD_ITEMS,
				/*!
				* @~English
				* @brief Constant that indicates Indent menu item
				* @~Japanese
				* @brief Indent のメニュー項目を表す定数
				*/
				TYPE_INDENT
			} Type;

			/*!
			 * @~English
			 * @brief Base class of each menu item class 
			 * @details Base class that holds shared methods for each menu item class 
			 * @~Japanese
			 * @brief 各メニュー項目クラスの基底クラス 
			 * @details 各メニュー項目クラスに共通するメソッドを持つ基底クラス 
			 */
			class AbstractMenuItem 
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~AbstractMenuItem(){}
				
				/*!
				 * @~English
				 * @brief Obtain the instance menu item type 
				 * @details This obtains the type of menu item for an instance. 
				 * @return Menu item type defined with sce::SampleUtil::Debug::Type 
				 * @~Japanese
				 * @brief インスタンスのメニュー項目の種類を取得 
				 * @details インスタンスがどの種類のメニュー項目かを取得します。 
				 * @return sce::SampleUtil::Debug::Typeで定義されるメニュー項目種別 
				 */
				virtual Type getType() const = 0;

				/*!
				 * @~English
				 * @brief Obtains the instance ID 
				 * @details This obtains the ID allocated at instance creation. 
				 * @return Instance ID 
				 * @~Japanese
				 * @brief インスタンスのIDを取得 
				 * @details インスタンス生成時に割り当てたIDを取得します。 
				 * @return インスタンスのID 
				 */
				virtual const char* getId() const = 0;

				/*!
				 * @~English
				 * @brief Obtains the instance label
				 * @details This obtains the label. 
				 * @return Instance label
				 * @~Japanese
				 * @brief インスタンスのラベルを取得 
				 * @details インスタンスのラベルを取得します。 
				 * @return インスタンスのラベル
				 */
				virtual const char* getLabel() const = 0;

				/*!
				 * @~English
				 * @brief Checks if a menu item is visible/hidden 
				 * @details Checks if a menu item is visible/hidden. 
				 * @retval true Visible
				 * @retval false Hidden
				 * @~Japanese
				 * @brief メニュー項目の可視/不可視を確認 
				 * @details メニュー項目の可視/不可視を確認します。 
				 * @retval true 可視
				 * @retval false 不可視
				 */
				virtual bool isVisible() const = 0;

				/*!
				 * @~English
				 * @brief Sets a menu item to visible/hidden 
				 * @details Sets if a menu item is visible/hidden. 
				 * @param visible visible (true)/hidden (false)
				 * @~Japanese
				 * @brief メニュー項目の可視/不可視を設定 
				 * @details メニュー項目の可視/不可視を設定します。 
				 * @param visible 可視(true)/不可視(false)
				 */
				virtual void setVisible(bool visible) = 0;

				/*!
				 * @~English
				 * @brief Checks if operation for a menu item is active or inactive 
				 * @details Checks if button operation for a menu item is active or inactive. If button operation is active, the value can be increased/decreased with the left/right buttons. The value increased/decreased with this operation will also be reflected in bound variables. 
				 * @retval true Operation for the menu is active
				 * @retval false Operation for the menu is inactive
				 * @~Japanese
				 * @brief メニュー項目への操作が有効か無効かを確認 
				 * @details メニュー項目へのボタン操作が有効か無効かを確認します。ボタン操作が有効な場合、左右ボタンで値の増減が出来ます。この操作で増減した値はバインドされた変数にも反映されます。 
				 * @retval true メニューへの操作は有効
				 * @retval false メニューへの操作は無効
				 */
				virtual bool isActive() const = 0;

				/*!
				 * @~English
				 * @brief Sets operation for a menu item to active/inactive 
				 * @details Switches operation for a menu item to active/inactive 
				 * @param active Active (true)/inactive (false)
				 * @~Japanese
				 * @brief メニュー項目への操作の有効化/無効化 
				 * @details メニュー項目への操作の有効化/無効化を切り替えます。 
				 * @param active 有効化(true)/無効化(false)
				 */
				virtual void setActive(bool active) = 0;

				/*!
				 * @~English
				 * @brief Generate character string from menu item 
				 * @details This generates a character string that reflects the menu item content. 
				 * @return Generated character string 
				 * @~Japanese
				 * @brief メニュー項目から文字列を生成 
				 * @details メニュー項目の内容を反映した文字列を生成します。 
				 * @return 生成された文字列 
				 */
				virtual const char* toString() const = 0;

				/*!
				 * @~English
				 * @brief Menu update 
				 * @details Updates the menu status. 
				 * @~Japanese
				 * @brief Menuの更新 
				 * @details Menuの状態を更新します。 
				 */
				virtual void update() = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage signed 8-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) signed 8-bit integer type values. 
			 * @~Japanese
			 * @brief 符号付8bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号付8bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Int8MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~Int8MenuItem(){}

				/*!
				 * @~English
				 * @brief Get signed 8-bit integer type value that is currently set 
				 * @details This obtains the signed 8-bit integer type value that is currently set. 
				 * @return Current signed 8-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号付8bit整数型の値を取得 
				 * @details 現在設定されている符号付8bit整数型の値を取得します。 
				 * @return 現在の符号付8bit整数型の値 
				 */
				virtual int8_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set signed 8-bit integer type value 
				 * @details This sets the signed 8-bit integer type value. 
				 * @param value Signed 8-bit integer type value
				 * @~Japanese
				 * @brief 符号付8bit整数型の値を設定 
				 * @details 符号付8bit整数型の値を設定します。 
				 * @param value 符号付8bit整数型の値
				 */
				virtual void setValue(int8_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 8-bit integer type variable to menu item 
				 * @details This binds a signed 8-bit integer type variable to a menu item. 
				 * @param refValue Signed 8-bit integer type pointer
				 * @~Japanese
				 * @brief 符号付8bit整数型の変数をメニュー項目にバインド 
				 * @details 符号付8bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号付8bit整数型ポインタ
				 */
				virtual void setReferenceValue(int8_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage signed 16-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) signed 16-bit integer type values. 
			 * @~Japanese
			 * @brief 符号付16bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号付16bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Int16MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Int16MenuItem(){}

				/*!
				 * @~English
				 * @brief Get signed 16-bit integer type value that is currently set 
				 * @details This obtains the signed 16-bit integer type value that is currently set. 
				 * @return Current signed 16-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号付16bit整数型の値を取得 
				 * @details 現在設定されている符号付16bit整数型の値を取得します。 
				 * @return 現在の符号付16bit整数型の値 
				 */
				virtual int16_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set signed 16-bit integer type value 
				 * @details This sets the signed 16-bit integer type value. 
				 * @param value Signed 16-bit integer type value
				 * @~Japanese
				 * @brief 符号付16bit整数型の値を設定 
				 * @details 符号付16bit整数型の値を設定します。 
				 * @param value 符号付16bit整数型の値
				 */
				virtual void setValue(int16_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 16-bit integer type variable to menu item 
				 * @details This binds a signed 16-bit integer type variable to a menu item. 
				 * @param refValue Signed 16-bit integer type pointer
				 * @~Japanese
				 * @brief 符号付16bit整数型の変数をメニュー項目にバインド 
				 * @details 符号付16bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号付16bit整数型ポインタ
				 */
				virtual void setReferenceValue(int16_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage signed 32-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) signed 32-bit integer type values. 
			 * @~Japanese
			 * @brief 符号付32bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号付32bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Int32MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Int32MenuItem(){}

				/*!
				 * @~English
				 * @brief Get signed 32-bit integer type value that is currently set 
				 * @details This obtains the signed 32-bit integer type value that is currently set. 
				 * @return Current signed 32-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号付32bit整数型の値を取得 
				 * @details 現在設定されている符号付32bit整数型の値を取得します。 
				 * @return 現在の符号付32bit整数型の値 
				 */
				virtual int32_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set signed 32-bit integer type value 
				 * @details This sets the signed 32-bit integer type value. 
				 * @param value Signed 32-bit integer type value
				 * @~Japanese
				 * @brief 符号付32bit整数型の値を設定 
				 * @details 符号付32bit整数型の値を設定します。 
				 * @param value 符号付32bit整数型の値
				 */
				virtual void setValue(int32_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 32-bit integer type variable to menu item 
				 * @details This binds a signed 32-bit integer type variable to a menu item. 
				 * @param refValue Signed 32-bit integer type pointer
				 * @~Japanese
				 * @brief 符号付32bit整数型の変数をメニュー項目にバインド 
				 * @details 符号付32bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号付32bit整数型ポインタ
				 */
				virtual void setReferenceValue(int32_t *refValue) = 0;
			};

            typedef Int32MenuItem IntMenuItem;

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage array(which has 2 values) of signed 32-bit integer type values
			 * @details This is the debug menu item class that can manage (set/obtain/bind) array(which has 2 values) of signed 32-bit integer type values.
			 * @~Japanese
			 * @brief 要素数2の符号付32bit整数型の配列を管理できる、デバッグメニューの項目クラス
			 * @details 要素数2の符号付32bit整数型の配列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class Int32v2MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~Int32v2MenuItem() {}
				/*!
				 * @~English
				 * @brief Get current signed 32-bit integer type array that is set
				 * @details This obtains the current signed 32-bit integer type array that is set.
				 * @param value Pointer to signed 32-bit integer type array
				 * @~Japanese
				 * @brief 設定されている現在の符号付32bit整数型の配列を取得
				 * @details 設定されている現在の符号付32bit整数型の配列を取得します。
				 * @param value 要素数2の符号付32bit整数型の配列へのポインタ
				 */
				virtual void getValue(int32_t* value) const = 0;

				/*!
				 * @~English
				 * @brief Set single precision signed 32-bit integer type array
				 * @details This sets the signed 32-bit integer type array.
				 * @param value Pointer to signed 32-bit integer type array
				 * @~Japanese
				 * @brief 符号付32bit整数型の配列を設定
				 * @details 符号付32bit整数型の配列を設定します。
				 * @param value 符号付32bit整数型の配列へのポインタ
				 */
				virtual void setValue(int32_t *value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 32-bit integer type value array to menu item
				 * @details This binds a signed 32-bit integer type value array to a menu item.
				 * @param refValue signed 32-bit integer type value array pointer
				 * @~Japanese
				 * @brief 符号付32bit整数型の配列をメニュー項目にバインド
				 * @details 符号付32bit整数型の配列をメニュー項目にバインドします。
				 * @param refValue 符号付32bit整数型の配列へのポインタ
				 */
				virtual void setReferenceValue(int32_t *refValue) = 0;
			};


			typedef Int32v2MenuItem Int2MenuItem;

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage array(which has 3 values) of signed 32-bit integer type values
			 * @details This is the debug menu item class that can manage (set/obtain/bind) array(which has 3 values) of signed 32-bit integer type values.
			 * @~Japanese
			 * @brief 要素数3の符号付32bit整数型の配列を管理できる、デバッグメニューの項目クラス
			 * @details 要素数3の符号付32bit整数型の配列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class Int32v3MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~Int32v3MenuItem() {}
				/*!
				 * @~English
				 * @brief Get current signed 32-bit integer type array that is set
				 * @details This obtains the current signed 32-bit integer type array that is set.
				 * @param value Pointer to signed 32-bit integer type array
				 * @~Japanese
				 * @brief 設定されている現在の符号付32bit整数型の配列を取得
				 * @details 設定されている現在の符号付32bit整数型の配列を取得します。
				 * @param value 要素数3の符号付32bit整数型の配列へのポインタ
				 */
				virtual void getValue(int32_t* value) const = 0;

				/*!
				 * @~English
				 * @brief Set single precision signed 32-bit integer type array
				 * @details This sets the signed 32-bit integer type array.
				 * @param value Pointer to signed 32-bit integer type array
				 * @~Japanese
				 * @brief 符号付32bit整数型の配列を設定
				 * @details 符号付32bit整数型の配列を設定します。
				 * @param value 符号付32bit整数型の配列へのポインタ
				 */
				virtual void setValue(int32_t *value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 32-bit integer type value array to menu item
				 * @details This binds a signed 32-bit integer type value array to a menu item.
				 * @param refValue signed 32-bit integer type value array pointer
				 * @~Japanese
				 * @brief 符号付32bit整数型の配列をメニュー項目にバインド
				 * @details 符号付32bit整数型の配列をメニュー項目にバインドします。
				 * @param refValue 符号付32bit整数型の配列へのポインタ
				 */
				virtual void setReferenceValue(int32_t *refValue) = 0;
			};


			typedef Int32v3MenuItem Int3MenuItem;

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage array(which has 4 values) of signed 32-bit integer type values
			 * @details This is the debug menu item class that can manage (set/obtain/bind) array(which has 4 values) of signed 32-bit integer type values.
			 * @~Japanese
			 * @brief 要素数4の符号付32bit整数型の配列を管理できる、デバッグメニューの項目クラス
			 * @details 要素数4の符号付32bit整数型の配列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class Int32v4MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~Int32v4MenuItem() {}
				/*!
				 * @~English
				 * @brief Get current signed 32-bit integer type array that is set
				 * @details This obtains the current signed 32-bit integer type array that is set.
				 * @param value Pointer to signed 32-bit integer type array
				 * @~Japanese
				 * @brief 設定されている現在の符号付32bit整数型の配列を取得
				 * @details 設定されている現在の符号付32bit整数型の配列を取得します。
				 * @param value 要素数4の符号付32bit整数型の配列へのポインタ
				 */
				virtual void getValue(int32_t* value) const = 0;

				/*!
				 * @~English
				 * @brief Set single precision signed 32-bit integer type array
				 * @details This sets the signed 32-bit integer type array.
				 * @param value Pointer to signed 32-bit integer type array
				 * @~Japanese
				 * @brief 符号付32bit整数型の配列を設定
				 * @details 符号付32bit整数型の配列を設定します。
				 * @param value 符号付32bit整数型の配列へのポインタ
				 */
				virtual void setValue(int32_t *value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 32-bit integer type value array to menu item
				 * @details This binds a signed 32-bit integer type value array to a menu item.
				 * @param refValue signed 32-bit integer type value array pointer
				 * @~Japanese
				 * @brief 符号付32bit整数型の配列をメニュー項目にバインド
				 * @details 符号付32bit整数型の配列をメニュー項目にバインドします。
				 * @param refValue 符号付32bit整数型の配列へのポインタ
				 */
				virtual void setReferenceValue(int32_t *refValue) = 0;
			};


			typedef Int32v4MenuItem Int4MenuItem;


			/*!
			 * @~English
			 * @brief Debug menu item class that can manage signed 64-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) signed 64-bit integer type values. 
			 * @~Japanese
			 * @brief 符号付64bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号付64bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Int64MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Int64MenuItem(){}

				/*!
				 * @~English
				 * @brief Get signed 64-bit integer type value that is currently set 
				 * @details This obtains the signed 64-bit integer type value that is currently set. 
				 * @return Current signed 64-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号付64bit整数型の値を取得 
				 * @details 現在設定されている符号付64bit整数型の値を取得します。 
				 * @return 現在の符号付64bit整数型の値 
				 */
				virtual int64_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set signed 64-bit integer type value 
				 * @details This sets the signed 64-bit integer type value. 
				 * @param value Signed 64-bit integer type value
				 * @~Japanese
				 * @brief 符号付64bit整数型の値を設定 
				 * @details 符号付64bit整数型の値を設定します。 
				 * @param value 符号付64bit整数型の値
				 */
				virtual void setValue(int64_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind signed 64-bit integer type variable to menu item 
				 * @details This binds a signed 64-bit integer type variable to a menu item. 
				 * @param refValue Signed 64-bit integer type pointer
				 * @~Japanese
				 * @brief 符号付64bit整数型の変数をメニュー項目にバインド 
				 * @details 符号付64bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号付64bit整数型ポインタ
				 */
				virtual void setReferenceValue(int64_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage unsigned 8-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) unsigned 8-bit integer type values. 
			 * @~Japanese
			 * @brief 符号なし8bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号なし8bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Uint8MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Uint8MenuItem(){}

				/*!
				 * @~English
				 * @brief Get unsigned 8-bit integer type value that is currently set 
				 * @details This obtains the unsigned 8-bit integer type value that is currently set. 
				 * @return Current unsigned 8-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号なし8bit整数型の値を取得 
				 * @details 現在設定されている符号なし8bit整数型の値を取得します。 
				 * @return 現在の符号なし8bit整数型の値 
				 */
				virtual uint8_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set unsigned 8-bit integer type value 
				 * @details This sets the unsigned 8-bit integer type value. 
				 * @param value Unsigned 8-bit integer type value
				 * @~Japanese
				 * @brief 符号なし8bit整数型の値を設定 
				 * @details 符号なし8bit整数型の値を設定します。 
				 * @param value 符号なし8bit整数型の値
				 */
				virtual void setValue(uint8_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind unsigned 8-bit integer type variable to menu item 
				 * @details This binds an unsigned 8-bit integer type variable to a menu item. 
				 * @param refValue Unsigned 8-bit integer type pointer
				 * @~Japanese
				 * @brief 符号なし8bit整数型の変数をメニュー項目にバインド 
				 * @details 符号なし8bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号なし8bit整数型ポインタ
				 */
				virtual void setReferenceValue(uint8_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage unsigned 16-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) unsigned 16-bit integer type values. 
			 * @~Japanese
			 * @brief 符号なし16bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号なし16bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Uint16MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Uint16MenuItem(){}

				/*!
				 * @~English
				 * @brief Get unsigned 16-bit integer type value that is currently set 
				 * @details This obtains the unsigned 16-bit integer type value that is currently set. 
				 * @return Current unsigned 16-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号なし16bit整数型の値を取得 
				 * @details 現在設定されている符号なし16bit整数型の値を取得します。 
				 * @return 現在の符号なし16bit整数型の値 
				 */
				virtual uint16_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set unsigned 16-bit integer type value 
				 * @details This sets the unsigned 16-bit integer type value. 
				 * @param value Unsigned 16-bit integer type value
				 * @~Japanese
				 * @brief 符号なし16bit整数型の値を設定 
				 * @details 符号なし16bit整数型の値を設定します。 
				 * @param value 符号なし16bit整数型の値
				 */
				virtual void setValue(uint16_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind unsigned 16-bit integer type variable to menu item 
				 * @details This binds an unsigned 16-bit integer type variable to a menu item. 
				 * @param refValue Unsigned 16-bit integer type pointer
				 * @~Japanese
				 * @brief 符号なし16bit整数型の変数をメニュー項目にバインド 
				 * @details 符号なし16bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号なし16bit整数型ポインタ
				 */
				virtual void setReferenceValue(uint16_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage unsigned 32-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) unsigned 32-bit integer type values. 
			 * @~Japanese
			 * @brief 符号なし32bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号なし32bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Uint32MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Uint32MenuItem(){}

				/*!
				 * @~English
				 * @brief Get unsigned 32-bit integer type value that is currently set 
				 * @details This obtains the unsigned 32-bit integer type value that is currently set. 
				 * @return Current unsigned 32-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号なし32bit整数型の値を取得 
				 * @details 現在設定されている符号なし32bit整数型の値を取得します。 
				 * @return 現在の符号なし32bit整数型の値 
				 */
				virtual uint32_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set unsigned 32-bit integer type value 
				 * @details This sets the unsigned 32-bit integer type value. 
				 * @param value Unsigned 32-bit integer type value
				 * @~Japanese
				 * @brief 符号なし32bit整数型の値を設定 
				 * @details 符号なし32bit整数型の値を設定します。 
				 * @param value 符号なし32bit整数型の値
				 */
				virtual void setValue(uint32_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind unsigned 32-bit integer type variable to menu item 
				 * @details This binds an unsigned 32-bit integer type variable to a menu item. 
				 * @param refValue Unsigned 32-bit integer type pointer
				 * @~Japanese
				 * @brief 符号なし32bit整数型の変数をメニュー項目にバインド 
				 * @details 符号なし32bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号なし32bit整数型ポインタ
				 */
				virtual void setReferenceValue(uint32_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage unsigned 64-bit integer type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) unsigned 64-bit integer type values. 
			 * @~Japanese
			 * @brief 符号なし64bit整数型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 符号なし64bit整数型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class Uint64MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Uint64MenuItem(){}

				/*!
				 * @~English
				 * @brief Get unsigned 64-bit integer type value that is currently set 
				 * @details This obtains the unsigned 64-bit integer type value that is currently set. 
				 * @return Current unsigned 64-bit integer type value 
				 * @~Japanese
				 * @brief 現在設定されている符号なし64bit整数型の値を取得 
				 * @details 現在設定されている符号なし64bit整数型の値を取得します。 
				 * @return 現在の符号なし64bit整数型の値 
				 */
				virtual uint64_t getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set unsigned 64-bit integer type value 
				 * @details This sets the unsigned 64-bit integer type value. 
				 * @param value Unsigned 64-bit integer type value
				 * @~Japanese
				 * @brief 符号なし64bit整数型の値を設定 
				 * @details 符号なし64bit整数型の値を設定します。 
				 * @param value 符号なし64bit整数型の値
				 */
				virtual void setValue(uint64_t value) = 0;

				/*!
				 * @~English
				 * @brief Bind unsigned 64-bit integer type variable to menu item 
				 * @details This binds an unsigned 64-bit integer type variable to a menu item. 
				 * @param refValue Unsigned 64-bit integer type pointer
				 * @~Japanese
				 * @brief 符号なし64bit整数型の変数をメニュー項目にバインド 
				 * @details 符号なし64bit整数型の変数をメニュー項目にバインドします。 
				 * @param refValue 符号なし64bit整数型ポインタ
				 */
				virtual void setReferenceValue(uint64_t *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage single precision floating-point type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) single precision floating-point type values. 
			 * @~Japanese
			 * @brief 単精度浮動小数点型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 単精度浮動小数点型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class FloatMenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~FloatMenuItem(){}
				/*!
				 * @~English
				 * @brief Get current single precision floating-point type value that is set 
				 * @details This obtains the current single precision floating-point type value that is set. 
				 * @return Current single precision floating-point value 
				 * @~Japanese
				 * @brief 設定されている現在の単精度浮動小数点型の値を取得 
				 * @details 設定されている現在の単精度浮動小数点型の値を取得します。 
				 * @return 現在の単精度浮動小数点型のの値 
				 */
				virtual float getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set single precision floating-point type value 
				 * @details This sets the single precision floating-point type value. 
				 * @param value Single precision floating-point type value
				 * @~Japanese
				 * @brief 単精度浮動小数点型の値を設定 
				 * @details 単精度浮動小数点型の値を設定します。 
				 * @param value 単精度浮動小数点型の値
				 */
				virtual void setValue(float value) = 0;

				/*!
				 * @~English
				 * @brief Bind single precision floating-point type value variable to menu item 
				 * @details This binds a single precision floating-point type value variable to a menu item. 
				 * @param refValue Single precision floating-point type value pointer
				 * @~Japanese
				 * @brief 単精度浮動小数点型の変数をメニュー項目にバインド 
				 * @details 単精度浮動小数点型の変数をメニュー項目にバインドします。 
				 * @param refValue 単精度浮動小数点型のポインタ
				 */
				virtual void setReferenceValue(float *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage array(which has 2 values) of single precision floating-point type values
			 * @details This is the debug menu item class that can manage (set/obtain/bind) array(which has 2 values) of single precision floating-point type values.
			 * @~Japanese
			 * @brief 要素数2の単精度浮動小数点型の配列を管理できる、デバッグメニューの項目クラス
			 * @details 要素数2の単精度浮動小数点型の配列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class Float2MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~Float2MenuItem() {}
				/*!
				 * @~English
				 * @brief Get current single precision floating-point type array that is set
				 * @details This obtains the current single precision floating-point type array that is set.
				 * @param value Pointer to single precision floating-point type array
				 * @~Japanese
				 * @brief 設定されている現在の単精度浮動小数点型の配列を取得
				 * @details 設定されている現在の単精度浮動小数点型の配列を取得します。
				 * @param value 要素数2の単精度浮動小数点型の配列へのポインタ
				 */
				virtual void getValue(float* value) const = 0;

				/*!
				 * @~English
				 * @brief Set single precision single precision floating-point type array
				 * @details This sets the single precision floating-point type array.
				 * @param value Pointer to single precision floating-point type array
				 * @~Japanese
				 * @brief 単精度浮動小数点型の配列を設定
				 * @details 単精度浮動小数点型の配列を設定します。
				 * @param value 単精度浮動小数点型の配列へのポインタ
				 */
				virtual void setValue(float *value) = 0;

				/*!
				 * @~English
				 * @brief Bind single precision floating-point type value array to menu item
				 * @details This binds a single precision floating-point type value array to a menu item.
				 * @param refValue single precision floating-point type value array pointer
				 * @~Japanese
				 * @brief 単精度浮動小数点型の配列をメニュー項目にバインド
				 * @details 単精度浮動小数点型の配列をメニュー項目にバインドします。
				 * @param refValue 単精度浮動小数点型の配列へのポインタ
				 */
				virtual void setReferenceValue(float *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage array(which has 3 values) of single precision floating-point type values
			 * @details This is the debug menu item class that can manage (set/obtain/bind) array(which has 3 values) of single precision floating-point type values.
			 * @~Japanese
			 * @brief 要素数3の単精度浮動小数点型の配列を管理できる、デバッグメニューの項目クラス
			 * @details 要素数3の単精度浮動小数点型の配列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class Float3MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~Float3MenuItem() {}
				/*!
				 * @~English
				 * @brief Get current single precision floating-point type array that is set
				 * @details This obtains the current single precision floating-point type array that is set.
				 * @param value Pointer to single precision floating-point type array
				 * @~Japanese
				 * @brief 設定されている現在の単精度浮動小数点型の配列を取得
				 * @details 設定されている現在の単精度浮動小数点型の配列を取得します。
				 * @param value 要素数3の単精度浮動小数点型の配列へのポインタ
				 */
				virtual void getValue(float* value) const = 0;

				/*!
				 * @~English
				 * @brief Set single precision single precision floating-point type array
				 * @details This sets the single precision floating-point type array.
				 * @param value Pointer to single precision floating-point type array
				 * @~Japanese
				 * @brief 単精度浮動小数点型の配列を設定
				 * @details 単精度浮動小数点型の配列を設定します。
				 * @param value 単精度浮動小数点型の配列へのポインタ
				 */
				virtual void setValue(float *value) = 0;

				/*!
				 * @~English
				 * @brief Bind single precision floating-point type value array to menu item
				 * @details This binds a single precision floating-point type value array to a menu item.
				 * @param refValue single precision floating-point type value array pointer
				 * @~Japanese
				 * @brief 単精度浮動小数点型の配列をメニュー項目にバインド
				 * @details 単精度浮動小数点型の配列をメニュー項目にバインドします。
				 * @param refValue 単精度浮動小数点型の配列へのポインタ
				 */
				virtual void setReferenceValue(float *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage array(which has 4 values) of single precision floating-point type values
			 * @details This is the debug menu item class that can manage (set/obtain/bind) array(which has 4 values) of single precision floating-point type values.
			 * @~Japanese
			 * @brief 要素数4の単精度浮動小数点型の配列を管理できる、デバッグメニューの項目クラス
			 * @details 要素数4の単精度浮動小数点型の配列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class Float4MenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~Float4MenuItem() {}
				/*!
				 * @~English
				 * @brief Get current single precision floating-point type array that is set
				 * @details This obtains the current single precision floating-point type array that is set.
				 * @param value Pointer to single precision floating-point type array
				 * @~Japanese
				 * @brief 設定されている現在の単精度浮動小数点型の配列を取得
				 * @details 設定されている現在の単精度浮動小数点型の配列を取得します。
				 * @param value 要素数4の単精度浮動小数点型の配列へのポインタ
				 */
				virtual void getValue(float* value) const = 0;

				/*!
				 * @~English
				 * @brief Set single precision single precision floating-point type array
				 * @details This sets the single precision floating-point type array.
				 * @param value Pointer to single precision floating-point type array
				 * @~Japanese
				 * @brief 単精度浮動小数点型の配列を設定
				 * @details 単精度浮動小数点型の配列を設定します。
				 * @param value 単精度浮動小数点型の配列へのポインタ
				 */
				virtual void setValue(float *value) = 0;

				/*!
				 * @~English
				 * @brief Bind single precision floating-point type value array to menu item
				 * @details This binds a single precision floating-point type value array to a menu item.
				 * @param refValue single precision floating-point type value array pointer
				 * @~Japanese
				 * @brief 単精度浮動小数点型の配列をメニュー項目にバインド
				 * @details 単精度浮動小数点型の配列をメニュー項目にバインドします。
				 * @param refValue 単精度浮動小数点型の配列へのポインタ
				 */
				virtual void setReferenceValue(float *refValue) = 0;
			};


			/*!
			 * @~English
			 * @brief Debug menu item class that can manage double precision floating-point type values 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) double precision floating-point type values. 
			 * @~Japanese
			 * @brief 倍精度浮動小数点型の値を管理できる、デバッグメニューの項目クラス 
			 * @details 倍精度浮動小数点型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class DoubleMenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~DoubleMenuItem(){}

				/*!
				 * @~English
				 * @brief Get current double precision floating-point type value that is set 
				 * @details This obtains the current double precision floating-point type value that is set. 
				 * @return Current double precision floating-point value 
				 * @~Japanese
				 * @brief 設定されている現在の倍精度浮動小数点型の値を取得 
				 * @details 設定されている現在の倍精度浮動小数点型の値を取得します。 
				 * @return 現在の倍精度浮動小数点型の値 
				 */
				virtual double getValue() const = 0;

				/*!
				 * @~English
				 * @brief Set double precision floating-point type value 
				 * @details This sets the double precision floating-point type value. 
				 * @param value Double precision floating-point type value
				 * @~Japanese
				 * @brief 倍精度浮動小数点型の値を設定 
				 * @details 倍精度浮動小数点型の値を設定します。 
				 * @param value 倍精度浮動小数点型の値
				 */
				virtual void setValue(double value) = 0;

				/*!
				 * @~English
				 * @brief Bind double precision floating-point type value variable to menu item 
				 * @details This binds a double precision floating-point type value variable to a menu item. 
				 * @param refValue Double precision floating-point type value pointer
				 * @~Japanese
				 * @brief 倍精度浮動小数点型の変数をメニュー項目にバインド 
				 * @details 倍精度浮動小数点型の変数をメニュー項目にバインドします。 
				 * @param refValue 倍精度浮動小数点型のポインタ
				 */
				virtual void setReferenceValue(double *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage Bool type values 
			 * @details This debug menu item class that can manage (set/obtain/bind) Bool type values. 
			 * @~Japanese
			 * @brief Bool型の値を管理できる、デバッグメニューの項目クラス 
			 * @details Bool型の値を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class BoolMenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~BoolMenuItem(){}
				/*!
				 * @~English
				 * @brief Obtains the current Bool type value that is set 
				 * @details This obtains the current Bool type value that is set. 
				 * @return Current Bool type value 
				 * @~Japanese
				 * @brief 設定されている現在のBool型の値を取得 
				 * @details 設定されている現在のBool型の値を取得します。 
				 * @return 現在のBool型の値 
				 */
				virtual bool getValue() const = 0;
				/*!
				 * @~English
				 * @brief Sets Bool type value 
				 * @details Sets the Bool type value. 
				 * @param value Bool type value
				 * @~Japanese
				 * @brief Bool型の値を設定 
				 * @details Bool型の値を設定します。 
				 * @param value Bool型の値
				 */
				virtual void setValue(bool value) = 0;
				/*!
				 * @~English
				 * @brief Binds Bool type variable to menu item 
				 * @details This binds a Bool type variable to a menu item. 
				 * @param refValue Bool type pointer
				 * @~Japanese
				 * @brief Bool型の変数をメニュー項目にバインド 
				 * @details Bool型の変数を、メニュー項目にバインドします。 
				 * @param refValue Bool型ポインタ
				 */
				virtual void setReferenceValue(bool *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage indexed character string set types 
			 * @details This is the debug menu item class that can manage (set/obtain/bind) indexed character string set types 
			 * @~Japanese
			 * @brief インデックス付文字列セット型を管理できる、デバッグメニューの項目クラス 
			 * @details インデックス付文字列セット型を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。 
			 */
			class EnumMenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~EnumMenuItem(){}
				/*!
				 * @~English
				 * @brief Obtains the current Enum index value that is set 
				 * @details This obtains the current Enum index value that is set. 
				 * @return Current Enum index value 
				 * @~Japanese
				 * @brief 設定されている現在のEnumのインデックス値を取得 
				 * @details 設定されている現在のEnumのインデックス値を取得します。 
				 * @return 現在のEnumのインデックス値 
				 */
				virtual int getValue() const = 0;
				/*!
				 * @~English
				 * @brief Sets Enum index value 
				 * @details Sets the Enum index value. 
				 * @param value Enum index value
				 * @~Japanese
				 * @brief Enumのインデックス値を設定 
				 * @details Enumのインデックス値を設定します。 
				 * @param value Enumのインデックス値
				 */
				virtual void setValue(int value) = 0;
				/*!
				 * @~English
				 * @brief Binds int type variable to Enum index value held by menu item 
				 * @details Binds an int type variable to an Enum index value held by a menu item. 
				 * @param refValue int type pointer
				 * @~Japanese
				 * @brief int型の変数をメニュー項目が持つEnumのインデックス値にバインド 
				 * @details int型の変数をメニュー項目が持つEnumのインデックス値にバインドします。 
				 * @param refValue int型ポインタ
				 */
				virtual void setReferenceValue(int *refValue) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can manage string type
			 * @details This is the debug menu item class that can manage (set/obtain/bind) string type
			 * @~Japanese
			 * @brief 文字列を管理できる、デバッグメニューの項目クラス
			 * @details 文字列を管理(設定/取得/バインド)できる、デバッグメニューの項目クラスです。
			 */
			class StringMenuItem : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor
				 *
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 *
				 * @details デストラクタです。
				 */
				virtual ~StringMenuItem() {}
				/*!
				 * @~English
				 * @brief Obtains the current string value that is set
				 * @details This obtains the current string value that is set.
				 * @return Current string value
				 * @~Japanese
				 * @brief 設定されている現在の文字列を取得
				 * @details 設定されている現在の文字列を取得します。
				 * @return 現在の文字列
				 */
				virtual const char* getValue() const = 0;
				/*!
				 * @~English
				 * @brief Sets string value
				 * @details Sets the string value.
				 * @param buf string value
				 * @~Japanese
				 * @brief 文字列を設定
				 * @details 文字列を設定します。
				 * @param buf 文字列
				 */
				virtual void setValue(const char* buf) = 0;
				/*!
				 * @~English
				 * @brief Binds string variable to string variable held by menu item
				 * @details Binds an string variable to string variable held by a menu item.
				 * @param refValue Pointer for string buffer
				 * @param refValueSize Max size of string buffer
				 * @~Japanese
				 * @brief 文字列バッファ変数をメニュー項目が持つ文字列バッファにバインド
				 * @details 文字列バッファ変数をメニュー項目が持つ文字列バッファにバインドします。
				 * @param refValue 文字列バッファポインタ
				 * @param refValueSize 文字列バッファの最大サイズ
				 */
				virtual void setReferenceValue(char *refValue, size_t refValueSize) = 0;
			};

			/*!
			 * @~English
			 * @brief Debug menu item class that can group multiple AbstractMenuItems 
			 * @details This debug menu item class can group multiple AbstractMenuItems. 
			 * @~Japanese
			 * @brief 複数のAbstractMenuItemをグルーピングできるデバッグメニューの項目クラス 
			 * @details 複数のAbstractMenuItemをグルーピングできるデバッグメニューの項目クラスです。 
			 */
			class MenuItemGroup : public AbstractMenuItem
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~MenuItemGroup(){}
				/*!
				 * @~English
				 * @brief Obtains pointer from id to AbstractMenuItem 
				 * @details Searches for a pointer to AbstractMenuItem linked to id and obtains it. 
				 * @param id Menu item id
				 * @retval AbstractMenuItem* When AbstractMenuItem linked to id is found, a pointer to AbstractMenuItem will be returned
				 * @retval NULL When AbstractMenuItem linked to id could not be found
				 * @~Japanese
				 * @brief idからAbstractMenuItemへのポインタを取得 
				 * @details idに紐づいた、AbstractMenuItemへのポインタを探索し、取得します。 
				 * @param id メニュー項目のid
				 * @retval AbstractMenuItem* idに紐づくAbstractMenuItemが見つかったとき、AbstractMenuItemへのポインタを返します
				 * @retval NULL idに紐づくAbstractMenuItemが見つからなかったとき
				 */
				virtual AbstractMenuItem* getItemById(const char* id) const = 0;
				/*!
				 * @~English
				 * @brief Obtains pointer from index to AbstractMenuItem 
				 * @details Searches for a pointer to index # AbstractMenuItem and obtains it. 
				 * @param index Menu item index
				 * @retval AbstractMenuItem* When index # AbstractMenuItem is found, a pointer to AbstractMenuItem will be returned
				 * @retval NULL When index # AbstractMenuItem could not be found
				 * @~Japanese
				 * @brief indexからAbstractMenuItemへのポインタを取得 
				 * @details index番目の、AbstractMenuItemへのポインタを探索し、取得します。 
				 * @param index メニュー項目のindex
				 * @retval AbstractMenuItem* index番目のAbstractMenuItemが見つかったとき、AbstractMenuItemへのポインタを返します
				 * @retval NULL index番目のAbstractMenuItemが見つかったとき
				 */
				virtual AbstractMenuItem* getItemByIndex(uint32_t index) const = 0;
				/*!
				 * @~English
				 * @brief Obtains the number of menu items belonging to this group 
				 * @details This obtains the number of menu items belonging to this group. 
				 * @return Number of menu items 
				 * @~Japanese
				 * @brief このグループに所属するメニュー項目の個数を取得 
				 * @details このグループに所属するメニュー項目の個数を取得します。 
				 * @return メニュー項目の個数 
				 */
				virtual uint32_t getNumItems() const = 0;

				/*!
				 * @~English
				 * @brief Deletes menu item from id 
				 * @details Searches for a menu item linked to id and deletes it if found 
				 * @param id Menu item id
				 * @~Japanese
				 * @brief idからメニュー項目を削除 
				 * @details idに紐づいた、メニュー項目を探索し、見つかれば削除します。 
				 * @param id メニュー項目のid
				 */
				virtual void deleteItemById(const char* id) = 0;
				/*!
				 * @~English
				 * @brief Deletes menu item from index 
				 * @details Searches for a menu item linked to the index # and deletes it if found 
				 * @param index Menu item index
				 * @retval SCE_OK Deletion successful
				 * @retval SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY When index # menu item could not be found
				 * @~Japanese
				 * @brief indexからメニュー項目を削除 
				 * @details index番目のメニュー項目を探索し、見つかれば削除します。 
				 * @param index メニュー項目のindex
				 * @retval SCE_OK 削除成功
				 * @retval SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY Index番目のメニュー項目が見つからなかった場合
				 */
				virtual int  deleteItemByIndex(uint32_t index) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including signed 8-bit integer type value 
				 * @details This adds a menu item including a signed 8-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: SCHAR_MIN)
				 * @param maxValue Maximum value available (default: SCHAR_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int8MenuItem instance 
				 * @~Japanese
				 * @brief 符号付8ビット整数型の値を含むメニュー項目の追加 
				 * @details メニューに、符号付8ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：SCHAR_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：SCHAR_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt8MenuItemインスタンスへのポインタ 
				 */
				virtual Int8MenuItem* addInt8Menuitem(const char* id, 
											const char *label, 
											int8_t value=0,
											int8_t step=1,
											int8_t minValue=SCHAR_MIN, 
											int8_t maxValue=SCHAR_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including signed 16-bit integer type value 
				 * @details This adds a menu item including a signed 16-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: SHRT_MIN)
				 * @param maxValue Maximum value available (default: SHRT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int16MenuItem instance 
				 * @~Japanese
				 * @brief 符号付16ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号付16ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：SHRT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：SHRT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt16MenuItemインスタンスへのポインタ 
				 */
				virtual Int16MenuItem* addInt16Menuitem(const char* id, 
											const char *label, 
											int16_t value=0, 
											int16_t step=1,
											int16_t minValue=SHRT_MIN, 
											int16_t maxValue=SHRT_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including signed 32-bit integer type value 
				 * @details This adds a menu item including a signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32MenuItem instance 
				 * @~Japanese
				 * @brief 符号付32ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32MenuItemインスタンスへのポインタ 
				 */
				virtual Int32MenuItem* addInt32Menuitem(const char* id, 
											const char *label, 
											int32_t value=0, 
											int32_t step=1,
											int32_t minValue=INT_MIN, 
											int32_t maxValue=INT_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;
				/*!
				 * @~English
				 * @brief Add menu item including array(which has 2 values) of signed 32-bit integer type values
				 * @details This adds a menu item including array(which has 2 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32v2MenuItem instance
				 * @~Japanese
				 * @brief 要素数2の符号付32ビット整数型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数2の符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32v2MenuItemインスタンスへのポインタ
				 */
				virtual Int32v2MenuItem* addInt32v2Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including array(which has 3 values) of signed 32-bit integer type values
				 * @details This adds a menu item including array(which has 3 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32v3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の符号付32ビット整数型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数3の符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32v3MenuItemインスタンスへのポインタ
				 */
				virtual Int32v3MenuItem* addInt32v3Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;
				/*!
				 * @~English
				 * @brief Add menu item including array(which has 4 values) of signed 32-bit integer type values
				 * @details This adds a menu item including array(which has 4 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32v4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の符号付32ビット整数型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数4の符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32v4MenuItemインスタンスへのポインタ
				 */
				virtual Int32v4MenuItem* addInt32v4Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including signed 32-bit integer type values
				 * @details This adds a slider menu item including signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32MenuItem instance
				 * @~Japanese
				 * @brief 符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32MenuItemインスタンスへのポインタ
				 */
				virtual Int32MenuItem* addSliderInt32Menuitem(const char* id,
					const char *label,
					int32_t value = 0,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 2 values) of signed 32-bit integer type values
				 * @details This adds a slider menu item including array(which has 2 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32v2MenuItem instance
				 * @~Japanese
				 * @brief 要素数2の符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数2の符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32v2MenuItemインスタンスへのポインタ
				 */
				virtual Int32v2MenuItem* addSliderInt32v2Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 3 values) of signed 32-bit integer type values
				 * @details This adds a slider menu item including array(which has 3 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32v3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数3の符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32v3MenuItemインスタンスへのポインタ
				 */
				virtual Int32v3MenuItem* addSliderInt32v3Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 4 values) of signed 32-bit integer type values
				 * @details This adds a slider menu item including array(which has 4 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int32v4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数4の符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt32v4MenuItemインスタンスへのポインタ
				 */
				virtual Int32v4MenuItem* addSliderInt32v4Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including signed 32-bit integer type value 
				 * @details This adds a menu item including a signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added IntMenuItem instance 
				 * @~Japanese
				 * @brief 符号付32ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したIntMenuItemインスタンスへのポインタ 
				 */
                IntMenuItem* addIntMenuitem(const char* id,
											const char *label, 
											int32_t value=0, 
											int32_t step=1,
											int32_t minValue=INT_MIN, 
											int32_t maxValue=INT_MAX, 
											int position=-1,
											const char *format = nullptr)
                {
                     return addInt32Menuitem(id, label, value, step, minValue, maxValue, position, format);
                }
				/*!
				 * @~English
				 * @brief Add menu item including array(which has 2 values) of signed 32-bit integer type values
				 * @details This adds a menu item including array(which has 2 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int2MenuItem instance
				 * @~Japanese
				 * @brief 要素数2の符号付32ビット整数型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数2の符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt2MenuItemインスタンスへのポインタ
				 */
				Int2MenuItem* addInt2Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addInt32v2Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}

				/*!
				 * @~English
				 * @brief Add menu item including array(which has 3 values) of signed 32-bit integer type values
				 * @details This adds a menu item including array(which has 3 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の符号付32ビット整数型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数3の符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt3MenuItemインスタンスへのポインタ
				 */
				Int3MenuItem* addInt3Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addInt32v3Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}

				/*!
				 * @~English
				 * @brief Add menu item including array(which has 4 values) of signed 32-bit integer type values
				 * @details This adds a menu item including array(which has 4 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の符号付32ビット整数型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数4の符号付32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt4MenuItemインスタンスへのポインタ
				 */
				Int4MenuItem* addInt4Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addInt32v4Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}

				/*!
				 * @~English
				 * @brief Add slider menu item including signed 32-bit integer type values
				 * @details This adds a slider menu item including signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added IntMenuItem instance
				 * @~Japanese
				 * @brief 符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したIntMenuItemインスタンスへのポインタ
				 */
				IntMenuItem* addSliderIntMenuitem(const char* id,
					const char *label,
					int32_t value = 0,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addSliderInt32Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 2 values) of signed 32-bit integer type values
				 * @details This adds a slider menu item including array(which has 2 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int2MenuItem instance
				 * @~Japanese
				 * @brief 要素数2の符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数2の符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt2MenuItemインスタンスへのポインタ
				 */
				Int2MenuItem* addSliderInt2Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addSliderInt32v2Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 3 values) of signed 32-bit integer type values
				 * @details This adds a slider menu item including array(which has 3 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数3の符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt3MenuItemインスタンスへのポインタ
				 */
				Int3MenuItem* addSliderInt3Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addSliderInt32v3Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 4 values) of signed 32-bit integer type values
				 * @details This adds a slider menu item including array(which has 4 values) of signed 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: INT_MIN)
				 * @param maxValue Maximum value available (default: INT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の符号付32ビット整数型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数4の符号付32ビット整数型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：INT_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：INT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt4MenuItemインスタンスへのポインタ
				 */
				Int4MenuItem* addSliderInt4Menuitem(const char* id,
					const char *label,
					int32_t* value = nullptr,
					int32_t step = 1,
					int32_t minValue = INT_MIN,
					int32_t maxValue = INT_MAX,
					int position = -1,
					const char *format = nullptr)
				{
					return addSliderInt32v4Menuitem(id, label, value, step, minValue, maxValue, position, format);
				}


				/*!
				 * @~English
				 * @brief Add menu item including signed 64-bit integer type value 
				 * @details This adds a menu item including a signed 64-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%ld) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: LLONG_MIN)
				 * @param maxValue Maximum value available (default: LLONG_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Int64MenuItem instance 
				 * @~Japanese
				 * @brief 符号付64ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号付64ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%ld)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：LLONG_MIN)
				 * @param maxValue 値の取り得る最大値(デフォルト：LLONG_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したInt64MenuItemインスタンスへのポインタ 
				 */
				virtual Int64MenuItem* addInt64Menuitem(const char* id, 
											const char *label, 
											int64_t value=0, 
											int64_t step=1,
											int64_t minValue=LLONG_MIN, 
											int64_t maxValue=LLONG_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including unsigned 8-bit integer type value 
				 * @details This adds a menu item including an unsigned 8-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d or %%x) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: 0)
				 * @param maxValue Maximum value available (default: UCHAR_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Uint8MenuItem instance 
				 * @~Japanese
				 * @brief 符号なし8ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号なし8ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d又は%%x)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：0)
				 * @param maxValue 値の取り得る最大値(デフォルト：UCHAR_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したUint8MenuItemインスタンスへのポインタ 
				 */
				virtual Uint8MenuItem* addUint8Menuitem(const char * id, 
											const char *label, 
											uint8_t value=0, 
											uint8_t step=1,
											uint8_t minValue=0, 
											uint8_t maxValue=UCHAR_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including unsigned 16-bit integer type value 
				 * @details This adds a menu item including an unsigned 16-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d or %%x) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: 0)
				 * @param maxValue Maximum value available (default: USHRT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Uint16MenuItem instance 
				 * @~Japanese
				 * @brief 符号なし16ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号なし16ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d又は%%x)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：0)
				 * @param maxValue 値の取り得る最大値(デフォルト：USHRT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したUint16MenuItemインスタンスへのポインタ 
				 */
				virtual Uint16MenuItem* addUint16Menuitem(const char* id, 
											const char *label, 
											uint16_t value=0, 
											uint16_t step=1,
											uint16_t minValue=0, 
											uint16_t maxValue=USHRT_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including unsigned 32-bit integer type value 
				 * @details This adds a menu item including an unsigned 32-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d or %%x) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: 0)
				 * @param maxValue Maximum value available (default: UINT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Uint32MenuItem instance 
				 * @~Japanese
				 * @brief 符号なし32ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号なし32ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d又は%%x)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：0)
				 * @param maxValue 値の取り得る最大値(デフォルト：UINT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したUint32MenuItemインスタンスへのポインタ 
				 */
				virtual Uint32MenuItem* addUint32Menuitem(const char* id, 
											const char *label, 
											uint32_t value=0, 
											uint32_t step=1,
											uint32_t minValue=0, 
											uint32_t maxValue=UINT_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including unsigned 64-bit integer type value 
				 * @details This adds a menu item including an unsigned 64-bit integer type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%ld or %%lx) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: 0)
				 * @param maxValue Maximum value available (default: ULLONG_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Uint64MenuItem instance 
				 * @~Japanese
				 * @brief 符号なし64ビット整数型の値を含むメニュー項目を追加する 
				 * @details メニューに、符号なし64ビット整数型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%ld又は%%lx)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：0)
				 * @param maxValue 値の取り得る最大値(デフォルト：ULLONG_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したUint64MenuItemインスタンスへのポインタ 
				 */
				virtual Uint64MenuItem* addUint64Menuitem(const char* id, 
											const char *label, 
											uint64_t value=0, 
											uint64_t step=1,
											uint64_t minValue=0, 
											uint64_t maxValue=ULLONG_MAX, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including single precision floating-point type value 
				 * @details This adds a menu item including a single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%f) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0.0f)
				 * @param step Value step (default: 0.1f)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added FloatMenuItem instance 
				 * @~Japanese
				 * @brief 単精度浮動小数点型の値を含むメニュー項目を追加する 
				 * @details メニューに、単精度浮動小数点型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%f)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0.0f)
				 * @param step 値のステップ(デフォルト：0.1f)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloatMenuItemインスタンスへのポインタ 
				 */
				virtual FloatMenuItem* addFloatMenuitem(const char* id, 
												const char *label, 
												float value=0.0f, 
												float step=0.1f,
												float minValue=-FLT_MAX, 
												float maxValue= FLT_MAX, 
												int position=-1,
												const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including array(which has 2 values) of single precision floating-point type values
				 * @details This adds a menu item including array(which has 2 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float2MenuItem instance
				 * @~Japanese
				 * @brief 要素数2の単精度浮動小数点型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数2の単精度浮動小数点型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat2MenuItemインスタンスへのポインタ
				 */
				virtual Float2MenuItem* addFloat2Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including array(which has 3 values) of single precision floating-point type values
				 * @details This adds a menu item including array(which has 3 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の単精度浮動小数点型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数3の単精度浮動小数点型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設 定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があり ます。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat3MenuItemインスタンスへのポインタ
				 */
				virtual Float3MenuItem* addFloat3Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item including array(which has 4 values) of single precision floating-point type values
				 * @details This adds a menu item including array(which has 4 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の単精度浮動小数点型の値を含むメニュー項目を追加する
				 * @details メニューに、要素数4の単精度浮動小数点型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設 定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があり ます。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat4MenuItemインスタンスへのポインタ
				 */
				virtual Float4MenuItem* addFloat4Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including single precision floating-point type values
				 * @details This adds a slider menu item including single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0)
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added FloatMenuItem instance
				 * @~Japanese
				 * @brief 単精度浮動小数点型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、単精度浮動小数点型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があり ます。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloatMenuItemインスタンスへのポインタ
				 */
				virtual FloatMenuItem* addSliderFloatMenuitem(const char* id,
					const char *label,
					float value = 0.0f,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 2 values) of single precision floating-point type values
				 * @details This adds a slider menu item including array(which has 2 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float2MenuItem instance
				 * @~Japanese
				 * @brief 要素数2の単精度浮動小数点型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数2の単精度浮動小数点型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式 の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる 必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat2MenuItemインスタンスへのポインタ
				 */
				virtual Float2MenuItem* addSliderFloat2Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 3 values) of single precision floating-point type values
				 * @details This adds a slider menu item including array(which has 3 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の単精度浮動小数点型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数3の単精度浮動小数点型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式 の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる 必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat3MenuItemインスタンスへのポインタ
				 */
				virtual Float3MenuItem* addSliderFloat3Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add slider menu item including array(which has 4 values) of single precision floating-point type values
				 * @details This adds a slider menu item including array(which has 4 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param step Value step (default: 1)
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の単精度浮動小数点型の値を含むスライダーメニュー項目を追加する
				 * @details メニューに、要素数4の単精度浮動小数点型の値を含むスライダーメニュー項目を追加します。format引数にprintf形式 の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる 必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param step 値のステップ(デフォルト：1)
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat4MenuItemインスタンスへのポインタ
				 */
				virtual Float4MenuItem* addSliderFloat4Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float step = 0.1f,
					float minValue = -FLT_MAX,
					float maxValue = FLT_MAX,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add color edit menu item including array(which has 3 values) of single precision floating-point type values
				 * @details This adds a color edit menu item including array(which has 3 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の単精度浮動小数点型の値を含むカラーエディットメニュー項目を追加する
				 * @details メニューに、要素数3の単精度浮動小数点型の値を含むカラーエディットメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat3MenuItemインスタンスへのポインタ
				 */
				virtual Float3MenuItem* addColorEditFloat3Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float minValue = 0.f,
					float maxValue = 1.f,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add color edit menu item including array(which has 4 values) of single precision floating-point type values
				 * @details This adds a color edit menu item including array(which has 4 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の単精度浮動小数点型の値を含むカラーエディットメニュー項目を追加する
				 * @details メニューに、要素数4の単精度浮動小数点型の値を含むカラーエディットメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat4MenuItemインスタンスへのポインタ
				 */
				virtual Float4MenuItem* addColorEditFloat4Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float minValue = 0.f,
					float maxValue = 1.f,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add color picker menu item including array(which has 3 values) of single precision floating-point type values
				 * @details This adds a color picker menu item including array(which has 3 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float3MenuItem instance
				 * @~Japanese
				 * @brief 要素数3の単精度浮動小数点型の値を含むカラーピッカーメニュー項目を追加する
				 * @details メニューに、要素数3の単精度浮動小数点型の値を含むカラーピッカーメニュー項目を追加します。format引数にprintf 形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入 れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat3MenuItemインスタンスへのポインタ
				 */
				virtual Float3MenuItem* addColorPickerFloat3Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float minValue = 0.f,
					float maxValue = 1.f,
					int position = -1,
					const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add color picker menu item including array(which has 4 values) of single precision floating-point type values
				 * @details This adds a color picker menu item including array(which has 4 values) of single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%d) must be inserted in the formatting in order to stringize the value.
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item value
				 * @param minValue Minimum value available (default: -FLT_MAX)
				 * @param maxValue Maximum value available (default: FLT_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added Float4MenuItem instance
				 * @~Japanese
				 * @brief 要素数4の単精度浮動小数点型の値を含むカラーピッカーメニュー項目を追加する
				 * @details メニューに、要素数4の単精度浮動小数点型の値を含むカラーピッカーメニュー項目を追加します。format引数にprintf 形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%d)を入 れる必要があります。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目配列へのポインタ
				 * @param minValue 値の取り得る最小値(デフォルト：-FLT_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：FLT_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したFloat4MenuItemインスタンスへのポインタ
				 */
				virtual Float4MenuItem* addColorPickerFloat4Menuitem(const char* id,
					const char *label,
					float* value = nullptr,
					float minValue = 0.f,
					float maxValue = 1.f,
					int position = -1,
					const char *format = nullptr) = 0;


				/*!
				 * @~English
				 * @brief Add menu item including double precision floating-point type value 
				 * @details This adds a menu item including a single precision floating-point type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%lf) must be inserted in the formatting in order to stringize the value. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: 0.0f)
				 * @param step Value step (default: 0.1f)
				 * @param minValue Minimum value available (default: -DBL_MAX)
				 * @param maxValue Maximum value available (default: DBL_MAX)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added DoubleMenuItem instance 
				 * @~Japanese
				 * @brief 倍精度浮動小数点型のの値を含むメニュー項目を追加する 
				 * @details メニューに、単精度浮動小数点型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%lf)を入れる必要があります。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：0.0f)
				 * @param step 値のステップ(デフォルト：0.1f)
				 * @param minValue 値の取り得る最小値(デフォルト：-DBL_MAX)
				 * @param maxValue 値の取り得る最大値(デフォルト：DBL_MAX)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したDoubleMenuItemインスタンスへのポインタ 
				 */
				virtual DoubleMenuItem* addDoubleMenuitem(const char* id, 
												const char *label, 
												double value=0.0f, 
												double step=0.1f,
												double minValue=-DBL_MAX, 
												double maxValue= DBL_MAX, 
												int position=-1,
												const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add menu item that includes a bool type value 
				 * @details This adds a menu item that includes a bool type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (true/false are displayed as their respective character strings). 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Menu item value (default: true)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added BoolMenuItem instance 
				 * @~Japanese
				 * @brief bool型の値を含むメニュー項目を追加する 
				 * @details メニューに、bool型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(true/falseはそれぞれ文字列として表示されます)。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の値(デフォルト：true)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したBoolMenuItemインスタンスへのポインタ 
				 */
				virtual BoolMenuItem* addBoolMenuitem(const char* id, 
												const char *label, 
												bool value=true, 
												int position=-1,
												const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add button menu item that includes a bool type value
				 * @details This adds a button menu item that includes a bool type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (true/false are displayed as their respective character strings).
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param width width of the button
				 * @param height height of the button
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added BoolMenuItem instance
				 * @~Japanese
				 * @brief bool型の値を含むボタンメニュー項目を追加する
				 * @details メニューに、bool型の値を含むボタンメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(true/falseはそれぞれ文字列として表示されます)。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param width ボタンの幅
				 * @param height ボタンの高さ
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したBoolMenuItemインスタンスへのポインタ
				 */
				virtual BoolMenuItem* addButtonMenuitem(const char* id,
												const char *label, 
												int32_t width, int32_t height,
												int position=-1,
												const char *format = nullptr) = 0;


				/*!
				 * @~English
				 * @brief Add menu item that includes indexed character string set type value 
				 * @details This adds a menu item that includes an indexed character string set type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed). 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param enumLabels Character string to be used for the menu item value
				 * @param numLabels Number of character strings included in enumLabel
				 * @param value Menu item value (default: 0)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added EnumMenuItem instance 
				 * @~Japanese
				 * @brief インデックス付文字列セット型の値を含むメニュー項目を追加 
				 * @details メニューに、インデックス付文字列セット型の値を含むメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param enumLabels メニュー項目の値に使用する文字列
				 * @param numLabels enumLabelに含まれる文字列の数
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したEnumMenuItemインスタンスへのポインタ 
				 */
				virtual EnumMenuItem* addEnumMenuitem(const char* id, 
											const char *label, 
											const char *enumLabels[],
											uint32_t numLabels,
											int value=0, 
											int position=-1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Add list box menu item that includes indexed character string set type value
				 * @details This adds a list box menu item that includes an indexed character string set type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed).
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param enumLabels Character string to be used for the menu item value
				 * @param numLabels Number of character strings included in enumLabel
				 * @param value Menu item value (default: 0)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added EnumMenuItem instance
				 * @~Japanese
				 * @brief インデックス付文字列セット型の値を含むリストボックスメニュー項目を追加
				 * @details メニューに、インデックス付文字列セット型の値を含むリストボックスメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param enumLabels メニュー項目の値に使用する文字列
				 * @param numLabels enumLabelに含まれる文字列の数
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したEnumMenuItemインスタンスへのポインタ
				 */
				virtual EnumMenuItem* addListBoxMenuitem(const char* id,
											const char *label,
											const char *enumLabels[],
											uint32_t numLabels,
											int value = 0,
											int position = -1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief radio button menu item that includes indexed character string set type value
				 * @details This adds a radio button menu item that includes an indexed character string set type value to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed).
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param enumLabels Character string to be used for the menu item value
				 * @param numLabels Number of character strings included in enumLabel
				 * @param value Menu item value (default: 0)
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added EnumMenuItem instance
				 * @~Japanese
				 * @brief インデックス付文字列セット型の値を含むラジオボタンメニュー項目を追加
				 * @details メニューに、インデックス付文字列セット型の値を含むラジオボタンメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param enumLabels メニュー項目の値に使用する文字列
				 * @param numLabels enumLabelに含まれる文字列の数
				 * @param value メニュー項目の値(デフォルト：0)
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したEnumMenuItemインスタンスへのポインタ
				 */
				virtual EnumMenuItem* addRadioButtonMenuitem(const char* id,
											const char *label,
											const char *enumLabels[],
											uint32_t numLabels,
											int value = 0,
											int position = -1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief editable text menu item
				 * @details This adds a editable text menu item to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed).
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item string
				 * @param col Color of string
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added StringMenuItem instance
				 * @~Japanese
				 * @brief 編集可能なテキストメニュー項目を追加
				 * @details メニューに、編集可能なテキストメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の文字列
				 * @param col 文字列のカラー値
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したStringMenuItemインスタンスへのポインタ
				 */
				virtual StringMenuItem* addInputTextMenuitem(const char* id,
											const char *label,
											const char *value,
											uint32_t col,
											int position = -1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Label text menu item
				 * @details This adds a label text menu item to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed).
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param value Pointer to menu item string
				 * @param col Color of string
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added StringMenuItem instance
				 * @~Japanese
				 * @brief ラベルテキストメニュー項目を追加
				 * @details メニューに、ラベルテキストメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param value メニュー項目の文字列
				 * @param col 文字列のカラー値
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したStringMenuItemインスタンスへのポインタ
				 */
				virtual StringMenuItem* addLabelTextMenuitem(const char* id,
											const char *label,
											const char *value,
											uint32_t col,
											int position = -1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Bullet text menu item
				 * @details This adds a bullet text menu item to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed).
				 * @param id Menu item id
				 * @param value Pointer to menu item string
				 * @param col Color of string
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added StringMenuItem instance
				 * @~Japanese
				 * @brief 箇条書きテキストメニュー項目を追加
				 * @details メニューに、箇条書きテキストメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。
				 * @param id メニュー項目のid
				 * @param value メニュー項目の文字列
				 * @param col 文字列のカラー値
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したStringMenuItemインスタンスへのポインタ
				 */
				virtual StringMenuItem* addBulletTextMenuitem(const char* id,
											const char *value,
											uint32_t col,
											int position = -1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Fixed text menu item
				 * @details This adds a fixed text menu item to a menu. By setting printf format formatting to the format argument, a value can be stringized with an arbitrary format. A specifier (%%s) must be inserted in the formatting in order to stringize the value (character strings for indices will be displayed).
				 * @param id Menu item id
				 * @param value Pointer to menu item string
				 * @param col Color of string
				 * @param position Menu item position (default: -1)
				 * @param format User-defined formatting
				 * @return Pointer to added StringMenuItem instance
				 * @~Japanese
				 * @brief 固定テキストメニュー項目を追加
				 * @details メニューに、固定テキストメニュー項目を追加します。format引数にprintf形式の書式を設定することで、任意のフォーマットで値を文字列化できます。書式の中には値を文字列化するために指定子(%%s)を入れる必要があります(インデックスに対応した文字列が表示されます)。
				 * @param id メニュー項目のid
				 * @param value メニュー項目の文字列
				 * @param col 文字列のカラー値
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param format ユーザ定義の書式
				 * @return 追加したStringMenuItemインスタンスへのポインタ
				 */
				virtual StringMenuItem* addTextMenuitem(const char* id,
											const char *value,
											uint32_t col,
											int position = -1,
											const char *format = nullptr) = 0;

				/*!
				 * @~English
				 * @brief Adds a separator menu item 
				 * @details Adds a separator menu item.
				 * @param position Menu item position (default: -1)
				 * @~Japanese
				 * @brief セパレータメニュー項目を追加
				 * @details メニューに、セパレータメニュー項目を追加します。
				 * @param position メニュー項目の位置(デフォルト：-1)
				 */
				virtual void addSeparatorMenuitem(int position = -1) = 0;

				/*!
				 * @~English
				 * @brief Adds a new line menu item
				 * @details Adds a new line menu item.
				 * @param position Menu item position (default: -1)
				 * @~Japanese
				 * @brief 改行メニュー項目を追加
				 * @details メニューに、改行メニュー項目を追加します。
				 * @param position メニュー項目の位置(デフォルト：-1)
				 */
				virtual void addNewLineMenuitem(int position = -1) = 0;

				/*!
				 * @~English
				 * @brief Adds a spacing menu item
				 * @details Adds a spacing menu item.
				 * @param position Menu item position (default: -1)
				 * @~Japanese
				 * @brief 空白メニュー項目を追加
				 * @details メニューに、空白メニュー項目を追加します。
				 * @param position メニュー項目の位置(デフォルト：-1)
				 */
				virtual void addSpacingMenuitem(int position = -1) = 0;

				/*!
				 * @~English
				 * @brief Adds a same line menu item
				 * @details Adds a same line menu item.
				 * @param pos_x align to specified x position
				 * @param spacing_w enforce spacing amount
				 * @param position Menu item position (default: -1)
				 * @~Japanese
				 * @brief 同一行メニュー項目を追加
				 * @details メニューに、同一行メニュー項目を追加します。
				 * @param pos_x X座標のアライメント
				 * @param spacing_w 追加するスペースの幅
				 * @param position メニュー項目の位置(デフォルト：-1)
				 */
				virtual void addSameLineMenuitem(float pos_x, float spacing_w, int position = -1) = 0;

				/*!
				 * @~English
				 * @brief Adds a dummy menu item
				 * @details Adds a dummy menu item.
				 * @param size_x width of a dummy menu item
				 * @param size_y height of a dummy menu item
				 * @param position Menu item position (default: -1)
				 * @~Japanese
				 * @brief ダミーメニュー項目を追加
				 * @details メニューに、ダミーメニュー項目を追加します。
				 * @param size_x ダミーの幅
				 * @param size_y ダミーの高さ
				 * @param position メニュー項目の位置(デフォルト：-1)
				 */
				virtual void addDummyMenuitem(float size_x, float size_y, int position = -1) = 0;

				/*!
				 * @~English
				 * @brief Adds menu item group 
				 * @details Adds a menu item group. 
				 * @param id Menu item id
				 * @param label Menu item descriptor
				 * @param position Menu item position (default: -1)
				 * @param expanded if true this group is expandede.(default ：true (expanded))
				 * @return Pointer to added MenuItemGroup instance 
				 * @~Japanese
				 * @brief メニュー項目のグループを追加 
				 * @details メニュー項目のグループを追加します。 
				 * @param id メニュー項目のid
				 * @param label メニュー項目の記述子
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @param expanded メニュー項目の開閉(デフォルト：true (開))
				 * @return 追加したMenuItemGroupインスタンスへのポインタ 
				 */
				virtual MenuItemGroup* addMenuItemGroup(const char* id, 
												const char *label,
												int position=-1,
												bool expanded = true) = 0;

				/*!
				 * @~English
				 * @brief Adds indented menu item group
				 * @details Adds a indented menu item group.
				 * @param id Menu item id
				 * @param indent_w width of indent
				 * @param position Menu item position (default: -1)
				 * @return Pointer to added MenuItemGroup instance
				 * @~Japanese
				 * @brief メニュー項目のインデントグループを追加
				 * @details メニュー項目のインデントグループを追加します。
				 * @param id メニュー項目のid
				 * @param indent_w インデント幅
				 * @param position メニュー項目の位置(デフォルト：-1)
				 * @return 追加したMenuItemGroupインスタンスへのポインタ
				 */
				virtual MenuItemGroup* addMenuItemIndentGroup (	const char* id,
																float indent_w,
																int position = -1) = 0;

				/*!
				 * @~English
				 * @brief Set expansion of this group.
				 * @details This function expand or collapse the group.
				 * @param expand if true, the group is expanded, otherwise the group is collapsed.
				 * @~Japanese
				 * @brief グループの開閉を支持します。
				 * @details この関数によりグループを開いたり閉じたりできます。
				 * @param expand trueの場合グループが開かれ、falseの場合はグループが閉じられます。
				 */
				virtual void setExpansion(bool expand) = 0;

				/*!
				 * @~English
				 * @brief Check expansion.
				 * @details Return true if the group is expanded, otherwise returns false.
				 * @return Whether if the group is expanded or not
				 * @~Japanese
				 * @brief グループの開閉をチェック
				 * @details グループが開かれていればtrue, そうでなければfalseを返します。
				 * @return グループが開かれている否か
				 */
				virtual bool isExpanded() = 0;

				/*!
				 * @~English
				 * @brief Sets label 
				 * @details This sets a label. 
				 * @param label Menu item descriptor
				 * @~Japanese
				 * @brief ラベルを設定 
				 * @details ラベルを設定します。 
				 * @param label メニュー項目の記述子
				 */
				virtual void setLabel(const char* label) = 0;
			};

			class MenuSetting
			{
			public:
				ImVec4 cursorItemColor;
				ImVec4 otherItemColor;
				sce::Vectormath::Simd::Aos::Vector4 cursorItemColorInVec;
				sce::Vectormath::Simd::Aos::Vector4 otherItemColorInVec;
				ImVec4 backgroundColor;
				uint32_t lineHeight;
				uint32_t fontHeight;
				uint32_t indent;

				ImVec4 getColor(bool isCursor) const
				{
					return isCursor ? cursorItemColor : otherItemColor;
				}
				sce::Vectormath::Simd::Aos::Vector4 getColorInVec(bool isCursor) const
				{
					return isCursor ? cursorItemColorInVec : otherItemColorInVec;
				}

				const AbstractMenuItem *cursorMenuItem;		/* set by Menu class */
			};

			/*!
			 * @~English
			 * @brief Class that performs management/rendering/etc. for a menu that is a tree structure 
			 * @details This class performs management/rendering/etc. for a menu that is a tree structure. 
			 * @~Japanese
			 * @brief ツリー構造になっているメニューの管理/描画などを行うクラス 
			 * @details ツリー構造になっているメニューの管理/描画などを行うクラスです。 
			 */
			class Menu
			{
			public:
				/*!
				 * @~English
				 * @brief Default constructor
				 * @details This is a default constructor.
				 * @~Japanese
				 * @brief デフォルトコンストラクタ
				 * @details デフォルトコンストラクタです。
				 */
				Menu();
				/*!
				 * @~English
				 * @brief Constructor
				 * @param label Menu item descriptor
				 * @param expanded if true this group is expandede.(default ：true (expanded))
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param label メニュー項目の記述子
				 * @param expanded メニュー項目の開閉(デフォルト：true (開))
				 * @details コンストラクタです。
				 */
				Menu(const char *label, bool expanded/*= true*/);
				/*!
				 * @~English
				 * @brief Constructor
				 * @param xml Entire XML string that describes the menu structure
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param xml メニュー構造が記載されたXML文字列全体
				 * @details コンストラクタです。
				 */
				Menu(const char *xml);

				/*!
				 * @~English
				 * @brief Destructor 
				 * 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * 
				 * @details デストラクタです。 
				 */
				virtual ~Menu();

				/*!
				 * @~English
				 * @brief Color setting for menu
				 * @details This function sets some color informations for menu.
				 * @param cursorItemColorRgba color for currently selected item
				 * @param otherItemColorRgba color for currently not selected item
				 * @param backgroundColorRgba color for background of item 
				 * @~Japanese
				 * @brief メニューのカラー設定
				 * @details メニューのカラー設定を行います
				 * @param cursorItemColorRgba 選択中の項目カラー
				 * @param otherItemColorRgba 非選択中の項目カラー
				 * @param backgroundColorRgba 背景カラー
				 */
				virtual void setColor(sce::Vectormath::Simd::Aos::Vector4_arg cursorItemColorRgba,
				                       sce::Vectormath::Simd::Aos::Vector4_arg otherItemColorRgba,
									   sce::Vectormath::Simd::Aos::Vector4_arg backgroundColorRgba
									   );
				/*!
				 * @~English
				 * @brief Font height setting for menu
				 * @details This function sets font height for menu.
				 * @param height font height (in pixel)
				 * @~Japanese
				 * @brief フォントの高さを設定
				 * @details フォントの高さをピクセルで設定します。
				 * @param height フォントの高さ（ピクセル）
				 */
				virtual void setFontHeightInPix(uint32_t height);

				/*!
				 * @~English
				 * @brief Line height setting for menu
				 * @details This function sets line height for menu.
				 * @param height line height (in pixel)
				 * @~Japanese
				 * @brief 一行の高さを設定
				 * @details 一行の高さをピクセルで設定します。
				 * @param height 一行の高さ（ピクセル）
				 */
				virtual void setLineHeightInPix(uint32_t height);

				/*!
				 * @~English
				 * @brief Indent setting for menu
				 * @details This function sets indent width for menu.
				 * @param indent indent width (in pixel)
				 * @~Japanese
				 * @brief インデントの幅を設定
				 * @details インデントの幅をピクセルで設定します。
				 * @param indent インデントの幅（ピクセル）
				 */
				virtual void setIndentInPix(uint32_t indent);

				/*!
				 * @~English
				 * @brief Obtains the root of the managed menu tree 
				 * @details This obtains the root of the managed menu tree. 
				 * @return Pointer to MenuItemGroup 
				 * @~Japanese
				 * @brief 管理しているメニューツリーのルートを取得 
				 * @details 管理しているメニューツリーのルートを取得します。 
				 * @return MenuItemGroupへのポインタ 
				 */
				virtual MenuItemGroup *getRoot() const;
				/*!
				 * @~English
				 * @brief Obtains pointer from id to AbstractMenuItem 
				 * @details Obtains a pointer to the AbstractMenuItem linked to id. This id includes a group id unlike getitemby. 
				 * @param id Menu item id that also includes a group id
				 * @retval AbstractMenuItem* When a menu item linked to id is found, a pointer to AbstractMenuItem will be returned
				 * @retval NULL When menu item linked to id could not be found
				 * @~Japanese
				 * @brief idからAbstractMenuItemへのポインタを取得 
				 * @details idに紐づいた、AbstractMenuItemへのポインタを得ます。このidはgetitembyとは違い、グループのidも含みます。 
				 * @param id グループのidも含めたメニュー項目のid
				 * @retval AbstractMenuItem* idに紐づいたメニュー項目が見つかった場合、AbstractMenuItemへのポインタを返します
				 * @retval NULL idに紐づいたメニュー項目が見つからなかった場合
				 */
				virtual AbstractMenuItem *findItem(const char *id) const;

				/*!
				 * @~English
				 * @brief Draw menu item 
				 * @details Renders a menu item. 
				 * @param xInPix x coordinate in pixels
				 * @param yInPix y coordinate in pixels
				 * @param wInPix width in pixels
				 * @param hInPix height in pixels
				 * @retval SCE_OK Success
				 * @retval <SCE_OK Failure (error code)
				 * @~Japanese
				 * @brief メニュー項目の描画 
				 * @details メニュー項目を描画します。 
				 * @param xInPix ピクセル単位でのx座標
				 * @param yInPix ピクセル単位でのy座標
				 * @param wInPix ピクセル単位での幅
				 * @param hInPix ピクセル単位での高さ
				 * @retval SCE_OK 成功
				 * @retval <SCE_OK 失敗(エラーコード)
				 */
				virtual int draw(
					uint32_t xInPix,
					uint32_t yInPix,
					uint32_t wInPix,
					uint32_t hInPix
					);

				/*!
				 * @~English
				 * @brief Initialize a menu
				 * @details This function initialize a menu.
				 * @param label name for menu
				 * @param expanded whether menu is expanded or not
				 * @retval SCE_OK Success
				 * @retval <SCE_OK Failure (error code)
				 * @~Japanese
				 * @brief メニューの初期化
				 * @details メニューを初期化します。
				 * @param label メニュー名として表示されるラベル
				 * @param expanded メニューを展開するか否か 
				 * @retval SCE_OK 成功
				 * @retval <SCE_OK 失敗(エラーコード)
				 */
				int initialize(const char* label, bool expanded);

				/*!
				 * @~English
				 * @brief Initialize a menu from XML strings
				 * @details This function initialize a menu from XML strings.
				 * @param xml Pointer to XML strings
				 * @retval SCE_OK Success
				 * @retval <SCE_OK Failure (error code)
				 * @~Japanese
				 * @brief XMLからのメニューの初期化
				 * @details XMLからのメニューを初期化します。
				 * @param xml XML文字列
				 * @retval SCE_OK 成功
				 * @retval <SCE_OK 失敗(エラーコード)
				 */
				int initializeFromXmlString(const char *xml);

				AbstractMenuItem *getCursorItem() const;
				void onGroupExpansionChange(MenuItemGroup *group, bool expand);
				void update(){}

			private:
				MenuItemGroup	*m_root;
				MenuSetting		m_setting;
				uint32_t		m_cursorIndex;

				int finalize();
			};
		}
	}
}
