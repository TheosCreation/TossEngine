/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2021 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

#include <string>
#include <list>
#include <atomic>
#include <sampleutil/thread/thread.h>

namespace sce {	namespace SampleUtil { namespace Thread	{
	class JobQueue;
	class JobThreadFunction;
	class JobItemQueue;
	/*!
	 * @~English
	 * @brief JobQueue behavior option
	 * @~Japanese
	 * @brief JobQueueの動作オプション
	 */
	struct JobQueueOption
	{
		/*!
		 * @~English
		 * @brief Default Constructor of JobQueueOption
		 * @~Japanese
		 * @brief JobQueueOptionのデフォルトコンストラクタ
		 */
		JobQueueOption();

		/*!
		 * @~English
		 * @brief Priority of threads created inside JobQueue
		 * @~Japanese
		 * @brief JobQueueが内部で生成するスレッドのプライオリティ
		 */
		int32_t m_priority;
		/*!
		 * @~English
		 * @brief Stack size of threads created inside JobQueue
		 * @~Japanese
		 * @brief JobQueueが内部で生成するスレッドのスタックサイズ
		 */
		uint32_t m_stackSize;
		/*!
		 * @~English
		 * @brief The number of threads created inside JobQueue
		 * @~Japanese
		 * @brief JobQueueが内部で生成するスレッドの本数
		 */
		uint32_t m_numThread;
		/*!
		 * @~English
		 * @brief Thread option of threads which JobQueue creates internally
		 * @~Japanese
		 * @brief JobQueueが内部で生成するスレッドのオプション
		 */
		const ThreadOption *m_threadOption;
	};

	/*!
	 * @~English
	 * @brief Struct of Job behavior option
	 * @~Japanese
	 * @brief Jobの動作オプション構造体
	 */
	struct JobItemOption
	{
		JobItemOption();
	};

	/*!
	 * @~English
	 * @brief JobItem status
	 * @~Japanese
	 * @brief JobItemのステータス
	 */
	enum JobItemStatus
	{
		/*!
		 * @~English
		 * @brief Job is not executing
		 * @~Japanese
		 * @brief Jobは実行していない
		 */
		kNone = 0,

		/*!
		 * @~English
		 * @brief Job is waiting
		 * @~Japanese
		 * @brief Jobは実行待ち状態
		 */
		kWaiting,

		/*!
		 * @~English
		 * @brief Job is running
		 * @~Japanese
		 * @brief Jobは実行中
		 */
		kRunning,

		/*!
		 * @~English
		 * @brief This is a special job to finailize JobQueue system.
		 * @~Japanese
		 * @brief JobはJobQueueシステムを終了させる特別なJob
		 */
		kFinishJob,
	};

	/*!
	 * @~English
	 * @brief The class whose instance contains Job processings.
	 * @details JobQueue system doesn't manage the lifetime of JobItem. Application needs to manage the lifetime of JobItem. Also, since JobQueue doesn't inherit Resource class, use normal constructo/destructo to create/destroy JobQueue instance.
	 * @~Japanese
	 * @brief Jobの1つの処理を表現するクラス
	 * @details JobQueueシステムは、JobItemの寿命管理はしません。アプリケーション側の責任で管理してください。また、Resourceの派生クラスではないユーザー管理のクラスとなるため生成・破棄は通常のコンストラクタ、デストラクタを使用してください。
	 */
	class JobItem
	{
		friend class JobQueue;
		friend class JobThreadFunction;
		friend class JobItemQueue;
	public:
		/*!
		 * @~English
		 *
		 * @brief Constructor of JobItem
		 *
		 * @param name The name of JobItem
		 * @param option Behavior option
		 *
		 * @~Japanese
		 *
		 * @brief JobItemのコンストラクタ
		 *
		 * @param name JobItemの名前
		 * @param option 動作オプション
		 */
		JobItem(const char *name, const JobItemOption *option = 0);

		JobItem(const JobItem &src)
			: m_name	(src.m_name)
			, m_status	(src.m_status.load())
			, m_result	(src.m_result)
			, m_option	(src.m_option)
		{}

		/*!
		 * @~English
		 * @brief Destructor of JobItem
		 * @~Japanese
		 * @brief JobItemのデストラクタ
		 */
		virtual ~JobItem();

		/*!
		 * @~English
		 * @brief Returns if JobItem is executing
		 * @return true if JobItem is not executing
		 * @~Japanese
		 * @brief JobItemが実行中かどうかを返す
		 * @return JobItemが実行していない場合はtrueを返す
		 */
		bool	isNone();
		/*!
		 * @~English
		 * @brief Obtains JobItem's return code
		 * @return JobItem's return code
		 * @~Japanese
		 * @brief JobItemの実行結果を取得
		 * @return JobItemのリターンコード
		 */
		int		getResult();

	protected:

		/*!
		 * @~English
		 *
		 * @brief Virtual function to describe Job's sub thread processing
		 * @return Job's return value. 
		 * @details　This function is called in sub thread
		 * @~Japanese
		 *
		 * @brief Jobのサブスレッド処理を記述するための仮想関数
		 * @return 結果値。
		 * @details　この関数は、サブスレッドの中で呼ばれます。
		 */
		virtual int run() = 0;

