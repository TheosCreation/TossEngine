/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2024 Sony Interactive Entertainment Inc. 
 * 
 */
#pragma once
#include <cstddef>
#include <sys/types.h>
#include <sys/dmem.h>
#include <kernel.h>

#ifndef SCE_SAMPLE_UTIL_DISABLE_AMPR
#define SCE_SAMPLE_UTIL_DISABLE_AMPR 1
#endif

namespace sce { namespace SampleUtil { namespace Memory {
	/*!
	 * @~English
	 * 
	 * @brief Structure for initializing dmem mapper(AMM)
	 * @details This is the structure for initializing dmem mapper(AMM). This is used by specifying it to the argument "option" of initializeMapper(). 
	 * @~Japanese
	 * 
	 * @brief ダイレクトメモリマッパー(AMM)の初期化用構造体 
	 * @details ダイレクトメモリマッパー(AMM)の初期化用構造体です。initializeMapper()の引数optionに指定することで利用します。 
	 */
	struct MemoryOption
	{
		/*!
		 * @~English
		 *
		 * @brief Specify VA range start
		 * @~Japanese
		 *
		 * @brief VAレンジ開始アドレス
		 */
		uint64_t	m_vaStart;
		/*!
		 * @~English
		 *
		 * @brief Specify VA range end
		 * @~Japanese
		 *
		 * @brief VAレンジ終了アドレス
		 */
		uint64_t	m_vaEnd;
		/*!
		 * @~English
		 *
		 * @brief Specify The amoumt of direct memory of AUTO usage
		 * @~Japanese
		 *
		 * @brief AUTOのダイレクトメモリのサイズ
		 */
		size_t		m_autoDirectMemorySize;
		/*!
		 * @~English
		 *
		 * @brief Specify The amoumt of direct memory of DIRECT usage
		 * @~Japanese
		 *
		 * @brief DIRECTのダイレクトメモリのサイズ
		 */
		size_t		m_directDirectMemorySize;

		MemoryOption()
			: m_vaStart					(0x80000000000ul)
			, m_vaEnd					(0x88000000000ul)
			, m_autoDirectMemorySize	(6ul * 1024ul * 1024ul * 1024ul)
			, m_directDirectMemorySize	(2ul * 1024ul * 1024ul * 1024ul)
		{}
	};

	/*!
	 * @~English
	 *
	 * @brief Initializes direct memory mapper(AMM)
	 * @param pOption Specify option to initialize dmem mapper. default option is used when null pointer is specified.
	 * @details AUTO/DIRECT memory is allocated by calling Ampr\:: Amm\:: giveDirectMemory, and specified VA address range is registered for sampleutil's memory allocator. VA range specified by option must not be used by application.
	 * @retval SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese 
	 *
	 * @brief ダイレクトメモリマッパー(AMM)を初期化する
	 * @param pOption ダイレクトメモリマッパー初期化のためのオプション。NULLポインタ指定時には既定のオプションが使用される。
	 * @details AUTO/DIRECTのダイレクトメモリがAmpr\:: Amm\:: giveDirectMemoryで確保され、指定されたVAアドレスレンジをsampleutilのメモリアロケータ用に登録する。指定したVAレンジはこれ以降、アプリケーションで使用・指定してはいけない。
	 * @retval SCE_OK 成功
	 * @retval (<0) エラーコード
	 */
	int initializeMapper(const MemoryOption	*pOption);

	/*!
	 * @~English
	 *
	 * @brief allocate virtual address range managed by AMM
	 * @param sizeInBytes	Size of virtual address range
	 * @param alignment	Alignment of virtual address range start offset
	 * @return Top address of virtual address range
	 * @~Japanese 
	 *
	 * @brief AMM管理下の仮想アドレスレンジを確保
	 * @param sizeInBytes	仮想アドレスレンジのサイズ
	 * @param alignment	仮想アドレスレンジ先頭オフセットのアライメント
	 * @return 仮想アドレスレンジの先頭アドレス
	 */
	off_t	allocateAmmVirtualAddressRange(size_t	sizeInBytes, size_t	alignment);

