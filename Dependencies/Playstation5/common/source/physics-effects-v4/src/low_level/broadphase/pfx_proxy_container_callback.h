/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_PROXY_CONTAINER_CALLBACK_H_
#define _SCE_PFX_PROXY_CONTAINER_CALLBACK_H_

namespace sce {
namespace pfxv4 {

typedef PfxBool(*pfxTraverseProxyContainerCallback)(
	PfxUInt32 proxyId,const PfxBv &bv,
	void *userData);

typedef PfxBool(*pfxTraverseProxyContainerCallbackForRayClipping)(
	PfxUInt32 proxyId,const PfxBv &bv,
	PfxRayInput &clipRay,
	PfxFloat &tmin,
	void *userData);

} //namespace pfxv4
} //namespace sce

#endif /* _SCE_PFX_PROXY_CONTAINER_CALLBACK_H_ */
