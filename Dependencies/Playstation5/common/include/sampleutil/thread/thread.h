/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc.
* 
*/

#pragma once

#include <string>
#include <kernel.h>
#include "sampleutil/sampleutil_common.h"

namespace sce
{
	namespace SampleUtil
	{
		/*!
		 * @~English
		 * @brief Thread-associated definitions
		 * @details sce::SampleUtil::Thread is the name space associated with the Thread of the SampleUtil library.
		 * @~Japanese
		 * @brief Thread関連の定義
		 * @details sce::SampleUtil::ThreadはSampleUtilライブラリのThreadの名前空間です。
		 */
		namespace Thread
		{

			/*!
			 * @~English
			 * @brief Microseconds
			 * @~Japanese
			 * @brief マイクロ秒
			 */
			typedef SceKernelUseconds	Useconds;

			/*!
			 * @~English
			 * @brief ID of thread
			 * @~Japanese
			 * @brief スレッドID
			 */
			typedef ScePthread			ThreadID;

			class Thread;

			/*!
			 * @~English
			 * @brief Mutex lock attriute
			 * @~Japanese
			 * @brief ミューテックス ロック時の属性
			 */
			enum class MutexLockAttr : uint32_t
			{
				/*!
				 * @~English
				 * @brief Wait by blocking if mutex cannot be locked immediately
				 * @~Japanese
				 * @brief 即座にmutexをロックできない場合にブロッキングで待つ
				 */
				kBlocking,
				/*!
				 * @~English
				 * @brief Return from function if mutex cannot be locked immediately
				 * @~Japanese
				 * @brief 即座にmutexをロックできない場合に関数からリターンする
				 */
				kNonBlocking,
			};


			/*!
			 * @~English
			 * @brief Mutex lock attriute
			 * @~Japanese
			 * @brief ミューテックスが再帰的にロック可能か
			 */
			enum class MutexRecursiveAttr : uint32_t
			{
				/*!
				 * @~English
				 * @brief Not recursive.
				 * @~Japanese
				 * @brief 再帰ロックしない
				 */
				kNonRecursive,
				/*!
				 * @~English
				 * @brief Not recursive.
				 * @~Japanese
				 * @brief 再帰ロック可能
				 */
				kRecursive
			};

			/*!
			 * @~English
			 * @brief Mutex option
			 * @~Japanese
			 * @brief ミューテックスのオプション
			 */
			struct MutexOption
			{
				MutexOption()
				{
					m_recursive = MutexRecursiveAttr::kNonRecursive;
				}

				/*!
				 * @~English
				 * @brief Mutex recursive setting
				 * @~Japanese
				 * @brief ミューテックスの再帰ロック設定
				 */
				MutexRecursiveAttr m_recursive;
			};

			/*!
			 * @~English
			 *
			 * @brief Mutex class
			 * @details This class provides mutex funtionality
			 * @~Japanese
			 *
			 * @brief ミューテックス クラス
			 * @details ミューテックス機能を提供します。
			 */
			class Mutex
			{
			public:
				/*!
				 * @~English
				 * @brief Maximum byte size of mutex name
				 * @details Maximum byte size of mutex name (up to 32 bytes including the NULL-terminator character)
				 * @~Japanese
				 * @brief ミューテックスの名前の最大バイト数
				 * @details 終端のNULL文字を含めたミューテックスの名前の最大バイト数
				 */
				static const int kNameLenMax = 32;

				Mutex() {}
				/*!
				 * @~English
				 * @brief Constructor
				 * @param name The name of Mutex
				 * @param pOption Pointer to Mutex option struct. Mutex is initiaized with default option if null pointer is specified
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param name Mutexの名前
				 * @param pOption Mutexのオプション構造体。nullptrを指定するとデフォルトの値で初期化されます。
				 * @details コンストラクタです。
				 */
				Mutex(const char *name, const MutexOption *pOption = nullptr);

				/*!
				 * @~English
				 * @brief Destructor
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 * @details デストラクタです。
				 */
				virtual ~Mutex();

