/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <sampleutil/graphics/shader_common.h>
#include <sampleutil/graphics/vertex_define.h>

namespace sce { namespace SampleUtil { namespace Graphics {
	//! This is constant user data
	struct MeshInstanceUserData
	{
		Matrix4x3Unaligned	m_transform;
		uint				m_materialIndex;
		uint				m_skeletonIndex;
		uint				m_bonesPerVertex;
		uint				__padding__;
	};

	//! This is per-frame userdata
	struct PackModelInstanceUserData
	{
		Matrix4x3Unaligned								m_transform;
		RegularBuffer<RegularBuffer<Matrix4Unaligned>>	m_skinningMatricesArray;
	};
}}} // namespace sce::SampleUtil::Graphics