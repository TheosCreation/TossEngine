/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 *                Copyright (C) 2020 Sony Interactive Entertainment Inc.
 *                                                
 */

///////////////////////////////////////////////////////////////////////////////

#ifndef	_SCE_PFX_POOL_ARRAY_IMPLEMENTATION_H
#define	_SCE_PFX_POOL_ARRAY_IMPLEMENTATION_H

namespace sce{
namespace pfxv4{

///////////////////////////////////////////////////////////////////////////////
// PfxPoolQueue

template <class T>
inline PfxUInt32 PfxPoolQueue<T>::getBytes(PfxUInt32 maxData)
{
	return SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData);
}

template <class T>
inline void PfxPoolQueue<T>::initialize(PfxHeapManager *pool,PfxUInt32 maxData)
{
	m_pool = pool;
	m_numData = 0;
	m_maxData = maxData;
	m_head = 0;
	m_tail = 0;
	m_data = (T*)m_pool->allocate(sizeof(T)*m_maxData);
}

template <class T>
inline void PfxPoolQueue<T>::finalize()
{
	m_pool->deallocate(m_data);
}

template <class T>
inline PfxUInt32 PfxPoolQueue<T>::push(const T& data)
{
	SCE_PFX_ASSERT(m_numData<m_maxData);
	PfxUInt32 id = m_tail;
	m_data[id] = data;
	m_tail = (m_tail+1)%m_maxData;
	m_numData++;
	return id;
}

template <class T>
inline void PfxPoolQueue<T>::pop()
{
	SCE_PFX_ASSERT(m_numData>0);
	m_head = (m_head+1)%m_maxData;
	m_numData--;
}

template <class T>
inline T& PfxPoolQueue<T>::front()
{
	SCE_PFX_ASSERT(m_numData>0);
	return m_data[m_head];
}

template <class T>
inline const T& PfxPoolQueue<T>::front() const
{
	SCE_PFX_ASSERT(m_numData>0);
	return m_data[m_head];
}

///////////////////////////////////////////////////////////////////////////////
// PfxPoolPriorityQueue

template <class T>
inline PfxUInt32 PfxPoolPriorityQueue<T>::getBytes(PfxUInt32 maxData)
{
	return SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData);
}

template <class T>
inline void PfxPoolPriorityQueue<T>::initialize(PfxHeapManager *pool,PfxUInt32 maxData)
{
	m_pool = pool;
	m_numData = 1;
	m_maxData = maxData+1;
	m_data = (T*)m_pool->allocate(sizeof(T)*m_maxData);
}

template <class T>
inline void PfxPoolPriorityQueue<T>::finalize()
{
	m_pool->deallocate(m_data);
}

template <class T>
void PfxPoolPriorityQueue<T>::upHeap(PfxUInt32 i)
{
	SCE_PFX_ASSERT(i>0 && i<m_numData);
	PfxUInt32 parent = i / 2;
	while(parent > 0) {
		if(m_data[i] < m_data[parent])
			break;
		
		SCE_PFX_SWAP(T,m_data[i],m_data[parent]);
		i = parent;
		parent = i / 2;
	}
}

template <class T>
void PfxPoolPriorityQueue<T>::downHeap(PfxUInt32 i)
{
	PfxUInt32 child = i * 2;
	while(child < m_numData) {
		if((child+1) < m_numData && m_data[child] < m_data[child+1])
			child++;
		
		if(m_data[child] < m_data[i])
			break;
		
		SCE_PFX_SWAP(T,m_data[i],m_data[child]);
		i = child;
		child = i * 2;
	}
}

template <class T>
inline PfxUInt32 PfxPoolPriorityQueue<T>::push(const T& data)
{
	SCE_PFX_ASSERT(m_numData<m_maxData);

	PfxUInt32 id = 	m_numData++;
	m_data[id] = data;
	upHeap(id);
	return id;
}

template <class T>
inline void PfxPoolPriorityQueue<T>::pop()
{
	SCE_PFX_ASSERT(m_numData>1);
	PfxUInt32 id = --m_numData;
	m_data[1] = m_data[id];
	downHeap(1);
}

template <class T>
inline T& PfxPoolPriorityQueue<T>::front()
{
	SCE_PFX_ASSERT(m_numData>1);
	return m_data[1];
}

template <class T>
inline const T& PfxPoolPriorityQueue<T>::front() const
{
	SCE_PFX_ASSERT(m_numData>1);
	return m_data[1];
}

///////////////////////////////////////////////////////////////////////////////
// PfxPoolStack

template <class T>
inline PfxUInt32 PfxPoolStack<T>::getBytes(PfxUInt32 maxData)
{
	return SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData);
}

