/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_DEBUG_RENDER_H
#define _SCE_PFX_DEBUG_RENDER_H

#include "../low_level/pfx_low_level_include.h"

namespace sce {
namespace pfxv4 {

/// @name Debug render

/// Debug render API

#define SCE_PFX_DRENDER_MESH_FLG_NONE           0x00 ///< No flag is set
#define SCE_PFX_DRENDER_MESH_FLG_ISLAND         0x01 ///< Renders the specified island mesh
#define SCE_PFX_DRENDER_MESH_FLG_EDGE           0x02 ///< Renders the specified edge of triangle
#define SCE_PFX_DRENDER_MESH_FLG_FACET_AABB     0x04 ///< Renders the specified bounding box (AABB) of triangle
#define SCE_PFX_DRENDER_MESH_FLG_NORMAL         0x08 ///< Renders the normal of triangle
#define SCE_PFX_DRENDER_MESH_FLG_THICKNESS      0x10 ///< Renders the thickness of triangle
#define SCE_PFX_DRENDER_MESH_FLG_ALL            0x1f ///< Specifies all the flags

/// @brief Callback function called during point rendering.
/// @details Implement the codes for rendering points at the specified positions.
/// @param position Position of the point
/// @param color Color of the point
typedef void (*pfxDebugRenderPointFunc)(const PfxVector3 &position,const PfxVector3 &color);

/// @brief Callback function called during line rendering.
/// @details Implement the codes for rendering lines at the specified positions.
/// @param position1 Start position of the line
/// @param position2 End position of the line
/// @param color Color of the line
typedef void (*pfxDebugRenderLineFunc)(const PfxVector3 &position1,const PfxVector3 &position2,const PfxVector3 &color);

/// @brief Callback function called during arc rendering.
/// @details Implement the codes for rendering arcs.
/// @param pos Center of the arc
/// @param axis Axis of rotation of the arc
/// @param dir Vector from the center to the arc
/// @param radius Radius of the arc
/// @param startRad Rendering start angle of the arc
/// @param endRad Rendering end angle of the arc
/// @param color Color of the arc
typedef void (*pfxDebugRenderArcFunc)(const PfxVector3 &pos,const PfxVector3 &axis,const PfxVector3 &dir,const PfxFloat radius,const PfxFloat startRad,const PfxFloat endRad,const PfxVector3 &color);

/// @brief Callback function called during bounding box rendering.
/// @details Implement the codes for rendering bounding boxes parallel to the XYZ axes.
/// @param center Center of the bounding box
/// @param halfExtent Extent of the bounding box
/// @param color Color of the bounding box
typedef void (*pfxDebugRenderAabbFunc)(const PfxVector3 &center,const PfxVector3 &halfExtent,const PfxVector3 &color);

/// @brief Callback function called during box rendering.
/// @details Implement the codes for rendering boxes.
/// @param transform Transform(position and orientation) of the box
/// @param halfExtent Extent of the bounding box
/// @param color Color of the bounding box
typedef void (*pfxDebugRenderBoxFunc)(const PfxTransform3 &transform,const PfxVector3 &halfExtent,const PfxVector3 &color);

/// @brief Debug render
/// @details The structure for the debug render
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxDebugRender
{
	PfxUInt8 reserved[640];
};

/// @brief Query bytes of the work buffer
/// @details Query bytes of the work buffer needed for the debug render.
/// @return Return bytes of the buffer
SCE_PFX_API PfxUInt32 pfxDebugRenderQueryMem();

/// @brief Initialization parameter for the debug render
/// @details This is an input for pfxDebugRenderInit() to initialize the debug render.
/// Render functions need to be specified in advance. 
/// Assertion is called when rendering if any render function isn't specified.
struct SCE_PFX_API PfxDebugRenderInitParam{
	pfxDebugRenderPointFunc pointFunc;	///< @brief Pointer to the point rendering function
	pfxDebugRenderLineFunc lineFunc;	///< @brief Pointer to the line rendering function
	pfxDebugRenderArcFunc arcFunc;		///< @brief Pointer to the arc rendering function
	pfxDebugRenderAabbFunc aabbFunc;	///< @brief Pointer to the AABB rendering function
	pfxDebugRenderBoxFunc boxFunc;		///< @brief Pointer to the box rendering function
};

/// @brief Initialize the debug render
/// @details Initialize the debug render.
/// @param debugRender Debug render
/// @param param Input parameter
/// @param workBuff Work buffer
/// @param workBytes Bytes of the work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxDebugRenderInit(PfxDebugRender &debugRender, const PfxDebugRenderInitParam &param, void *workBuff, PfxUInt32 workBytes);

/// @brief Terminate the debug render
/// @details Terminate the debug render.
/// @param debugRender Debug render
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxDebugRenderTerm(PfxDebugRender &debugRender);

/// @brief Clear the debug render
/// @details Clear the debug render.
/// @param debugRender Debug render
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxDebugRenderClear(PfxDebugRender &debugRender);

/// @brief Enable rigid body visibility
/// @details Enable rendering visibility of the specified rigid body.
/// @param debugRender Debug render
/// @param rigidbodyId Rigid body ID
SCE_PFX_API void pfxDebugRenderEnableVisible(PfxDebugRender &debugRender, PfxUInt32 rigidbodyId);

/// @brief Disable rigid body visibility
/// @details Disable rendering visibility of the specified rigid body.
/// @param debugRender Debug render
/// @param rigidbodyId Rigid body ID
SCE_PFX_API void pfxDebugRenderDisableVisible(PfxDebugRender &debugRender, PfxUInt32 rigidbodyId);

/// @brief Set scaling value of the debug render
/// @details This value changes the length of debug lines.
/// @param debugRender Debug render
/// @param scale Scale
SCE_PFX_API void pfxDebugRenderSetScale(PfxDebugRender &debugRender, PfxFloat scale);

/// @brief Get scaling value of the debug render
/// @details Get scaling value of the debug render.
/// @param debugRender Debug render
/// @return Return the value of the scale
SCE_PFX_API PfxFloat pfxDebugRenderGetScale(PfxDebugRender &debugRender);

/// @brief Set segment position of the debug render
/// @details Set segment position of the render scene. It works if a large position is enabled.
/// @param debugRender Debug render
/// @param segment Segment position
SCE_PFX_API void pfxDebugRenderSetSegment(PfxDebugRender &debugRender, const PfxSegment &segment);

/// @brief Specify the range of rendering inside the view frustum
/// @details The range type depends on the number of planes.
/// @arg numPlanes=0 Renders everything
/// @arg numPlanes=1 Renders objects inside the sphere specified by a planes[0] whose first three elements represents the center and fourth element represents the radius of the sphere.
/// @arg numPlanes>1 Renders objects inside the range surrounded by the planes whose first three elements represents the normal vector and fourth element represents the distance from the origin.
/// @param debugRender Debug render
/// @param planes Array of planes making up view frustum
/// @param numPlanes Number of planes making up view frustum
SCE_PFX_API void pfxDebugRenderSetFrustum(PfxDebugRender &debugRender, const PfxVector4 *planes, PfxUInt32 numPlanes);

/// @brief Render debug information of bounding boxes of a rigid body
/// @details Render debug information of bounding boxes of a rigid body
/// @param debugRender Debug render
/// @param broadphaseProxyContainer Broadphase proxy container
SCE_PFX_API void pfxDebugRenderRenderAabb(PfxDebugRender &debugRender,
	const PfxBroadphaseProxyContainer &broadphaseProxyContainer);

/// @brief Render debug information of local coordinate axes
/// @details Render debug information of local coordinate axes.
/// @param debugRender Debug render
/// @param states Array of the rigid body states
/// @param numRigidbodies Number of rigid bodies
SCE_PFX_API void pfxDebugRenderRenderLocalAxis(PfxDebugRender &debugRender,
	const PfxRigidState *states,PfxUInt32 numRigidbodies);

/// @brief Render debug information of contacts
/// @details Render debug information of contacts.
/// @param debugRender Debug render
/// @param contactContainer Contact container
/// @param states Array of the rigid body states
SCE_PFX_API void pfxDebugRenderRenderContact(PfxDebugRender &debugRender,
	const PfxContactContainer &contactContainer,
	const PfxRigidState *states);

/// @brief Render debug information of joints
/// @details Render debug information of joints.
/// @param debugRender Debug render
/// @param states Array of the rigid body states
/// @param joints Array of joints
/// @param numJoints Number of joints
/// @param filter Filtering joints to be rendered (Ex. filter = 1 << kPfxJointBall | 1 << kPfxJointSwingTwist)
SCE_PFX_API void pfxDebugRenderRenderJoint(PfxDebugRender &debugRender,
	const PfxRigidState *states,const PfxJoint *joints,PfxUInt32 numJoints, PfxUInt32 filter = 0xffff);

/// @brief Render debug information of large meshes
/// @details Render debug information of large meshes.
/// Flag can be specified by combination of following types.
/// @arg SCE_PFX_DRENDER_MESH_FLG_ISLAND     Render island meshes
/// @arg SCE_PFX_DRENDER_MESH_FLG_EDGE       Render edges of triangles
/// @arg SCE_PFX_DRENDER_MESH_FLG_FACET_AABB Render bounding box (AABB) of triangles
/// @arg SCE_PFX_DRENDER_MESH_FLG_NORMAL     Render normal of triangles
/// @arg SCE_PFX_DRENDER_MESH_FLG_THICKNESS  Render thickness of triangles
/// @param debugRender Debug render
/// @param states Array of the rigid body states
/// @param collidables Array of the collision shapes
/// @param numRigidbodies Number of rigid bodies
/// @param flag Mesh rendering type flag
SCE_PFX_API void pfxDebugRenderRenderLargeMesh(PfxDebugRender &debugRender,
	const PfxRigidState *states,const PfxCollidable *collidables,PfxUInt32 numRigidbodies,
	PfxUInt32 flag);

/// @brief Render debug information of primitives
/// @details Render debug information of primitives
/// @param debugRender Debug render
/// @param states Array of the rigid body states
/// @param collidables Array of the collision shapes
/// @param numRigidbodies Number of rigid bodies
SCE_PFX_API void pfxDebugRenderRenderPrimitives(PfxDebugRender &debugRender,
	const PfxRigidState *states, const PfxCollidable *collidables, PfxUInt32 numRigidbodies);

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_DEBUG_RENDER_H