				/*!
				 * @~English
				 *
				 * @brief Obtain mutex lock
				 *
				 * @param attr Lock attribute
				 * @param pTimeout Pointer where time out value(in micro seconds) is stored. This is available if kBlocking is specified, and it waits until specified timeout value passes or lock is obtained. If null pointer is specified, it waits indefinitely until lock is obtained.
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function obtains lock by the way specified with attribute parameter.
				 * @~Japanese
				 *
				 * @brief ミューテックスをロックする
				 *
				 * @param attr ロック時の属性
				 * @param pTimeout kBlocking時のみ有効。直ちにロックできない場合に待つ時間（マイクロ秒）を格納するポインタ。nullptrの場合は、ロックできるまでブロッキングで待ち続けます。
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 属性に応じた待機方法でミューテックスをロックします。
				 */
				int lock(MutexLockAttr attr = MutexLockAttr::kBlocking, Useconds *pTimeout = nullptr);

				/*!
				 * @~English
				 *
				 * @brief Release mutex lock
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This funtion releases mutex lock
				 * @~Japanese
				 *
				 * @brief ミューテックスをアンロックする
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details ミューテックスをアンロックします。
				 */
				int unlock();

				/*!
				 * @~English
				 * @brief Initializes mutex
				 * @param name Name
				 * @param pOption Initialize otion(default option is used when nullptr is specified)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief ミューテックスの初期化
				 * @param name 名前
				 * @param pOption 初期化オプション(nullptrを指定した場合は既定オプションが使用される)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int initialize(const char	*name, const MutexOption	*pOption = nullptr);
				/*!
				 * @~English
				 * @brief Finalizes mutex
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief ミューテックスの終了処理
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int finalize();

			protected:
				ScePthreadMutex m_mutex;
			};


			/*!
			 * @~English
			 * @brief Semaphore option
			 * @~Japanese
			 * @brief セマフォのオプション
			 */
			struct SemaphoreOption {
			};

			/*!
			 * @~English
			 *
			 * @brief Semaphore class
			 * @details This class provides semaphore functionality
			 * @~Japanese
			 *
			 * @brief セマフォ クラス
			 * @details セマフォ機能を提供します。
			 */
			class Semaphore
			{
			public:
				/*!
				 * @~English
				 * @brief Maximum byte size of semaphore name
				 * @details Maximum byte size of semaphore name (up to 32 bytes including the NULL-terminator character)
				 * @~Japanese
				 * @brief セマフォの名前の最大バイト数
				 * @details 終端のNULL文字を含めたセマフォの名前の最大バイト数
				 */
				static const int kNameLenMax = 32;

				Semaphore() {}
				/*!
				 * @~English
				 * @brief Constructor
				 * @param name The name of Semaphore
				 * @param initCount Initial semaphore resource counts
				 * @param maxCount Max semaphore resource counts
				 * @param pOption Pointer to Semaphore option struct. Semaphore is initiaized with default option if null pointer is specified
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param name Semaphoreの名前
				 * @param initCount セマフォ資源数の初期値
				 * @param maxCount セマフォ資源数の最大値
				 * @param pOption Semaphoreのオプション構造体。nullptrを指定するとデフォルトの値で初期化されます。
				 * @details コンストラクタです。
				 */
				Semaphore(const char *name, int initCount, int maxCount, const SemaphoreOption *pOption = nullptr);

				/*!
				 * @~English
				 * @brief Destructor
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 * @details デストラクタです。
				 */
				virtual ~Semaphore();

				/*!
				 * @~English
				 *
				 * @brief Return semaphore resource
				 *
				 * @param count Semaphore resource counts which are returned by this function
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function returns specified counts of semaphore resources
				 * @~Japanese
				 *
				 * @brief セマフォ資源を返却する
				 *
				 * @param count 返却するセマフォ資源の個数
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details この関数は指定した数のセマフォリソースを返却する。
				 */
				int	signal(int	count);

				/*!
				 * @~English
				 *
				 * @brief Acquire semaphore resources by non-blocking
				 *
				 * @param count Semaphore resource counts to be acquired
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function acquires specified counts of semaphore resources by non-blocking
				 * @~Japanese
				 *
				 * @brief セマフォ資源をブロックせずに獲得する
				 *
				 * @param count 獲得するセマフォ資源数
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details この関数は指定した数のセマフォリソースをブロックせずに獲得します
				 */
				int	poll(int	count);

				/*!
				 * @~English
				 *
				 * @brief Acquire semaphore resources by blocking
				 *
				 * @param count Semaphore resource counts to be acquired
				 * @param pTimeout Pointer where max wait time(in micro sec) is stored on calling, and remained wait time is stored on function return. It waits indefinitely until specified counts of semaphore resources are acquired if null pointer is specified.
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function acquires specified counts of semaphore resources by blocking
				 * @~Japanese
				 *
				 * @brief セマフォ資源の獲得を待つ
				 *
				 * @param count 獲得するセマフォ資源数
				 * @param pTimeout 待ち時間の上限（マイクロ秒）および残り時間の格納先。nullptrが指定された場合は指定した数のセマフォリソースを獲得するまで待ち続ける
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details
				 */
				int	wait(int	count, Useconds	*pTimeout = nullptr);

