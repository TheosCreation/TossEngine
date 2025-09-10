/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include <sampleutil/debug/menu.h>
#include "debug_menu_internal.h"
#include "xml_internal.h"
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/sampleutil_error.h"

using namespace sce::SampleUtil::Debug;

namespace ssg  = sce::SampleUtil::Graphics;
namespace ssd  = sce::SampleUtil::Debug;
namespace ssi  = sce::SampleUtil::Input;
namespace vm   = sce::Vectormath::Simd::Aos;

static ImU32 getColorFromColorLabel(std::string color_string)
{
	struct {
		const char* color_label;
		ImU32		color_value;
	} color_table[] = {
		{"white", 0xFFFFFFFF},
		{"red", 0xFF0000FF},
		{"magenda", 0xFFFF00FF},
		{"blue", 0xFFFF0000},
		{"green", 0xFF00FF00},
	};

	for (auto i : color_table) {
		if (color_string == i.color_label) {
			return i.color_value;
		}
	}

	return 0xFFFFFFFF;
}

void parseGroup(ssd::MenuItemGroup *group, ssd::XmlElement* e)
{
	group->setLabel(e->Attribute("label"));
	for (ssd::XmlElement *c = e->getFirstChildElement();
		 c != nullptr;
		 c = c->getNextSiblingElement())
	{
		std::string elemName = c->Value();
		if(elemName == "int8"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label")	? c->Attribute("label")			: id;
			int8_t minValue		= c->Attribute("min")	? ATOI8(c->Attribute("min"))	: SCHAR_MIN;
			int8_t maxValue		= c->Attribute("max")	? ATOI8(c->Attribute("max"))	: SCHAR_MAX;
			int8_t step			= c->Attribute("step")	? ATOI8(c->Attribute("step"))	: 1;
			int8_t value		= c->Attribute("value")	? ATOI8(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addInt8Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(elemName == "int16"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			int16_t minValue	= c->Attribute("min")   ? ATOI16(c->Attribute("min"))	: SHRT_MIN;
			int16_t maxValue	= c->Attribute("max")   ? ATOI16(c->Attribute("max"))	: SHRT_MAX;
			int16_t step		= c->Attribute("step")  ? ATOI16(c->Attribute("step"))	: 1;
			int16_t value		= c->Attribute("value") ? ATOI16(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addInt16Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(
			elemName == "int32" 
			|| elemName == "int"  || elemName == "InputInt"
			|| elemName == "int2" || elemName == "InputInt2"
			|| elemName == "int3" || elemName == "InputInt3"
			|| elemName == "int4" || elemName == "InputInt4"
			|| elemName == "SliderInt"
			|| elemName == "SliderInt2"
			|| elemName == "SliderInt3"
			|| elemName == "SliderInt4"
			){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			int32_t minValue	= c->Attribute("min")   ? ATOI32(c->Attribute("min"))	: (elemName.find("Slider") != std::string::npos ?   0 : INT_MIN);
			int32_t maxValue	= c->Attribute("max")   ? ATOI32(c->Attribute("max"))	: (elemName.find("Slider") != std::string::npos ? 100 : INT_MAX);

			if (elemName.find("Slider") != std::string::npos) {
				minValue = std::max(minValue, INT_MIN / 2);
				maxValue = std::min(maxValue, INT_MAX / 2);
			}

			int32_t step		= c->Attribute("step")  ? ATOI32(c->Attribute("step"))	: 1;
			int32_t value		= c->Attribute("value") ? ATOI32(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			int32_t defaultValueArray[] = { value, value, value, value };

			if (elemName == "SliderInt") {
				group->addSliderInt32Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "SliderInt2") {
				group->addSliderInt2Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "SliderInt3") {
				group->addSliderInt3Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "SliderInt4") {
				group->addSliderInt4Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "int2" || elemName == "InputInt2") {
				group->addInt2Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "int3" || elemName == "InputInt3") {
				group->addInt3Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "int4" || elemName == "InputInt4") {
				group->addInt4Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else {
				group->addInt32Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
			}
		}else if(elemName == "int64"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			int64_t minValue	= c->Attribute("min")   ? ATOI64(c->Attribute("min"))	: LLONG_MIN;
			int64_t maxValue	= c->Attribute("max")   ? ATOI64(c->Attribute("max"))	: LLONG_MAX;
			int64_t step		= c->Attribute("step")  ? ATOI64(c->Attribute("step"))	: 1;
			int64_t value		= c->Attribute("value") ? ATOI64(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addInt64Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);	
		}else if(elemName == "uint8"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label")	? c->Attribute("label")			: id;
			uint8_t minValue	= c->Attribute("min")	? ATOUI8(c->Attribute("min"))	: 0;
			uint8_t maxValue	= c->Attribute("max")	? ATOUI8(c->Attribute("max"))	: UCHAR_MAX;
			uint8_t step		= c->Attribute("step")	? ATOUI8(c->Attribute("step"))	: 1;
			uint8_t value		= c->Attribute("value")	? ATOUI8(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addUint8Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(elemName == "uint16"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			uint16_t minValue	= c->Attribute("min")   ? ATOUI16(c->Attribute("min"))	: 0;
			uint16_t maxValue	= c->Attribute("max")   ? ATOUI16(c->Attribute("max"))	: USHRT_MAX;
			uint16_t step		= c->Attribute("step")  ? ATOUI16(c->Attribute("step"))	: 1;
			uint16_t value		= c->Attribute("value") ? ATOUI16(c->Attribute("value")): minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addUint16Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(elemName == "uint32"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			uint32_t minValue	= c->Attribute("min")   ? ATOUI32(c->Attribute("min"))	: 0;
			uint32_t maxValue	= c->Attribute("max")   ? ATOUI32(c->Attribute("max"))	: UINT_MAX;
			uint32_t step		= c->Attribute("step")  ? ATOUI32(c->Attribute("step"))	: 1;
			uint32_t value		= c->Attribute("value") ? ATOUI32(c->Attribute("value")): minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addUint32Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(elemName == "uint64"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			uint64_t minValue	= c->Attribute("min")   ? ATOUI64(c->Attribute("min"))	: 0;
			uint64_t maxValue	= c->Attribute("max")   ? ATOUI64(c->Attribute("max"))	: ULLONG_MAX;
			uint64_t step		= c->Attribute("step")  ? ATOUI64(c->Attribute("step"))	: 1;
			uint64_t value		= c->Attribute("value") ? ATOUI64(c->Attribute("value")): minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addUint64Menuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(
			elemName == "float" 
			|| elemName == "float2" || elemName == "InputFloat"
			|| elemName == "float3" || elemName == "InputFloat2"
			|| elemName == "float4" || elemName == "InputFloat3"
			|| elemName == "InputFloat4"
			|| elemName == "SliderFloat"
			|| elemName == "SliderFloat2"
			|| elemName == "SliderFloat3"
			|| elemName == "SliderFloat4"
			|| elemName == "ColorEdit3"
			|| elemName == "ColorEdit4"
			|| elemName == "ColorPicker3"
			|| elemName == "ColorPicker4"
			){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")					: id;
			float minValue		= c->Attribute("min")   ? ATOF(c->Attribute("min"))		: ((elemName.find("Slider") != std::string::npos) ? 0.f : -FLT_MAX);
			float maxValue		= c->Attribute("max")   ? ATOF(c->Attribute("max"))		: ((elemName.find("Slider") != std::string::npos) ? 1.f :  FLT_MAX);
			float step			= c->Attribute("step")  ? ATOF(c->Attribute("step"))	: 0.1f;
			float value			= c->Attribute("value") ? ATOF(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			float defaultValueArray[] = { value, value, value, value };
			if (elemName == "SliderFloat") {
				group->addSliderFloatMenuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "SliderFloat2") {
				group->addSliderFloat2Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "SliderFloat3") {
				group->addSliderFloat3Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "SliderFloat4") {
				group->addSliderFloat4Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "ColorEdit3") {
				group->addColorEditFloat3Menuitem(id.c_str(), label.c_str(), defaultValueArray, minValue, maxValue, -1, format);
			}
			else if (elemName == "ColorEdit4") {
				group->addColorEditFloat4Menuitem(id.c_str(), label.c_str(), defaultValueArray, minValue, maxValue, -1, format);
			}
			else if (elemName == "ColorPicker3") {
				group->addColorPickerFloat3Menuitem(id.c_str(), label.c_str(), defaultValueArray, minValue, maxValue, -1, format);
			}
			else if (elemName == "ColorPicker4") {
				group->addColorPickerFloat4Menuitem(id.c_str(), label.c_str(), defaultValueArray, minValue, maxValue, -1, format);
			}
			else if (elemName == "float2" || elemName == "InputFloat2") {
				group->addFloat2Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "float3" || elemName == "InputFloat3") {
				group->addFloat3Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else if (elemName == "float4" || elemName == "InputFloat4") {
				group->addFloat4Menuitem(id.c_str(), label.c_str(), defaultValueArray, step, minValue, maxValue, -1, format);
			}
			else {
				group->addFloatMenuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
			}
		}else if(elemName == "double"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			double minValue		= c->Attribute("min")   ? ATOD(c->Attribute("min"))		: -DBL_MAX;
			double maxValue		= c->Attribute("max")   ? ATOD(c->Attribute("max"))		:  DBL_MAX;
			double step			= c->Attribute("step")  ? ATOD(c->Attribute("step"))	: 0.1f;
			double value		= c->Attribute("value") ? ATOD(c->Attribute("value"))	: minValue;
			const char* format  = c->Attribute("format")? c->Attribute("format")	    : nullptr;

			group->addDoubleMenuitem(id.c_str(), label.c_str(), value, step, minValue, maxValue, -1, format);
		}else if(elemName == "bool" || elemName == "CheckBox"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			bool value			= c->Attribute("value") ? ATOBOOL(c->Attribute("value")): true;
			
			group->addBoolMenuitem(id.c_str(), label.c_str(), value);
		}else if(elemName == "Button"){
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")			: id;
			int width			= c->Attribute("width") ? ATOI32(c->Attribute("width")): 0;
			int height			= c->Attribute("height") ? ATOI32(c->Attribute("height")): 0;
			
			group->addButtonMenuitem(id.c_str(), label.c_str(), width, height);
		}else if(elemName == "enum" || elemName == "Combo" || elemName == "ListBox" || elemName == "RadioButton"){
			std::vector<std::string> enumString;
			std::string id		= c->Attribute("id");
			std::string label	= c->Attribute("label") ? c->Attribute("label")        : id;
			int index			= c->Attribute("value") ? atoi(c->Attribute("value")) : 0;
			for (ssd::XmlElement *c2 = c->getFirstChildElement("item");
				c2 != nullptr;
				c2 = c2->getNextSiblingElement("item"))
			{
				if(c2->Attribute("label")){
					enumString.push_back(c2->Attribute("label"));
				}
			}

			if(index >= (int)enumString.size()){
				index =  enumString.size() -1;
			}else if(index < 0){
				index = 0;
			}
			const char** labels = new const char*[enumString.size()];
			for(uint32_t i=0; i<enumString.size(); i++){
				labels[i] = enumString.at(i).c_str();
			}

			if (elemName == "ListBox") {
				group->addListBoxMenuitem(id.c_str(), label.c_str(), labels, enumString.size(), index);
			}
			else if (elemName == "RadioButton") {
				group->addRadioButtonMenuitem(id.c_str(), label.c_str(), labels, enumString.size(), index);
			}
			else {
				group->addEnumMenuitem(id.c_str(), label.c_str(), labels, enumString.size(), index);
			}

			delete [] labels;
		}
		else if (elemName == "InputText" || elemName == "LabelText" || elemName == "Text" || elemName == "BulletText") {
			std::string id = c->Attribute("id") ? c->Attribute("id") : "";
			SCE_SAMPLE_UTIL_ASSERT_MSG(!((elemName == "InputText") && !c->Attribute("id")), "[Error] InputText must have 'id' ATTRIBUTE.");

			std::string label = c->Attribute("label") ? c->Attribute("label") : id;
			std::string value = c->Attribute("value");
			const char* format = c->Attribute("format") ? c->Attribute("format") : nullptr;
			ImU32 col = c->Attribute("col") ? getColorFromColorLabel(c->Attribute("col")) : 0xFFFFFFFF;

			if (elemName == "InputText") {
				group->addInputTextMenuitem(id.c_str(), label.c_str(), value.c_str(), col, -1, format);
			}
			else if (elemName == "LabelText") {
				group->addLabelTextMenuitem(id.c_str(), label.c_str(), value.c_str(), col, -1, format);
			}
			else if (elemName == "BulletText") {
				group->addBulletTextMenuitem(id.c_str(), value.c_str(), col, -1, format);
			}
			else {
				group->addTextMenuitem(id.c_str(), value.c_str(), col, -1, format);
			}
		}
		else if (elemName == "group" || elemName == "TreeNode" || elemName == "Indent") {
			std::string id = c->Attribute("id") ? c->Attribute("id") : "";
			std::string label = c->Attribute("label") ? c->Attribute("label") : id;
			bool expanded = c->Attribute("expanded") ? ATOBOOL(c->Attribute("expanded")) : true;

			ssd::MenuItemGroup* g2;
			if (elemName == "Indent") {
				float indent_w = c->Attribute("indent_w") ? ATOF(c->Attribute("indent_w")) : 0.0f;
				g2 = group->addMenuItemIndentGroup(id.c_str(), indent_w, -1);
			}
			else {
				g2 = group->addMenuItemGroup(id.c_str(), label.c_str(), -1, expanded);
			}
			parseGroup(g2, c);
		}
		// 移行は、id/lableを持たない
		else if (elemName == "Separator") {
			group->addSeparatorMenuitem(-1);
		}
		else if (elemName == "NewLine") {
			group->addNewLineMenuitem(-1);
		}
		else if (elemName == "Spacing") {
			group->addSpacingMenuitem(-1);
		}
		else if (elemName == "SameLine") {
			float pos_x = c->Attribute("pos_x") ? ATOF(c->Attribute("pos_x")) : 0.f;
			float spacing_w = c->Attribute("spacing_w") ? ATOF(c->Attribute("spacing_w")) : 0.f;
			group->addSameLineMenuitem(pos_x, spacing_w, -1);
		}
		else if (elemName == "Dummy") {
			float size_x = c->Attribute("size_x") ? ATOF(c->Attribute("size_x")) : 0.f;
			float size_y = c->Attribute("size_y") ? ATOF(c->Attribute("size_y")) : 0.f;
			group->addDummyMenuitem(size_x, size_y, -1);
		}
	}
}

ssd::Menu::Menu()
{
	m_root = nullptr;

	m_setting.cursorItemColor = ImVec4(1.f, 0.f, 0.f, 1.f);
	m_setting.otherItemColor = ImVec4(1.f, 1.f, 1.f, 1.f);
	m_setting.cursorItemColorInVec = sce::Vectormath::Simd::Aos::Vector4(1.0f, 0.0f, 0.0f, 1.0f);;
	m_setting.otherItemColorInVec = sce::Vectormath::Simd::Aos::Vector4(1.0f);
	m_setting.backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
	m_setting.fontHeight = 18;
	m_setting.lineHeight = m_setting.fontHeight + 2;
	m_setting.indent = 16;
}

ssd::Menu::Menu(const char *label, bool expanded)
{
	m_root = nullptr;

	m_setting.cursorItemColor = ImVec4(1.f, 0.f, 0.f, 1.f);
	m_setting.otherItemColor = ImVec4(1.f, 1.f, 1.f, 1.f);
	m_setting.cursorItemColorInVec = sce::Vectormath::Simd::Aos::Vector4(1.0f, 0.0f, 0.0f, 1.0f);;
	m_setting.otherItemColorInVec = sce::Vectormath::Simd::Aos::Vector4(1.0f);
	m_setting.backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
	m_setting.fontHeight = 18;
	m_setting.lineHeight = m_setting.fontHeight + 2;
	m_setting.indent = 16;

	initialize(label, expanded);
}

ssd::Menu::Menu(const char *xml)
{
	m_root = nullptr;

	m_setting.cursorItemColor = ImVec4(1.f, 0.f, 0.f, 1.f);
	m_setting.otherItemColor = ImVec4(1.f, 1.f, 1.f, 1.f);
	m_setting.cursorItemColorInVec = sce::Vectormath::Simd::Aos::Vector4(1.0f, 0.0f, 0.0f, 1.0f);;
	m_setting.otherItemColorInVec = sce::Vectormath::Simd::Aos::Vector4(1.0f);
	m_setting.backgroundColor = ImVec4(0.0f, 0.0f, 0.0f, 0.2f);
	m_setting.fontHeight = 18;
	m_setting.lineHeight = m_setting.fontHeight + 2;
	m_setting.indent = 16;

	initializeFromXmlString(xml);
}

ssd::Menu::~Menu()
{
	this->finalize();
}

int ssd::Menu::initialize(const char *label, bool expanded)
{
	finalize();
	
	ssd::MenuItemGroupImpl *root = new ssd::MenuItemGroupImpl("", label, expanded, this);
	int ret = root->initialize(TYPE_GROUP);
	if(ret != SCE_OK){
		return ret;
	}
	m_root = root;

	m_cursorIndex = 0;

	return SCE_OK;
}

int ssd::Menu::initializeFromXmlString(const char *xml)
{
	if (xml == nullptr)
	{
		return SCE_SAMPLE_UTIL_ERROR_NULL_POINTER;
	}
	SCE_SAMPLE_UTIL_ASSERT(xml != nullptr);

	int ret = initialize("", true);

	XmlDocumentParser::create();
	{
		XmlDocumentParser	*xmlParser = XmlDocumentParser::getInstance();
		ret = xmlParser->parse(xml);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		XmlElement	*rootElem = xmlParser->getRootElem();
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
		}

		if (rootElem)
		{
			std::string rootElemName = rootElem->Value();
			if (rootElemName != "group")
			{
				return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
			}
			parseGroup(getRoot(), rootElem);
		}
	}
	XmlDocumentParser::destroy();

	return ret;
}

int ssd::Menu::finalize()
{
	if (m_root != nullptr)
	{
		delete m_root;
		m_root = nullptr;
	}

	return SCE_OK;
}

MenuItemGroup *ssd::Menu::getRoot() const
{
	return m_root;
}

AbstractMenuItem *ssd::Menu::findItem(const char* id) const
{
	return m_root->getItemById(id);
}

int ssd::Menu::draw(
	uint32_t xInPix,
	uint32_t yInPix,
	uint32_t wInPix,
	uint32_t hInPix
)
{
	ImGuiWindowFlags window_flags = 0;
	
	window_flags |= ImGuiWindowFlags_NoTitleBar;
	//window_flags |= ImGuiWindowFlags_NoScrollbar;
	//window_flags |= ImGuiWindowFlags_MenuBar;
	//window_flags |= ImGuiWindowFlags_NoMove;
	//window_flags |= ImGuiWindowFlags_NoResize;
	//window_flags |= ImGuiWindowFlags_NoCollapse;
	//window_flags |= ImGuiWindowFlags_NoNav;
	//window_flags |= ImGuiWindowFlags_NoBackground;
	//window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus;

	ImGui::SetNextWindowPos(ImVec2(xInPix, yInPix), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(wInPix, hInPix), ImGuiCond_FirstUseEver);
	ImGui::PushFont(ImGui_ImplSampleUtil_defaultFont);
	ImGui::Begin("Debug Menu", nullptr, window_flags);
	float scale = m_setting.fontHeight / (float)ImGui_ImplSampleUtil_defaultFontSize;
	ImGui::SetWindowFontScale(scale);
	ImGui::PushStyleColor(ImGuiCol_NavHighlight, m_setting.cursorItemColor);
	ImGui::PushStyleColor(ImGuiCol_WindowBg, m_setting.backgroundColor);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8.f * scale, m_setting.lineHeight - m_setting.fontHeight));
	ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, (float)m_setting.indent);

	m_root->update();

	ImGui::PopStyleColor();
	ImGui::PopStyleColor();
	ImGui::PopStyleVar();
	ImGui::PopStyleVar();

	ImGui::End();
	ImGui::PopFont();

	return SCE_OK;
}


void ssd::Menu::setColor(sce::Vectormath::Simd::Aos::Vector4_arg cursorItemColorRgba,
	sce::Vectormath::Simd::Aos::Vector4_arg otherItemColorRgba,
	sce::Vectormath::Simd::Aos::Vector4_arg backgroundColorRgba
	)
{
	m_setting.cursorItemColor.x = cursorItemColorRgba.getX();
	m_setting.cursorItemColor.y = cursorItemColorRgba.getY();
	m_setting.cursorItemColor.z = cursorItemColorRgba.getZ();
	m_setting.cursorItemColor.w = cursorItemColorRgba.getW();
	m_setting.otherItemColor.x = otherItemColorRgba.getX();
	m_setting.otherItemColor.y = otherItemColorRgba.getY();
	m_setting.otherItemColor.z = otherItemColorRgba.getZ();
	m_setting.otherItemColor.w = otherItemColorRgba.getW();
	m_setting.cursorItemColorInVec = cursorItemColorRgba;
	m_setting.otherItemColorInVec  = otherItemColorRgba;
	m_setting.backgroundColor.x  = backgroundColorRgba.getX();
	m_setting.backgroundColor.y  = backgroundColorRgba.getY();
	m_setting.backgroundColor.z  = backgroundColorRgba.getZ();
	m_setting.backgroundColor.w  = backgroundColorRgba.getW();
}

void ssd::Menu::setFontHeightInPix(uint32_t height)
{
	m_setting.fontHeight = height;
}
void ssd::Menu::setLineHeightInPix(uint32_t height)
{
	m_setting.lineHeight = height;
}
void ssd::Menu::setIndentInPix(uint32_t indent)
{
	m_setting.indent = indent;
}

void ssd::Menu::onGroupExpansionChange(ssd::MenuItemGroup *group, bool expand)
{
	static_cast<ssd::MenuItemGroupImpl*>(group)->doExpand(expand);
}
