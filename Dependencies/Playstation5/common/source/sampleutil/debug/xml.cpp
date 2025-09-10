/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */

#include <cstdlib>
#include <algorithm>
#include <libsysmodule.h>
#include "sampleutil/sampleutil_common.h"
#include "sampleutil/sampleutil_error.h"
#include "xml_internal.h"

#pragma comment(lib, "libSceXml_stub_weak.a")

namespace sce { namespace SampleUtil { namespace Debug {
	XmlDocumentParser	*XmlDocumentParser::m_instance = nullptr;

	class String : public sce::Xml::String
	{
	public:
		String(const char	*name = nullptr)
		{
			buffer = name;
			size = name ? strlen(name) : 0;
		}

		String(const String	&in)
		{
			buffer = in.buffer;
			size = in.size;
		}

		String	&operator=(const sce::Xml::String	&in)
		{
			buffer = in.buffer;
			size = in.size;

			return *this;
		}

		const char	*c_str() const
		{
			return (isAvailable()) ? buffer : nullptr;
		}
	};

	// 指定のnodeIdを持つものがリストにあるかを検索し、なければ作ってリストに追加する
	XmlElement	*XmlElement::createChildIfNotBeInList(sce::Xml::Dom::NodeId	node)
	{
		auto it = std::find_if(m_children.begin(), m_children.end(),
			[&node](XmlElement*& p)	{ return (p->m_nodeId == node); }
		);

		XmlElement	*element = nullptr;

		if (it == m_children.end())
		{
			element = new XmlElement(m_doc, node);
			m_children.push_back(element);
		} else {
			element = *it;
		}
		SCE_SAMPLE_UTIL_ASSERT(element != nullptr);

		return	element;
	}

	XmlElement	*XmlElement::getNextSiblingElementForSpecifiedNode(sce::Xml::Dom::NodeId node, const char	*name)
	{
		// Elementノードを探す
		while (node != sce::Xml::Dom::invalidNodeId)
		{
			sce::Xml::Dom::NodeType nodeType = m_doc.getNodeType(node);
			bool isNameOk = true;

			// 名前で探す場合
			if (name != nullptr)
			{
				isNameOk = false;

				String	nodeName;
				nodeName = m_doc.getNodeName(node);
				if (nodeName.isAvailable())
				{
					std::string	strNodeName = nodeName.c_str();
					isNameOk = (strNodeName == name);
				}
			}
			if (nodeType == sce::Xml::Dom::nodeElement && isNameOk)
			{

				// 見つかったらリストに加える
				auto	*child = createChildIfNotBeInList(node);

				return child;

			}
			node = m_doc.getSibling(node);
		}

		return nullptr;
	}

	XmlElement::XmlElement(const sce::Xml::Dom::Document	doc, sce::Xml::Dom::NodeId	id)
		: m_doc(doc), m_nodeId(id)
	{
		if (m_nodeId != sce::Xml::Dom::invalidNodeId)
		{
			// ノードタイプがElementでない場合はノードIDを無効値に設定
			if (m_doc.getNodeType(id) != sce::Xml::Dom::nodeElement)
			{
				m_nodeId = sce::Xml::Dom::invalidNodeId;
			}
		}
	}

	XmlElement::~XmlElement()
	{
		for (auto *pChild : m_children)
		{
			if (pChild != nullptr)
			{
				delete pChild;
			}
		}
	}

	const char	*XmlElement::Value()
	{
		String	name;
		name = m_doc.getNodeName(m_nodeId);

		return	name.c_str();
	}

	const char	*XmlElement::Attribute(const char	*name)
	{
		String	attrName(name);
		String	attrValue;
		attrValue = m_doc.getAttribute(m_nodeId, &attrName);

		return	attrValue.c_str();
	}

	XmlElement	*XmlElement::getFirstChildElement(const char	*name)
	{
		sce::Xml::Dom::NodeId nodeChild = m_doc.getFirstChild(m_nodeId);

		return getNextSiblingElementForSpecifiedNode(nodeChild, name);
	}