				/*!
				 * @~English
				 * @brief Initializes semaphore
				 * @param name Name
				 * @param initCount Initial resource count
				 * @param maxCount Max resource count
				 * @param pOption Initialize otion(default option is used when nullptr is specified)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief セマフォの初期化
				 * @param name 名前
				 * @param initCount 初期資源数
				 * @param maxCount 最大資源数
				 * @param pOption 初期化オプション(nullptrを指定した場合は既定オプションが使用される)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int initialize(const char *name, int initCount, int maxCount, const SemaphoreOption *pOption = nullptr);
				/*!
				 * @~English
				 * @brief Finalizes semaphore
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief セマフォの終了処理
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int finalize();

			protected:
				SceKernelSema  m_sema;
			};

			/*!
			 * @~English
			 * @brief Condition variable option
			 * @~Japanese
			 * @brief 条件変数のオプション
			 */
			struct CondOption {

			};

			/*!
			 * @~English
			 *
			 * @brief Condition variable class
			 * @details This class provides condition variable functionality
			 * @~Japanese
			 *
			 * @brief 条件変数 クラス
			 * @details 条件変数の機能を提供します。
			 */
			class Cond {
			public:
				/*!
				 * @~English
				 * @brief Maximum byte size of condition variable name
				 * @details Maximum byte size of condition variable name (up to 30 bytes including the NULL-terminator character)
				 * @~Japanese
				 * @brief 条件変数の名前の最大バイト数
				 * @details 終端のNULL文字を含めたスレッドの名前の最大バイト数。
				 */
				static const int kNameLenMax = 30;

				Cond() {}
				/*!
				 * @~English
				 * @brief Constructor
				 * @param name The name of Cond
				 * @param pOption Pointer to Cond option struct. Cond is initiaized with default option if null pointer is specified
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param name Condの名前
				 * @param pOption Condのオプション構造体。nullptrを指定するとデフォルトの値で初期化されます。
				 * @details コンストラクタです。
				 */
				Cond(const char *name, const CondOption *pOption = nullptr);

				/*!
				 * @~English
				 * @brief Destructor
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 * @details デストラクタです。
				 */
				virtual ~Cond();

				/*!
				 * @~English
				 *
				 * @brief Wake up all threads waiting on condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function wakes up all threads waiting on this condition variable
				 * @~Japanese
				 *
				 * @brief 条件変数で待っているすべてのスレッドを起床させる
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details この関数はこの条件変数を待っている全てのスレッドを起床させる
				 */
				int broadcast();

				/*!
				 * @~English
				 *
				 * @brief Wake up a thread waiting on condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function wakes up a thread waiting on this condition variable
				 * @~Japanese
				 *
				 * @brief 条件変数で待っているスレッドを起床させる
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details この関数はこの条件変数で待っているスレッドを起床させる
				 */
				int signal();

				/*!
				 * @~English
				 *
				 * @brief Wake up specified thread waiting on condition variable
				 *
				 * @param pThread Thread to be woken up
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function wakes up specified thread waiting on this condition variable
				 * @~Japanese
				 *
				 * @brief 条件変数で待っている特定のスレッドを起床させる
				 *
				 * @param pThread 起床させるスレッド
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details この関数はこの条件変数で待っている指定されたスレッドを起床させる
				 */
				int signalTo(Thread *pThread);

				/*!
				 * @~English
				 *
				 * @brief Wait by condition variable
				 *
				 * @param pTimeout Pointer to store max timeout value(in micro sec) or null pointer.
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details If null pointer is specified to pTimeout, it waits indefinitely until condition is met.
				 * @~Japanese
				 *
				 * @brief 条件変数で待つ
				 *
				 * @param pTimeout 待ち時間の上限（マイクロ秒）の格納先、またはnullptr
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details pTimeoutがnullptrの場合は、条件が成立するまで無限にブロックします。
				 */
				int	wait(Useconds	*pTimeout = nullptr);