	/*!
	 * @~English
	 *
	 * @brief Free virtual address range managed by AMM
	 * @param virtualAddr	Top address of virtual address range
	 * @~Japanese 
	 *
	 * @brief AMM管理下の仮想アドレスレンジを解放
	 * @param virtualAddr	仮想アドレスレンジの先頭アドレス
	 */
	void	freeAmmVirtualAddressRange(off_t	virtualAddr);

	/*!
	 * @~English
	 * @brief Map operation descriptor used in \e batchMap
	 * @~Japanese
	 * @brief \e batchMap で使用するマップ操作記述子
	 */
	struct BatchMapEntry
	{
		/*!
		 * @~English
		 * @brief Map operation
		 * @~Japanese
		 * @brief マップ操作
		 */
		enum class Operation : int
		{
			/*!
			 * @~English
			 * @brief Map direct
			 * @~Japanese
			 * @brief マップダイレクト
			 */
			kMapDirect		= SCE_KERNEL_MAP_OP_MAP_DIRECT,
			/*!
			 * @~English
			 * @brief Unmap
			 * @~Japanese
			 * @brief アンマップ
			 */
			kUnamp			= SCE_KERNEL_MAP_OP_UNMAP,
			/*!
			 * @~English
			 * @brief Sets protection attribute
			 * @~Japanese
			 * @brief プロテクション属性設定
			 */
			kProtect		= SCE_KERNEL_MAP_OP_PROTECT,
			/*!
			 * @~English
			 * @brief Sets protection attribute and memory type
			 * @~Japanese
			 * @brief プロテクション属性とメモリタイプを設定
			 */
			kTypeProtect	= SCE_KERNEL_MAP_OP_TYPE_PROTECT
		};

		void	*m_start;
		off_t	m_offset;
		size_t	m_length;
		char	m_protection;
		char	m_type;
		short	__pad1__;
		int		m_operation;
	};