template <class T>
inline void PfxPoolStack<T>::initialize(PfxHeapManager *pool,PfxUInt32 maxData)
{
	m_pool = pool;
	m_numData = 0;
	m_maxData = maxData;
	m_data = (T*)m_pool->allocate(sizeof(T)*m_maxData);
}

template <class T>
inline void PfxPoolStack<T>::finalize()
{
	m_pool->deallocate(m_data);
}

template <class T>
inline PfxUInt32 PfxPoolStack<T>::push(const T& data)
{
	SCE_PFX_ASSERT(m_numData<m_maxData);
	PfxUInt32 id = m_numData++;
	m_data[id] = data;
	return id;
}

template <class T>
inline void PfxPoolStack<T>::pop()
{
	SCE_PFX_ASSERT(m_numData>0);
	m_numData--;
}

template <class T>
inline T& PfxPoolStack<T>::top()
{
	SCE_PFX_ASSERT(m_numData>0);
	return m_data[m_numData-1];
}

template <class T>
inline const T& PfxPoolStack<T>::top() const
{
	SCE_PFX_ASSERT(m_numData>0);
	return m_data[m_numData-1];
}

///////////////////////////////////////////////////////////////////////////////
// PfxPoolArray

template <class T>
inline PfxUInt32 PfxPoolArray<T>::getBytes(PfxUInt32 maxData)
{
	PfxUInt32 bytes = 0;
	bytes += PfxPoolStack<PfxUInt32>::getBytes(maxData); 
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData); 
	bytes += SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxUInt32)*((maxData+31)>>5));
	return bytes;
}

template <class T>
inline void PfxPoolArray<T>::initialize(PfxHeapManager *pool,PfxUInt32 maxData)
{
	m_pool = pool;
	m_numData = 0;
	m_maxData = maxData;
	m_dataLength = 0;
	m_poolId.initialize(pool,maxData);
	m_data = (T*)m_pool->allocate(sizeof(T)*m_maxData);
	m_poolTable = (PfxUInt32*)m_pool->allocate(sizeof(PfxUInt32)*((m_maxData+31)>>5));
	clear();
}

template <class T>
inline void PfxPoolArray<T>::finalize()
{
	m_pool->deallocate(m_poolTable);
	m_pool->deallocate(m_data);
	m_poolId.finalize();
}

template <class T>
inline T& PfxPoolArray<T>::operator[](PfxUInt32 i)
{
	SCE_PFX_ASSERT(i < m_maxData);
#ifdef SCE_PFX_POOL_ARRAY_ASSERT_REMOVED_DATA
	SCE_PFX_ASSERT((m_poolTable[i>>5]&(1<<(i&31)))>0);
#endif
	return m_data[i];
}

template <class T>
inline const T& PfxPoolArray<T>::operator[](PfxUInt32 i) const
{
	SCE_PFX_ASSERT(i < m_maxData);
#ifdef SCE_PFX_POOL_ARRAY_ASSERT_REMOVED_DATA
	SCE_PFX_ASSERT((m_poolTable[i>>5]&(1<<(i&31)))>0);
#endif
	return m_data[i];
}

template <class T>
inline void PfxPoolArray<T>::assign(PfxUInt32 num,const T &initData)
{
	clear();
	
	SCE_PFX_ASSERT(num <= m_maxData);
	
	for(PfxUInt32 i=0;i<num;i++) {
		push(initData);
	}
}
template <class T>
inline void PfxPoolArray<T>::assign(PfxUInt32 num)
{
	clear();

	SCE_PFX_ASSERT(num <= m_maxData);

	for(PfxUInt32 i=0;i<num;i++) {
		SCE_PFX_ASSERT(!m_poolId.empty());
		PfxUInt32 id = m_poolId.top();
		m_poolId.pop();
		m_poolTable[id>>5] |= (1<<(id&31));
		if(id == m_dataLength) m_dataLength++;
		m_numData++;
	}
	
}

template <class T>
inline PfxUInt32 PfxPoolArray<T>::push(const T& data)
{
	SCE_PFX_ASSERT(!m_poolId.empty());
	PfxUInt32 id = m_poolId.top();
	m_poolId.pop();
	m_data[id] = data;
	m_poolTable[id>>5] |= (1<<(id&31));
	if(id == m_dataLength) m_dataLength++;
	m_numData++;
	return id;
}

template <class T>
inline bool PfxPoolArray<T>::remove(PfxUInt32 i)
{
	if(i>=m_dataLength || (m_poolTable[i>>5]&(1<<(i&31)))==0) {
		return false;
	}
	
	m_poolId.push(i);
	m_poolTable[i>>5] &= ~(1<<(i&31));
	m_numData--;
	
	return true;
}

template <class T>
inline bool PfxPoolArray<T>::isRemoved(PfxUInt32 i) const
{
	return (m_poolTable[i>>5]&(1<<(i&31))) == 0;
}