				/*!
				 * @~English
				 *
				 * @brief Lock a mutex to be paired with a condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief 条件変数とペアになるミューテックスをロック
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int lock();

				/*!
				 * @~English
				 *
				 * @brief Unlock a mutex to be paired with a condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief 条件変数とペアになるミューテックスをアンロック
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int unlock();

				/*!
				 * @~English
				 * @brief Initializes condition variable
				 * @param name Name
				 * @param pOption Initialize otion(default option is used when nullptr is specified)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief 条件変数の初期化
				 * @param name 名前
				 * @param pOption 初期化オプション(nullptrを指定した場合は既定オプションが使用される)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int initialize(const char *name, const CondOption *pOption = nullptr);
				/*!
				 * @~English
				 * @brief Finalizes condition variable
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief 条件変数の終了処理
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int finalize();

			protected:
				ScePthreadCond  m_cond;
				ScePthreadMutex m_mutex;
			};

			/*!
			 * @~English
			 * @brief Reader/Writer lock option
			 * @~Japanese
			 * @brief リーダ/ライタロックのオプション
			 */
			struct RwlockOption {

			};


			/*!
			 * @~English
			 *
			 * @brief Reader/Writer lock class
			 * @details This class provides Reader/Writer lock functionality
			 * @~Japanese
			 *
			 * @brief リーダ/ライタロック クラス
			 * @details リーダ/ライタロックの機能を提供します。
			 */
			class Rwlock {
			public:
				/*!
				 * @~English
				 * @brief Maximum byte size of Reader/Writer lock name
				 * @details Maximum byte size of Reader/Writer lock name (up to 30 bytes including the NULL-terminator character)
				 * @~Japanese
				 * @brief リーダ/ライタロックの名前の最大バイト数
				 * @details 終端のNULL文字を含めたリーダ/ライタロックの名前の最大バイト数。
				 */
				static const int kNameLenMax = 32;

				Rwlock() {}
				/*!
				 * @~English
				 * @brief Constructor
				 * @param name The name of Rwlock
				 * @param pOption Pointer to Rwlock option struct. Rwlock is initiaized with default option if null pointer is specified
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param name Rwlockの名前
				 * @param pOption Rwlockのオプション構造体。nullptrを指定するとデフォルトの値で初期化されます。
				 * @details コンストラクタです。
				 */
				Rwlock(const char *name, const RwlockOption *pOption = nullptr);

				/*!
				 * @~English
				 * @brief Destructor
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 * @details デストラクタです。
				 */
				virtual ~Rwlock();

				/*!
				 * @~English
				 *
				 * @brief Lock a read/write lock for reading
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief リーダ/ライタロックを読み込み用にロックする
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int rdlock();

				/*!
				 * @~English
				 *
				 * @brief Lock a read/write lock for writing
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief リーダ/ライタロックを書き込み用にロックする
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int wrlock();

				/*!
				 * @~English
				 *
				 * @brief Unlock a Reader/Writer lock
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 *
				 * @brief リーダ/ライタロックのロックを解除する
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int unlock();

				/*!
				 * @~English
				 * @brief Initializes Reader/Writer lock
				 * @param name Name
				 * @param pOption Initialize otion(default option is used when nullptr is specified)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief リーダ/ライタロックの初期化
				 * @param name 名前
				 * @param pOption 初期化オプション(nullptrを指定した場合は既定オプションが使用される)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int initialize(const char *name, const RwlockOption *pOption = nullptr);
				/*!
				 * @~English
				 * @brief Finalizes Reader/Writer lock
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief リーダ/ライタロックの終了処理
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int finalize();

			protected:
				ScePthreadRwlock m_rwlock;
			};

			/*!
			 * @~English
			 * @brief LockableObject option
			 * @~Japanese
			 * @brief LockableObjectのロックオプション
			 */
			enum class LockableObjectAccessAttr : uint32_t
			{
				/*!
				 * @~English
				 * @brief Ontain lock for read access
				 * @~Japanese
				 * @brief 読み込みアクセス用のロックを取得
				 */
				kRead = 1 << 0,
				/*!
				 * @~English
				 * @brief Ontain lock for write access
				 * @~Japanese
				 * @brief 書き込みアクセス用のロックを取得
				 */
				kWrite = 1 << 1
			};

