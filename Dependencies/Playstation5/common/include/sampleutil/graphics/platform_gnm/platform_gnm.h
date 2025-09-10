/* SIE CONFIDENTIAL
* PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
* Copyright (C) 2019 Sony Interactive Entertainment Inc.
* 
*/

#ifndef _SCE_SAMPLE_UTIL_GRAPHICS_PLATFORM_GNM_H
#define _SCE_SAMPLE_UTIL_GRAPHICS_PLATFORM_GNM_H


#include <scebase_common.h>
#if defined(_SCE_TARGET_OS_ORBIS) && _SCE_TARGET_OS_ORBIS

#include <sampleutil/graphics/buffer.h>
#include <gnm.h>
#include <gnmx.h>

namespace sce
{
	namespace SampleUtil
	{
		namespace Graphics
		{
			namespace Gnm
			{
				class BufferGnm : public BufferInterface
				{
				public:
					virtual ~BufferGnm(){}

					/*!
					 * @~English
					 * @brief Returns the pointer to the data area of a buffer object 
					 * @details This returns the pointer to the data area of a buffer object. 
					 * 
					 * @return Pointer to the data area of a buffer object 
					 * @~Japanese
					 * @brief バッファオブジェクトのデータ領域へのポインタを返す 
					 * @details バッファオブジェクトのデータ領域へのポインタを返します。 
					 * 
					 * @return バッファオブジェクトのデータ領域へのポインタ 
					 */
					virtual void *getData() = 0;
					
					/*!
					 * @~English
					 * @brief Returns the pointer to the data area of a buffer object 
					 * @details This returns the pointer to the data area of a buffer object. 
					 * 
					 * @return Pointer to the data area of a buffer object 
					 * @~Japanese
					 * @brief バッファオブジェクトのデータ領域へのポインタを返す 
					 * @details バッファオブジェクトのデータ領域へのポインタを返します。 
					 * 
					 * @return バッファオブジェクトのデータ領域へのポインタ 
					 */
					virtual const void *getData() const = 0;
					
					/*!
					 * @~English
					 * @brief Returns the data area size of a buffer object 
					 * @details This returns the data area size of a buffer object. 
					 * 
					 * @return Data area size of a buffer object 
					 * @~Japanese
					 * @brief バッファオブジェクトのデータ領域のサイズを返す 
					 * @details バッファオブジェクトのデータ領域のサイズを返します。 
					 * 
					 * @return バッファオブジェクトのデータ領域のサイズ 
					 */
					virtual size_t  getSize() const = 0;

					/*!
					 * @~English
					 * @brief Returns the pointer to the data area of a buffer object 
					 * @details This returns the pointer to the data area of a buffer object. 
					 * 
					 * @return Pointer to the data area of a buffer object 
					 * @~Japanese
					 * @brief バッファオブジェクトのデータ領域へのポインタを返す 
					 * @details バッファオブジェクトのデータ領域へのポインタを返します。 
					 * 
					 * @return バッファオブジェクトのデータ領域へのポインタ 
					 */
					static BufferGnm *cast(BufferInterface *buffer);
					/*!
					 * @~English
					 * @brief Returns the pointer to the data area of a buffer object 
					 * @details This returns the pointer to the data area of a buffer object. 
					 * 
					 * @return Pointer to the data area of a buffer object 
					 * @~Japanese
					 * @brief バッファオブジェクトのデータ領域へのポインタを返す 
					 * @details バッファオブジェクトのデータ領域へのポインタを返します。 
					 * 
					 * @return バッファオブジェクトのデータ領域へのポインタ 
					 */
                    static const BufferGnm *cast(const BufferInterface *buffer);


				};
				class RenderTargetGnm : public RenderTarget
				{
				public:
					
					virtual sce::Gnm::RenderTarget* getGnmRenderTarget() = 0;
					virtual ~RenderTargetGnm(){}
				};

				class GraphicsLoaderGnm : public GraphicsLoader
				{
				public:
					static GraphicsLoaderGnm* cast(GraphicsLoader *from);
					static const GraphicsLoaderGnm* cast(const GraphicsLoader *from);

					virtual void* memalignSystemSharedMemory(size_t boundary, size_t size) = 0;
					virtual void  freeSystemSharedMemory(void* ptr) = 0;
					virtual void* memalignVideoSharedMemory(size_t boundary, size_t size) = 0;
					virtual void  freeVideoSharedMemory(void* ptr) = 0;

					virtual ~GraphicsLoaderGnm(){}
				};
				class GraphicsContextGnm : public GraphicsContext, public GraphicsLoaderGnm
                {
                public:
					static GraphicsContextGnm* cast(GraphicsContext *from);
                    static const GraphicsContextGnm* cast(const GraphicsContext *from);
                    virtual ~GraphicsContextGnm(){}

				    virtual int drawIndexAuto(Primitive primitive, uint32_t count) = 0;
                    virtual sce::Gnmx::GfxContext* getGfxContext() = 0;


                    virtual int readBuffer(void *dst, const BufferInterface* from, uint64_t offset, uint64_t numBytes) = 0;

					virtual int32_t getVideoOutHandle() const = 0;
					virtual int submitOnly() = 0;
					virtual int prepareNextFrame() = 0;
                };
			}
		}
	}
}


#endif // defined(_SCE_TARGET_OS_ORBIS) && _SCE_TARGET_OS_ORBIS
#endif // _SCE_SAMPLE_UTIL_GRAPHICS_PLATFORM_GNM_H
