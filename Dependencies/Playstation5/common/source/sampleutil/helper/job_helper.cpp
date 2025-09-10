/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
 * 
 */

#include <sampleutil/helper/job_helpler.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Helper
		{
			JobQueue::JobQueue(JobQueueOption* pOption) : m_isCancelRequested(false)
			{
				Thread::JobQueueOption* pBaseOption = nullptr;
				if (pOption) {
					pBaseOption = &pOption->m_baseOption;
				}
				m_jobQueue = new Thread::JobQueue(pBaseOption);
				SCE_SAMPLE_UTIL_ASSERT(m_jobQueue);
			}

			JobQueue::~JobQueue()
			{
				SCE_SAMPLE_UTIL_SAFE_DELETE(m_jobQueue);
			}

			int JobQueue::enqueue(Helper::JobItem* pJobItem)
			{
				// 互換を取るため、このAPIを呼ぶ場合は何もしないDeleterを設定する
				struct NullDeleter {
					void operator ()(Helper::JobItem* ptr) const {
						(void)ptr;
					}
				};
				std::shared_ptr<Helper::JobItem> jobItemPtr(pJobItem, NullDeleter());

				return enqueue(jobItemPtr);
			}

			int JobQueue::enqueue(std::shared_ptr<Helper::JobItem> jobItemPtr)
			{
				Thread::ScopedLock lock(&m_finishQueue, Thread::LockableObjectAccessAttr::kWrite);
				int ret = m_jobQueue->enqueue(jobItemPtr.get());

				if (ret == SCE_OK) {
					// enqueue出来たらFinish監視キューに積む
					m_finishQueue.push_back(jobItemPtr);
				}
				return ret;
			}

			bool JobQueue::isEmpty() {
				Thread::ScopedLock lock(&m_finishQueue, Thread::LockableObjectAccessAttr::kRead);
				return ((m_jobQueue->numItems()) == 0 && (m_finishQueue.size() == 0)) ? true : false;
			}

			int JobQueue::check() {
				Thread::ScopedLock lock(&m_finishQueue, Thread::LockableObjectAccessAttr::kWrite);

				if (m_isCancelRequested) {
					// ここで、キャンセルリクエストを処理する事でfinish()との排他が成立する
					std::for_each(m_finishQueue.begin(), m_finishQueue.end(),
						[](std::shared_ptr<Helper::JobItem> ptr) {
						// 既に終了している場合はキャンセルしない
						if (ptr->isNone() == false) {
							ptr->cancel();
						}
					});
					m_isCancelRequested = false;
				}

				auto it = m_finishQueue.begin();
				while (it != m_finishQueue.end()) {
					// 終了したら、Finishを呼び出す
					if ((*it)->isNone()) {
						std::shared_ptr<Helper::JobItem> jobItemPtr = *it;

						// finish()中はロックを外す
						m_finishQueue.unlock();
						jobItemPtr->finish(jobItemPtr->getResult());
						m_finishQueue.lock(Thread::LockableObjectAccessAttr::kWrite);

						// unlock中にenqueueされる可能性がある為、itは使えない
						// よってリストを再検索する
						it = std::find(m_finishQueue.begin(), m_finishQueue.end(), jobItemPtr);
						it = m_finishQueue.erase(it);
					}
					else {
						++it;
					}
				}

				return SCE_OK;
			}

			int JobQueue::changePriority(int32_t priority) {
				m_jobQueue->changePriority(priority);
				return SCE_OK;
			}

			int JobQueue::cancelAllItems() {
				Thread::ScopedLock lock(&m_finishQueue, Thread::LockableObjectAccessAttr::kWrite);
				m_isCancelRequested = true;
				return SCE_OK;
			}

		}
	}
}
