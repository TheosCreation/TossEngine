/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2019 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <sampleutil.h>
#include <algorithm>
#include <vector>
#include <memory>

namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Utility of sample util.
		 * @~Japanese
		 * @brief SampleUtilのユーティリティです。
		 */
		namespace Helper
		{
			class JobQueue;

			/*!
			 * @~English
			 * @brief Struct of Job behavior option
			 * @~Japanese
			 * @brief Jobの動作オプション構造体
			 */
			struct JobItemOption {

			};

			/*!
			 * @~English
			 * @brief The class is expanded from Thread::JobItem for a specified use case.
			 * @details Currently, it supports finish() function.
			 * @~Japanese
			 * @brief Thread::JobItemを特定の用途向けに拡張したクラスです。
			 * @details 現状では、finish()が使用できます。
			 */
			class JobItem : public Thread::JobItem {
				friend class Helper::JobQueue;
			public:
				/*!
				 * @~English
				 * @brief Constructor of JobItem
				 * @param name Name
				 * @param pOption Initialize option(default option is used when nullptr is specified)
				 * @~Japanese
				 * @brief JobItemのコンストラクタ
				 * @param name 名前
				 * @param pOption 初期化オプション(nullptr指定時は既定オプションが使用される)
				 */
				JobItem(const char *name, JobItemOption* pOption = nullptr) : Thread::JobItem(name)
				{
					(void)pOption;
				}

				/*!
				 * @~English
				 * @brief Destructor of JobItem
				 * @~Japanese
				 * @brief JobItemのデストラクタ
				 */
				virtual ~JobItem() {}

				/*!
				 * @~English
				 *
				 * @brief Vritual function to describe job's processing on termination
				 *
				 * @param runResult return value of run()
				 * @details This function is called from main thread
				 * @~Japanese
				 *
				 * @brief Jobの終了通知処理を記述するための仮想関数
				 * @param runResult runの返値
				 * @details この関数は、JobQueue::check()の中で呼ばれます。
				 *
				 */
				virtual void finish(int runResult) = 0;

			protected:

				/*!
				 * @~English
				 *
				 * @brief Vritual function to describe job's cancel
				 * @details This function is called from Helper::JobQueue::cancelAllItems().
				 * This function can be called, regardless of whether run() is running or not. In any case, JobItem is responsible for working it correctly.
				 * Helper::JobQueue don't call this function after starting finish().
				 *
				 * @~Japanese
				 *
				 * @brief Jobのキャンセル処理を記述するための仮想関数
				 * @details この関数は、Helper::JobQueue::cancelAllItems()を通じて呼び出される。
				 * run()実行期間以外に呼び出される場合もあるが、その場合もJobItem側で動作に関しての責任を負う。
				 * finish()の呼び出し以降にHelper::JobQueueがこの関数を呼ぶことはない。
				 *
				 */
				virtual void cancel() {}
			};


			/*!
			 * @~English
			 * @brief JobQueue behavior option
			 * @~Japanese
			 * @brief JobQueueの動作オプション
			 */
			struct JobQueueOption {
				/*!
				 * @~English
				 * @brief Base JobQueue option
				 * @~Japanese
				 * @brief 基底JobQueueのオプション
				 */
				Thread::JobQueueOption m_baseOption;
			};

			/*!
			 * @~English
			 *
			 * @brief The class is expanded from Thread::JobQueue for a specified use case.
			 * @details After it returns from Helper::JobItems::run(), the class calls Helper::JobItems::finish() in check().
			 * @~Japanese
			 *
			 * @brief Thread::JobQueueを特定の用途向けに拡張したクラスです。
			 * @details Helper::JobItem::run()が終了するとHelper::JobItem::finish()をcheck()の中で呼び出します
			 */
			class JobQueue {
				Thread::JobQueue*				m_jobQueue;
				class JobItemList : public std::vector<std::shared_ptr<Helper::JobItem>>, public Thread::LockableObject {
				} m_finishQueue;

				bool m_isCancelRequested;
			public:
				/*!
				 * @~English
				 *
				 * @brief Constructor of JobQueue
				 * @param pOption Initialize option(default option is used when nullptr is specified)
				 * @~Japanese
				 *
				 * @brief JobQueueのコンストラクタ
				 * @param pOption 初期化オプション(nullptr指定時には既定オプションが使用される)
				 */
				JobQueue(JobQueueOption* pOption = nullptr);

				/*!
				 * @~English
				 *
				 * @brief Destructor of JobQueue
				 * @details Before destroying JobQueue job queue needs to be empty, and never destroy JobQueue in finish() whose Job is enqueued in that JobQueue.
				 * @~Japanese
				 *
				 * @brief JobQueueのデストラクタ
				 * @details JobQueueを破棄するには、キューが空になっている必要があります。また finish() のなかで そのJobItemが所属するJobQueueを破棄してはいけません。
				 */
				virtual ~JobQueue();

				/*!
				 * @~English
				 *
				 * @brief Enqueue JobItem object to job queue
				 *
				 * @param pJobItem JobItem object
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief JobItemオブジェクトをキューの末尾に追加
				 *
				 * @param pJobItem JobItemオブジェクト
				 *
				 * @retval >=SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int enqueue(Helper::JobItem* pJobItem);

				/*!
				 * @~English
				 *
				 * @brief Enqueue JobItem object to job queue using std::shared_ptr
				 * @param jobItemPtr std::shared_ptr of JobItem object
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Application can control a lifetime of JobItem using this funciton. 
				 * @~Japanese
				 *
				 * @brief std::shared_ptrを使ってJobItemオブジェクトをキューの末尾に追加
				 * @param jobItemPtr JobItemオブジェクトのstd::shared_ptr
				 *
				 * @retval >=SCE_OK 成功
				 * @retval (<0) エラーコード
				 * @details この関数を使う事で、JobItemの削除をstd::shared_ptr側で制御可能です。
				 */
				int enqueue(std::shared_ptr<Helper::JobItem> jobItemPtr);

				/*!
				 * @~English
				 *
				 * @brief Return a boolean value that indicates there is JobItems or not queued in JobQueue.
				 * @details If it is true, then it is enable to delete JobItems.
				 * @retval true JobQueue is empty
				 * @retval false JobQueue is not empty
				 * @~Japanese
				 *
				 * @brief JobOueueは空かのチェック。
				 * @details 空であればJobItemは解放できる。
				 * @retval true JobQueueが空
				 * @retval false JobQueueが空ではない
				 */
				bool isEmpty();

				/*!
				 * @~English
				 *
				 * @brief Check JobItems to call it's finish() after each JobItem is over.
				 * @details This function needs to be called once from main thread every frame.
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief JobQueue内に追加されたJobItemを監視して、終了時にfinish()を呼び出す。
				 * @details この関数はメインスレッドで1フレームに一度呼び出してください。
				 * @retval >=SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int check();

				/*!
				 * @~English
				 * @brief Change thread priority
				 * @param priority New thread priority
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief スレッドプライオリティを変更する
				 * @param priority スレッドプライオリティ
				 * @retval >=SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int changePriority(int32_t priority);

				/*!
				 * @~English
				 * @brief cancel all enqueued jobs. 
				 * @details This function reqeusts a cancel for all enqueued jobs.
				 * It can be used isEmpty() in order to confirm the completion of cancel.
				 * check() has to be called every frame during wating the completion of cancel.
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief enqueueされたすべてのjobをキャンセルする
				 * @details enqueueされたすべてのjobに対してキャンセルリクエストを行います。
				 * キャンセルの完了は、isEmpty()にて通常のJobの終了と同様に確認してください。
				 * キャンセル待ちの間も、check()をフレーム毎に呼び出す必要があります。
				 * @retval >=SCE_OK 成功
				 * @retval (<0) エラーコード
				 */
				int cancelAllItems();

			};
		}
	}
}