			/*!
			 * @~English
			 *
			 * @brief Base class for lockable object class
			 * @details The class which inherits this class has access protection by read/write lock 
			 * @~Japanese
			 *
			 * @brief ロック可能なオブジェクトの基本クラス
			 * @details このクラスを継承したクラスはリーダ・ライタロックによりアクセス保護の機能を持ちます
			 *
			 *  class SomeObject : public LockableObject { ...
			 *
			 *  SomeObject *obj = new SomeObject(...
			 *
			 *  obj->lock();
			 *
			 *  obj->... アクセス
			 *
			 *  obj->unlock();
			 *
			 */
			class LockableObject
			{
			public:
				/*!
				 * @~English
				 *
				 * @brief lockable object counstructor
				 * @details Please call destructor to finalize LockableObject instead of destroy(), because it is used to inherits this class.
				 *
				 * @~Japanese
				 *
				 * @brief ロック可能なオブジェクトのコンストラクタ
				 * @details このクラスは継承して使用するのでコンストラクタで生成、デストラクタで破棄します
				 */
				LockableObject()
				{
					m_rwlock = new Rwlock("", nullptr);
					SCE_SAMPLE_UTIL_ASSERT(m_rwlock != nullptr);
				}

				/*!
				 * @~English
				 * @brief Destructor
				 * @details This is a destructor.
				 * @~Japanese
				 * @brief デストラクタ
				 * @details デストラクタです。
				 */
				virtual ~LockableObject()
				{
					SCE_SAMPLE_UTIL_SAFE_DELETE(m_rwlock);
				}

				/*!
				 * @~English
				 *
				 * @brief Lock an object
				 *
				 * @param attr Access protection specified by LockableObjectAccessAttr enum values
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Lock an instance of this class with specified access protection attributes
				 * @~Japanese
				 *
				 * @brief オブジェクトをロック
				 *
				 * @param attr アクセス保護属性。LockableObjectAccessAttrで指定
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details このクラスのインスタンスを指定したアクセス保護属性でロックします。
				 */
				int	lock(LockableObjectAccessAttr attr);

				/*!
				 * @~English
				 *
				 * @brief Unlock an object
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Unlock an instance of this class
				 * @~Japanese
				 *
				 * @brief オブジェクトをアンロック
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details このクラスのインスタンスをアンロックします
				 */
				int	unlock()
				{
					return m_rwlock->unlock();
				}

			private:
				Rwlock *m_rwlock;
			};

			/*!
			 * @~English
			 *
			 * @brief Scoped lock class
			 * @details Instance of this class creates lock section which starts on construction and ends on destruction of this class
			 * @~Japanese
			 *
			 * @brief スコープロック
			 * @details このクラスのインスタンスはインスタンス生成時に始まりインスタンス破棄時に終了するロックセクションを生成する
			 *
			 *
			 *  {
			 *
			 *    ScopedLock(obj, LocableObject::kAccessWrite);
			 *
			 *    obj->....  // アクセス
			 *
			 *  }
			 *
			 * LockableObjectを簡易に使うためのヘルパクラスです。
			 * SampleUtilのリソースではないため、destory()で破棄出来ません。
			 *
			 */
			class ScopedLock
			{
			public:
				/*!
				 * @~English
				 *
				 * @brief Constructor
				 *
				 * @param obj An object to be locked
				 * @param attr Access protection specified by logical OR of AccessAttr enum values
				 *
				 * @details Lock is obtained on constructor call. An object to be locked needs to be class instance which inherits LockableObject
				 * @~Japanese
				 *
				 * @brief コンストラクタ
				 *
				 * @param obj ロックするオブジェクト
				 * @param attr アクセス保護属性。AccessAttrの論理和で指定
				 *
				 * @details コンストラクタ呼び出し時にロックを取得。LockableObjectを継承したクラスである必要がある
				 */
				ScopedLock(LockableObject *obj, LockableObjectAccessAttr attr)
					: m_obj(obj)
				{
					m_obj->lock(attr);
				}

				/*!
				 * @~English
				 *
				 * @brief Destructor
				 * @details Lock is released on destructor call.
				 * @~Japanese
				 *
				 * @brief デストラクタ
				 * @details デストラクタ呼び出し時にロックを開放
				 */
				~ScopedLock()
				{
					m_obj->unlock();
				}
			private:
				/*!
				 * @~English
				 *
				 * @brief An object instance which is being locked
				 * @~Japanese
				 *
				 * @brief ロック中のオブジェクトインスタンス
				 */
				LockableObject	*m_obj;
			};

