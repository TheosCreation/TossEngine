/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#include "pfx_precompiled.h"

#include "../../../include/physics_effects/base_level/base/pfx_common.h"
#include "../../../include/physics_effects/base_level/base/pfx_perf_counter.h"

namespace sce {
namespace pfxv4 {

// セグメントサイズを格納するグローバル変数
// 基本書き込み禁止
PfxInt32 gSegmentWidth;
PfxFloat gSegmentWidthInv;

} // namespace pfxv4
} // namespace sce
