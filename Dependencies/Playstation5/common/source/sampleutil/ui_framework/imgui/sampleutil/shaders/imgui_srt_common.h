/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc. 
 * 
 */

#include "sampleutil/graphics/shader_common.h"

namespace sce {	namespace SampleUtil { namespace UIFramework {

struct ImDrawVert
{
	Vector2Unaligned	m_pos;
	Vector2Unaligned	m_uv;
	uint				m_col;
	float				m_depth;
};

struct PerFrameData
{
	Matrix4Unaligned	m_mvp;
	uint				m_hdr;
	RegularBuffer<ImDrawVert>	m_vertex;
};

struct PerDrawData
{
	Texture2D<Vector4Unaligned>	m_texture;
	uint						m_vtxOffset;
};

struct ImguiSrtData
{
	PerFrameData	*m_frameData;
	PerDrawData		*m_drawData;
};

}}}	// sce::SampleUtil::UIFramework
