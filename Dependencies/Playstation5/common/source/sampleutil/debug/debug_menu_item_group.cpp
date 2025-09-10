/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include "debug_menu_internal.h"

namespace ssg  = sce::SampleUtil::Graphics;
namespace ssd  = sce::SampleUtil::Debug;
namespace ssi  = sce::SampleUtil::Input;
namespace vm   = sce::Vectormath::Simd::Aos;

ssd::MenuItemGroupImpl::MenuItemGroupImpl(const char* id, const char *label, bool expanded, ssd::Menu *menu)
:  ValuedMenuItemCore<bool, MenuItemGroup>(id, label)
{

	m_isExpanded = expanded;
	SCE_SAMPLE_UTIL_ASSERT(menu != nullptr);
	m_menu = menu;
}

int ssd::MenuItemGroupImpl::initialize(Type type, float indent_w)
{
	bool init_value = true;
	int ret = ValuedMenuItemCore<bool, MenuItemGroup>::initialize(this, &init_value, type, nullptr);

	m_indentW = indent_w;

	return ret;
}

ssd::MenuItemGroupImpl::~MenuItemGroupImpl()
{
	for(uint32_t i=0; i<m_items.size(); i++){
		AbstractMenuItem* item = m_items.at(i);
		delete (item);
	}
	m_items.clear();
}

ssd::AbstractMenuItem* ssd::MenuItemGroupImpl::getItemById(const char* _id) const
{
	if(_id == nullptr){
		return nullptr;
	}
	std::string id(_id);
	for(uint32_t i=0; i<m_items.size(); i++){
		ssd::AbstractMenuItem* item = m_items.at(i);
		if(std::string(item->getId()) == id){
			return item;
		}
		if(item->getType() >= TYPE_HAS_CHILD_ITEMS) {
			ssd::MenuItemGroupImpl *g = static_cast<ssd::MenuItemGroupImpl*>(item);
			ssd::AbstractMenuItem* r = g->getItemById(_id);
			if (r != nullptr) {
				return r;
			}
		}

	}

	return nullptr;
}

ssd::AbstractMenuItem* ssd::MenuItemGroupImpl::getItemByIndex(uint32_t index) const
{
	if(index < m_items.size()){
		return m_items.at(index);
	} else {
		return nullptr;
	}
}

uint32_t          ssd::MenuItemGroupImpl::getNumItems() const
{
	return m_items.size();
}

void ssd::MenuItemGroupImpl::deleteItemById(const char* _id)
{
	if(_id == nullptr){
		_id = "";
	}
	std::string id(_id);
	std::vector<ssd::AbstractMenuItem*>::iterator it;
	for(it = m_items.begin(); it != m_items.end(); it++){
		ssd::AbstractMenuItem* item = *it;
		if(std::string(item->getId()) == id){
			delete item;
			m_items.erase(it);
			updateStringExp();

			return;
		}
	}
}

int ssd::MenuItemGroupImpl::deleteItemByIndex(uint32_t index)
{
	if( index < m_items.size() ){
		ssd::AbstractMenuItem* item = m_items.at(index);
		delete item;
		m_items.erase (m_items.begin()+index);
		updateStringExp();

		return SCE_OK;
	} else {
		return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
	}
}

