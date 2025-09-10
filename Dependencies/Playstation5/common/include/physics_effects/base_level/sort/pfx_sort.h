/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_SORT_H
#define _SCE_PFX_SORT_H

#include "pfx_sort_data.h"

namespace sce {
namespace pfxv4 {


///////////////////////////////////////////////////////////////////////////////
// Single Sort

/// @brief Sort data
/// @details It requires a work buffer with the same size as the input data.
/// @param[in,out] data Data to be sorted
/// @param buff Work buffer
/// @param n Number of the data
SCE_PFX_API void pfxSort(PfxSortData16 *data,PfxSortData16 *buff,PfxUInt32 n);

/// @brief Sort data
/// @details It requires a work buffer with the same size as the input data.
/// @param[in,out] data Data to be sorted
/// @param buff Work buffer
/// @param n Number of the data
SCE_PFX_API void pfxSort(PfxSortData32 *data,PfxSortData32 *buff,PfxUInt32 n);


} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_SORT_H
