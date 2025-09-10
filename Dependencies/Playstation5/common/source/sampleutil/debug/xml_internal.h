/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */
#pragma once

#include <vector>
#include <xml.h>

// XMLライブラリクラス
namespace sce {	namespace SampleUtil { namespace Debug {
	class XmlElement
	{
		sce::Xml::Dom::Document		m_doc;
		sce::Xml::Dom::NodeId		m_nodeId;
		std::vector<XmlElement *>	m_children;

		XmlElement	*createChildIfNotBeInList(sce::Xml::Dom::NodeId	node);
		XmlElement	*getNextSiblingElementForSpecifiedNode(sce::Xml::Dom::NodeId	node, const char	*name);
	public:
		XmlElement() :m_nodeId(sce::Xml::Dom::invalidNodeId) {}
		XmlElement(const sce::Xml::Dom::Document	doc, sce::Xml::Dom::NodeId	id);
		virtual	~XmlElement();

		bool	isAvailable()
		{
			return (m_nodeId != sce::Xml::Dom::invalidNodeId);
		}
		const char	*Value();
		const char	*Attribute(const char	*name);
		XmlElement	*getFirstChildElement(const char	*name = nullptr);
		XmlElement	*getNextSiblingElement(const char	*name = nullptr);
	};

	class XmlAllocator : public sce::Xml::MemAllocator
	{
	public:
		virtual void	*allocate(size_t	size, void	*user_data);
		virtual void	deallocate(void	*ptr, void	*user_data);
		static XmlAllocator	*create();
		static void	destory(XmlAllocator	*p);
	};

	class XmlDocumentParser
	{
		static XmlDocumentParser		*m_instance;	// singletonポインタ

		XmlAllocator					*m_allocator;
		sce::Xml::Initializer			*m_initializer;
		sce::Xml::Dom::DocumentBuilder	*m_docBuilder;

		XmlElement						*m_rootElem;

	public:
		static int	create();
		static int	destroy();

		static XmlDocumentParser	*getInstance()
		{
			return	m_instance;
		}

		virtual int parse(const char	*stringBuffer);

		virtual XmlElement	*getRootElem()
		{
			return	m_rootElem;
		}

	protected:
		XmlDocumentParser();
		virtual	~XmlDocumentParser();
	};
}}}