			/*!
			 * @~English
			 *
			 * @brief Condition variable template class
			 * @details Condition variable template class of specified variable type
			 * @details Please call destructor to finalize CondVar instead of destroy()
			 * @~Japanese
			 *
			 * @brief 条件変数テンプレートクラス
			 * @details 指定した型の条件変数テンプレートクラスです。
			 * @details destory()ではなく、デストラクタで破棄して下さい。
			 */
			template<typename T>
			class CondVar
			{
			public:
				/*!
				 * @~English
				 * @brief Maximum byte size of condition variable name
				 * @details Maximum byte size of condition variable name (up to 30 bytes including the NULL-terminator character)
				 * @~Japanese
				 * @brief 条件変数の名前の最大バイト数
				 * @details 終端のNULL文字を含めた条件変数の名前の最大バイト数
				 */
				static const int kNameLenMax = Cond::kNameLenMax;

				/*!
				 * @~English
				 *
				 * @brief Constructor
				 *
				 * @param name The name of condition variable
				 *
				 * @~Japanese
				 *
				 * @brief コンストラクタ
				 *
				 * @param name 条件変数の名前
				 */
				CondVar(const char *name)
				{
					m_cond = new Cond(name);
				}

				/*!
				 * @~English
				 * @brief Destructor
				 * @~Japanese
				 * @brief デストラクタ
				 */
				virtual ~CondVar()
				{
					SCE_SAMPLE_UTIL_SAFE_DELETE(m_cond);
				}

