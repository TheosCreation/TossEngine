/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_LARGE_TRI_MESH_IMPL_H
#define _SCE_PFX_LARGE_TRI_MESH_IMPL_H

#include "pfx_tri_mesh.h"
#include "../../../include/physics_effects/base_level/collision/pfx_large_tri_mesh.h"

#define SCE_PFX_MAX_LARGETRIMESH_STACK	 512

#define SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY		0x00
#define SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH		0x03
#define SCE_PFX_LARGE_MESH_TYPE_HIGH_COMPRESSION	0x07

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Island BVH Node

struct PfxIslandBvhNode {
	// Left is branch(0), leaf(1), empty(2)
	// Right is branch(0), leaf(1), empty(2)
	PfxUInt16 left,right;
	PfxUInt8 flag;
	union {
		PfxUInt8 aabb[8];
		PfxUInt64 i64; 
	};
	
	PfxIslandBvhNode()
	{
		i64 = 0;
		flag = 0;
		left = right = 0;
	}
};

///////////////////////////////////////////////////////////////////////////////
// Large Mesh

struct SCE_PFX_ALIGNED(16) PfxLargeTriMeshImpl
{
	PfxVector3 m_offset;
	PfxVector3 m_half;

	PfxUInt8 m_name[SCE_PFX_LARGETRIMESH_NAME_STR_MAX] = { '\0' };

	//J ラージメッシュの種別を決める
	//E Defines the structure of the large mesh
	PfxUInt32 m_type;

	//J 面の厚み（デフォルト値）
	//E Thickness of facets(Deafult value)
	PfxFloat m_defaultThickness;

	//J ラージメッシュのサイズ
	//E Size of a large mesh
	PfxFloat m_length; //E length of m_half
	
	//J 含まれるアイランドの総数
	//E Number of islands
	PfxUInt16 m_numIslands;

	//J アイランドAABB配列
	//E Array of island AABB
	PfxAabb16 *m_aabbList;
	
	// Number of islands
	PfxUInt32 m_numBvhNodes;

	// BVH tree
	PfxIslandBvhNode *m_bvhNodes;

	//J バッファ
	//E Buffer
	/*
		type                                   |facet                |vertex         |island
		---------------------------------------|---------------------|---------------|------------------------
		SCE_PFX_LARGE_MESH_TYPE_EXPANDED_ARRAY |PfxExpandedFacet     |PfxVector3     |PfxExpandedTriMesh
		SCE_PFX_LARGE_MESH_TYPE_QUANTIZED_BVH  |PfxQuantizedFacetBvh |PfxQuantize3   |PfxQuantizedTriMeshBvh
	 */
	
	void *m_facetBuffer;   // PfxExpandedFacet / PfxQuantizedFacet
	void *m_edgeBuffer;    // PfxEdge
	void *m_vertexBuffer;  // PfxVector3 / PfxQuantize3
	void *m_bvhNodeBuffer; // PfxAabb16
	void *m_islands;	   // PfxExpandedTriMesh / PfxQuantizedTriMeshBvh

	PfxUInt32 m_facetBuffBytes;
	PfxUInt32 m_edgeBuffBytes;
	PfxUInt32 m_vertexBuffBytes;
	PfxUInt32 m_bvhNodeBuffBytes;

	SCE_PFX_PADDING(3,4)

	PfxLargeTriMeshImpl()
	: m_type(0)
	, m_defaultThickness(0.025f)
	, m_numIslands(0)
	, m_aabbList(0)
	, m_numBvhNodes(0)
	, m_bvhNodes(0)
	, m_facetBuffer(0)
	, m_edgeBuffer(0)
	, m_vertexBuffer(0)
	, m_bvhNodeBuffer(0)
	, m_islands(0)
	, m_facetBuffBytes(0)
	, m_edgeBuffBytes(0)
	, m_vertexBuffBytes(0)
	, m_bvhNodeBuffBytes(0)
	{
	}
	
	void setHalf(const PfxVector3 &half) {m_half = half;}
	void setOffset(const PfxVector3 &offset) {m_offset = offset;}

	const PfxVector3 &getHalf() const { return m_half; }
	const PfxVector3 &getOffset() const {return m_offset;}

	//J ワールド座標値をラージメッシュローカルに変換する
	//E Convert a position in the world coordinate into a position in the local coordinate
	inline PfxVecInt3 getLocalPosition(const PfxVector3 &worldPosition) const;
	inline void getLocalPosition(
		const PfxVector3 &worldMinPosition,const PfxVector3 &worldMaxPosition,
		PfxVecInt3 &localMinPosition,PfxVecInt3 &localMaxPosition) const;
	
	//J ラージメッシュローカル座標値をワールドに変換する
	//E Convert a position in the local coordinate into a position in the world coordinate
	inline PfxVector3 getWorldPosition(const PfxVecInt3 &localPosition) const;
	
	//J 量子化
	//E Quantize
	inline PfxQuantize3 quantizePosition(const PfxVector3 &p) const;
	inline PfxQuantize3 quantizeVector(const PfxVector3 &v) const;
	inline PfxQuantize2 quantizeNormal(const PfxVector3 &n) const;
	inline PfxQuantize  quantizeFloat(PfxFloat value) const;
	
