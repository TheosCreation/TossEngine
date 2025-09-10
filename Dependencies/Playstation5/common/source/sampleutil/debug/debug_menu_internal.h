/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2022 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once

#include <stdio.h>
#include <sampleutil/debug/menu.h>
#include <sampleutil/ui_framework/ui_framework.h>

#define ATOI8(s)	((int8_t)atoi(s))
#define ATOI16(s)	((int16_t)atoi(s))
#define ATOI32(s)	((int32_t)atoi(s))
#define ATOI64(s)	((int64_t)atoll(s))
#define ATOUI8(s)	((uint8_t)strtoull(s, nullptr, 0))
#define ATOUI16(s)	((uint16_t)strtoull(s, nullptr, 0))
#define ATOUI32(s)	((uint32_t)strtoull(s, nullptr, 0))
#define ATOUI64(s)	((uint64_t)strtoull(s, nullptr, 0))
#define ATOF(s)		((float)atof(s))
#define ATOD(s)		((double)atof(s))
#define ATOBOOL(s)	((std::string(s)=="true") ? true : false)

namespace sce {	namespace SampleUtil { namespace Debug {
	template<class Interface>
	class MenuItemImplCore : public Interface
	{
	public:
		MenuItemImplCore(const char* id, const char* label)
		{
			if (id == nullptr) {
				id = "";
			}
			if (label == nullptr) {
				label = "";
			}
			this->m_type = TYPE_GROUP;
			this->m_id = id;

			this->m_label = label;
			if (label) {
				this->setLabelForImgui(label);
			}

			this->m_isVisible = true;
			this->m_isActive = true;

			m_stringExp = m_label;
		}

		void setLabel(const char *label)
		{
			if (label == nullptr) {
				this->m_label = "";
			}
			else {
				this->m_label = label;
			}
			setLabelForImgui(this->m_label.c_str());
			updateStringExp();
		}

		virtual Type getType() const
		{
			return this->m_type;
		}

		const char* getId() const
		{
			return this->m_id.c_str();
		}

		const char* getLabel() const
		{
			return this->m_label.c_str();
		}

		void setLabelForImgui(const char *label)
		{
			// label is ID and caption to display for ImGui, the following special label format can be used to avoid issue caused by using same labels. 
			this->m_labelForImgui = std::string(label) + "##" + std::to_string((uint64_t)this);
		}
		const char* getLabelForImgui() const
		{
			return this->m_labelForImgui.c_str();
		}

		bool isVisible() const
		{
			return this->m_isVisible;
		}

		void setVisible(bool visible)
		{
			this->m_isVisible = visible;
		}

		bool isActive() const
		{
			return this->m_isActive;
		}

		void setActive(bool active)
		{
			this->m_isActive = active;
		}

		virtual const char* toString() const
		{
			return this->m_stringExp.c_str();
		}

		ImGuiTreeNodeFlags beginImgui() 
		{
			ImGuiInputTextFlags flags = this->isActive() ? 0 : ImGuiInputTextFlags_ReadOnly;
			if (flags) ImGui::PushStyleColor(ImGuiCol_Text, 0xFF808080);
			return flags;
		}

		void endImgui() 
		{
			ImGuiInputTextFlags flags = this->isActive() ? 0 : ImGuiInputTextFlags_ReadOnly;
			if (flags) ImGui::PopStyleColor();
		}

		virtual void update() = 0;
		virtual void updateStringExp() = 0;
	protected:
		Type m_type;

		std::string m_id;
		std::string m_label;
		std::string m_labelForImgui;
		std::string m_stringExp;

		bool m_isVisible;
		bool m_isActive;
	};

	// 値を操作しないレイアウト型
	class LayoutMenuItemImpl : public MenuItemImplCore<AbstractMenuItem>
	{
	public:
		LayoutMenuItemImpl(Type type) : MenuItemImplCore<AbstractMenuItem>("", "")
		{
			this->m_type = type;
		}
		virtual ~LayoutMenuItemImpl() {}
		virtual void updateStringExp() {};
	};

	class SeparatorMenuItemImpl : public LayoutMenuItemImpl
	{
	public:
		SeparatorMenuItemImpl() : LayoutMenuItemImpl(TYPE_SEPARATOR){}
		virtual ~SeparatorMenuItemImpl() {}

		int initialize() 
		{
			return SCE_OK;
		}

		void update()
		{
			ImGui::Separator();
		}
	};

	class SameLineMenuItemImpl : public LayoutMenuItemImpl
	{
		float m_posX;
		float m_spacingW;
	public:
		SameLineMenuItemImpl() : LayoutMenuItemImpl(TYPE_SAMELINE) {}
		virtual ~SameLineMenuItemImpl() {}

		int initialize(float pos_x, float spacing_w)
		{
			m_posX = pos_x;
			m_spacingW = spacing_w;
			return SCE_OK;
		}

		void update()
		{
			ImGui::SameLine(m_posX, m_spacingW);
		}
	};

	class NewLineMenuItemImpl : public LayoutMenuItemImpl
	{
	public:
		NewLineMenuItemImpl() : LayoutMenuItemImpl(TYPE_NEWLINE) {}
		virtual ~NewLineMenuItemImpl() {}
		int initialize()
		{
			return SCE_OK;
		}
		void update()
		{
			ImGui::NewLine();
		}
	};

	class SpacingMenuItemImpl : public LayoutMenuItemImpl
	{
	public:
		SpacingMenuItemImpl() : LayoutMenuItemImpl(TYPE_SPACING) {}
		virtual ~SpacingMenuItemImpl() {}
		int initialize()
		{
			return SCE_OK;
		}
		void update()
		{
			ImGui::Spacing();
		}
	};

