/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_BROADPHASE_H_
#define _SCE_PFX_BROADPHASE_H_

#include "../../base_level/broadphase/pfx_broadphase_pair.h"
#include "../../base_level/rigidbody/pfx_rigid_state.h"
#include "../../base_level/collision/pfx_collidable.h"

#include "../rigidbody/pfx_rigid_body_context.h"
#include "../broadphase/pfx_broadphase_proxy_container.h"
#include "../collision/pfx_contact_container.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// Update Broadphase Proxies

/// @brief Specify the behavior of a rigid body by a user
/// @details This callback is called when a body moves beyond the world's border
/// @param rigidBodyId Rigid body ID
/// @param userData User data
typedef void(*pfxBodyOutOfWorldCallback)(PfxUInt32 rigidBodyId, void *userData);

/// @brief Update proxy container parameter
/// @details This parameter is a input for pfxUpdateBroadphaseProxies().
struct SCE_PFX_API PfxUpdateBroadphaseProxiesParam {
	PfxBool fixOutOfWorldBody = false;						///< @brief Whether a rigid body changes to a fixed state when it moves beyond the world border
	PfxBool resetShiftFlag = true;							///< @brief Shift flag is used to check any changes in nodes
	PfxBroadphaseProxyContainer *proxyContainer = nullptr;	///< @brief Proxy container
	pfxBodyOutOfWorldCallback outOfWorldCallback = nullptr;	///< @brief Callback called when a body moves beyond the world border
	void *userDataForOutOfWorldCallback = nullptr;			///< @brief User data transferred to the callback
};

/// @brief Update broadpahse proxies
/// @details Update the internal structure of a proxy container, according to current states of rigid bodies.
/// If a rigid body remains unchanged, it's proxy will not be updated.
/// @param context Rigid body context
/// @param sharedParam shared parameter
/// @param param Input parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxUpdateBroadphaseProxies(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxUpdateBroadphaseProxiesParam &param);

/// @brief Non blocking version of pfxUpdateBroadphaseProxies()
SCE_PFX_API PfxInt32 pfxDispatchUpdateBroadphaseProxies(PfxRigidBodyContext &context, PfxUpdateBroadphaseProxiesParam &param);

///////////////////////////////////////////////////////////////////////////////
// Find Pairs

/// @brief Find overlapped pair callback
/// @details This callback is called when finding an overlapped proxy pair.
/// @param rigidBodyIdA Rigid body A's ID
/// @param rigidBodyIdB Rigid body B's ID
/// @param userData User data
/// @return Return true if a pair needs to be added.
typedef PfxBool(*pfxFindOverlapPairsCallback)(PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB,void *userData);

/// @brief Find overlapped pairs parameter
/// @details This parameter is a input for pfxFindOverlapPairs()
struct SCE_PFX_API PfxFindOverlapPairsParam {
	const PfxBroadphaseProxyContainer *proxyContainerA = nullptr;	///< @brief Proxy container A
	const PfxBroadphaseProxyContainer *proxyContainerB = nullptr;	///< @brief Proxy container B
	PfxBroadphasePair *pairs = nullptr;						///< @brief [out] Array of pairs which takes output pairs
	PfxInt32 *numPairsPtr = nullptr;						///< @brief [out] Pointer to a valuable storing the number of contact pairs
	pfxFindOverlapPairsCallback callback = nullptr;			///< @brief Callback
	void *userData = nullptr;								///< @brief User data transferred to the callback
};

/// @brief Find overlapped pairs of proxies from two broadphase proxy containers
/// @details Traverse proxies in each broadphase proxy container and store overlapped pairs into the array in found order.
/// @param context Rigid body context
/// @param sharedParam Shared parameter
/// @param param Parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxFindOverlapPairs(PfxRigidBodyContext &context,PfxRigidBodySharedParam &sharedParam,PfxFindOverlapPairsParam &param);

/// @brief Non blocking version of pfxFindOverlapPairs()
SCE_PFX_API PfxInt32 pfxDispatchFindOverlapPairs(PfxRigidBodyContext &context,PfxFindOverlapPairsParam &param);

///////////////////////////////////////////////////////////////////////////////
// Refine Pairs

/// @brief Callback for the status of a pair
/// @details This callback is called to inform a status of pair after refining.
/// @param status 0:Add 1:Remove 2:Keep
/// @param rigidBodyIdA Rigid body A's ID
/// @param rigidBodyIdB Rigid body B's ID
/// @param userData User data
typedef void(*pfxRefinePairsCallback)(PfxUInt32 status,PfxUInt32 rigidBodyIdA,PfxUInt32 rigidBodyIdB,void *userData);

/// @brief Refine pairs parameter
/// @details This parameter is a input for pfxRefinePairs().
struct SCE_PFX_API PfxRefinePairsParam {
	PfxBool dontWakeUp = false;					///< @brief Rigid bodies remain asleep if relevant pairs are added or removed
	PfxContactContainer *contactContainer = nullptr;///< @brief [in,out] Contact container
	PfxBroadphasePair *pairs = nullptr;	///< @brief [in] Array of pairs which takes output pairs (results from find overlap pairs)
	PfxInt32 *numPairsPtr = nullptr;	///< @brief [in] Pointer to a valuable storing the number of contact pairs (results from find overlap pairs)
	pfxRefinePairsCallback callback = nullptr;	///< @brief Callback are called when pairs are kept, added or removed
	void *userData = nullptr;					///< @brief User data transferred to the callback
};

/// @brief Compare the previous and current pairs, then merge them.
/// @details New pairs found by pfxFindOverlapPairs() are inserted into a contact container
/// to construct a pair graph. New contact manifold is generated from the contact container and assigned to the new pair.
/// If a specific pair isn't found in a previously stored pairs, the pair will be removed from a contact graph.
/// @param context Rigid body context
/// @param sharedParam Shared parameter
/// @param param Parameter
/// @return Return SCE_PFX_OK(0) upon normal termination.
SCE_PFX_API PfxInt32 pfxRefinePairs(PfxRigidBodyContext &context, PfxRigidBodySharedParam &sharedParam, PfxRefinePairsParam &param);

/// @brief Non blocking version of pfxRefinePairs()
SCE_PFX_API PfxInt32 pfxDispatchRefinePairs(PfxRigidBodyContext &context, PfxRefinePairsParam &param);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_BROADPHASE_H_ */