	inline PfxVector3 decodePosition(const PfxQuantize3 &q) const;
	inline PfxVector3 decodeVector(const PfxQuantize3 &q) const;
	inline PfxVector3 decodeNormal(const PfxQuantize2 &q) const;
	inline PfxFloat   decodeFloat(PfxQuantize q) const;
	
	inline void quantizeAabbMinMax(
		PfxQuantize3 &qMin,PfxQuantize3 &qMax,const PfxVector3 &pMin,const PfxVector3 &pMax) const;
	
	// do not use. it causes a bad result
	//	inline void quantizeAabbCenterHalf(
	//		PfxQuantize3 &qCenter,PfxQuantize3 &qHalf,const PfxVector3 &pCenter,const PfxVector3 &pHalf) const;
	
	/**
	 * whether the large meshes are operated using BVH or not
	 * 
	 * @return true if using BVH
	 */
	inline PfxBool isUsingBvh() const {return (m_type&0x02)!=0;}
	
	/**
	 * whether the large meshes are compressd by quantized buffers
	 * 
	 * @return true if quantized
	 */
	inline PfxBool isQuantized() const {return (m_type&0x01)!=0;}

	PfxUInt32 getType() const {return m_type;}
};

inline
PfxVecInt3 PfxLargeTriMeshImpl::getLocalPosition(const PfxVector3 &worldPosition) const 
{
	const PfxVector3 offset = getOffset();
	const PfxVector3 half = getHalf();
	const PfxVector3 sz(65535.0f);
	PfxVector3 tmp = divPerElem(worldPosition- offset + half,2.0f*half);
	tmp = mulPerElem(sz,minPerElem(maxPerElem(tmp,PfxVector3::zero()),PfxVector3(1.0f))); // clamp 0.0 - 1.0
	return PfxVecInt3(tmp);
}

inline
void PfxLargeTriMeshImpl::getLocalPosition(
		const PfxVector3 &worldMinPosition,const PfxVector3 &worldMaxPosition,
		PfxVecInt3 &localMinPosition,PfxVecInt3 &localMaxPosition) const
{
	const PfxVector3 offset = getOffset();
	const PfxVector3 half = getHalf();
	const PfxVector3 sz(65535.0f);
	PfxVector3 qmin = divPerElem(worldMinPosition- offset + half,2.0f*half);
	qmin = mulPerElem(sz, clampPerElem(qmin,PfxVector3::zero(),PfxVector3(1.0f)));

	PfxVector3 qmax = divPerElem(worldMaxPosition- offset + half,2.0f*half);
	qmax = mulPerElem(sz, clampPerElem(qmax,PfxVector3::zero(),PfxVector3(1.0f)));

	localMinPosition = PfxVecInt3(pfxFloorPerElem(qmin));
	localMaxPosition = PfxVecInt3(pfxCeilPerElem(qmax));
}

inline
PfxVector3 PfxLargeTriMeshImpl::getWorldPosition(const PfxVecInt3 &localPosition) const 
{
	const PfxVector3 offset = getOffset();
	const PfxVector3 half = getHalf();
	PfxVector3 sz(65535.0f),lp(localPosition);
	PfxVector3 tmp = divPerElem(lp,sz);
	return mulPerElem(tmp,2.0f*half)+ offset - half;
}

inline PfxQuantize3 PfxLargeTriMeshImpl::quantizePosition(const PfxVector3 &p) const
{
	const PfxVector3 offset = getOffset();
	const PfxVector3 half = getHalf();
	const PfxVector3 sz(SCE_PFX_QUANTIZE_MAX);
	PfxVector3 tmp = divPerElem(p- offset,2.0f*half);
	tmp = mulPerElem(sz, clampPerElem(tmp,PfxVector3(-0.5f),PfxVector3(0.5f)));
	return pfxConvertToQuantize3(tmp);
}

inline PfxQuantize3 PfxLargeTriMeshImpl::quantizeVector(const PfxVector3 &v) const
{
	const PfxVector3 half = getHalf();
	const PfxVector3 sz(SCE_PFX_QUANTIZE_MAX);
	PfxVector3 tmp = divPerElem(v,2.0f*half);
	tmp = mulPerElem(sz, clampPerElem(tmp,PfxVector3(-0.5f),PfxVector3(0.5f)));
	return pfxConvertToQuantize3(tmp);
}

inline PfxQuantize2 PfxLargeTriMeshImpl::quantizeNormal(const PfxVector3 &n) const
{
	const PfxFloat PI2 = SCE_PFX_PI * 2.0f;
	PfxFloat r1=0.0f,r2=0.0f;
	PfxVector3 n_ = clampPerElem(n,PfxVector3(-1.0f),PfxVector3(1.0f));
	r1 = acosf(n_[2]); // ０～π
	r2 = atan2f(n_[1],n_[0]); // -π～π
	return PfxQuantize2(
		(PfxInt16)(((r1-0.5f*SCE_PFX_PI)/SCE_PFX_PI)*SCE_PFX_QUANTIZE_MAX),
		(PfxInt16)((r2/PI2)*SCE_PFX_QUANTIZE_MAX));
}