	class DummyMenuItemImpl : public LayoutMenuItemImpl
	{
		float m_sizeX;
		float m_sizeY;
	public:
		DummyMenuItemImpl() : LayoutMenuItemImpl(TYPE_DUMMY) {}
		virtual ~DummyMenuItemImpl() {}
		int initialize(float size_x, float size_y)
		{
			m_sizeX = size_x;
			m_sizeY = size_y;
			return SCE_OK;
		}
		void update()
		{
			ImGui::Dummy(ImVec2(m_sizeX, m_sizeY));
		}
	};

	// 値を操作するタイプ
	template<typename ValueType, class Interface, int NumArray = 1>
	class ValuedMenuItemCoreSimple : public MenuItemImplCore<Interface>
	{
	public:
		ValuedMenuItemCoreSimple(const char* id, const char* label) : MenuItemImplCore<Interface>(id, label)
		{
			m_refValue = nullptr;
		}

		virtual ~ValuedMenuItemCoreSimple()
		{
			;
		}

		int initialize(sce::SampleUtil::Debug::AbstractMenuItem *owner, ValueType* value, Type type, const char *userFormat)
		{
			this->m_owner = this;

			setValueArray(value);

			MenuItemImplCore<Interface>::m_type = type;

			const char *d;
			switch (MenuItemImplCore<Interface>::m_type)
			{
			case TYPE_FLOAT:
			case TYPE_FLOAT2:
			case TYPE_FLOAT3:
			case TYPE_FLOAT4:
			case TYPE_FLOAT_SLIDER:
			case TYPE_FLOAT2_SLIDER:
			case TYPE_FLOAT3_SLIDER:
			case TYPE_FLOAT4_SLIDER:
			case TYPE_FLOAT3_COLOR_EDIT:
			case TYPE_FLOAT4_COLOR_EDIT:
			case TYPE_FLOAT3_COLOR_PICKER:
			case TYPE_FLOAT4_COLOR_PICKER:
			{ d = "%.3f";                break; }

			case TYPE_DOUBLE: { d = "%.3lf";             break; }
			case TYPE_INT8: { d = "%d";                      break; }
			case TYPE_INT16: { d = "%d";                      break; }

			case TYPE_INT32:
			case TYPE_INT32_V2:
			case TYPE_INT32_V3:
			case TYPE_INT32_V4:
			case TYPE_INT32_SLIDER:
			case TYPE_INT32_V2_SLIDER:
			case TYPE_INT32_V3_SLIDER:
			case TYPE_INT32_V4_SLIDER:
			{ d = "%d";                      break; }

			case TYPE_INT64: { d = "%lld";                break; }
			case TYPE_UINT8: { d = "0x%02x";          break; }
			case TYPE_UINT16: { d = "0x%04x";          break; }
			case TYPE_UINT32: { d = "0x%08x";          break; }
			case TYPE_UINT64: { d = "0x%016llx"; break; }
			default: { d = "%s"; break; }
			}
			if (userFormat != nullptr) {
				this->m_normalFormat = userFormat;
				this->m_activeFormat = userFormat;
			}
			else {
				this->m_normalFormat = this->m_activeFormat = d;
			}

			return SCE_OK;
		}

		const char* getFormat()
		{
			const char *format = nullptr;

			if (this->isActive()) {
				format = this->m_activeFormat.c_str();
			}
			else {
				format = this->m_normalFormat.c_str();
			}
			return format;
		}
	protected:
		sce::SampleUtil::Debug::AbstractMenuItem *m_owner;

		ValueType m_value[NumArray];
		constexpr int getArraySize() const { return NumArray; }
		void setValueArray(ValueType* v) {
			for (int i = 0; i < getArraySize(); i++) {
				m_value[i] = v[i];
			}
		}
		void setRefValueToValue(ValueType* ref = nullptr)
		{
			if (!ref) {
				ref = m_refValue;
			}
			if (ref) {
				for (int i = 0; i < getArraySize(); i++) {
					m_value[i] = ref[i];
				}
			}
		}
		void setValueToRefValue(ValueType* ref = nullptr) const
		{
			if (!ref) {
				ref = m_refValue;
			}
			if (ref) {
				for (int i = 0; i < getArraySize(); i++) {
					ref[i] = m_value[i];
				}
			}
		}

		ValueType *m_refValue;

		std::string m_normalFormat;
		std::string m_activeFormat;
	};

	template<typename ValueType, class Interface, int NumArray = 1>
	class ValuedMenuItemCore : public ValuedMenuItemCoreSimple<ValueType, Interface, NumArray>
	{
	public:
		ValuedMenuItemCore(const char* id, const char* label) : ValuedMenuItemCoreSimple<ValueType, Interface, NumArray>(id, label){}
		virtual ~ValuedMenuItemCore(){}

		void setReferenceValue(ValueType *refValue)
		{
			this->m_refValue = refValue;
			if (this->isActive()) {
				this->setValueToRefValue();
			}
		}
	};

	template<class ValueType, class InterfaceClass, Type menuType, int NumArray>
	class NumericalMenuItemCore : public ValuedMenuItemCore<ValueType, InterfaceClass, NumArray>
	{
	public:
		NumericalMenuItemCore(const char* id, const char* label)
			: ValuedMenuItemCore<ValueType, InterfaceClass, NumArray>(id, label)
			, NOT_REPEATED(-1)
			, REPEAT_IGNORE_FRAMES(20)
			, REPRAT_INTERVAL_TO_DO(5)
			, REPRAT_SPEED_UP_PER_X_FRAMES(60 * 3)
		{}
					
