/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

#ifndef _SCE_PFX_HASH_ARRAY_H
#define _SCE_PFX_HASH_ARRAY_H

#include "../../include/physics_effects/base_level/base/pfx_common.h"
#include "pfx_heap_manager.h"

namespace sce {
namespace pfxv4 {

///////////////////////////////////////////////////////////////////////////////
// PfxHashArray

template <class T>
class PfxHashArray
{
private:
	PfxHeapManager *m_pool;
	PfxUInt32 m_numData;
	PfxUInt32 m_maxData;
	
	struct PfxNode {
		PfxUInt32 key;
		T *data;
		PfxNode *prev;
		PfxNode *next;
	};
	
	PfxNode *m_nodes;
	PfxNode **m_headers;
	T *m_data;
	
	inline PfxUInt32 getHashId(const PfxUInt32 key) const
	{
		return (0x39532BA9 * key) % m_maxData;
	}
	
public:
	static inline PfxUInt32 getBytes(PfxUInt32 maxData);
	
	inline void initialize(PfxHeapManager *pool,PfxUInt32 maxData);
	
	inline void finalize();
	
	PfxUInt32 size() const {return m_numData;}
	
	PfxUInt32 capacity() const {return m_maxData;}
	
	inline bool insert(const PfxUInt32 key, const T& data);
	
	inline bool find(const PfxUInt32 key,T& data) const;
	
	inline bool find(const PfxUInt32 key,T& data);
	
	inline bool erase(const PfxUInt32 key);
	
	inline bool empty() const {return m_numData==0;}
	
	inline void clear();

	inline T* ptr(){return m_data;}
};

template <class T>
inline PfxUInt32 PfxHashArray<T>::getBytes(PfxUInt32 maxData)
{
	PfxUInt32 bytes = 0;
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxNode)*maxData);
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxNode*)*maxData); 
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T)*maxData);
	return bytes;
}

template <class T>
inline void PfxHashArray<T>::initialize(PfxHeapManager *pool,PfxUInt32 maxData)
{
	m_maxData = maxData;
	m_pool = pool;
	m_nodes = (PfxNode*)m_pool->allocate(sizeof(PfxNode)*m_maxData);
	m_headers = (PfxNode**)m_pool->allocate(sizeof(PfxNode*)*m_maxData);
	m_data = (T*)m_pool->allocate(sizeof(T)*m_maxData);
	clear();
}

template <class T>
inline void PfxHashArray<T>::finalize()
{
	m_pool->deallocate(m_data);
	m_pool->deallocate(m_headers);
	m_pool->deallocate(m_nodes);
}

template <class T>
inline bool PfxHashArray<T>::insert(const PfxUInt32 key, const T& data)
{
	SCE_PFX_ASSERT(m_numData<m_maxData);
	
	PfxUInt32 id = getHashId(key);

	if(!m_headers[id]) {
		int pos = m_numData++;
		m_data[pos] = data;
		PfxNode &newNode = m_nodes[pos];
		newNode.key = key;
		newNode.data = m_data + pos;
		newNode.prev = NULL;
		newNode.next = NULL;
		m_headers[id] = &newNode;
		return true;
	}
	
	for(PfxNode *iterator=m_headers[id];iterator;iterator=iterator->next) {
		if(iterator->key == key) {
			return false;
		}
	}

	int pos = m_numData++;
	m_data[pos] = data;
	PfxNode &newNode = m_nodes[pos];
	newNode.key = key;
	newNode.data = m_data + pos;
	newNode.prev = NULL;
	newNode.next = m_headers[id];
	m_headers[id]->prev = &newNode;
	m_headers[id] = &newNode;
	return true;
}

template <class T>
inline bool PfxHashArray<T>::find(const PfxUInt32 key, T& data) const
{
	PfxUInt32 id = getHashId(key);
	
	for(PfxNode *iterator=m_headers[id];iterator!=NULL;iterator=iterator->next) {
		if(iterator->key == key) {
			data = *(iterator->data);
			return true;
		}
	}
	
	return false;
}

template <class T>
inline bool PfxHashArray<T>::find(const PfxUInt32 key, T& data)
{
	PfxUInt32 id = getHashId(key);
	
	for(PfxNode *iterator=m_headers[id];iterator!=NULL;iterator=iterator->next) {
		if(iterator->key == key) {
			data = *(iterator->data);
			return true;
		}
	}
	
	return false;
}

template <class T>
inline bool PfxHashArray<T>::erase(const PfxUInt32 key)
{
	PfxUInt32 id = getHashId(key);
	
	for(PfxNode *iterator=m_headers[id];iterator!=NULL;iterator=iterator->next) {
		if(iterator->key == key) {
			//J ノード削除
			//E Remove nodes
			if(iterator->prev) {
				iterator->prev->next = iterator->next;
			}
			else {
				m_headers[id] = iterator->next;
			}
			
			if(iterator->next) {
				iterator->next->prev = iterator->prev;
			}
			
			//J ノード移動
			//E Move nodes
			int pos = m_numData-1;
			PfxNode *lastNode = &m_nodes[pos];
			if(iterator != lastNode) {
				if(lastNode->prev) {
					lastNode->prev->next = iterator;
				}
				else {
					PfxUInt32 lastId =getHashId(lastNode->key);
					m_headers[lastId] = iterator;
				}
				
				if(lastNode->next) {
					lastNode->next->prev = iterator;
				}
				
				T *data = iterator->data;
				*(data) = m_data[pos];
				*iterator = *lastNode;
				iterator->data = data;
			}
			
			m_numData--;
			
			return true;
		}
	}

	return false;
}

template <class T>
inline void PfxHashArray<T>::clear()
{
	m_numData = 0;
	memset(m_nodes,0,sizeof(PfxNode)*m_maxData);
	memset(m_headers,0,sizeof(PfxNode*)*m_maxData);
	memset(m_data,0,sizeof(T)*m_maxData);
}

} //namespace pfxv4
} //namespace sce

#endif // _SCE_PFX_HASH_ARRAY_H