template <class T>
inline void PfxPoolArray<T>::clear()
{
	m_poolId.clear();
	m_numData = m_dataLength = 0;
	for(PfxUInt32 i=0;i<m_maxData;i++) {
		PfxUInt32 id = m_maxData-1-i;
		m_poolId.push(id);
	}
	memset(m_poolTable,0,sizeof(PfxUInt32)*((m_maxData+31)>>5));
}

///////////////////////////////////////////////////////////////////////////////
// PfxPoolMap

template <class KEY, class T>
inline PfxUInt32 PfxPoolMap<KEY,T>::getBytes(PfxUInt32 maxData)
{
	return	SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxMapNode) * maxData) + 
			SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(PfxMapNode*) * maxData) +
			SCE_PFX_ALLOC_BYTES_ALIGN16(sizeof(T) * maxData);
}

template <class KEY, class T>
inline PfxUInt32 PfxPoolMap<KEY,T>::getHashId(const KEY& key)
{
	const unsigned char *p = (const unsigned char *)&key;
	PfxUInt32 id = 0;
	for(int i=0;i<sizeof(KEY);i++) {
		id *= 0x39532BA9;
		id += p[i];
	}
	return id % (m_maxData==0?1:m_maxData);
}

template <class KEY, class T>
inline void PfxPoolMap<KEY,T>::initialize(PfxHeapManager *pool,PfxUInt32 maxData)
{
	m_pool = pool;
	m_maxData = maxData;
	m_nodes = m_pool->allocateArray<PfxMapNode>(m_maxData, 16);
	m_headers = m_pool->allocateArray<PfxMapNode*>(m_maxData, 16);
	m_data = m_pool->allocateArray<T>(m_maxData, 16);
	
	clear();
}

template <class KEY, class T>
inline void PfxPoolMap<KEY,T>::finalize()
{
	m_pool->deallocate(m_data);
	m_pool->deallocate(m_headers);
	m_pool->deallocate(m_nodes);
}

template <class KEY, class T>
inline bool PfxPoolMap<KEY,T>::insert(const KEY& key, const T& data)
{
	SCE_PFX_ASSERT(m_numData<m_maxData);
	
	PfxUInt32 id = getHashId(key);

	if(!m_headers[id]) {
		int pos = m_numData++;
		m_data[pos] = data;
		PfxMapNode &newNode = m_nodes[pos];
		newNode.key = key;
		newNode.data = m_data + pos;
		newNode.prev = NULL;
		newNode.next = NULL;
		m_headers[id] = &newNode;
		return true;
	}
	
	for(PfxMapNode *iterator=m_headers[id];iterator;iterator=iterator->next) {
		if(iterator->key == key) {
			return false;
		}
	}

	int pos = m_numData++;
	m_data[pos] = data;
	PfxMapNode &newNode = m_nodes[pos];
	newNode.key = key;
	newNode.data = m_data + pos;
	newNode.prev = NULL;
	newNode.next = m_headers[id];
	m_headers[id]->prev = &newNode;
	m_headers[id] = &newNode;
	return true;
}

template <class KEY, class T>
inline bool PfxPoolMap<KEY,T>::find(const KEY& key, T& data) const
{
	PfxUInt32 id = getHashId(key);
	
	for(PfxMapNode *iterator=m_headers[id];iterator!=NULL;iterator=iterator->next) {
		if(iterator->key == key) {
			data = *(iterator->data);
			return true;
		}
	}
	
	return false;
}

template <class KEY, class T>
inline bool PfxPoolMap<KEY,T>::find(const KEY& key, T& data)
{
	PfxUInt32 id = getHashId(key);
	
	for(PfxMapNode *iterator=m_headers[id];iterator!=NULL;iterator=iterator->next) {
		if(iterator->key == key) {
			data = *(iterator->data);
			return true;
		}
	}
	
	return false;
}

template <class KEY, class T>
inline bool PfxPoolMap<KEY,T>::erase(const KEY& key)
{
	PfxUInt32 id = getHashId(key);
	
	for(PfxMapNode *iterator=m_headers[id];iterator!=NULL;iterator=iterator->next) {
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
			PfxMapNode *lastNode = &m_nodes[pos];
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

template <class KEY, class T>
inline void PfxPoolMap<KEY,T>::clear()
{
	m_numData = 0;
	memset(m_nodes,0,sizeof(PfxMapNode)*m_maxData);
	memset(m_headers,0,sizeof(PfxMapNode*)*m_maxData);
	memset(m_data,0,sizeof(T)*m_maxData);
}

} // namespace pfxv4
} // namespace sce

#endif//_SCE_PFX_POOL_ARRAY_IMPLEMENTATION_H