		int initialize(
			ValueType step, ValueType minValue, ValueType maxValue, ValueType* value, 
			const char *userFormat,
			sce::SampleUtil::Debug::AbstractMenuItem *owner=nullptr)
		{
			for (int i = 0; i < this->getArraySize(); i++) {
				if (value[i] < minValue) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}
				if (value[i] > maxValue) {
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}
			}
			this->ValuedMenuItemCore<ValueType, InterfaceClass, NumArray>::initialize((owner==nullptr)?this:owner, value, menuType, userFormat);

			this->m_step     = step;

			this->m_minValue = minValue;
			this->m_maxValue = maxValue;

			return SCE_OK;
		}

		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			bool ret;
			ImGuiInputTextFlags flags = this->beginImgui();
			ret = ImGui::InputScalarN(this->getLabelForImgui(), data_type, v, this->getArraySize(), &this->m_step, &this->m_step, this->getFormat(), flags);
			this->endImgui();
			return ret;
		}

		virtual void update()
		{
			this->setRefValueToValue();

			bool is_changed = false;
			ImGuiDataType data_type = ImGuiDataType_S32;

			switch (menuType) {

			case TYPE_INT64:
			{
				data_type = ImGuiDataType_S64;
				break;
			}

			case TYPE_UINT8:
			case TYPE_UINT16:
			case TYPE_UINT32:
			{
				data_type = ImGuiDataType_U32;
				break;
			}

			case TYPE_UINT64:
			{
				data_type = ImGuiDataType_U64;
				break;
			}

			case TYPE_FLOAT:
			case TYPE_FLOAT2:
			case TYPE_FLOAT3:
			case TYPE_FLOAT4:
			case TYPE_FLOAT_SLIDER:
			case TYPE_FLOAT2_SLIDER:
			case TYPE_FLOAT3_SLIDER:
			case TYPE_FLOAT4_SLIDER:
			case TYPE_FLOAT3_COLOR_EDIT:
			case TYPE_FLOAT4_COLOR_EDIT:
			case TYPE_FLOAT3_COLOR_PICKER:
			case TYPE_FLOAT4_COLOR_PICKER:
			{
				data_type = ImGuiDataType_Float;
				break;
			}

			case TYPE_DOUBLE:
			{
				data_type = ImGuiDataType_Double;
				break;
			}

			case TYPE_INT8:
			case TYPE_INT16:
			case TYPE_INT32:
			case TYPE_INT32_V2:
			case TYPE_INT32_V3:
			case TYPE_INT32_V4:
			case TYPE_INT32_SLIDER:
			case TYPE_INT32_V2_SLIDER:
			case TYPE_INT32_V3_SLIDER:
			case TYPE_INT32_V4_SLIDER:
			default:
			{
				data_type = ImGuiDataType_S32;
				break;
			}

			}

			if (menuType == TYPE_INT8 || menuType == TYPE_INT16) {
				int32_t v[4];
				for (int i = 0; i < this->getArraySize(); i++) {
					v[i] = this->m_value[i];
				}
				updateImgui(data_type, &v);
				for (int i = 0; i < this->getArraySize(); i++) {
					this->m_value[i] = (ValueType)v[i];
				}
			}
			else if (menuType == TYPE_UINT8 || menuType == TYPE_UINT16) {
				uint32_t v[4];
				for (int i = 0; i < this->getArraySize(); i++) {
					v[i] = this->m_value[i];
				}
				updateImgui(data_type, &v);
				for (int i = 0; i < this->getArraySize(); i++) {
					this->m_value[i] = (ValueType)v[i];
				}
			}
			else {
				updateImgui(data_type, this->m_value);
			}

			//check
			for (int i = 0; i < this->getArraySize(); i++) {
				if (this->m_value[i] > this->m_maxValue) {
					this->m_value[i] = this->m_maxValue;
				}
				if (this->m_value[i] < this->m_minValue) {
					this->m_value[i] = this->m_minValue;
				}
			}

			if (is_changed) {
				//output

				this->setValueToRefValue();
			}
			updateStringExp();
		}

		virtual void updateStringExp()
		{
			char buf[1024];
			char tmp[1024];
			const char *format = ValuedMenuItemCore<ValueType, InterfaceClass, NumArray>::getFormat();

			ValuedMenuItemCore<ValueType, InterfaceClass, NumArray>::m_stringExp = "";
			for (int i = 0; i < this->getArraySize(); i++) {
				snprintf(tmp, 1024, format, this->m_value, this->m_minValue, this->m_maxValue);
				tmp[1023] = '\0';

				snprintf(buf, 1024, "%s: %s ", this->m_label.c_str(), tmp);
				buf[1023] = '\0';

				this->ValuedMenuItemCore<ValueType, InterfaceClass, NumArray>::m_stringExp += buf;
			}
		}

	protected:
		ValueType m_step;
		ValueType m_minValue;
		ValueType m_maxValue;

		const int NOT_REPEATED;
		const int REPEAT_IGNORE_FRAMES;
		const int REPRAT_INTERVAL_TO_DO;
		const int REPRAT_SPEED_UP_PER_X_FRAMES;

		int m_repeatCountButtonLeft;
		int m_repeatCountButtonRight;
		bool needToAction(int repeatCount) {
			return (repeatCount == 0 || (repeatCount % REPRAT_INTERVAL_TO_DO == 0 && repeatCount > REPEAT_IGNORE_FRAMES));
		}
	};

	// スカラー型のベースクラス
	template<class ValueType, class InterfaceClass, Type menuType>
	class NumericalMenuItem : public NumericalMenuItemCore<ValueType, InterfaceClass, menuType, 1>
	{
	public:
		NumericalMenuItem(const char* id, const char* label)
			: NumericalMenuItemCore<ValueType, InterfaceClass, menuType, 1>(id, label)
		{}