	/*!
	 * @~English
	 * @brief Reserves virtual address range
	 * @details This function doesn't map memory, but only allocates virtual address range.
	 * @param addr In: The virtual address from which free area query is started, Out: start virtual address of reserved area
	 * @param len Reserve area size(bytes, multiple of 16KiB)
	 * @param flags Specify logical-or of SCE_KERNEL_MAP_FIXED and SCE_KERNEL_MAP_NO_OVERWRITE. Specify zero if not needed.
	 * @param alignment Memory alignment(in bytes)
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief 仮想アドレス領域を予約する
	 * @details メモリをマップすることなく、仮想アドレス領域の割り当てだけを行う関数です。
	 * @param addr 入力：空き領域の検索を始める仮想アドレス, 出力：予約した領域の先頭仮想アドレス
	 * @param len 予約する領域のサイズ（バイト数、16KiBの倍数）
	 * @param flags SCE_KERNEL_MAP_FIXED, SCE_KERNEL_MAP_NO_OVERWRITEの論理和を指定。必要なければ0を指定
	 * @param alignment メモリアライメント(バイト)
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int	reserveVirtualRange(void	**addr, size_t	len, int	flags, size_t	alignment);
	/*!
	 * @~English
	 * @brief Allocates and maps direct memory
	 * @details Allocated from AUTO region
	 * @param addr Destination of allocated memory
	 * @param len Size of allocated memroy(in bytes)
	 * @param memoryType Memory type
	 * @param prot Protection attribute
	 * @param flags Specify logical-or of SCE_KERNEL_MAP_FIXED and SCE_KERNEL_MAP_NO_OVERWRITE. Specify zero if not needed.
	 * @param alignment Memory alignment(in bytes)
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief ダイレクトメモリをマップ
	 * @details AUTO領域から確保される
	 * @param addr 確保メモリのアドレスの格納先
	 * @param len マップするダイレクトメモリのサイズ(バイト)
	 * @param memoryType メモリタイプ
	 * @param prot プロテクション属性
	 * @param flags SCE_KERNEL_MAP_FIXED, SCE_KERNEL_MAP_NO_OVERWRITEの論理和を指定。必要なければ0を指定
	 * @param alignment メモリアライメント(バイト)
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int	mapDirectMemory(void	**addr, size_t	len, int	memoryType, int	prot, int	flags, size_t	alignment);
	/*!
	 * @~English
	 * @brief Allocates and maps direct memory for video out
	 * @details Allocated from DIRECT memory, and physical address is guranteed to be contiguous
	 * @param addr Destination of allocated memory
	 * @param offset Offset of allocated direct memory. Specify the offset value from the top of DIRECT memory.
	 * @param len Size of allocated memroy(in bytes)
	 * @param memoryType Memory type
	 * @param prot Protection attribute
	 * @param flags Specify logical-or of SCE_KERNEL_MAP_FIXED and SCE_KERNEL_MAP_NO_OVERWRITE. Specify zero if not needed.
	 * @param alignment Memory alignment(in bytes)
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief ビデオアウト用メモリをマップ
	 * @details DIRECTメモリから確保されれ、物理アドレス連続が保証される
	 * @param addr 確保メモリのアドレスの格納先
	 * @param offset 確保するダイレクトメモリのオフセット。DIRECTメモリの先頭からのオフセット値で指定
	 * @param len マップするダイレクトメモリのサイズ(バイト)
	 * @param memoryType メモリタイプ
	 * @param prot プロテクション属性
	 * @param flags SCE_KERNEL_MAP_FIXED, SCE_KERNEL_MAP_NO_OVERWRITEの論理和を指定。必要なければ0を指定
	 * @param alignment メモリアライメント(バイト)
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int	mapVideoOutMemory(void	**addr, off_t offset, size_t	len, int	memoryType, int	prot, int	flags, size_t	alignment);
	/*!
	 * @~English
	 * @brief Unmap allocated direct memory
	 * @param addr Top address of allocated memory
	 * @param len Size of memory to be unmapped(in bytes)
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief マップ済みのダイレクトメモリをアンマップ
	 * @param addr アンマップするメモリの先頭アドレス
	 * @param len アンマップするダイレクトメモリのサイズ(バイト)
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int	munmap(void *addr, size_t	len);
	/*!
	 * @~English
	 * @brief Batch map
	 * @param entries Array of map operation descriptors
	 * @param numberOfEntries Entris of map operation descriptors
	 * @param numberOfEntriesOut Entries of completed operations is stored
	 * @param flags Specify logical-or of SCE_KERNEL_MAP_FIXED and SCE_KERNEL_MAP_NO_OVERWRITE. Specify zero if not needed.
	 * @retval >=SCE_OK Success
	 * @retval (<0) Error code
	 * @~Japanese
	 * @brief バッチマップ
	 * @param entries マップ操作指定子の配列
	 * @param numberOfEntries マップ操作指定子の配列のエントリ数
	 * @param numberOfEntriesOut 操作が完了したエントリ数を格納
	 * @param flags SCE_KERNEL_MAP_FIXED, SCE_KERNEL_MAP_NO_OVERWRITEの論理和を指定。必要なければ0を指定
	 * @retval  SCE_OK 成功。
	 * @retval (<0) エラーコード
	 */
	int	batchMap(BatchMapEntry *entries, int numberOfEntries, int *numberOfEntriesOut, int flags);

