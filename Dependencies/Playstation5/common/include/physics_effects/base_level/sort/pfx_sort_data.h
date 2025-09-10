/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */


#ifndef _SCE_PFX_SORT_DATA_H
#define _SCE_PFX_SORT_DATA_H

#include "../base/pfx_common.h"

namespace sce {
namespace pfxv4 {


#define SCE_PFX_SENTINEL_KEY	0xffffffff ///< @brief Sentinel key

/// @brief Sort data (16bytes)
/// @details This is the structure for representing the sort data
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxSortData16{
	union {
		PfxUInt8   i8data[16];
		PfxUInt16  i16data[8];
		PfxUInt32  i32data[4];
		PfxUInt64  i64data[2];
	};

	void set8(int slot,PfxUInt8 data)   {i8data[slot] = data;}
	void set16(int slot,PfxUInt16 data) {i16data[slot] = data;}
	void set32(int slot,PfxUInt32 data) {i32data[slot] = data;}
	void set64(int slot,PfxUInt64 data) {i64data[slot] = data;}
	PfxUInt8 get8(int slot)   const {return i8data[slot];}
	PfxUInt16 get16(int slot) const {return i16data[slot];}
	PfxUInt32 get32(int slot) const {return i32data[slot];}
	PfxUInt64 get64(int slot) const {return i64data[slot];}
};

/// @brief Sort data (32bytes)
/// @details This is the structure for representing the sort data
struct SCE_PFX_API SCE_PFX_ALIGNED(16) PfxSortData32{
	union {
		PfxUInt8   i8data[32];
		PfxUInt16  i16data[16];
		PfxUInt32  i32data[8];
		PfxUInt64  i64data[4];
	};

	void set8(int slot,PfxUInt8 data)   {i8data[slot] = data;}
	void set16(int slot,PfxUInt16 data) {i16data[slot] = data;}
	void set32(int slot,PfxUInt32 data) {i32data[slot] = data;}
	void set64(int slot,PfxUInt64 data) {i64data[slot] = data;}
	PfxUInt8 get8(int slot)   const {return i8data[slot];}
	PfxUInt16 get16(int slot) const {return i16data[slot];}
	PfxUInt32 get32(int slot) const {return i32data[slot];}
	PfxUInt64 get64(int slot) const {return i64data[slot];}
};

/// @brief Set key to the sort data
/// @details Set key to the sort data.
/// @param sortData Sort data
/// @param key key
SCE_PFX_FORCE_INLINE
void pfxSetKey(PfxSortData16 &sortData,PfxUInt32 key) {sortData.set32(3,key);}

SCE_PFX_FORCE_INLINE
void pfxSetKey(PfxSortData32 &sortData,PfxUInt32 key) {sortData.set32(7,key);}

/// @brief Get key from the sort data
/// @details Get key from the sort data.
/// @param sortData Sort data
/// @return Return key
SCE_PFX_FORCE_INLINE
PfxUInt32 pfxGetKey(const PfxSortData16 &sortData) {return sortData.get32(3);}

SCE_PFX_FORCE_INLINE
PfxUInt32 pfxGetKey(const PfxSortData32 &sortData) {return sortData.get32(7);}

/// @brief Create an unique key
/// @details Create an unique key from 2 indices.
/// @param i Index
/// @param j Index
/// @return Return an unique key
SCE_PFX_FORCE_INLINE
PfxUInt32 pfxCreateUniqueKey(PfxUInt32 i,PfxUInt32 j)
{
	SCE_PFX_ASSERT(i<=j);
	return (j<<16)|(i&0xffff);
}

/// @brief Decode 2 indices from a key
/// @details Decode 2 indices from a key.
/// @param key Key
/// @param[out] i Index
/// @param[out] j Index
SCE_PFX_FORCE_INLINE
void pfxDecodeKey(const PfxUInt32 key,PfxUInt32 &i,PfxUInt32 &j)
{
	i = key&0xffff; // min
	j = key>>16; // max
}

} // namespace pfxv4
} // namespace sce

#endif // _SCE_PFX_SORT_DATA_H