		int initialize(
			ValueType step, ValueType minValue, ValueType maxValue, ValueType value,
			const char *userFormat,
			sce::SampleUtil::Debug::AbstractMenuItem *owner = nullptr)
		{
			return NumericalMenuItemCore<ValueType, InterfaceClass, menuType, 1>::initialize(step, minValue, maxValue, &value, userFormat, owner);
		}

		ValueType getValue() const
		{
			return this->m_value[0];
		}

		void setValue(ValueType value)
		{
			this->m_value[0] = value;
			this->updateStringExp();
		}
	};

	typedef NumericalMenuItem< int8_t,  Int8MenuItem, TYPE_INT8>		Int8MenuItemImpl;
	typedef NumericalMenuItem<int16_t, Int16MenuItem, TYPE_INT16>		Int16MenuItemImpl;
	typedef NumericalMenuItem<int32_t, Int32MenuItem, TYPE_INT32>		Int32MenuItemImpl;
	typedef NumericalMenuItem<int64_t, Int64MenuItem, TYPE_INT64>		Int64MenuItemImpl;

	typedef NumericalMenuItem< uint8_t,  Uint8MenuItem, TYPE_UINT8>		Uint8MenuItemImpl;
	typedef NumericalMenuItem<uint16_t, Uint16MenuItem, TYPE_UINT16>	Uint16MenuItemImpl;
	typedef NumericalMenuItem<uint32_t, Uint32MenuItem, TYPE_UINT32>	Uint32MenuItemImpl;
	typedef NumericalMenuItem<uint64_t, Uint64MenuItem, TYPE_UINT64>	Uint64MenuItemImpl;

	typedef NumericalMenuItem< float,  FloatMenuItem, TYPE_FLOAT>		FloatMenuItemImpl;
	typedef NumericalMenuItem<double, DoubleMenuItem, TYPE_DOUBLE>		DoubleMenuItemImpl;

	// スカラー型のスライダーメニュー項目
	template<class ValueType, class InterfaceClass, Type menuType>
	class SliderNumericalMenuItem : public NumericalMenuItem<ValueType, InterfaceClass, menuType>
	{
	public:
		SliderNumericalMenuItem(const char* id, const char* label)
			: NumericalMenuItem<ValueType, InterfaceClass, menuType>(id, label)
		{}

