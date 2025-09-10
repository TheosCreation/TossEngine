/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_CONTACT_CONTAINER_H_
#define _SCE_PFX_CONTACT_CONTAINER_H_


#include "../../base_level/base/pfx_common.h"
#include "../../base_level/collision/pfx_contact_manifold.h"
#include "../../base_level/broadphase/pfx_broadphase_pair.h"

namespace sce {
namespace pfxv4 {

/// @name Contact container

/// @brief Contact container
/// @details This container stores contact information.
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxContactContainer{
	PfxUInt8 reserved[512];
};

/// @brief Query bytes of the work buffer
/// @details Query bytes of the work buffer needed for contact container. Specify additional buffer, if CCD is enabled.
/// @param maxContacts Maximum number of contacts stored in the container
/// @param maxRigidBodies Maximum number of rigid bodies
/// @param additionalBytes Size of additional buffer for closest points
/// @return Return bytes of the buffer
SCE_PFX_API PfxUInt32 pfxContactContainerQueryMem(PfxUInt32 maxContacts,PfxUInt32 maxRigidBodies,PfxUInt32 additionalBytes);

/// @brief Initialize the contact container
/// @details Initialize the contact container.
/// @param[in,out] contactContainer Contact container
/// @param maxContacts Maximum number of contacts stored in the container
/// @param maxRigidBodies Maximum number of rigid bodies
/// @param workBuff Work buffer
/// @param workBytes Bytes of the work buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxContactContainerInit(PfxContactContainer &contactContainer,PfxUInt32 maxContacts,PfxUInt32 maxRigidBodies,void *workBuff,PfxUInt32 workBytes);

/// @brief Terminate the contact container
/// @details Terminate the contact container.
/// @param[in,out] contactContainer Contact container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxContactContainerTerm(PfxContactContainer &contactContainer);

/// @brief Clear the contact container
/// @details Clear the content of the contact container.
/// @param[in,out] contactContainer Contact container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxContactContainerClear(PfxContactContainer &contactContainer);

/// @brief Get the number of contact pairs
/// @details Get the number of contact pairs which affects the whole calculation cost of a simulation.
/// @param contactContainer Contact container
/// @return Return the number of contact pairs
SCE_PFX_API PfxUInt32 pfxContactContainerGetNumContactPairs(const PfxContactContainer &contactContainer);

/// @brief Get the contact information
/// @details Get the contact information related to the specified pair.
/// @param contactContainer Contact container
/// @param rigidbodyIdA Rigid body A's ID
/// @param rigidbodyIdB Rigid body B's ID
/// @return Return the contact information. Return NULL if there isn't.
SCE_PFX_API PfxContactHolder *pfxContactContainerGetContactHolder(PfxContactContainer &contactContainer,PfxUInt32 rigidbodyIdA, PfxUInt32 rigidbodyIdB);
SCE_PFX_API const PfxContactHolder *pfxContactContainerGetContactHolder(const PfxContactContainer &contactContainer, PfxUInt32 rigidbodyIdA, PfxUInt32 rigidbodyIdB);

/// @brief Callback used to query contact information
/// @details pfxContactContainerTraverse() calls this callback for each contact stored in the contact container.
/// @param contact Contact information
/// @param rigidbodyIdA Rigid body A's ID
/// @param rigidbodyIdB Rigid body B's ID
/// @param userData User data
/// @return Return true if query is continued.
typedef PfxBool(*pfxContactContainerTraverseCallback)(PfxContactHolder &contact,PfxUInt32 rigidbodyIdA,PfxUInt32 rigidbodyIdB,void *userData);

/// @brief Query contact information
/// @details Query all contacts stored in the contact container.
/// @param contactContainer Contact container
/// @param callback Callback
/// @param userData User data transferred to the callback
SCE_PFX_API void pfxContactContainerTraverse(const PfxContactContainer &contactContainer,pfxContactContainerTraverseCallback callback,void *userData);

/// @brief Query relevant contact information for a specified rigid body
/// @details Query all relevant contacts for a specified rigid body from the contact container.
/// @param contactContainer Contact container
/// @param rigidbodyId Rigid body's ID
/// @param callback Callback
/// @param userData User data transferred to the callback
SCE_PFX_API void pfxContactContainerTraverse(const PfxContactContainer &contactContainer,PfxUInt32 rigidbodyId,pfxContactContainerTraverseCallback callback,void *userData);

/// @brief Remove a contact
/// @details Remove the specific contact stored in a contact container
/// @param contactContainer Contact container
/// @param rigidbodyIdA Rigid body A's ID
/// @param rigidbodyIdB Rigid body B's ID
/// @return Return SCE_PFX_OK if there isn't any problem.
SCE_PFX_API PfxInt32 pfxContactContainerRemoveContact(const PfxContactContainer &contactContainer, PfxUInt32 rigidbodyIdA, PfxUInt32 rigidbodyIdB);

/// @brief Remove all related contacts
/// @details Remove all related contacts of the specified rigid body stored in a contact container
/// @param contactContainer Contact container
/// @param rigidbodyId Rigid body's ID
/// @return Return SCE_PFX_OK if there isn't any problem.
SCE_PFX_API PfxInt32 pfxContactContainerRemoveAllRelatedContacts(const PfxContactContainer &contactContainer, PfxUInt32 rigidbodyId);

/// @brief Query the size of a serialized buffer of the contact container
/// @details Query the size of a serialized buffer of the contact container
/// @param contactContainer Contact container
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxUInt32 pfxContactContainerQuerySerializeBytes(const PfxContactContainer &contactContainer);

/// @brief Save the contact container to a buffer
/// @details Save the contact container to a buffer. The size of buffer should be enough for string the container.
/// Use pfxContactContainerQuerySerializeBytes() to get the proper size of a serialied buffer.
/// @param contactContainer Contact container
/// @param buffer Pointer to a buffer storing the serialized container
/// @param bytes Size of a buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxContactContainerWrite(const PfxContactContainer &contactContainer, PfxUInt8 *buffer, PfxUInt32 bytes);

/// @brief Load the contact container from a buffer
/// @details Load the contact container from a buffer. The specified container has to have enough space for a serialized data.
/// @param contactContainer Contact container
/// @param buffer Pointer to a buffer storing the serialized container
/// @param bytes Size of a buffer
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxContactContainerRead(PfxContactContainer &contactContainer, const PfxUInt8 *buffer, PfxUInt32 bytes);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_CONTACT_CONTAINER_H_ */