	/*!
	 * @~English
	 * @brief Pooled memory
	 * @details Simple pooled memory emuration on AMM
	 * @~Japanese
	 * @brief プールドメモリ
	 * @details AMMでプールドメモリの機能を簡易的にエミュレーションします
	 */
	namespace Pool
	{
		/*!
		 * @~English
		 * @brief Pooled memory operation descriptor used in batch
		 * @~Japanese
		 * @brief batchで使用するプールドメモリ操作記述子
		 */
		struct BatchEntry
		{
			/*!
			 * @~English
			 * @brief Pooled memory operation
			 * @~Japanese
			 * @brief プールドメモリ操作
			 */
			enum class Operation : unsigned
			{
				/*!
				 * @~English
				 * @brief Commit physical memory
				 * @~Japanese
				 * @brief 物理メモリをコミット
				 */
				kCommit			= SCE_KERNEL_MEMORY_POOL_OP_COMMIT,
				/*!
				 * @~English
				 * @brief Decommit physical memory
				 * @~Japanese
				 * @brief 物理メモリをデコミット
				 */
				kDecommit		= SCE_KERNEL_MEMORY_POOL_OP_DECOMMIT,
				/*!
				 * @~English
				 * @brief Sets protection attribute
				 * @~Japanese
				 * @brief プロテクション属性設定
				 */
				kProtect		= SCE_KERNEL_MEMORY_POOL_OP_PROTECT,
				/*!
				 * @~English
				 * @brief Sets protection attribute and memory type
				 * @~Japanese
				 * @brief プロテクション属性とメモリタイプを設定
				 */
				kTypeProtect	= SCE_KERNEL_MEMORY_POOL_OP_TYPE_PROTECT,
				/*!
				 * @~English
				 * @brief Changes virtual address mapping destination
				 * @~Japanese
				 * @brief 仮想アドレスへのマッピング先の変更
				 */
				kMove			= SCE_KERNEL_MEMORY_POOL_OP_MOVE
			};

			/*!
			 * @~English
			 * @brief Operation. Specify \e Operation enum value
			 * @~Japanese
			 * @brief 操作。\e Operation enum値で指定
			 */
			unsigned		m_op;
			/*!
			 * @~English
			 * @brief Logical-or of either or both of SCE_KERNEL_MAP_FIXED and SCE_KERNEL_MAP_NO_OVERWRITE. Specify zero if not needed.
			 * @~Japanese
			 * @brief SCE_KERNEL_MAP_FIXED, SCE_KERNEL_MAP_NO_OVERWRITEの論理和を指定。必要なければ0を指定
			 */
			unsigned		m_flags;
			union {
				/*!
				 * @~English
				 * @brief Parameters used for physical memory commit
				 * @~Japanese
				 * @brief 物理メモリコミット操作のパラメータ
				 */
				struct {
					/*!
					 * @~English
					 * @brief Mapping virtual address
					 * @~Japanese
					 * @brief マップ先の仮想アドレス
					 */
					void			*m_addr;
					/*!
					 * @~English
					 * @brief Mapping memory size(in bytes)
					 * @~Japanese
					 * @brief マップするメモリサイズ(バイト)
					 */
					size_t			m_len;
					/*!
					 * @~English
					 * @brief Protection attribute
					 * @~Japanese
					 * @brief プロテクション属性
					 */
					unsigned char	m_prot;
					/*!
					 * @~English
					 * @brief Memory type
					 * @~Japanese
					 * @brief メモリタイプ
					 */
					unsigned char	m_type;
				}	m_commit;
				/*!
				 * @~English
				 * @brief Parameters used for physical memory decommit
				 * @~Japanese
				 * @brief 物理デメモリコミット操作のパラメータ
				 */
				struct {
					/*!
					 * @~English
					 * @brief Unmapping virtual address
					 * @~Japanese
					 * @brief アンマップする仮想アドレス
					 */
					void			*m_addr;
					/*!
					 * @~English
					 * @brief Unmapping memory size(in bytes)
					 * @~Japanese
					 * @brief アンマップするメモリサイズ(バイト)
					 */
					size_t			m_len;
				}	m_decommit;
				/*!
				 * @~English
				 * @brief Parameters used for setting protection attribute
				 * @~Japanese
				 * @brief プロテクション属性設定操作のパラメータ
				 */
				struct {
					/*!
					 * @~English
					 * @brief Virtual address where protection attribute is set
					 * @~Japanese
					 * @brief プロテクション属性を設定する仮想アドレス
					 */
					void			*m_addr;
					/*!
					 * @~English
					 * @brief Size(in bytes) of direct memory where protection attribute is set
					 * @~Japanese
					 * @brief プロテクション属性を設定するメモリサイズ(バイト)
					 */
					size_t			m_len;
					/*!
					 * @~English
					 * @brief Protection attribute to be set
					 * @~Japanese
					 * @brief 設定するプロテクション属性
					 */
					unsigned char	m_prot;
				}	m_protect;
				/*!
				 * @~English
				 * @brief Parameters used for setting protection attribute and memory type
				 * @~Japanese
				 * @brief プロテクション属性とメモリタイプ設定操作のパラメータ
				 */
				struct {
					/*!
					 * @~English
					 * @brief Virtual address where protection attribute and memory type are set
					 * @~Japanese
					 * @brief プロテクション属性とメモリタイプを設定する仮想アドレス
					 */
					void			*m_addr;
					/*!
					 * @~English
					 * @brief Size(in bytes) of direct memory where protection attribute and memory type are set
					 * @~Japanese
					 * @brief プロテクション属性とメモリタイプを設定するメモリサイズ(バイト)
					 */
					size_t			m_len;
					/*!
					 * @~English
					 * @brief Protection attribute to be set
					 * @~Japanese
					 * @brief 設定するプロテクション属性
					 */
					unsigned char	m_prot;
					/*!
					 * @~English
					 * @brief Memory to be set
					 * @~Japanese
					 * @brief 設定するメモリタイプ
					 */
					unsigned char	m_type;
				}	m_typeProtect;
				/*!
				 * @~English
				 * @brief Parameters used for changing virtual address mapping
				 * @~Japanese
				 * @brief 仮想アドレスへのメモリマッピング先変更操作のパラメータ
				 */
				struct {
					/*!
					 * @~English
					 * @brief Destination virtual address
					 * @~Japanese
					 * @brief 変更後の仮想アドレス
					 */
					void			*m_dst;
					/*!
					 * @~English
					 * @brief Source virtual address
					 * @~Japanese
					 * @brief 変更前の仮想アドレス
					 */
					void			*m_src;
					/*!
					 * @~English
					 * @brief Size(in bytes) of direct memory where mapping is changed
					 * @~Japanese
					 * @brief マッピングを変更するするメモリサイズ(バイト)
					 */
					size_t			m_len;
				}	m_move;
				uintptr_t __padding__[3];
			};
		};

