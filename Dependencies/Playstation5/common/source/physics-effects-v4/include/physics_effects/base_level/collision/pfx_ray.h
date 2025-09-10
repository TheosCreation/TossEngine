/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_RAY_H
#define _SCE_PFX_RAY_H

#include "../base/pfx_common.h"
#include "../base/pfx_large_position.h"
#include "pfx_sub_data.h"

namespace sce {
namespace pfxv4 {


#define SCE_PFX_RAY_FACET_MODE_FRONT_ONLY     0 ///< Ray hits only front surface of a facet
#define SCE_PFX_RAY_FACET_MODE_BACK_ONLY      1 ///< Ray hits only back surface of a facet
#define SCE_PFX_RAY_FACET_MODE_FRONT_AND_BACK 2 ///< Ray hits front and back surface of a facet

/// @brief Structure used when the callback of discarding triangles is called
/// @details The data of triangle passed to (*pfxRayHitDiscardTriangleCallback)().
struct PfxRayHitDiscardTriangleCallbackTriData
{
	PfxVector3 m_vertices[3];	///< @brief Vertices of the triangle
	PfxVector3 m_normal;		///< @brief Normal vector of the triangle
	PfxUInt32 m_userData;		///< @brief User data of the triangle
};

/// @brief Callback for pfxSingleRay()
/// @details This callback is called for all triangles in every PfxConvexMesh and PfxLargeTriMesh shapes.
/// @param triangle Data of the triangle
/// @param localToWorldTransform Transform from local coordinate to world coordinate
/// @param userData User data
/// @return If it returns false, then the triangle will not intersect with the ray. 
typedef PfxBool(*pfxRayHitDiscardTriangleCallback)(const PfxRayHitDiscardTriangleCallbackTriData &triangle, const PfxTransform3 &localToWorldTransform, void *userData);

/// @brief Ray input structure
/// @details This ray structure is expressed with a start point and a direction.
/// It is specified so that the magnitude of the m_direction direction vector is the same as the length of the ray.
/// The contact filter is a filter that allows individual specification of whether to perform judgment of intersection with rigid bodies and shapes.
/// m_facetMode is a flag that specifies whether to take into consideration the front and back of the triangle when judging intersection with a large mesh.
/// For m_facetMode, set one of the following values. If a value other than the following values is specified, intersection judgment is not performed. 
/// SCE_PFX_RAY_FACET_MODE_FRONT_ONLY:Judgment only when ray intersects from front to back
/// SCE_PFX_RAY_FACET_MODE_BACK_ONLY:Judgment only when ray intersects from back to front
/// SCE_PFX_RAY_FACET_MODE_FRONT_AND_BACK:Both front and back used for judgment
struct SCE_PFX_API PfxRayInput
{
	PfxLargePosition m_startPosition;	///< @brief Start position of the ray
	PfxVector3 m_direction;				///< @brief Direction of the ray
	PfxUInt32 m_contactFilterSelf;		///< @brief Self contact filter
	PfxUInt32 m_contactFilterTarget;	///< @brief Target contact filter
	PfxUInt16 m_collisionIgnoreGroup[2];///< @brief Collision ignore group
	PfxUInt8  m_facetMode;				///< @brief Facet mode used for intersection with a large mesh
	SCE_PFX_PADDING3