	XmlElement	*XmlElement::getNextSiblingElement(const char	*name)
	{
		sce::Xml::Dom::NodeId nodeSibling = m_doc.getSibling(m_nodeId);

		return getNextSiblingElementForSpecifiedNode(nodeSibling, name);
	}

	void	*XmlAllocator::allocate(size_t	size, void	*user_data)
	{
		(void)user_data;
		return malloc(size);
	}

	void	XmlAllocator::deallocate(void	*ptr, void	*user_data)
	{
		(void)user_data;
		free(ptr);
	}

	XmlAllocator	*XmlAllocator::create()
	{
		return new XmlAllocator;
	}

	void	XmlAllocator::destory(XmlAllocator	*p)
	{
		delete p;
	}

	int	XmlDocumentParser::create()
	{
		if (m_instance == nullptr)
		{
			m_instance = new XmlDocumentParser();
			SCE_SAMPLE_UTIL_ASSERT(m_instance != nullptr);
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}

		return SCE_OK;
	}

	int XmlDocumentParser::destroy()
	{
		if (m_instance != nullptr)
		{
			delete m_instance;
			m_instance = nullptr;
		}

		return SCE_OK;
	}

	int	XmlDocumentParser::parse(const char	*stringBuffer)
	{
		int ret = SCE_OK; (void)ret;

		sce::Xml::XmlText	xmlText;
		xmlText.buffer	= stringBuffer;
		xmlText.size	= strlen(stringBuffer);
		ret = m_docBuilder->parse(&xmlText, true);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
		if (ret != SCE_OK)
		{
			return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
		}

		// 前回のパース結果のルートは捨てる
		if (m_rootElem != nullptr)
		{
			delete m_rootElem;
			m_rootElem = nullptr;
		}

		// パース成功したらルートエレメントを生成する
		sce::Xml::Dom::Document doc = m_docBuilder->getDocument();
		m_rootElem = new XmlElement(doc, doc.getRoot());
		SCE_SAMPLE_UTIL_ASSERT(m_rootElem != nullptr);
		if (m_rootElem == nullptr)
		{
			return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
		}

		return SCE_OK;
	}

	XmlDocumentParser::XmlDocumentParser()
		: m_allocator	(nullptr)
		, m_initializer	(nullptr)
		, m_docBuilder	(nullptr)
		, m_rootElem	(nullptr)
	{
		int ret = SCE_OK; (void)ret;

		sceSysmoduleLoadModule(SCE_SYSMODULE_XML);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_allocator = XmlAllocator::create();
		SCE_SAMPLE_UTIL_ASSERT(m_allocator != nullptr);

		sce::Xml::InitParameter initParam;
		initParam.allocator = m_allocator;
		initParam.userData = (void*)0xDeadBeef;

		m_initializer = new sce::Xml::Initializer;
		SCE_SAMPLE_UTIL_ASSERT(m_initializer != nullptr);

		ret = m_initializer->initialize(&initParam);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_docBuilder = new sce::Xml::Dom::DocumentBuilder;
		SCE_SAMPLE_UTIL_ASSERT(m_docBuilder != nullptr);

		ret = m_docBuilder->initialize(m_initializer);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);

		m_docBuilder->setSkipIgnorableWhiteSpace(true);
		m_docBuilder->setSkipIgnorableText(true);
		m_docBuilder->setResolveEntity(true);
	}

	XmlDocumentParser::~XmlDocumentParser()
	{
		if (m_rootElem != nullptr)
		{
			delete m_rootElem;
			m_rootElem = nullptr;
		}
		delete m_docBuilder;
		m_docBuilder = nullptr;

		delete m_initializer;
		m_initializer = nullptr;

		XmlAllocator::destory(m_allocator);
		m_allocator = nullptr;

		int ret = sceSysmoduleUnloadModule(SCE_SYSMODULE_XML);
		SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	}
}}}