				/*!
				 * @~English
				 *
				 * @brief Wait until the condition of condition variable is met
				 *
				 * @param pCondFunc Comparison function
				 * @param arg Argument of comparison function
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details condFunc is called with argument of arg, and waits until condFunc returns true.
				 * @~Japanese
				 *
				 * @brief 条件変数が条件を満たすまで待つ
				 * @param pCondFunc 比較関数
				 * @param arg 比較関数への引数
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details condFuncが引数argを指定して呼ばれ、trueを返すまで待ち続ける。
				 */
				int waitCond(bool(*pCondFunc)(uint64_t), uint64_t arg)
				{
					m_cond->lock();
					while (!pCondFunc(arg))
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Wait condition variable until equal condition is met
				 *
				 * @param compVal A value to be compared with condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Wait until condition variable value becomes equal to compVal
				 *
				 * @~Japanese
				 *
				 * @brief 条件Equalで条件変数を待つ
				 *
				 * @param compVal 条件変数と比較する値
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 条件変数がcompValになるまで待機
				 */
				int waitEqual(const T &compVal)
				{
					m_cond->lock();
					while (m_varVal != compVal)
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Wait condition variable until not-equal condition is met
				 *
				 * @param compVal A value to be compared with condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Wait until condition variable value becomes not equal to compVal
				 *
				 * @~Japanese
				 *
				 * @brief 条件NotEqualで条件変数を待つ
				 *
				 * @param compVal 条件変数と比較する値
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 条件変数がcompValと異なる値となるまで待機
				 */
				int waitNotEqual(const T &compVal)
				{
					m_cond->lock();
					while (m_varVal == compVal)
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Wait condition variable until greater condition is met
				 *
				 * @param compVal A value to be compared with condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Wait until condition variable value becomes greater than compVal
				 *
				 * @~Japanese
				 *
				 * @brief 条件Greaterで条件変数を待つ
				 *
				 * @param compVal 条件変数と比較する値
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 条件変数がcompValより大きい値となるまで待機
				 */
				int waitGreater(const T &compVal)
				{
					m_cond->lock();
					while (!(m_varVal > compVal))
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Wait condition variable until greater-equal condition is met
				 *
				 * @param compVal A value to be compared with condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Wait until condition variable value becomes greater than or equal to compVal
				 *
				 * @~Japanese
				 *
				 * @brief 条件GreaterEqualで条件変数を待つ
				 *
				 * @param compVal 条件変数と比較する値
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 条件変数がcompVal以上の値となるまで待機
				 */
				int waitGreaterEqual(const T &compVal)
				{
					m_cond->lock();
					while (!(m_varVal >= compVal))
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Wait condition variable until less condition is met
				 *
				 * @param compVal A value to be compared with condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Wait until condition variable value becomes less than compVal
				 *
				 * @~Japanese
				 *
				 * @brief 条件Lessで条件変数を待つ
				 *
				 * @param compVal 条件変数と比較する値
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 条件変数がcompVal未満の値となるまで待機
				 */
				int waitLess(const T &compVal)
				{
					m_cond->lock();
					while (!(m_varVal < compVal))
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Wait condition variable until less-equal condition is met
				 *
				 * @param compVal A value to be compared with condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details Wait until condition variable value becomes less than or equal to compVal
				 *
				 * @~Japanese
				 *
				 * @brief 条件LessEqualで条件変数を待つ
				 *
				 * @param compVal 条件変数と比較する値
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 条件変数がcompVal以下の値となるまで待機
				 */
				int waitLessEqual(const T &compVal)
				{
					m_cond->lock();
					while (!(m_varVal <= compVal))
					{
						m_cond->wait();
					}
					m_cond->unlock();
					return SCE_OK;
				}

				/*!
				 * @~English
				 *
				 * @brief Set a value to condition variable
				 *
				 * @param newValue New value to be set to condition variable
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details
				 * @~Japanese
				 *
				 * @brief 条件変数に値を設定
				 * @param newValue 設定する値
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int set(const T & newValue)
				{
					m_cond->lock();

					m_varVal = newValue;
					m_cond->broadcast();

					m_cond->unlock();

					return SCE_OK;
				}
			protected:
				/*!
				 * @~English
				 *
				 * @brief Condition variable
				 * @~Japanese
				 *
				 * @brief 条件変数
				 */
				Cond *m_cond;
				/*!
				 * @~English
				 *
				 * @brief A variable to store condition variable value
				 * @~Japanese
				 *
				 * @brief 条件変数の値を格納する変数
				 */
				T m_varVal;
			};


			/*!
			 * @~English
			 * @brief Thread option
			 * @~Japanese
			 * @brief Threadの動作オプション構造体
			 */
			struct ThreadOption
			{
				/*!
				 * @~English
				 *
				 * @brief Thread attribute
				 * @~Japanese
				 *
				 * @brief スレッド属性
				 */
				uint32_t m_attr;
				/*!
				 * @~English
				 *
				 * @brief CPU mask value
				 * @~Japanese
				 *
				 * @brief CPUマスク
				 */
				int32_t m_cpuMask;
				/*!
				 * @~English
				 *
				 * @brief Memory type
				 * @~Japanese
				 *
				 * @brief メモリタイプ
				 */
				uint32_t m_memType;

				ThreadOption() : m_attr(0), m_cpuMask(0), m_memType(0) {}
			};

			/*!
			 * @~English
			 *
			 * @brief Base class to describe processings to be executed by thread
			 * @details If you have some processings that you want to make them run as thread, create new class which inherits this class to describe new thread processings. The processing contents are described by overriding run() virtual function. Note that this class doesn't inherit Resource class hence instance of this class cannot be destroyed by destroy().
			 * @~Japanese
			 *
			 * @brief スレッドで実行したい処理の実装するための基本クラス
			 * @details Threadクラスで実行したい処理は、このクラスの派生クラスを定義して、run()をオーバライドして下さい。このクラスはResourceクラスを派生しません。destory()では破棄できませんので注意してください。
			 */
			class ThreadFunction
			{
			public:
				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~ThreadFunction() {}

				/*!
				 * @~English
				 * @brief Implement this function with processings you want to run as thread.
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief スレッドで実行したい処理を実装
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				virtual int run() = 0;

				/*!
				 * @~English
				 * @brief Suspend thread execution for specified amount of time
				 * @param milisec suspend duration time (in mili sec)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @~Japanese
				 * @brief スレッドの実行を一定時間停止する
				 * @param milisec 停止時間 (ミリ秒単位)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				static int sleep(int milisec);
			};

			/*!
			 * @~English
			 *
			 * @brief Platform independent thread class
			 * @details After creating instance of Thread class by calling createThread(), you can start thread by passing sub class object of ThreadFunction class to start().
			 * @~Japanese
			 *
			 * @brief プラットフォーム非依存のスレッドクラス
			 * @details ThreadクラスをcreateThreadにて生成後、処理実装のあるThreadFunctionのサブクラスオブジェクトをstart()に渡してスレッドを起動します。
			 */
			class Thread 
			{
			public:
				/*!
				 * @~English
				 * @brief Maximum byte size of thread name
				 * @details Maximum byte size of thread name (up to 32 bytes including the NULL-terminator character)
				 * @~Japanese
				 * @brief スレッドの名前の最大バイト数
				 * @details 終端のNULL文字を含めたスレッドの名前の最大バイト数
				 */
				static const int kNameLenMax = 32;

				Thread();
				/*!
				 * @~English
				 * @brief Constructor
				 * @param name The name of Thread
				 * @param priority Thread priority
				 * @param stackSize Thread stack size
				 * @param option Pointer to Thread option struct. Thread is initiaized with default option if null pointer is specified
				 * @details This is a constructor.
				 * @~Japanese
				 * @brief コンストラクタ
				 * @param name Threadの名前
				 * @param priority Threadのプライオリティ
				 * @param stackSize Threadのスタックサイズ
				 * @param option Threadのオプション構造体。nullptrを指定するとデフォルトの値で初期化されます。
				 * @details コンストラクタです。
				 */
				Thread(const char *name, int32_t priority, uint32_t stackSize, const ThreadOption *option = nullptr);

				/*!
				 * @~English
				 * @brief Destructor 
				 * @details This is a destructor. 
				 * @~Japanese
				 * @brief デストラクタ 
				 * @details デストラクタです。 
				 */
				virtual ~Thread();

				/*!
				 * @~English
				 *
				 * @brief Start a thread
				 *
				 * @param pFuncObj Pointer to an instance of ThreadFunction derived class where processings you want to run as thread are described.
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details A thread gets started, and pFuncObj->run() is executed by that thread. start() can be called only once per Thread object.
				 * @~Japanese
				 *
				 * @brief スレッドの開始
				 *
				 * @param pFuncObj スレッドで処理させてたい実装を記述するThreadFunction派生クラスオブジェクトのポインタ
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details 起動したスレッドにて、pFuncObj->run()が実行されます。一つのThreadオブジェクトにつき、start()は1度だけしか呼ぶことができません。
				 */
				int start(ThreadFunction *pFuncObj);

				/*!
				 * @~English
				 *
				 * @brief Wait for thread completion
				 *
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 * @details This function blocks caller thread until thread finishes processings.
				 * @~Japanese
				 *
				 * @brief スレッドの終了待ち
				 *
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 * @details　スレッドの実行を終了するまで、呼び出しもとのスレッドをサスペンドさせます。
				 */
				int join();

				/*!
				 * @~English
				 *
				 * @brief Check if a thread is alive
				 *
				 * @retval true This thread is alive
				 * @retval false This thread is not alive
				 * @~Japanese
				 *
				 * @brief スレッドが生存しているかどうかをチェックする
				 * @retval true スレッドは生存している
				 * @retval false スレッドは生存していない
				 */
				bool isAlive();

				/*!
				 * @~English
				 *
				 * @brief Obtain thread ID
				 * @return thread ID
				 * @details
				 * @~Japanese
				 *
				 * @brief スレッドIDを取得する
				 * @return スレッドID
				 */
				ThreadID getId();

				/*!
				 * @~English
				 *
				 * @brief Change thread priority
				 * @param priority New priority
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 *
				 * @~Japanese
				 *
				 * @brief スレッドプライオリティを変更する
				 * @param priority スレッドプライオリティ
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				int changePriority(int32_t priority);

				/*!
				 * @~English
				 *
				 * @brief Obtain registered ThreadFunction class instance
				 * @return Pointer to an instance of ThreadFunction class
				 * @details
				 * @~Japanese
				 *
				 * @brief 登録されているThreadFunctionを取得する
				 * @return ThreadFunctionのポインタ
				 */
				ThreadFunction *getFuncObject()
				{
					return m_pFuncObj;
				}

				/*!
				 * @~English
				 *
				 * @brief Suspend thread execution for specified amount of time
				 * @param milisec suspend duration time (in mili sec)
				 * @retval >=SCE_OK Success
				 * @retval (<0) Error code
				 *
				 * @~Japanese
				 *
				 * @brief 自スレッドの実行を一定時間停止する
				 * @param milisec 停止時間 (ミリ秒単位)
				 * @retval  SCE_OK 成功。
				 * @retval (<0) エラーコード
				 */
				static int sleep(int milisec);

				int initialize(const char *name, int32_t priority, uint32_t stackSize, const ThreadOption *pOption = nullptr);
				int finalize();
				void setDefaultOption(const ThreadOption &option);

			private:
				static void			*threadFunc(void *pArgs);
				static ThreadOption m_defaultOption;

				ThreadID			m_thID;				///< スレッドID
				std::string			m_threadName;
				ScePthreadAttr		m_attr;
				bool				m_started;			///< スレッドがスタートしたことがあるか
				bool				m_alive;			///< スレッドが生存中であるか
				ThreadFunction		*m_pFuncObj;
			};
		}
	}
}