inline PfxQuantize PfxLargeTriMeshImpl::quantizeFloat(PfxFloat value) const
{
	PfxFloat tmp = value / (2.0f*m_length);
	tmp = SCE_PFX_QUANTIZE_MAX * SCE_PFX_CLAMP(tmp,-0.5f,0.5f);
	return PfxQuantize((PfxInt16)tmp);
}

inline PfxVector3 PfxLargeTriMeshImpl::decodePosition(const PfxQuantize3 &q) const
{
	const PfxVector3 offset = getOffset();
	const PfxVector3 half = getHalf();
	const PfxVector3 szInv(1.0f/(PfxFloat)SCE_PFX_QUANTIZE_MAX),lp((PfxFloat)q.elem[0],(PfxFloat)q.elem[1],(PfxFloat)q.elem[2]);
	PfxVector3 tmp = mulPerElem(lp,szInv);
	return mulPerElem(tmp,2.0f*half)+ offset;
}

inline PfxVector3 PfxLargeTriMeshImpl::decodeVector(const PfxQuantize3 &q) const
{
	const PfxVector3 half = getHalf();
	const PfxVector3 szInv(1.0f/(PfxFloat)SCE_PFX_QUANTIZE_MAX),lp((PfxFloat)q.elem[0],(PfxFloat)q.elem[1],(PfxFloat)q.elem[2]);
	PfxVector3 tmp = mulPerElem(lp,szInv);
	return mulPerElem(tmp,2.0f*half);
}

inline PfxVector3 PfxLargeTriMeshImpl::decodeNormal(const PfxQuantize2 &q) const
{
	#ifdef PFX_ENABLE_AVX
	__m128 vr1 = _mm_set1_ps(((q.elem[0]/SCE_PFX_QUANTIZE_MAX) * SCE_PFX_PI) + 0.5f*SCE_PFX_PI);
	__m128 vr2 = _mm_set1_ps((q.elem[1]/SCE_PFX_QUANTIZE_MAX) * SCE_PFX_PI * 2.0f);
	vr1 = _mm_add_ps(vr1,_mm_set_ps(0.0f,0.0f,-SCE_PFX_PI*0.5f,-SCE_PFX_PI*0.5f));
	vr2 = _mm_add_ps(vr2,_mm_set_ps(0.0f,0.0f,-SCE_PFX_PI*0.5f,0.0f));
	((float*)&vr2)[2] = 0.0f;
	return PfxVector3(sce_vectormath_mul(sce_vectormath_cosf4(sce_vectormath_asfloat4(vr1)),sce_vectormath_cosf4(sce_vectormath_asfloat4(vr2))));
	#else
	const PfxFloat PI2 = SCE_PFX_PI * 2.0f;
	PfxFloat rr1=0.0f,rr2=0.0f;
	rr1 = ((q.elem[0]/SCE_PFX_QUANTIZE_MAX)*SCE_PFX_PI) + 0.5f*SCE_PFX_PI;
	rr2 = ((q.elem[1]/SCE_PFX_QUANTIZE_MAX)*PI2);
	return PfxVector3(
		sinf(rr1)*cosf(rr2),
		sinf(rr1)*sinf(rr2),
		cosf(rr1));
	#endif
}

inline PfxFloat PfxLargeTriMeshImpl::decodeFloat(PfxQuantize q) const
{
	PfxFloat tmp = q.elem / SCE_PFX_QUANTIZE_MAX;
	return (tmp * 2.0f * m_length);
}

inline void PfxLargeTriMeshImpl::quantizeAabbMinMax(
	PfxQuantize3 &qMin,PfxQuantize3 &qMax,const PfxVector3 &pMin,const PfxVector3 &pMax) const
{
	const PfxVector3 offset = getOffset();
	const PfxVector3 half = getHalf();
	const PfxVector3 sz(SCE_PFX_QUANTIZE_MAX);
	PfxVector3 tmpMin = divPerElem(pMin- offset,2.0f*half);
	tmpMin = mulPerElem(sz,clampPerElem(tmpMin,PfxVector3(-0.5f),PfxVector3(0.5f)));
	PfxVector3 tmpMax = divPerElem(pMax- offset,2.0f*half);
	tmpMax = mulPerElem(sz,clampPerElem(tmpMax,PfxVector3(-0.5f),PfxVector3(0.5f)));
	tmpMin = clampPerElem(tmpMin,PfxVector3(-32768.0f),PfxVector3(32767.0f)); // clamp -32768 , 32767
	tmpMax = clampPerElem(tmpMax,PfxVector3(-32768.0f),PfxVector3(32767.0f)); // clamp -32768 , 32767
	tmpMin = pfxFloorPerElem(tmpMin);
	tmpMax = pfxCeilPerElem(tmpMax);
	qMin = pfxConvertToQuantize3(tmpMin);
	qMax = pfxConvertToQuantize3(tmpMax);
}

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_LARGE_TRI_MESH_IMPL_H
