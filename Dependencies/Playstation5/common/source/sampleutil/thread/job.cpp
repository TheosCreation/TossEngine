/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
 * 
 */

#include <vector>
#include <string>
#include <atomic>

#include <sampleutil/sampleutil_error.h>
#include <sampleutil/thread/job.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Thread
		{
			class JobItemQueue;

			class JobThreadFunction : public ThreadFunction
			{
			public:
				JobThreadFunction(JobItemQueue* itemQueue)
					: m_item(nullptr)
					, m_queue(itemQueue)
				{
				}
				virtual ~JobThreadFunction()
				{
				}

			protected:

				int run() override;

			private:
				JobItem*		m_item;
				JobItemQueue*	m_queue;
			};

			// JobItemを実際にキューイングするクラス
			// JobItemのステータスはキューの操作とともにこのクラスのみで行う
			class JobItemQueue {
				friend class JobQueue;

			public:
				JobItemQueue(const char* name) : 
					m_numWaitingWorkers(0)
					, m_name(name)
					, m_isEndJobEnqueued(false)
				{
					m_jobitemQueueCond = new Cond("SampleUtilJobItemQueueCond");
					SCE_SAMPLE_UTIL_ASSERT(m_jobitemQueueCond != nullptr);
				}

				~JobItemQueue()
				{
					SCE_SAMPLE_UTIL_SAFE_DELETE(m_jobitemQueueCond);
				}

				int push(JobItem* jobItem, bool isEndJob=false)
				{
					m_jobitemQueueCond->lock();

					if (m_isEndJobEnqueued && !isEndJob)
					{
						m_jobitemQueueCond->unlock();
						return SCE_SAMPLE_UTIL_ERROR_INVALID_STATE;
					}

					// JobItemのステータスはキューの状態と一緒に変更
					jobItem->m_status.store(isEndJob ? JobItemStatus::kFinishJob : JobItemStatus::kWaiting, std::memory_order_release);
					m_listRunnable.push_back(jobItem);

					// 終了ジョブが来たら、以降は終了ジョブしか受け付けない
					m_isEndJobEnqueued = isEndJob;

					if (m_numWaitingWorkers) {
						m_jobitemQueueCond->signal();
					}

					m_jobitemQueueCond->unlock();

					return SCE_OK;
				}

				void erase(JobItem* jobItem)
				{
					m_jobitemQueueCond->lock();

					SCE_SAMPLE_UTIL_ASSERT(jobItem);
					SCE_SAMPLE_UTIL_ASSERT(jobItem->m_status != JobItemStatus::kNone);

					bool isFound = false;
					for (auto i = m_listConsumed.begin(); i != m_listConsumed.end(); ++i)
					{
						if ((*i) == jobItem) {
							// JobItemのステータスはキューの状態と一緒に変更
							m_listConsumed.erase(i);
							jobItem->m_status.store(JobItemStatus::kNone, std::memory_order_release);
							isFound = true;
							break;
						}
					}

					SCE_SAMPLE_UTIL_ASSERT(isFound);

					m_jobitemQueueCond->unlock();
				}
			public:

				JobItem* getRunnableJobItem()
				{
					m_jobitemQueueCond->lock();

					// キューに積まれるまで通知を待つ
					while (m_listRunnable.size() == 0) {

						m_numWaitingWorkers++;

						m_jobitemQueueCond->wait();

						m_numWaitingWorkers--;

					}

					JobItem* jobItem = nullptr;

					// 先頭のJobItemを消費済みリストに移動
					jobItem = *m_listRunnable.begin();
					m_listRunnable.erase(m_listRunnable.begin());
					m_listConsumed.push_back(jobItem);

					SCE_SAMPLE_UTIL_ASSERT(jobItem != nullptr);
					SCE_SAMPLE_UTIL_ASSERT((jobItem->m_status == JobItemStatus::kWaiting) || (jobItem->m_status == JobItemStatus::kFinishJob));

					// JobItemのステータスはキューの状態と一緒に変更
					JobItemStatus statusWaiting = JobItemStatus::kWaiting; (void)statusWaiting;
					jobItem->m_status.compare_exchange_strong((int&)statusWaiting, (int)JobItemStatus::kRunning, std::memory_order_acq_rel, std::memory_order_acquire);

					m_jobitemQueueCond->unlock();

					return jobItem;
				}

				int numItems()
				{
					m_jobitemQueueCond->lock();
					int num = m_listRunnable.size() + m_listConsumed.size();
					m_jobitemQueueCond->unlock();
					return num;
				}

			private:
				Cond					*m_jobitemQueueCond;
				std::vector<JobItem *>	m_listRunnable;
				std::vector<JobItem *>	m_listConsumed;
				int						m_numWaitingWorkers;
				std::string				m_name;
				bool					m_isEndJobEnqueued;
			};

			int JobThreadFunction::run()
			{
				bool isEndJobArrived = false;
				while (false == isEndJobArrived)
				{
					m_item = m_queue->getRunnableJobItem();

					if (m_item)
					{
						SCE_SAMPLE_UTIL_ASSERT((m_item->m_status == JobItemStatus::kRunning) || (m_item->m_status == JobItemStatus::kFinishJob));

						// 終了ジョブが来たらループを抜ける
						isEndJobArrived = m_item->m_status.load(std::memory_order_acquire) == JobItemStatus::kFinishJob;

						m_item->m_result = m_item->run();

						// 実行終了済みのJobItemは即削除
						m_queue->erase(m_item);
						m_item = nullptr;
					}
				}

				return SCE_OK;
			}

			JobItemOption::JobItemOption()
			{
			}

			JobItem::JobItem(const char *name, const JobItemOption *option)
				: m_name(name)
				, m_status(JobItemStatus::kNone)
				, m_result(0)
			{
				if (option) {
					m_option = *option;
				}
			}

			JobItem::~JobItem()
			{
			}

			bool JobItem::isNone()
			{
				return m_status.load(std::memory_order_acquire) == JobItemStatus::kNone;
			}

			int JobItem::getResult()
			{
				return m_result;
			}

			class JobFinalizeItem : public JobItem {
			public:
				JobFinalizeItem() : JobItem("SampleUtilDummyJob"){}
			protected:
				// 終了通知を出すためだけに投入させるダミーJob
				int run() override
				{
					return SCE_OK;
				}
			};

			JobQueueOption::JobQueueOption()
				: m_priority	(0)
				, m_stackSize	(16 * 1024)
				, m_numThread	(8)
				, m_threadOption(nullptr)
			{}

			JobQueue::JobQueue()
				: m_name("")
			{
				m_itemList = new JobItemQueue("SampleUtilJobItemQueue");
				SCE_SAMPLE_UTIL_ASSERT(m_itemList);
			}

			JobQueue::JobQueue(JobQueueOption *option)
				: m_name("")
			{
				m_itemList = new JobItemQueue("SampleUtilJobItemQueue");
				SCE_SAMPLE_UTIL_ASSERT(m_itemList);

				initialize("SampleUtilJobQueue", option);
			}

			JobQueue::~JobQueue()
			{
				finalize();
				delete m_itemList;
			}

			int JobQueue::initialize(const char *name, const JobQueueOption *option)
			{
				int ret;
				if (option != nullptr)
				{
					m_option = *option;
				}
				for (uint32_t i = 0; i < m_option.m_numThread; ++i)
				{
					JobThreadFunction	*jobfunc = new JobThreadFunction(m_itemList);
					SCE_SAMPLE_UTIL_ASSERT(jobfunc != nullptr);
					if (jobfunc == nullptr)
					{
						return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
					}

					Thread *t = new Thread(name, m_option.m_priority, m_option.m_stackSize, m_option.m_threadOption);
					SCE_SAMPLE_UTIL_ASSERT(t != nullptr);
					if (t == nullptr)
					{
						delete jobfunc;
						return SCE_SAMPLE_UTIL_ERROR_OUT_OF_MEMORY;
					}

					ret = t->start(jobfunc);
					SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
					if (ret != SCE_OK)
					{
						delete jobfunc;
						delete t;
						return ret;
					}

					m_threadList.push_back(t);
				}

				return	SCE_OK;
			}

			int JobQueue::finalize()
			{
				// ワーカースレッドを終わらせるJobFinalizeItemをワーカースレッド数だけキューに積む。
				// JobFinalizeItemを受け取ったワーカースレッドは次のJobは取らずに必ず終了するため、
				// これで全ワーカースレッドを終了させられる
				std::vector<JobFinalizeItem> finalJobs;
				finalJobs.reserve(m_threadList.size());
				for (int i = 0; i < m_threadList.size(); ++i)
				{
					finalJobs.emplace_back();
					enqueueCore(&finalJobs.back(), /*isEndJob=*/true); // これ以降は通常のenqueueは失敗する
				}

				// ワーカースレッド終了の終了待ち
				for (auto *pWorkerThread : m_threadList)
				{
					pWorkerThread->join();

					delete pWorkerThread->getFuncObject();
					delete pWorkerThread;
				}
				m_threadList.clear();

				return SCE_OK;
			}

			int JobQueue::enqueue(JobItem	*jobItem)
			{
				// 外部からは終了Jobは追加できない
				return enqueueCore(jobItem, /*isEndJob=*/false);
			}

			void JobQueue::check()
			{
			}

			void JobQueue::getOption(JobQueueOption	*option)
			{
				SCE_SAMPLE_UTIL_ASSERT(option != nullptr);
				if (option != nullptr)
				{
					*option = m_option;
				}
			}

			int JobQueue::enqueueCore(JobItem	*jobItem, bool isEndJob)
			{
				SCE_SAMPLE_UTIL_ASSERT(jobItem->m_status == JobItemStatus::kNone);

				if (jobItem->m_status.load(std::memory_order_acquire) != JobItemStatus::kNone)
				{
					return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
				}

				if (m_threadList.size() == 0)
				{
					SCE_SAMPLE_UTIL_ASSERT(0);
					return SCE_SAMPLE_UTIL_ERROR_JOB_NO_THREAD;
				}

				return m_itemList->push(jobItem, isEndJob);
			}

			uint32_t JobQueue::numItems() const
			{
				return m_itemList->numItems();
			}

			void JobQueue::waitEmpty()
			{
				while (numItems() > 0)
				{
					Thread::sleep(10);
				}
			}

			void JobQueue::changePriority(int32_t priority)
			{
				for (auto *pWorkerThread : m_threadList)
				{
					pWorkerThread->changePriority(priority);
				}
			}
		}
	}
}