	private:
		std::string	m_name;
		std::atomic<int> m_status;
		int32_t m_result;
		JobItemOption m_option;
	public:
		/*!
		 * @~English
		 * @brief Get the name of job
		 * @return Job's name
		 * @~Japanese
		 * @brief Jobの名前を取得
		 * @return Jobの名前
		 */
		const char* getName() const
		{
			return m_name.c_str();
		}
	};

	/*!
	 * @~English
	 *
	 * @brief Job queue class which executes multiple jobs on multiple worker threads
	 * @~Japanese
	 *
	 * @brief 複数のJobをマルチスレッドで処理するためのJobキュークラス
	 */
	class JobQueue
	{
	public:

		JobQueue();
		/*!
		 * @~English
		 *
		 * @brief Constructor
		 * @param pOption Struct of JobQueue option. JobQueue is initialized with default value if nullptr is specified.
		 * @details This is a constructor.
		 * @~Japanese
		 *
		 * @brief コンストラクタ
		 * @param pOption JobQueueのオプション構造体。nullptrを指定するとデフォルトの値で初期化されます。
		 * @details コンストラクタです。
		 */
		JobQueue(JobQueueOption *pOption);

		/*!
		 * @~English
		 *
		 * @brief Destructor of JobQueue
		 * @details Before destroying JobQueue job queue needs to be empty.
		 * @~Japanese
		 *
		 * @brief JobQueueのデストラクタ
		 * @details JobQueueを破棄するには、キューが空になっている必要があります。
		 */
		~JobQueue();

		/*!
		 * @~English
		 *
		 * @brief Initialize JobQueue
		 *
		 * @param name The name of JobQueue
		 * @param option JobQueueOption struct. Default value is used if nullptr is specified.
		 *
		 * @retval >=SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 *
		 * @brief JobQueueの初期化
		 *
		 * @param name JobQueueの名前
		 * @param option JobQueueOption構造体。nullptrを指定するとデフォルト値が使用されます。
		 *
		 * @retval >=SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int initialize(const char * name, const JobQueueOption * option = nullptr);

		/*!
		 * @~English
		 *
		 * @brief Finalize JobQueue
		 *
		 * @retval >=SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 *
		 * @brief JobQueueの終了処理
		 *
		 * @retval >=SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int finalize();

		/*!
		 * @~English
		 *
		 * @brief Enqueue JobItem object to job queue
		 *
		 * @param jobItem JobItem object
		 *
		 * @retval >=SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 *
		 * @brief JobItemオブジェクトをキューの末尾に追加
		 *
		 * @param jobItem JobItemオブジェクト
		 *
		 * @retval >=SCE_OK 成功
		 * @retval (<0) エラーコード
		 */
		int enqueue(JobItem* jobItem);

		/*!
		 * @~English
		 *
		 * @brief Get the number of JobItem objects enqueued in queue
		 *
		 * @return The number of JobItem objects enqueued in queue
		 *
		 * @~Japanese
		 *
		 * @brief キューに登録されているJobItemオブジェクトの個数を取得する
		 *
		 * @return キューに登録されているJobItemオブジェクトの個数
		 */
		uint32_t numItems() const;

		/*!
		 * @~English
		 *
		 * @brief Wait until job queue gets empty
		 * @details This function needs to be called from sub thread. If you call this function from main thread or from run() of a Job queued in JobQueue, deadlock will occur.
		 * @~Japanese
		 *
		 * @brief キューが空になるまで待機する
		 * @details この関数は、サブスレッドの中で呼び出す必要があります。もし、この関数をメインスレッドで呼び出したり、このJobQueueに登録されているJobのrunのなかで呼び出したりすると、デッドロックが発生します。
		 */
		void waitEmpty();

		/*!
		 * @~English
		 * @brief Change thread priority
		 * @param priority New thread priority
		 * @~Japanese
		 * @brief スレッドプライオリティを変更する
		 * @param priority スレッドプライオリティ
		 */
		void changePriority(int32_t priority);

		/*!
		 * @~English
		 *
		 * @brief Check JobItems queued in JobQueue
		 * @details This function needs to be called once from main thread every frame.
		 * @~Japanese
		 *
		 * @brief JobQueue内に追加されたJobItemを監視する。
		 * @details この関数はメインスレッドで1フレームに一度呼び出してください。
		 */
		void check();

		/*!
		 * @~English
		 * @brief Returns option specified to JobQueue initialize
		 * @param option Option specified to JobQueue initialize
		 * @~Japanese
		 * @brief JobQueue初期化に指定したオプションを返す
		 * @param option JobQueue初期化に指定したオプション
		 */
		void getOption(JobQueueOption* option);

	private:
		friend class JobThreadFunction;
		int enqueueCore(JobItem* jobItem, bool isEndJob);

		std::string m_name;

		typedef std::list<Thread*> ThreadList;
		ThreadList m_threadList;

		JobItemQueue* m_itemList;
		JobQueueOption m_option;
	};
} } } // namespace sce::SampleUtil::Thread

