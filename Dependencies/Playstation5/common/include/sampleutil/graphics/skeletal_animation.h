/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

#pragma once

// C/C++ standard library
#include <cinttypes>
#include <vector>
#include <string>
#include <mspace.h>
// SDK library
#include <scebase_common.h>
#include <edgeanim.h>
#include <vectormath/cpp/vectormath_aos.h>
// SampleUtil
#include <sampleutil/memory.h>
#include <sampleutil/debug/perf.h>
#include <sampleutil/graphics/graphics_memory.h>
#include <sampleutil/graphics/compat.h>
#include <pack/pack_file.h>

namespace sce { namespace SampleUtil { namespace Graphics {

class PackModel;

/*!
 * @~English
 * @brief Rotation designation to the joint specified by joint index
 * @~Japanese
 * @brief ジョイントインデックスで指定したジョイントに対する回転指定
 */
struct JointRotationDesignation
{
	int								m_jointIndex;	//!< The joint index to which rotation is applied
	Vectormath::Simd::Aos::Vector3	m_rotation;		//!< Joint rotation(in radians[XYZ])
	JointRotationDesignation() : m_rotation(0.f) {}
	JointRotationDesignation(int	jointIndex, Vectormath::Simd::Aos::Vector3_arg	rotation) : m_jointIndex(jointIndex), m_rotation(rotation) {}
};

/*!
 * @~English
 * @brief Transform designation to the joint specified by joint index
 * @~Japanese
 * @brief ジョイントインデックスで指定したジョイントに対する変換指定
 */
struct JointTransformDesignation
{
	int								m_jointIndex;	//!< The joint index to which transform is applied
	Vectormath::Simd::Aos::Vector3	m_scale;		//!< Joint scale
	Vectormath::Simd::Aos::Vector3	m_rotation;		//!< Joint rotation(in radians[XYZ])
	Vectormath::Simd::Aos::Vector3	m_translation;	//!< Joint translation

	JointTransformDesignation() : m_scale(1.f), m_rotation(0.f), m_translation(0.f) {}
	JointTransformDesignation(int	jointIndex, Vectormath::Simd::Aos::Vector3_arg	scale,
		Vectormath::Simd::Aos::Vector3_arg	rotation,
		Vectormath::Simd::Aos::Vector3_arg	translation)
		: m_jointIndex(jointIndex)
		, m_scale(scale)
		, m_rotation(rotation)
		, m_translation(translation)
	{}
	JointTransformDesignation(const JointRotationDesignation &rotDesig)
		: m_jointIndex(rotDesig.m_jointIndex)
		, m_scale(1.f)
		, m_rotation(rotDesig.m_rotation)
		, m_translation(0.f)
	{}
};

struct SkeletalAnimation
{
	const EdgeAnimSkeleton				*m_pEdgeSkeleton;
	const EdgeAnimAnimation				*m_pEdgeAnimation;
	const uint32_t						*m_pBoneHashes;
	std::vector<int>					m_meshBindOrder;
	Memory::Cpu::unique_ptr<float[]>	m_invBindMatrices;
	bool								m_isResolved;
	PackModel							*m_pBasePackModel;

	SkeletalAnimation()
		: m_pEdgeSkeleton	(nullptr)
		, m_pEdgeAnimation	(nullptr)
		, m_pBoneHashes		(nullptr)
		, m_isResolved		(false)
		, m_pBasePackModel	(nullptr)
	{
		TAG_THIS_CLASS;
	}
	SkeletalAnimation(PackModel	*pBasePackModel, const PackFile::Skeleton	&skeleton, const EdgeAnimSkeleton	&edgeSkeleton, const EdgeAnimAnimation	*pEdgeAnimation, SceLibcMspace	cpuMemory);
	SkeletalAnimation(const SkeletalAnimation &) = delete;
	const SkeletalAnimation &operator=(const SkeletalAnimation &) = delete;

	SkeletalAnimation(SkeletalAnimation &&rhs)
	{
		TAG_THIS_CLASS;
		*this = std::move(rhs);
	}

	virtual ~SkeletalAnimation()
	{
		UNTAG_THIS_CLASS;
	}

