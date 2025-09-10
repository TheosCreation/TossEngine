/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_SUB_DATA_H
#define _SCE_PFX_SUB_DATA_H

namespace sce {
namespace pfxv4 {

/// @brief Contact sub data
struct SCE_PFX_API PfxSubData {
	enum {
		NONE = 0,
		SHAPE_INFO,
		MESH_INFO,
	};
	
	union {
		PfxUInt32 param32[4];
		PfxUInt16 param16[8];
		PfxUInt8 param8[16];
	};

	struct SCE_PFX_API PfxShapeInfo {
		PfxUInt32 shapeId;
		PfxUInt32 userData;
	};
	
	struct SCE_PFX_API PfxMeshInfo {
		PfxUInt32 shapeId;
		PfxUInt16 islandId;
		PfxUInt8 facetId;
		PfxUInt32 userData;
		PfxFloat s, t;
	};

	/// @brief Default constructor (clear parameters)
	PfxSubData()
	{
		param32[0] = param32[1] = param32[2] = 0;
	}
	
	/// @brief Construct Sub Data from the shape information
	PfxSubData(PfxUInt8 type_,PfxUInt32 userData_,PfxUInt32 shapeId_)
	{
		param32[0] = param32[1] = param32[2] = 0;
		setType(type_);
		setUserData(userData_);
		setShapeId(shapeId_);
	}
	
	/// @brief Set type of data
	void setType(PfxUInt8 t) {param8[0] = (param8[0] & 0xf8) | t;}

	/// @brief Get type of data
	PfxUInt8 getType() const {return param8[0] & 0x07;}

	/// @brief Set flag (4 bits)
	void setCcdPriority(PfxUInt8 f) { param8[0] = (1 << 7) | (f << 3) | (param8[0] & 0x07); }

	/// @brief Get flag (4 bits)
	PfxUInt8 getCcdPriority() const { return param8[0] >> 3; }
	
	/// @brief Set island Id of the large mesh
	void setIslandId(PfxUInt16 i) {param16[1] = i;}

	/// @brief Set facet Id
	void setFacetId(PfxUInt8 i) {param8[1] = i;}

	/// @brief Set facet local position S
	void setFacetLocalS(PfxFloat s) {param16[4] = (PfxUInt16)(SCE_PFX_CLAMP(s,0.0f,1.0f) * 65535.0f);}

	/// @brief Set facet local position T
	void setFacetLocalT(PfxFloat t) {param16[5] = (PfxUInt16)(SCE_PFX_CLAMP(t,0.0f,1.0f) * 65535.0f);}
	
	/// @brief Set user data of the facet
	void setUserData(PfxUInt32 data) {param32[3] = data;}

	/// @brief Get island Id of the large mesh
	PfxUInt16 getIslandId() const {return param16[1];}

	/// @brief Get facet Id
	PfxUInt8 getFacetId() const {return param8[1];}

	/// @brief Get facet local position S
	PfxFloat getFacetLocalS() const {return param16[4] / 65535.0f;}

	/// @brief Get facet local position S
	PfxFloat getFacetLocalT() const {return param16[5] / 65535.0f;}

	/// @brief Get user data of the facet
	PfxUInt32 getUserData() const {return param32[3];}

	/// @brief Set shape Id
	void setShapeId(PfxUInt32 id) {param32[1] = id;}
	
	/// @brief Get shape Id
	PfxUInt32 getShapeId() const {return param32[1];}
	
	/// @brief Return gathered shape info
	PfxBool getShapeInfo(PfxShapeInfo &info) const
	{
		if(getType() == SHAPE_INFO) {
			info.shapeId = getShapeId();
			info.userData = getUserData();
			return true;
		}
		return false;
	}
	
	/// @brief Return gathered mesh info
	PfxBool getMeshInfo(PfxMeshInfo &info) const
	{
		if(getType() == MESH_INFO) {
			info.shapeId = getShapeId();
			info.islandId = getIslandId();
			info.facetId = getFacetId();
			info.s = getFacetLocalS();
			info.t = getFacetLocalT();
			info.userData = getUserData();
			return true;
		}
		return false;
	}
};


} //namespace pfxv4
} //namespace sce
#endif // _SCE_PFX_SUB_DATA_H