	/// @brief Reset parameters
	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
		m_collisionIgnoreGroup[0] = m_collisionIgnoreGroup[1] = 0xffff;
		m_facetMode = SCE_PFX_RAY_FACET_MODE_FRONT_ONLY;
	}
};

/// @brief Ray output structure
/// @details This structure receives the output from the raycast.
/// Upon detection of the intersection of a ray and object, m_contactFlag is set to true,
/// the rigid body index is returned to m_objectId, and the index indicating which number the shape
/// is among the rigid body shapes is returned to m_shapeId. Moreover,
/// in the case of intersection with a large mesh, the data indicating which part the intersection occurred at is stored in m_subData.
struct SCE_PFX_API PfxRayOutput
{
	PfxLargePosition m_contactPoint;	///< @brief Contact position
	PfxVector3 m_contactNormal;			///< @brief Contact normal
	PfxFloat   m_variable;				///< @brief Position on the ray
	PfxUInt16  m_objectId;				///< @brief Object ID (= Rigid Body ID)
	PfxUInt8   m_shapeId;				///< @brief Shape ID
	PfxBool    m_contactFlag : 1;		///< @brief true = Hit
	PfxSubData m_subData;				///< @brief Additional information
};

struct SCE_PFX_API PfxSphereInput
{
	PfxLargePosition m_startPosition;	///< @brief Start position of the sphere
	PfxVector3 m_direction;				///< @brief Direction of the sphere
	PfxFloat m_radius;					///< @brief Radius of the sphere
	PfxUInt32 m_contactFilterSelf;		///< @brief Self contact filter
	PfxUInt32 m_contactFilterTarget;	///< @brief Target contact filter
	PfxUInt16 m_collisionIgnoreGroup[2];///< @brief Collision ignore group

	/// @brief Reset parameters
	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
		m_collisionIgnoreGroup[0] = m_collisionIgnoreGroup[1] = 0xffff;
		m_radius = 1.0f;
	}
};

struct SCE_PFX_API PfxSphereOutput
{
	PfxLargePosition m_contactPoint;	///< @brief Contact position
	PfxVector3 m_contactNormal;			///< @brief Contact normal
	PfxFloat   m_variable;				///< @brief Position on the ray
	PfxUInt16  m_objectId;				///< @brief Object ID (= Rigid Body ID)
	PfxUInt8   m_shapeId;				///< @brief Shape ID
	PfxBool    m_contactFlag : 1;		///< @brief true = Hit
	PfxSubData m_subData;				///< @brief Additional information
};

struct SCE_PFX_API PfxCapsuleInput
{
	PfxLargePosition m_startPosition;	///< @brief Start position of the capsule
	PfxVector3 m_direction;				///< @brief Direction of the capsule
	PfxQuat m_orientation;				///< @brief Orientation of the capsule
	PfxFloat m_radius;					///< @brief Radius of the capsule
	PfxFloat m_halfLength;				///< @brief Half length of the capsule
	PfxUInt32 m_contactFilterSelf;		///< @brief Self contact filter
	PfxUInt32 m_contactFilterTarget;	///< @brief Target contact filter
	PfxUInt16 m_collisionIgnoreGroup[2];///< @brief Collision ignore group

	/// @brief Reset parameters
	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
		m_collisionIgnoreGroup[0] = m_collisionIgnoreGroup[1] = 0xffff;
		m_radius = 1.0f;
		m_halfLength = 1.0f;
		m_orientation = PfxQuat::identity();
	}
};

struct SCE_PFX_API PfxCapsuleOutput
{
	PfxLargePosition m_contactPoint;	///< @brief Contact position
	PfxVector3 m_contactNormal;			///< @brief Contact normal
	PfxFloat   m_variable;				///< @brief Position on the ray
	PfxUInt16  m_objectId;				///< @brief Object ID (= Rigid Body ID)
	PfxUInt8   m_shapeId;				///< @brief Shape ID
	PfxBool    m_contactFlag : 1;		///< @brief true = Hit
	PfxSubData m_subData;				///< @brief Additional information
};

struct SCE_PFX_API PfxCircleInput
{
	PfxLargePosition m_startPosition;	///< @brief Start position of the circle
	PfxVector3 m_direction;				///< @brief Direction of the circle
	PfxQuat m_orientation;				///< @brief Orientation of the circle
	PfxFloat m_radius;					///< @brief Radius of the circle
	PfxUInt32 m_contactFilterSelf;		///< @brief Self contact filter
	PfxUInt32 m_contactFilterTarget;	///< @brief Target contact filter
	PfxUInt16 m_collisionIgnoreGroup[2];///< @brief Collision ignore group