ssd::Int8MenuItem *ssd::MenuItemGroupImpl::addInt8Menuitem(const char* id, const char* label, int8_t value, int8_t step, int8_t minValue, int8_t maxValue, int position, const char *format)
{
	Int8MenuItemImpl *item = new ssd::Int8MenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int16MenuItem *ssd::MenuItemGroupImpl::addInt16Menuitem(const char* id, const char* label, int16_t value, int16_t step, int16_t minValue, int16_t maxValue, int position, const char *format)
{
	Int16MenuItemImpl *item = new ssd::Int16MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32MenuItem *ssd::MenuItemGroupImpl::addInt32Menuitem(const char* id, const char* label, int32_t value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char *format)
{
	Int32MenuItemImpl *item = new ssd::Int32MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32v2MenuItem * ssd::MenuItemGroupImpl::addInt32v2Menuitem(const char * id, const char * label, int32_t * value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	Int32v2MenuItemImpl *item = new ssd::Int32v2MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32v3MenuItem * ssd::MenuItemGroupImpl::addInt32v3Menuitem(const char * id, const char * label, int32_t * value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	Int32v3MenuItemImpl *item = new ssd::Int32v3MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32v4MenuItem * ssd::MenuItemGroupImpl::addInt32v4Menuitem(const char * id, const char * label, int32_t * value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	Int32v4MenuItemImpl *item = new ssd::Int32v4MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32MenuItem * ssd::MenuItemGroupImpl::addSliderInt32Menuitem(const char * id, const char * label, int32_t value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	SliderIntMenuItemImpl *item = new ssd::SliderIntMenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32v2MenuItem * ssd::MenuItemGroupImpl::addSliderInt32v2Menuitem(const char * id, const char * label, int32_t * value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	SliderInt32v2MenuItemImpl *item = new ssd::SliderInt32v2MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32v3MenuItem * ssd::MenuItemGroupImpl::addSliderInt32v3Menuitem(const char * id, const char * label, int32_t * value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	SliderInt32v3MenuItemImpl *item = new ssd::SliderInt32v3MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int32v4MenuItem * ssd::MenuItemGroupImpl::addSliderInt32v4Menuitem(const char * id, const char * label, int32_t * value, int32_t step, int32_t minValue, int32_t maxValue, int position, const char * format)
{
	SliderInt32v4MenuItemImpl *item = new ssd::SliderInt32v4MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Int64MenuItem *ssd::MenuItemGroupImpl::addInt64Menuitem(const char* id, const char* label, int64_t value, int64_t step, int64_t minValue, int64_t maxValue, int position, const char *format)
{
	Int64MenuItemImpl *item = new ssd::Int64MenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Uint8MenuItem *ssd::MenuItemGroupImpl::addUint8Menuitem(const char* id, const char* label, uint8_t value, uint8_t step, uint8_t minValue, uint8_t maxValue, int position, const char *format)
{
	Uint8MenuItemImpl *item = new ssd::Uint8MenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Uint16MenuItem *ssd::MenuItemGroupImpl::addUint16Menuitem(const char* id, const char* label, uint16_t value, uint16_t step, uint16_t minValue, uint16_t maxValue, int position, const char *format)
{
	Uint16MenuItemImpl *item = new ssd::Uint16MenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Uint32MenuItem *ssd::MenuItemGroupImpl::addUint32Menuitem(const char* id, const char* label, uint32_t value, uint32_t step, uint32_t minValue, uint32_t maxValue, int position, const char *format)
{
	Uint32MenuItemImpl *item = new ssd::Uint32MenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Uint64MenuItem *ssd::MenuItemGroupImpl::addUint64Menuitem(const char* id, const char* label, uint64_t value, uint64_t step, uint64_t minValue, uint64_t maxValue, int position, const char *format)
{
	Uint64MenuItemImpl *item = new ssd::Uint64MenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::FloatMenuItem *ssd::MenuItemGroupImpl::addFloatMenuitem(const char* id, const char* label, float value, float step, float minValue, float maxValue, int position, const char *format)
{
	FloatMenuItemImpl *item = new ssd::FloatMenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float2MenuItem * ssd::MenuItemGroupImpl::addFloat2Menuitem(const char * id, const char * label, float * value, float step, float minValue, float maxValue, int position, const char * format)
{
	Float2MenuItemImpl *item = new ssd::Float2MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return (Float2MenuItem *)item;
	}
}

ssd::Float3MenuItem * ssd::MenuItemGroupImpl::addFloat3Menuitem(const char * id, const char * label, float * value, float step, float minValue, float maxValue, int position, const char * format)
{
	Float3MenuItemImpl *item = new ssd::Float3MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float4MenuItem * ssd::MenuItemGroupImpl::addFloat4Menuitem(const char * id, const char * label, float * value, float step, float minValue, float maxValue, int position, const char * format)
{
	Float4MenuItemImpl *item = new ssd::Float4MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::FloatMenuItem * ssd::MenuItemGroupImpl::addSliderFloatMenuitem(const char * id, const char * label, float value, float step, float minValue, float maxValue, int position, const char * format)
{
	SliderFloatMenuItemImpl *item = new ssd::SliderFloatMenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float2MenuItem * ssd::MenuItemGroupImpl::addSliderFloat2Menuitem(const char * id, const char * label, float * value, float step, float minValue, float maxValue, int position, const char * format)
{
	SliderFloat2MenuItemImpl *item = new ssd::SliderFloat2MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float3MenuItem * ssd::MenuItemGroupImpl::addSliderFloat3Menuitem(const char * id, const char * label, float * value, float step, float minValue, float maxValue, int position, const char * format)
{
	SliderFloat3MenuItemImpl *item = new ssd::SliderFloat3MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float4MenuItem * ssd::MenuItemGroupImpl::addSliderFloat4Menuitem(const char * id, const char * label, float * value, float step, float minValue, float maxValue, int position, const char * format)
{
	SliderFloat4MenuItemImpl *item = new ssd::SliderFloat4MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(step, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float3MenuItem * ssd::MenuItemGroupImpl::addColorEditFloat3Menuitem(const char * id, const char * label, float * value, float minValue, float maxValue, int position, const char * format)
{
	ColorEditFloat3MenuItemImpl *item = new ssd::ColorEditFloat3MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(0, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float4MenuItem * ssd::MenuItemGroupImpl::addColorEditFloat4Menuitem(const char * id, const char * label, float * value, float minValue, float maxValue, int position, const char * format)
{
	ColorEditFloat4MenuItemImpl *item = new ssd::ColorEditFloat4MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(0, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float3MenuItem * ssd::MenuItemGroupImpl::addColorPickerFloat3Menuitem(const char * id, const char * label, float * value, float minValue, float maxValue, int position, const char * format)
{
	ColorPickerFloat3MenuItemImpl *item = new ssd::ColorPickerFloat3MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(0, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::Float4MenuItem * ssd::MenuItemGroupImpl::addColorPickerFloat4Menuitem(const char * id, const char * label, float * value, float minValue, float maxValue, int position, const char * format)
{
	ColorPickerFloat4MenuItemImpl *item = new ssd::ColorPickerFloat4MenuItemImpl(id, label);

	if (SCE_OK != item->initialize(0, minValue, maxValue, value, format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::DoubleMenuItem *ssd::MenuItemGroupImpl::addDoubleMenuitem(const char* id, const char* label, double value, double step, double minValue, double maxValue, int position, const char *format)
{
	DoubleMenuItemImpl *item = new ssd::DoubleMenuItemImpl(id, label);

	if(SCE_OK != item->initialize(step, minValue, maxValue, value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::BoolMenuItem *ssd::MenuItemGroupImpl::addBoolMenuitem(const char* id, const char* label, bool value, int position, const char *format)
{
	BoolMenuItemImpl *item = new ssd::BoolMenuItemImpl(id, label);

	if(SCE_OK != item->initialize(value, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::BoolMenuItem *ssd::MenuItemGroupImpl::addButtonMenuitem(const char* id, const char* label, int32_t width, int32_t height, int position, const char *format)
{
	ButtonMenuItemImpl *item = new ssd::ButtonMenuItemImpl(id, label, ImVec2(width, height));

	if(SCE_OK != item->initialize(false, format)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::EnumMenuItem *ssd::MenuItemGroupImpl::addEnumMenuitem(const char* id, const char* label, 
	const char *enumLabels[],
	uint32_t numLabels,
	int value, int position, const char *format)
{
	SCE_SAMPLE_UTIL_ASSERT(enumLabels != nullptr);
	EnumMenuItemImpl *item = new ssd::EnumMenuItemImpl(id, label);

	std::vector<std::string> labels;
	for(uint32_t i=0; i<numLabels; i++){
		labels.push_back(enumLabels[i]);
	}
	if(SCE_OK != item->initialize(&labels, value, format)){
		delete item;
		return nullptr;
	} else {
		insertItem(item, position);
		return item;
	}	
}

ssd::EnumMenuItem * ssd::MenuItemGroupImpl::addListBoxMenuitem(const char * id, const char * label, const char * enumLabels[], uint32_t numLabels, int value, int position, const char * format)
{
	SCE_SAMPLE_UTIL_ASSERT(enumLabels != nullptr);
	ListBoxMenuItemImpl *item = new ssd::ListBoxMenuItemImpl(id, label);

	std::vector<std::string> labels;
	for (uint32_t i = 0; i < numLabels; i++) {
		labels.push_back(enumLabels[i]);
	}
	if (SCE_OK != item->initialize(&labels, value, format)) {
		delete item;
		return nullptr;
	}
	else {
		insertItem(item, position);
		return item;
	}
}

ssd::EnumMenuItem * ssd::MenuItemGroupImpl::addRadioButtonMenuitem(const char * id, const char * label, const char * enumLabels[], uint32_t numLabels, int value, int position, const char * format)
{
	SCE_SAMPLE_UTIL_ASSERT(enumLabels != nullptr);
	RadioButtonMenuItemImpl *item = new ssd::RadioButtonMenuItemImpl(id, label);

	std::vector<std::string> labels;
	for (uint32_t i = 0; i < numLabels; i++) {
		labels.push_back(enumLabels[i]);
	}
	if (SCE_OK != item->initialize(&labels, value, format)) {
		delete item;
		return nullptr;
	}
	else {
		insertItem(item, position);
		return item;
	}
}

ssd::StringMenuItem * ssd::MenuItemGroupImpl::addInputTextMenuitem(const char * id, const char * label, const char * value, uint32_t col, int position, const char * format)
{
	InputTextMenuItemImpl *item = new ssd::InputTextMenuItemImpl(id, label, col);

	if (SCE_OK != item->initialize(std::string(value), format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::StringMenuItem * ssd::MenuItemGroupImpl::addLabelTextMenuitem(const char * id, const char * label, const char * value, uint32_t col, int position, const char * format)
{
	LabelTextMenuItemImpl *item = new ssd::LabelTextMenuItemImpl(id, label, col);

	if (SCE_OK != item->initialize(std::string(value), format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::StringMenuItem * ssd::MenuItemGroupImpl::addTextMenuitem(const char * id, const char * value, uint32_t col, int position, const char * format)
{
	TextMenuItemImpl *item = new ssd::TextMenuItemImpl(id, "", col);

	if (SCE_OK != item->initialize(std::string(value), format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

ssd::StringMenuItem * ssd::MenuItemGroupImpl::addBulletTextMenuitem(const char * id, const char * value, uint32_t col, int position, const char * format)
{
	BulletTextMenuItemImpl *item = new ssd::BulletTextMenuItemImpl(id, "", col);

	if (SCE_OK != item->initialize(std::string(value), format)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}

void sce::SampleUtil::Debug::MenuItemGroupImpl::addSeparatorMenuitem(int position)
{
	auto *item = new ssd::SeparatorMenuItemImpl();
	if (SCE_OK != item->initialize()) {
		delete item;
	}
	else {
		insertItem((ssd::AbstractMenuItem *)item, position);
	}
}

void sce::SampleUtil::Debug::MenuItemGroupImpl::addNewLineMenuitem(int position)
{
	auto *item = new ssd::NewLineMenuItemImpl();
	if (SCE_OK != item->initialize()) {
		delete item;
	}
	else {
		insertItem((ssd::AbstractMenuItem *)item, position);
	}
}

void sce::SampleUtil::Debug::MenuItemGroupImpl::addSpacingMenuitem(int position)
{
	auto *item = new ssd::SpacingMenuItemImpl();
	if (SCE_OK != item->initialize()) {
		delete item;
	}
	else {
		insertItem((ssd::AbstractMenuItem *)item, position);
	}
}

void sce::SampleUtil::Debug::MenuItemGroupImpl::addSameLineMenuitem(float pos_x, float spacing_w, int position)
{
	auto *item = new ssd::SameLineMenuItemImpl();
	if (SCE_OK != item->initialize(pos_x, spacing_w)) {
		delete item;
	}
	else {
		insertItem((ssd::AbstractMenuItem *)item, position);
	}
}

void sce::SampleUtil::Debug::MenuItemGroupImpl::addDummyMenuitem(float size_x, float size_y, int position)
{
	auto *item = new ssd::DummyMenuItemImpl();
	if (SCE_OK != item->initialize(size_x, size_y)) {
		delete item;
	}
	else {
		insertItem((ssd::AbstractMenuItem *)item, position);
	}
}

ssd::MenuItemGroup *ssd::MenuItemGroupImpl::addMenuItemGroup(const char* id, const char* label, int position, bool expanded)
{
	MenuItemGroupImpl *item = new ssd::MenuItemGroupImpl(id, label, expanded, this->m_menu);

	if(SCE_OK != item->initialize(TYPE_GROUP)){
		delete item;

		return nullptr;
	} else {
		insertItem(item, position);

		return item;
	}
}

ssd::MenuItemGroup * ssd::MenuItemGroupImpl::addMenuItemIndentGroup(const char * id, float indent_w, int position)
{
	MenuItemGroupImpl *item = new ssd::MenuItemGroupImpl(id, "", true, this->m_menu);

	if (SCE_OK != item->initialize(TYPE_INDENT, indent_w)) {
		delete item;

		return nullptr;
	}
	else {
		insertItem(item, position);

		return item;
	}
}



static void	updateLoop(std::vector<ssd::AbstractMenuItem *>	&items)
{
	for (auto *pItem : items)
	{
		if (pItem->isVisible())
		{
			pItem->update();
		}
	}
}

void ssd::MenuItemGroupImpl::update()
{
	if (this->getType() == TYPE_INDENT)
	{
		ImGui::Indent(m_indentW);
		updateLoop(m_items);
		ImGui::Unindent(m_indentW);
	} else {
		if (ImGui::TreeNodeEx(getLabelForImgui(), ImGuiTreeNodeFlags_DefaultOpen))
		{
			updateLoop(m_items);
			ImGui::TreePop();
		}
	}
}


void ssd::MenuItemGroupImpl::insertItem(AbstractMenuItem* item, int position)
{
	if((position < 0) || (position >= (int)m_items.size() )){
		m_items.push_back(item);
	}else{
		std::vector<AbstractMenuItem*>::iterator it = m_items.begin() + position;
		m_items.insert(it, item);
	}
	updateStringExp();
}

void ssd::MenuItemGroupImpl::setExpansion(bool expand)
{
	if (m_isExpanded == expand) {
		return;
	}
	updateStringExp();
	m_menu->onGroupExpansionChange(this, expand);
}

bool ssd::MenuItemGroupImpl::isExpanded()
{
	return m_isExpanded;
}

void ssd::MenuItemGroupImpl::doExpand(bool expand)
{
	m_isExpanded = expand;
}

void ssd::MenuItemGroupImpl::setActive(bool isActive)
{
	this->ValuedMenuItemCore<bool, MenuItemGroup>::setActive(true);
	for (auto i = m_items.begin(); i != m_items.end(); ++i)
	{
		(*i)->setActive(isActive);
	}
}