		/*!
		 * @~English
		 * @brief sceKernelMemoryPoolReserve equivalence
		 * @param addrIn Start virtual address of the area to allocate (aligned to 2 MiB), virtual address to begin searching for free space, or NULL
		 * @param len Size of the area to allocate (bytes, multiple of 2 MiB)
		 * @param alignment Alignment of the start virtual address (bytes, 2MiB or more and a power of 2, only valid when SCE_KERNEL_MAP_FIXED is not specified for flags), or 0
		 * @param flags Flags, or 0
		 * @param addrOut Destination to store start virtual address for the allocated area
 		 * @retval SCE_OK Success
 		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief sceKernelMemoryPoolReserve相当
		 * @param addrIn 割り当てる領域の先頭仮想アドレス（2MiBアライン）、空き領域の検索を始める仮想アドレス、またはNULL
		 * @param len 割り当てる領域のサイズ（バイト数、2MiBの倍数）
		 * @param alignment 先頭仮想アドレスのアライメント（バイト数、2MiB以上かつ2のべき乗、flagsにSCE_KERNEL_MAP_FIXEDが指定されていない場合のみ有効）、または0
		 * @param flags フラグ、または0
		 * @param addrOut 割り当てた領域の先頭仮想アドレスの格納先
 		 * @retval SCE_OK 成功。
 		 * @retval (<0) エラーコード
		 */
		int	reserve(void	*addrIn, size_t	len, size_t	alignment, int	flags, void	**addrOut);
		/*!
		 * @~English
		 * @brief Releases virtuad address range reserved by \e reserve
		 * @param addr Start virtual address of the area to unmap (aligned to 16 KiB)
		 * @param len Size of the area to unmap (bytes)
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief \e reserve で確保した仮想アドレス領域の解放
		 * @param addr 解除する領域の先頭仮想アドレス（16KiBアライン）
		 * @param len 解除する領域のサイズ（バイト数）
 		 * @retval SCE_OK 成功。
 		 * @retval (<0) エラーコード
		 */
		int munmap(void *addr, size_t len);
		/*!
		 * @~English
		 * @brief sceKernelMemoryPoolCommit equivalence
		 * @param addr Start virtual address of the commit destination area (aligned to 64 KiB)
		 * @param len Size of memory to commit (bytes, multiple of 64 KiB)
		 * @param type Memory type to set for memory to commit
		 * @param prot Memory protection to set for memory to commit (see details below)
		 * @param flags Reserved (specify 0)
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief sceKernelMemoryPoolCommit相当
		 * @param addr コミット先の領域の先頭仮想アドレス（64KiBアライン）
		 * @param len コミットするメモリのサイズ（バイト数、64KiBの倍数）
		 * @param type コミットするメモリに設定するメモリタイプ
		 * @param prot コミットするメモリに設定するメモリプロテクション（詳細下記）
		 * @param flags 予約（0を指定すること）
 		 * @retval SCE_OK 成功。
 		 * @retval (<0) エラーコード
		 */
		int	commit(void	*addr, size_t	len, int	type, int	prot, int	flags);
		/*!
		 * @~English
		 * @brief sceKernelMemoryPoolDecommit equivalence
		 * @param addr Start virtual address of the area to de-commit (aligned to 64 KiB)
		 * @param len Size of area to de-commit (bytes, multiple of 64 KiB)
		 * @param flags Reserved (specify 0)
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief sceKernelMemoryPoolDecommit相当
		 * @param addr コミット解除する領域の先頭仮想アドレス（64KiBアライン）
		 * @param len コミット解除する領域のサイズ（バイト数、64KiBの倍数）
		 * @param flags 予約（0を指定すること）
 		 * @retval SCE_OK 成功。
 		 * @retval (<0) エラーコード
		 */
		int	decommit(void	*addr, size_t	len, int	flags);
		/*!
		 * @~English
		 * @brief sceKernelMemoryPoolMove equivalence
		 * @param dst Start virtual address of the move destination area (aligned to 64 KiB)
		 * @param src Start virtual address of the move source area (aligned to 64 KiB)
		 * @param len Size of memory to move (bytes, multiple of 64 KiB)
		 * @param flags Reserved (specify 0)
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief sceKernelMemoryPoolMove相当
		 * @param dst 移動先の領域の先頭仮想アドレス（64KiBアライン）
		 * @param src 移動元の領域の先頭仮想アドレス（64KiBアライン）
		 * @param len 移動するメモリのサイズ（バイト数、64KiBの倍数）
		 * @param flags 予約（0を指定すること）
 		 * @retval SCE_OK 成功。
 		 * @retval (<0) エラーコード
		 */
		int	move(void	*dst, void	*src, size_t	len, int	flags);
		/*!
		 * @~English
		 * @brief sceKernelMemoryPoolBatch equivalence
		 * @param entries Array of operation information to apply
		 * @param n Number of elements for entries
		 * @param indexOut Destination to store the number of elements from among the entries elements that have actually been processed, or NULL
		 * @param flags Reserved (specify 0)
		 * @retval SCE_OK Success
		 * @retval (<0) Error code
		 * @~Japanese
		 * @brief sceKernelMemoryPoolBatch相当
		 * @param entries 適用する操作情報の配列
		 * @param n entriesの要素数
		 * @param indexOut entriesの要素のうち実際に処理された要素数の格納先、またはNULL
		 * @param flags 予約（0を指定すること）
 		 * @retval SCE_OK 成功。
 		 * @retval (<0) エラーコード
		 */
		int	batch(const BatchEntry	*entries, int	n, int	*indexOut, int	flags);
	} // namespace Pool
}}} // namespace sce::SampleUtil::Memory