	/// @brief Reset parameters
	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
		m_collisionIgnoreGroup[0] = m_collisionIgnoreGroup[1] = 0xffff;
		m_radius = 1.0f;
		m_orientation = PfxQuat::identity();
	}
};

struct SCE_PFX_API PfxCircleOutput
{
	PfxLargePosition m_contactPoint;	///< @brief Contact position
	PfxVector3 m_contactNormal;			///< @brief Contact normal
	PfxFloat   m_variable;				///< @brief Position on the ray
	PfxUInt16  m_objectId;				///< @brief Object ID (= Rigid Body ID)
	PfxUInt8   m_shapeId;				///< @brief Shape ID
	PfxBool    m_contactFlag : 1;		///< @brief true = Hit
	PfxSubData m_subData;				///< @brief Additional information
};

struct SCE_PFX_API PfxRayInputInternal
{
	PfxSegment m_segment;
	PfxVector3 m_startPosition;
	PfxVector3 m_direction;
	PfxUInt32 m_contactFilterSelf;
	PfxUInt32 m_contactFilterTarget;
	PfxUInt8  m_facetMode;
};

struct SCE_PFX_API PfxRayOutputInternal
{
	PfxVector3 m_contactPoint;
	PfxVector3 m_contactNormal;
	PfxFloat   m_variable;
	PfxUInt16  m_objectId;
	PfxUInt8   m_shapeId;
	PfxBool    m_contactFlag : 1;
	PfxSubData m_subData;
};

struct SCE_PFX_API PfxSphereInputInternal
{
	PfxSegment m_segment;
	PfxVector3 m_startPosition;
	PfxVector3 m_direction;
	PfxFloat m_radius;
	PfxUInt32 m_contactFilterSelf;
	PfxUInt32 m_contactFilterTarget;
	
	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
	}
};

struct SCE_PFX_API PfxSphereOutputInternal
{
	PfxVector3 m_contactPoint;
	PfxVector3 m_contactNormal;
	PfxFloat   m_variable;
	PfxUInt16  m_objectId;
	PfxUInt8   m_shapeId;
	PfxBool    m_contactFlag : 1;
	PfxSubData m_subData;
};

struct SCE_PFX_API PfxCapsuleInputInternal
{
	PfxSegment m_segment;
	PfxVector3 m_startPosition;
	PfxVector3 m_direction;
	PfxQuat m_orientation;
	PfxFloat m_radius;
	PfxFloat m_halfLength;
	PfxUInt32 m_contactFilterSelf;
	PfxUInt32 m_contactFilterTarget;
	
	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
	}
};

struct SCE_PFX_API PfxCapsuleOutputInternal
{
	PfxVector3 m_contactPoint;
	PfxVector3 m_contactNormal;
	PfxFloat   m_variable;
	PfxUInt16  m_objectId;
	PfxUInt8   m_shapeId;
	PfxBool    m_contactFlag : 1;
	PfxSubData m_subData;
};

struct SCE_PFX_API PfxCircleInputInternal
{
	PfxSegment m_segment;
	PfxVector3 m_startPosition;
	PfxVector3 m_direction;
	PfxQuat m_orientation;
	PfxFloat m_radius;
	PfxUInt32 m_contactFilterSelf;
	PfxUInt32 m_contactFilterTarget;

	void reset()
	{
		m_contactFilterSelf = m_contactFilterTarget = 0xffffffff;
	}
};

struct SCE_PFX_API PfxCircleOutputInternal
{
	PfxVector3 m_contactPoint;
	PfxVector3 m_contactNormal;
	PfxFloat   m_variable;
	PfxUInt16  m_objectId;
	PfxUInt8   m_shapeId;
	PfxBool    m_contactFlag : 1;
	PfxSubData m_subData;
};

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_RAY_H