	protected:
		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			bool ret;
			//ImGuiInputTextFlags flags = this->beginImgui();
			ret = ImGui::SliderScalarN(this->getLabelForImgui(), data_type, v, this->getArraySize(), &this->m_minValue, &this->m_maxValue, this->getFormat());
			//this->endImgui();
			return ret;
		}
	};

	typedef SliderNumericalMenuItem< float, FloatMenuItem, TYPE_FLOAT_SLIDER>	SliderFloatMenuItemImpl;
	typedef SliderNumericalMenuItem< int32_t, Int32MenuItem, TYPE_INT32_SLIDER>		SliderIntMenuItemImpl;

	// 文字列型のベースクラス
	class StringMenuItemImpl : public ValuedMenuItemCoreSimple<std::string, StringMenuItem>
	{
	public:
		StringMenuItemImpl(const char* id, const char* label, ImU32 col)
			: ValuedMenuItemCoreSimple<std::string, StringMenuItem>(id, label)
			, m_refValueStr(nullptr)
			, m_refValueStrSize(0)
			, m_col(col)
		{}

		virtual ~StringMenuItemImpl() {}

		int initialize(std::string value, const char *userFormat)
		{
			this->ValuedMenuItemCoreSimple<std::string, StringMenuItem>::initialize(this, &value, TYPE_TEXT_INPUT, userFormat);
			updateStringExp();
			return SCE_OK;
		}

		const char *toString() const
		{
			return m_stringExp.c_str();
		}

		virtual bool updateImguiText(char* buf, size_t buf_size)
		{
			return ImGui::InputText(this->getLabelForImgui(), buf, buf_size);
		}

		virtual bool updateImguiTextColored(char* buf, size_t buf_size)
		{
			ImGui::PushStyleColor(ImGuiCol_Text, m_col);
			bool ret = updateImguiText(buf, buf_size);
			ImGui::PopStyleColor();
			return ret;
		}

		void update()
		{
			// リファレンスがあれば取得
			if (this->m_refValueStr) {
				this->m_value[0] = this->m_refValueStr;
			}

			bool is_changed = false;

			static char txtBuffer[1024];
			snprintf(txtBuffer, sizeof(txtBuffer), "%s", this->m_value[0].c_str());
			is_changed = updateImguiTextColored(txtBuffer, sizeof(txtBuffer));
			this->m_value[0] = txtBuffer;

			if (is_changed) {
				//output
				// リファレンスがあれば戻す
				if (this->m_refValueStr) {
					snprintf(this->m_refValueStr, this->m_refValueStrSize, "%s", this->m_value[0].c_str());
				}
			}
			updateStringExp();
		}

		const char* getValue() const
		{
			return this->m_value[0].c_str();
		}

		void setValue(const char* buf)
		{
			this->m_value[0] = buf;
			this->updateStringExp();
		}

		void setReferenceValue(char *refValue, size_t refValueSize)
		{
			this->m_refValueStr = refValue;
			this->m_refValueStrSize = refValueSize;
		}

	protected:
		char* m_refValueStr;
		size_t m_refValueStrSize;
		ImU32 m_col;

		void updateStringExp()
		{
			char tmp[1024];
			const char *format = ValuedMenuItemCoreSimple<std::string, StringMenuItem>::getFormat();

			m_stringExp = "";

			snprintf(tmp, 1024, format, this->m_value[0].c_str());
			tmp[1023] = '\0';

			m_stringExp = tmp;
		}
	};

	typedef StringMenuItemImpl InputTextMenuItemImpl;

	class TextMenuItemImpl : public StringMenuItemImpl
	{
	public:
		TextMenuItemImpl(const char* id, const char* label, ImU32 col)
			: StringMenuItemImpl(id, label, col)
		{}

		virtual ~TextMenuItemImpl() {}

		int initialize(std::string value, const char *userFormat)
		{
			this->ValuedMenuItemCoreSimple<std::string, StringMenuItem>::initialize(this, &value, TYPE_TEXT, userFormat);
			updateStringExp();

			return SCE_OK;
		}

		bool updateImguiText(char* buf, size_t buf_size)
		{
			ImGui::Text("%s", buf);

			return false;
		}

		void update()
		{
			// リファレンスがあれば取得
			if (this->m_refValueStr) {
				this->m_value[0] = this->m_refValueStr;
			}

			static char txtBuffer[1024];
			snprintf(txtBuffer, sizeof(txtBuffer), "%s", this->m_value[0].c_str());
			updateImguiTextColored(txtBuffer, sizeof(txtBuffer));
		}
	};

	class LabelTextMenuItemImpl : public StringMenuItemImpl
	{
	public:
		LabelTextMenuItemImpl(const char* id, const char* label, ImU32 col)
			: StringMenuItemImpl(id, label, col)
		{}

		virtual ~LabelTextMenuItemImpl() {}

		int initialize(std::string value, const char *userFormat)
		{
			this->ValuedMenuItemCoreSimple<std::string, StringMenuItem>::initialize(this, &value, TYPE_TEXT_LABEL, userFormat);
			updateStringExp();

			return SCE_OK;
		}

		bool updateImguiText(char* buf, size_t buf_size)
		{
			ImGui::LabelText(this->getLabelForImgui(), "%s", buf);

			return false;
		}
	};

	class BulletTextMenuItemImpl : public StringMenuItemImpl
	{
	public:
		BulletTextMenuItemImpl(const char* id, const char* label, ImU32 col)
			: StringMenuItemImpl(id, label, col)
		{}

		virtual ~BulletTextMenuItemImpl() {}

		int initialize(std::string value, const char *userFormat)
		{
			this->ValuedMenuItemCoreSimple<std::string, StringMenuItem>::initialize(this, &value, TYPE_TEXT_BULLET, userFormat);
			updateStringExp();

			return SCE_OK;
		}

		bool updateImguiText(char* buf, size_t buf_size)
		{
			ImGui::BulletText("%s", buf);

			return false;
		}
	};

	class BoolMenuItemImpl : public ValuedMenuItemCore<bool, BoolMenuItem>
	{
	public:
		BoolMenuItemImpl(const char* id, const char* label)
			: ValuedMenuItemCore<bool, BoolMenuItem>(id, label)
		{}

		virtual ~BoolMenuItemImpl() {}
					
		int initialize(bool value, const char *userFormat)
		{
			this->ValuedMenuItemCore<bool, BoolMenuItem>::initialize(this, &value, TYPE_BOOL, userFormat);
			updateStringExp();

			return SCE_OK;
		}

		const char *toString() const 
		{
			return m_stringExp.c_str();
		}

		void update()
		{
			this->setValueToRefValue();

			bool is_changed = false;

			is_changed = ImGui::Checkbox(getLabelForImgui(), &this->m_value[0]);

			if (is_changed) {
				//output
				this->setRefValueToValue();
			}
			updateStringExp();
		}

		bool getValue() const
		{
			return this->m_value[0];
		}

		void setValue(bool value)
		{
			this->m_value[0] = value;
			this->updateStringExp();
		}

	private:
		void updateStringExp()
		{
			char tmp[1024];
			char buf[1024];
			const char *format = getFormat();

			snprintf(tmp, 1024, format, m_value[0] ? "true" : "false");
			tmp[1023] = '\0';

			snprintf(buf, 1024, "%s: %s", this->m_label.c_str(), tmp);
			buf[1023] = '\0';
			m_stringExp = buf;
		}
	};

	class ButtonMenuItemImpl : public ValuedMenuItemCore<bool, BoolMenuItem>
	{
	public:
		ButtonMenuItemImpl(const char* id, const char* label, ImVec2 size)
			: ValuedMenuItemCore<bool, BoolMenuItem>(id, label), m_size(size), m_prevValue(false)
		{}

		virtual ~ButtonMenuItemImpl() {}
					
		int initialize(bool value, const char *userFormat)
		{
			this->ValuedMenuItemCore<bool, BoolMenuItem>::initialize(this, &value, TYPE_BOOL, userFormat);
			updateStringExp();
			return SCE_OK;
		}

		const char *toString() const 
		{
			return m_stringExp.c_str();
		}

		void update()
		{
			this->setRefValueToValue();
			this->m_value[0] = ImGui::Button(getLabelForImgui(), m_size);

			//output
			this->setValueToRefValue();

			updateStringExp();
			this->m_prevValue = this->m_value[0];
		}

		bool getValue() const
		{
			return this->m_value[0];
		}

		void setValue(bool value)
		{
			this->m_value[0] = value;
			this->updateStringExp();
		}

	private:
		void updateStringExp()
		{
			char tmp[1024];
			char buf[1024];
			const char *format = getFormat();

			snprintf(tmp, 1024, format, m_value[0] ? "true" : "false");
			tmp[1023] = '\0';

			snprintf(buf, 1024, "%s: %s", this->m_label.c_str(), tmp);
			buf[1023] = '\0';
			m_stringExp = buf;
		}

		ImVec2 m_size;
		bool m_prevValue;
	};

	class EnumMenuItemImpl : public NumericalMenuItem<int32_t, EnumMenuItem, TYPE_ENUM>
	{
	public:
		EnumMenuItemImpl(const char* id, const char* label)
			: NumericalMenuItem<int32_t, EnumMenuItem, TYPE_ENUM>(id, label)
		{}

		virtual ~EnumMenuItemImpl() {}

		Type getType() const
		{
			return TYPE_ENUM;
		}
					
		int initialize(const std::vector<std::string>* enumLabel, int value, const char *userFormat)
		{
			//TODO : check enumLabel before calling initialize.
			int ret = NumericalMenuItem<int32_t, EnumMenuItem, TYPE_ENUM>::initialize(1, 0, enumLabel->size()-1, value, userFormat, this);
			if(ret != SCE_OK){
				return ret;
			}
			this->m_enumLabel.assign( enumLabel->begin(), enumLabel->end() );

			return SCE_OK;
		}

		static bool _itemGetter(void*data, int idx, const char** out_str) 
		{
			EnumMenuItemImpl* enumObj = (EnumMenuItemImpl*)data;
			*out_str = enumObj->m_enumLabel[idx].c_str();

			return true;
		}

		virtual bool updateImguiForEnum(int* value)
		{
			return ImGui::Combo(this->getLabelForImgui(), value, &EnumMenuItemImpl::_itemGetter, this, this->m_enumLabel.size());
		}

		virtual void update()
		{
			this->setRefValueToValue();
			updateImguiForEnum(&this->m_value[0]);

			//check
			if (this->m_value[0] > this->m_maxValue) {
				this->m_value[0] = this->m_maxValue;
			}
			if (this->m_value[0] < this->m_minValue) {
				this->m_value[0] = this->m_minValue;
			}

			updateStringExp();
		}

		void updateStringExp()
		{
			char buf[1024];
			char tmp[1024];
			const char *format = getFormat();

			snprintf(tmp, 1024, format, m_enumLabel.at(m_value[0]).c_str());
			tmp[1023] = '\0';

			snprintf(buf, 1024, "%s: %s", this->m_label.c_str(), tmp);
			buf[1023] = '\0';

			m_stringExp = buf;
		}
	protected:
		std::vector<std::string> m_enumLabel;
	};

	// ListBox 
	class ListBoxMenuItemImpl : public EnumMenuItemImpl
	{
	public:
		ListBoxMenuItemImpl(const char* id, const char* label)
			: EnumMenuItemImpl(id, label)
		{}

		virtual ~ListBoxMenuItemImpl() {}

		Type getType() const
		{
			return TYPE_LISTBOX;
		}

		virtual bool updateImguiForEnum(int* value)
		{
			return ImGui::ListBox(this->getLabelForImgui(), value, &EnumMenuItemImpl::_itemGetter, this, this->m_enumLabel.size(), 4);
		}
	};

	// ListBox 
	class RadioButtonMenuItemImpl : public EnumMenuItemImpl
	{
	public:
		RadioButtonMenuItemImpl(const char* id, const char* label)
			: EnumMenuItemImpl(id, label)
		{}

		virtual ~RadioButtonMenuItemImpl() {}

		Type getType() const
		{
			return TYPE_RADIOBUTTON;
		}

		virtual bool updateImguiForEnum(int* value)
		{
			int original = *value;
			ImGui::BeginGroup();
			ImGui::Text("%s", this->getLabel());
			for (int i = 0; i < this->m_enumLabel.size(); i++) {
				const char* label;
				EnumMenuItemImpl::_itemGetter(this, i , &label);
				if(i != 0) ImGui::SameLine();
				ImGui::RadioButton(label, value, i);
			}
			ImGui::EndGroup();

			return (original != *value);
		}
	};

	// ベクトル型のベースクラス
	template<class ValueType, class InterfaceClass, Type menuType, int NumArray>
	class NumericalMenuItemVector : public NumericalMenuItemCore<ValueType, InterfaceClass, menuType, NumArray>
	{
	public:
		NumericalMenuItemVector(const char* id, const char* label)
			: NumericalMenuItemCore<ValueType, InterfaceClass, menuType, NumArray>(id, label)
		{}

		void getValue(ValueType* value) const
		{
			this->setValueToRefValue(value);
		}

		void setValue(ValueType* value)
		{
			this->setRefValueToValue(value);
			this->updateStringExp();
		}
	};

	typedef NumericalMenuItemVector<int32_t, Int32v2MenuItem, TYPE_INT32_V2, 2>		Int32v2MenuItemImpl;
	typedef NumericalMenuItemVector<int32_t, Int32v3MenuItem, TYPE_INT32_V3, 3>		Int32v3MenuItemImpl;
	typedef NumericalMenuItemVector<int32_t, Int32v4MenuItem, TYPE_INT32_V4, 4>		Int32v4MenuItemImpl;

	typedef NumericalMenuItemVector<float, Float2MenuItem, TYPE_FLOAT2, 2>			Float2MenuItemImpl;
	typedef NumericalMenuItemVector<float, Float3MenuItem, TYPE_FLOAT3, 3>			Float3MenuItemImpl;
	typedef NumericalMenuItemVector<float, Float4MenuItem, TYPE_FLOAT4, 4>			Float4MenuItemImpl;

	// ベクトル型のスライダーメニュー項目
	template<class ValueType, class InterfaceClass, Type menuType, int NumArray>
	class SliderNumericalMenuItemVector : public NumericalMenuItemVector<ValueType, InterfaceClass, menuType, NumArray>
	{
	public:
		SliderNumericalMenuItemVector(const char* id, const char* label)
			: NumericalMenuItemVector<ValueType, InterfaceClass, menuType, NumArray>(id, label)
		{}

	protected:
		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			ValueType mins[NumArray];
			ValueType maxs[NumArray];
			for (int i = 0; i < this->getArraySize(); i++) {
				mins[i] = this->m_minValue;
				maxs[i] = this->m_maxValue;
			}

			return ImGui::SliderScalarN(this->getLabelForImgui(), data_type, v, this->getArraySize(), &mins[0], &maxs[0], this->getFormat());
		}
	};

	typedef SliderNumericalMenuItemVector<int32_t, Int32v2MenuItem, TYPE_INT32_V2, 2>	SliderInt32v2MenuItemImpl;
	typedef SliderNumericalMenuItemVector<int32_t, Int32v3MenuItem, TYPE_INT32_V3, 3>	SliderInt32v3MenuItemImpl;
	typedef SliderNumericalMenuItemVector<int32_t, Int32v4MenuItem, TYPE_INT32_V4, 4>	SliderInt32v4MenuItemImpl;

	typedef SliderNumericalMenuItemVector<float, Float2MenuItem, TYPE_FLOAT2, 2>		SliderFloat2MenuItemImpl;
	typedef SliderNumericalMenuItemVector<float, Float3MenuItem, TYPE_FLOAT3, 3>		SliderFloat3MenuItemImpl;
	typedef SliderNumericalMenuItemVector<float, Float4MenuItem, TYPE_FLOAT4, 4>		SliderFloat4MenuItemImpl;

	// ベクトル型のColorEditメニュー項目
	class ColorEditFloat3MenuItemImpl : public NumericalMenuItemVector<float, Float3MenuItem, TYPE_FLOAT3_COLOR_EDIT, 3>
	{
	public:
		ColorEditFloat3MenuItemImpl(const char* id, const char* label)
			: NumericalMenuItemVector<float, Float3MenuItem, TYPE_FLOAT3_COLOR_EDIT, 3>(id, label)
		{}

	protected:
		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			return ImGui::ColorEdit3(this->getLabelForImgui(), (float*)v);
		}
	};

	class ColorEditFloat4MenuItemImpl : public NumericalMenuItemVector<float, Float4MenuItem, TYPE_FLOAT4_COLOR_EDIT, 4>
	{
	public:
		ColorEditFloat4MenuItemImpl(const char* id, const char* label)
			: NumericalMenuItemVector<float, Float4MenuItem, TYPE_FLOAT4_COLOR_EDIT, 4>(id, label)
		{}

	protected:
		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			return ImGui::ColorEdit4(this->getLabelForImgui(), (float*)v);
		}
	};

	// ベクトル型のColorPickerメニュー項目
	class ColorPickerFloat3MenuItemImpl : public NumericalMenuItemVector<float, Float3MenuItem, TYPE_FLOAT3_COLOR_PICKER, 3>
	{
	public:
		ColorPickerFloat3MenuItemImpl(const char* id, const char* label)
			: NumericalMenuItemVector<float, Float3MenuItem, TYPE_FLOAT3_COLOR_PICKER, 3>(id, label)
		{}

	protected:
		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			return ImGui::ColorPicker3(this->getLabelForImgui(), (float*)v);
		}
	};

	class ColorPickerFloat4MenuItemImpl : public NumericalMenuItemVector<float, Float4MenuItem, TYPE_FLOAT4_COLOR_PICKER, 4>
	{
	public:
		ColorPickerFloat4MenuItemImpl(const char* id, const char* label)
			: NumericalMenuItemVector<float, Float4MenuItem, TYPE_FLOAT4_COLOR_PICKER, 4>(id, label)
		{}

	protected:
		virtual bool updateImgui(ImGuiDataType data_type, void* v)
		{
			return ImGui::ColorPicker4(this->getLabelForImgui(), (float*)v);
		}
	};

	class MenuItemGroupImpl : public ValuedMenuItemCore<bool, MenuItemGroup>
	{
	public:
		MenuItemGroupImpl(const char* id, const char* label, bool expanded, Menu *menu);
		int initialize(Type type, float indent_w = 0.f);

		~MenuItemGroupImpl();
		AbstractMenuItem* getItemById(const char* id) const;
		AbstractMenuItem* getItemByIndex(uint32_t index) const;
		uint32_t          getNumItems() const;



		void deleteItemById(const char* id);
		int deleteItemByIndex(uint32_t index);

		Int8MenuItem* addInt8Menuitem    (const char* id, const char* label, int8_t value,   int8_t step,	  int8_t minValue,    int8_t maxValue,   int position,	const char *format);
		Int16MenuItem* addInt16Menuitem  (const char* id, const char* label, int16_t value,  int16_t step,  int16_t minValue,  int16_t maxValue,   int position,	const char *format);

		Int32MenuItem* addInt32Menuitem  (const char* id, const char* label, int32_t value,  int32_t step1, int32_t minValue,  int32_t maxValue,	 int position, const char *format);
		Int32v2MenuItem* addInt32v2Menuitem(const char* id, const char *label, int32_t* value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format);
		Int32v3MenuItem* addInt32v3Menuitem(const char* id, const char *label, int32_t* value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format);
		Int32v4MenuItem* addInt32v4Menuitem(const char* id, const char *label, int32_t* value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format);

		Int32MenuItem* addSliderInt32Menuitem(const char* id, const char* label, int32_t value, int32_t step1, int32_t minValue, int32_t maxValue, int position, const char *format);
		Int32v2MenuItem* addSliderInt32v2Menuitem(const char* id, const char *label, int32_t* value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format);
		Int32v3MenuItem* addSliderInt32v3Menuitem(const char* id, const char *label, int32_t* value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format);
		Int32v4MenuItem* addSliderInt32v4Menuitem(const char* id, const char *label, int32_t* value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format);

		Int64MenuItem* addInt64Menuitem  (const char* id, const char* label, int64_t value,  int64_t step,  int64_t minValue,  int64_t maxValue,   int position, const char *format);
		Uint8MenuItem* addUint8Menuitem  (const char* id, const char* label, uint8_t value,  uint8_t step,  uint8_t minValue,  uint8_t maxValue,   int position, const char *format);
		Uint16MenuItem* addUint16Menuitem(const char* id, const char* label, 	uint16_t value, uint16_t step, uint16_t minValue, uint16_t maxValue, int position,  const char *format);
		Uint32MenuItem* addUint32Menuitem(const char* id, const char* label, uint32_t value, uint32_t step, uint32_t minValue,  uint32_t maxValue, int position, const char *format);
		Uint64MenuItem* addUint64Menuitem(const char* id, const char* label, 	uint64_t value, uint64_t step, uint64_t minValue, uint64_t maxValue,  int position,	const char *format);

		FloatMenuItem* addFloatMenuitem  (const char* id, const char* label, float value,    float step,   float minValue,    float maxValue,    int position, const char *format);
		Float2MenuItem* addFloat2Menuitem(const char* id, const char *label, float* value, float step, float minValue, float maxValue, int position, const char *format);
		Float3MenuItem* addFloat3Menuitem(const char* id, const char *label, float* value, float step, float minValue, float maxValue, int position, const char *format);
		Float4MenuItem* addFloat4Menuitem(const char* id, const char *label, float* value, float step, float minValue, float maxValue, int position, const char *format);

		FloatMenuItem* addSliderFloatMenuitem(const char* id, const char* label, float value, float step, float minValue, float maxValue, int position, const char *format);
		Float2MenuItem* addSliderFloat2Menuitem(const char* id, const char *label, float* value, float step, float minValue, float maxValue, int position, const char *format);
		Float3MenuItem* addSliderFloat3Menuitem(const char* id, const char *label, float* value, float step, float minValue, float maxValue, int position, const char *format);
		Float4MenuItem* addSliderFloat4Menuitem(const char* id, const char *label, float* value, float step, float minValue, float maxValue, int position, const char *format);

		Float3MenuItem* addColorEditFloat3Menuitem(const char* id, const char *label, float* value, float minValue, float maxValue, int position, const char *format);
		Float4MenuItem* addColorEditFloat4Menuitem(const char* id, const char *label, float* value, float minValue, float maxValue, int position, const char *format);
		Float3MenuItem* addColorPickerFloat3Menuitem(const char* id, const char *label, float* value, float minValue, float maxValue, int position, const char *format);
		Float4MenuItem* addColorPickerFloat4Menuitem(const char* id, const char *label, float* value, float minValue, float maxValue, int position, const char *format);

		DoubleMenuItem* addDoubleMenuitem(const char* id, const char* label, double value,   double step, double minValue,   double maxValue,   int position,	const char *format);
		BoolMenuItem* addBoolMenuitem    (const char* id, const char* label, bool value, int position,	const char *format);
		BoolMenuItem* addButtonMenuitem    (const char* id, const char* label, int32_t width, int32_t height, int position, const char *format);

		EnumMenuItem* addEnumMenuitem		(const char* id, const char* label, const char *enumLabels[], uint32_t numLabels, int value, int position, const char *format);
		EnumMenuItem* addListBoxMenuitem	(const char* id, const char* label, const char *enumLabels[], uint32_t numLabels, int value, int position, const char *format);
		EnumMenuItem* addRadioButtonMenuitem(const char* id, const char* label, const char *enumLabels[], uint32_t numLabels, int value, int position, const char *format);

		StringMenuItem* addInputTextMenuitem(const char* id, const char *label, const char *value, uint32_t col, int position, const char *format);
		StringMenuItem* addLabelTextMenuitem(const char* id, const char *label, const char *value, uint32_t col, int position, const char *format);
		StringMenuItem* addTextMenuitem(const char* id, const char *value, uint32_t col, int position, const char *format);
		StringMenuItem* addBulletTextMenuitem(const char* id, const char *value, uint32_t col, int position, const char *format);

		void addSeparatorMenuitem(int position);
		void addNewLineMenuitem(int position);
		void addSpacingMenuitem(int position);
		void addSameLineMenuitem(float pos_x, float spacing_w, int position);
		void addDummyMenuitem(float size_x, float size_y, int position);

		MenuItemGroup* addMenuItemGroup  (const char* id, const char* label,	int position, bool expanded);
		MenuItemGroup* addMenuItemIndentGroup(const char* id, float indent_w, int position);

		void setExpansion(bool expand);
		bool isExpanded();
		void doExpand(bool expand);
		void setActive(bool isActive);

		void update();

		void updateStringExp()
		{
			if(m_isExpanded){
				m_stringExp = m_label;
			}else{
				if(m_items.size() > 0){
					m_stringExp = m_label + " >>";
				}else{
					m_stringExp = m_label;
				}
			}
		}

	private:
		void insertItem(AbstractMenuItem* item, int position);

		bool m_isExpanded;
		std::vector<AbstractMenuItem*> m_items;
		Menu *m_menu;

		float m_indentW;
	};
}}}
