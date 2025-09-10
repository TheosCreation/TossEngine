/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_TRI_MESH_H
#define _SCE_PFX_TRI_MESH_H

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/base/pfx_simd_utils.h"
#include "../../../include/physics_effects/base_level/collision/pfx_aabb.h"

//J メッシュのリソース制限
//E Define limitations of a triangle mesh
#define SCE_PFX_NUMMESHFACETS		128
#define SCE_PFX_NUMMESHEDGES		192
#define SCE_PFX_NUMMESHVERTICES		192

//J エッジの角
//E Edge types
#define SCE_PFX_EDGE_FLAT    0
#define SCE_PFX_EDGE_CONVEX  1
#define SCE_PFX_EDGE_CONCAVE 2

#define SCE_PFX_QUANTIZE_MAX 65535.0f

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// PfxQuantize

struct PfxQuantize
{
	PfxInt16 elem;

	PfxQuantize() {}

	PfxQuantize(PfxInt16 value)
	{
		elem = value;
	}
};

struct PfxQuantize2
{
	PfxInt16 elem[2];

	PfxQuantize2() {}

	PfxQuantize2(PfxInt16 value1,PfxInt16 value2)
	{
		elem[0] = value1;
		elem[1] = value2;
	}
};

struct PfxQuantize3
{
	PfxInt16 elem[3];

	PfxQuantize3() {}

	PfxQuantize3(PfxInt16 value1,PfxInt16 value2,PfxInt16 value3)
	{
		elem[0] = value1;
		elem[1] = value2;
		elem[2] = value3;
	}
};

static SCE_PFX_FORCE_INLINE
PfxQuantize3 pfxConvertToQuantize3(const PfxVector3 &vec)
{
#ifdef PFX_ENABLE_AVX
	union {
		__m128i i128;
		PfxInt32 i[4];
	} vi;
	vi.i128 = _mm_cvttps_epi32(sce_vectormath_asm128(vec.get128()));
	return PfxQuantize3((PfxInt16)vi.i[0], (PfxInt16)vi.i[1], (PfxInt16)vi.i[2]);
#else
	return PfxQuantize3((PfxInt16)vec[0], (PfxInt16)vec[1], (PfxInt16)vec[2]);
#endif
}

///////////////////////////////////////////////////////////////////////////////
// PfxFacetBvhNode

struct PfxFacetBvhNode {
	// bit0 Left is leaf
	// bit1 Left is empty
	// bit2 Right is leaf
	// bit3 Right is empty
	PfxUInt8 flag;
	PfxUInt8 left,right;
	PfxUInt8 aabb[6]; // xmin,xmax,ymin,ymax,zmin,zmax

	PfxFacetBvhNode()
	{
		aabb[0] = 0;
		aabb[1] = 0;
		aabb[2] = 0;
		aabb[3] = 0;
		aabb[4] = 0;
		aabb[5] = 0;
		flag = left = right = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
// PfxFloat3

class PfxFloat3
{
private:
	PfxFloat m_v[3];

public:
	PfxFloat3() {m_v[0] = m_v[1] = m_v[2] = 0.0f;}
	PfxFloat3(const PfxVector3 & vec) {pfxStoreVector3(vec,(PfxFloat*)m_v);}
	PfxFloat3(PfxFloat i0,PfxFloat i1,PfxFloat i2) {m_v[0]=i0; m_v[1] = i1; m_v[2] = i2;}
	PfxFloat3(PfxFloat iv) {m_v[0] = m_v[1] = m_v[2] = iv;}

	inline PfxFloat3 &operator =(const PfxFloat3 &vec)
	{
		m_v[0] = vec.m_v[0];
		m_v[1] = vec.m_v[1];
		m_v[2] = vec.m_v[2];
		return *this;
	}

	inline PfxFloat &operator [](const PfxInt32 i)
	{
		return m_v[i];
	}

	inline PfxFloat operator [](const PfxInt32 i) const
	{
		return PfxFloat(m_v[i]);
	}

	inline operator PfxVector3() const
	{
		return pfxReadVector3(m_v);
	}
};

///////////////////////////////////////////////////////////////////////////////
// Edge

struct PfxEdge
{
	PfxUInt8 m_vertId[2];
	PfxUInt8 m_angleType;
	PfxUInt8 m_tilt;
};

inline
bool operator ==(const PfxEdge &e1,const PfxEdge &e2)
{
	return  (e1.m_vertId[0] == e2.m_vertId[0] && e1.m_vertId[1] == e2.m_vertId[1]) ||
			(e1.m_vertId[1] == e2.m_vertId[0] && e1.m_vertId[0] == e2.m_vertId[1]);
}

///////////////////////////////////////////////////////////////////////////////
// Facet

struct PfxExpandedFacet
{
	PfxFloat3 m_normal;
	PfxFloat3 m_half;
	PfxFloat3 m_center;
	PfxFloat m_thickness;
	PfxUInt8 m_vertIds[3];
	PfxUInt8 m_edgeIds[3];
	SCE_PFX_PADDING(1,2)
	PfxUInt32 m_userData;
};

struct PfxQuantizedFacetBvh
{
	PfxQuantize2 m_normal;
	PfxQuantize m_thickness;
	PfxUInt8 m_vertIds[3];
	PfxUInt8 m_edgeIds[3];
	PfxUInt32 m_userData;
};

struct PfxCompressedFacet2
{
	PfxUInt16 m_edgeInfo; // 0x0000-0x03FF : edge type (2bits x5)
	PfxUInt16 m_facetInfo; // 0x0000-0x003F : offset facetId(6bits x1), 0x8000 : 0 = signle 1 = double
	PfxUInt8 m_vertIds[4];
	PfxUInt32 m_userData[2];
};

///////////////////////////////////////////////////////////////////////////////
// TriMesh

struct PfxExpandedTriMesh
{
	PfxUInt8 m_numVerts;
	PfxUInt8 m_numEdges;
	PfxUInt8 m_numFacets;
	PfxUInt8 m_reserved;
	PfxEdge *m_edges;
	PfxExpandedFacet *m_facets;
	PfxFloat3 *m_verts;
};

struct PfxQuantizedTriMeshBvh
{
	PfxUInt8 m_numVerts;
	PfxUInt8 m_numEdges;
	PfxUInt8 m_numFacets;
	PfxUInt8 m_reserved;
	PfxFloat m_aabbMin[3];
	PfxFloat m_aabbMax[3];
	PfxEdge  *m_edges;
	PfxQuantizedFacetBvh *m_facets;
	PfxQuantize3 *m_verts;
	PfxFacetBvhNode *m_bvhNodes;
};

struct PfxCompressedTriMesh
{
	PfxUInt8 m_numVerts;
	PfxUInt8 m_numEdges;
	PfxUInt8 m_numFacets;
	PfxUInt8 m_reserved;
	PfxFloat m_aabbMin[3];
	PfxFloat m_aabbMax[3];
	PfxUInt32 m_facets;
	PfxUInt32 m_verts;
	PfxUInt32 m_bvhNodes;
};

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_TRI_MESH_H