	const SkeletalAnimation &operator=(SkeletalAnimation &&rhs)
	{
		m_pEdgeSkeleton		= rhs.m_pEdgeSkeleton; rhs.m_pEdgeSkeleton = nullptr;
		m_pEdgeAnimation	= rhs.m_pEdgeAnimation; rhs.m_pEdgeAnimation = nullptr;
		m_pBoneHashes		= rhs.m_pBoneHashes; rhs.m_pBoneHashes = nullptr;
		m_meshBindOrder		= std::move(rhs.m_meshBindOrder); rhs.m_meshBindOrder.clear();
		m_invBindMatrices	= std::move(rhs.m_invBindMatrices); rhs.m_invBindMatrices.release();
		m_isResolved		= rhs.m_isResolved; rhs.m_isResolved = false;
		m_pBasePackModel	= rhs.m_pBasePackModel; rhs.m_pBasePackModel = nullptr;

		return	*this;
	}

	void	resolve();
}; // SkeletalAnimation

struct SkeletalAnimationInstance : public EdgeAnimContext
{
	static const uint32_t kMaxPoseStackDepth = 16;

	SkeletalAnimation						*m_pSkeletalAnimation;
	Memory::Cpu::unique_ptr<uint8_t[]>		m_edgeAnimWorkMemory;
	Memory::Cpu::unique_ptr<float[]>		m_jointWorldMatrices;
	Compat::Buffer							m_skinningMatrixBuffer;

	SkeletalAnimationInstance()
		: m_pSkeletalAnimation(nullptr)
	{
		TAG_THIS_CLASS;
	}
	SkeletalAnimationInstance(SkeletalAnimation &skeletalAnimation, SceLibcMspace	cpuMemory, const std::string &name = "");
	SkeletalAnimationInstance(const SkeletalAnimationInstance &) = delete;
	const SkeletalAnimationInstance &operator=(const SkeletalAnimationInstance &) = delete;

	SkeletalAnimationInstance(SkeletalAnimationInstance &&rhs)
	{
		TAG_THIS_CLASS;
		*this = std::move(rhs);
	}

	virtual	~SkeletalAnimationInstance()
	{
		UNTAG_THIS_CLASS;
	}

	const SkeletalAnimationInstance &operator=(SkeletalAnimationInstance &&rhs)
	{
		static_cast<EdgeAnimContext *>(this)->operator=(static_cast<EdgeAnimContext &&>(rhs));
		m_pSkeletalAnimation	= rhs.m_pSkeletalAnimation; rhs.m_pSkeletalAnimation = nullptr;
		m_edgeAnimWorkMemory	= std::move(rhs.m_edgeAnimWorkMemory);
		m_jointWorldMatrices	= std::move(rhs.m_jointWorldMatrices);
#if _SCE_TARGET_OS_PROSPERO
		m_skinningMatrixBuffer	= rhs.m_skinningMatrixBuffer; rhs.m_skinningMatrixBuffer.init();
#endif
#if _SCE_TARGET_OS_ORBIS
		m_skinningMatrixBuffer	= rhs.m_skinningMatrixBuffer; rhs.m_skinningMatrixBuffer.setBaseAddress(nullptr);
#endif

		return	*this;
	}

	int	setIdentitiesToSkinningMatrices(uint32_t	numPoseMatrices, VideoRingAllocator &graphicsRingMemory);
	int	convertPoseToSkinningMatrices(const EdgeAnimPoseInfo &pose, VideoRingAllocator &graphicsRingMemory);
	int	evaluateSkinningMatrices(float	timeInSec, VideoRingAllocator &graphicsRingMemory, const std::vector<JointTransformDesignation> &auxTransforms = {});
	int	evaluateAndBlendSkinningMatrices(float	timeInSec1, float	timeInSec2, float	ratio, VideoRingAllocator &graphicsRingMemory, const std::vector<JointTransformDesignation> &auxTransforms = {});
	void drawJoints(Compat::DrawCommandBuffer &dcb, Vectormath::Simd::Aos::Transform3_arg	model, Vectormath::Simd::Aos::Transform3_arg	view, Vectormath::Simd::Aos::Matrix4_arg	proj);
}; // SkeletalAnimationInstance

}}} // namespace sce::SampleUtil::Graphics