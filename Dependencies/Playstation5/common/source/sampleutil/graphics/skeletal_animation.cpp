/* SIE CONFIDENTIAL
 * PlayStation(R)5 Programmer Tool Runtime Library Release 11.00.00.40-00.00.00.0.1
 * Copyright (C) 2023 Sony Interactive Entertainment Inc.
 * 
 */

// C/C++ standard library
#include <cmath>
#include <cstdio>
// SDK library
#include <scebase_common.h>
#if _SCE_TARGET_OS_PROSPERO
#include <agc/core/buffer.h>
#endif
#if _SCE_TARGET_OS_ORBIS
#include <gnm/buffer.h>
#endif
// SampleUtil
#include <sampleutil/sampleutil_error.h>
#include <sampleutil/sampleutil_common.h>
#include <sampleutil/graphics/matrix4unaligned.h>
#include <sampleutil/graphics/sprite_utility.h>
#include <sampleutil/graphics/pack_model.h>
#include <sampleutil/graphics/skeletal_animation.h>

#pragma comment(lib, "libEdgeAnim.a")

using namespace sce::Vectormath::Simd::Aos;

namespace
{
	Vector3	hashColor(uint32_t	value)
	{
		uint32_t	uColor[3] = { 0, 0, 0 };
		for (int i = 0; i < 8; i++) {
			for(int j = 0; j < 3; j++) {
				if (value & 1) uColor[j] |= 1u << (7 - i);
				value >>= 1;
			}
		}
		return Vector3((float)uColor[0] / 255.f, (float)uColor[1] / 255.f, (float)uColor[2] / 255.f);
	}
}

namespace sce { namespace SampleUtil { namespace Graphics {

SkeletalAnimation::SkeletalAnimation(PackModel	*pBasePackModel, const PackFile::Skeleton &skeleton, const EdgeAnimSkeleton &edgeSkeleton, const EdgeAnimAnimation	*pEdgeAnimation, SceLibcMspace	cpuMemory)
: m_pEdgeSkeleton	(&edgeSkeleton)
, m_pEdgeAnimation	(pEdgeAnimation)
, m_meshBindOrder	(skeleton.bind_pose.getCount())
, m_invBindMatrices	(Memory::Cpu::make_unique<float>(16 * skeleton.bind_pose.getCount(), 16, cpuMemory, "invbind mat"))
, m_isResolved		(false)
, m_pBasePackModel	(pBasePackModel)
{
	TAG_THIS_CLASS;

	for (int i = 0; i < skeleton.bind_pose.getCount(); i++) {
		int bone_i = skeleton.node_index[i];
		m_meshBindOrder[i] = bone_i;
	}

	// copy inverse bind matrices
	std::memcpy(m_invBindMatrices.get(), skeleton.bind_pose.data(), sizeof(Matrix4) * skeleton.bind_pose.getCount());
	// to get inverse bind matrices, apply inverse operator to bind matrices obtained from pack
	for (int pose_i = 0; pose_i < skeleton.bind_pose.getCount(); pose_i++) {
		auto *mat = &(reinterpret_cast<Matrix4 *>(m_invBindMatrices.get()))[pose_i];
		*mat = inverse(*mat);
	}
}

void	SkeletalAnimation::resolve()
{
	if (!m_isResolved) {
		m_pBasePackModel->resolve();

		SCE_SAMPLE_UTIL_ASSERT(m_meshBindOrder.size() <= m_pEdgeSkeleton->numJoints);
		m_pBoneHashes	= EDGE_OFFSET_GET_POINTER(const unsigned int, m_pEdgeSkeleton->offsetJointNameHashArray);

		m_isResolved	= true;
	}
}

SkeletalAnimationInstance::SkeletalAnimationInstance(SkeletalAnimation	&skeletalAnimation, SceLibcMspace	cpuMemory, const std::string	&name)
	: m_pSkeletalAnimation(&skeletalAnimation)
{
	TAG_THIS_CLASS;

	const EdgeAnimSkeleton	&edgeSkeleton = *m_pSkeletalAnimation->m_pEdgeSkeleton;

	// allocate memory for edgeAnim's pose stack & work buffers
	const uint32_t edgeAnimWorkBufferSize = edgeAnimComputeContextSize(edgeSkeleton.numJoints, edgeSkeleton.numUserChannels, kMaxPoseStackDepth);
	m_edgeAnimWorkMemory = Memory::Cpu::make_unique<uint8_t>(edgeAnimWorkBufferSize, 16, cpuMemory, "EdgeAnim WorkBuffer : " + (name != "" ? name : "(noname)"));
	m_jointWorldMatrices = Memory::Cpu::make_unique<float>(16 * edgeSkeleton.numJoints, 16, cpuMemory, "Joint matrices : " + (name != "" ? name : "(noname)"));

	// initialise edgeAnim context
	edgeAnimInitializeContext(this, m_edgeAnimWorkMemory.get(), edgeAnimWorkBufferSize, &edgeSkeleton);
#if _SCE_TARGET_OS_PROSPERO
	m_skinningMatrixBuffer.init();
#endif
#if _SCE_TARGET_OS_ORBIS
	m_skinningMatrixBuffer.setBaseAddress(nullptr);
#endif
}

int	SkeletalAnimationInstance::setIdentitiesToSkinningMatrices(uint32_t	numPoseMatrices, VideoRingAllocator	&graphicsRingMemory)
{
	int	ret = SCE_OK; (void)ret;

	Matrix4	*pSkinningMatrices	= reinterpret_cast<Matrix4 *>(graphicsRingMemory.allocate(EDGE_ALIGN(numPoseMatrices, 4) * sizeof(Matrix4), EDGE_SIMD_ALIGNMENT));
	for (int i = 0; i < numPoseMatrices; i++) {
		pSkinningMatrices[i] = Matrix4::identity();
	}
#if _SCE_TARGET_OS_PROSPERO
	ret = sce::Agc::Core::initialize(&m_skinningMatrixBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(pSkinningMatrices, sizeof(float) * 16, numPoseMatrices));
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}
#endif
#if _SCE_TARGET_OS_ORBIS
	m_skinningMatrixBuffer.initAsRegularBuffer(pSkinningMatrices, sizeof(float) * 16, pSkeleton->numJoints);
#endif

	return	SCE_OK;
}

int	SkeletalAnimationInstance::convertPoseToSkinningMatrices(const EdgeAnimPoseInfo	&pose, VideoRingAllocator	&graphicsRingMemory)
{
	int ret = SCE_OK; (void)ret;

	EdgeAnimJointTransform	rootJoint;
	storeXYZW(Quat::identity(), rootJoint.rotation);
	storeXYZ(Point3::origin(), rootJoint.translation);
	storeXYZ(Vector3(1.f), rootJoint.scale);

	// convert pose to world matrices
	float *pWorldMatrices = m_jointWorldMatrices.get();
	edgeAnimLocalJointsToWorldMatrices4x4(pWorldMatrices, pose.jointArray, &rootJoint, pSkeleton->jointLinkage, pSkeleton->numJointLinkages);
	const uint32_t	numPoseMatrices = m_pSkeletalAnimation->m_meshBindOrder.size();
	float *pPoseMatrices = reinterpret_cast<float *>(EDGE_ALLOCA_ALIGNED(EDGE_ALIGN(numPoseMatrices, 4) * sizeof(float) * 16, EDGE_SIMD_ALIGNMENT));
	// reorder matrices;
	for (int i = 0; i < numPoseMatrices; i++) {
		memcpy(&pPoseMatrices[i * 16], &pWorldMatrices[m_pSkeletalAnimation->m_meshBindOrder[i] * 16], sizeof(float) * 16);
	}

	void *pSkinningMatrices = graphicsRingMemory.allocate(EDGE_ALIGN(numPoseMatrices, 4) * sizeof(float) * 16, EDGE_SIMD_ALIGNMENT);
	edgeAnimMultiplyMatrices4x4(pSkinningMatrices, pPoseMatrices, m_pSkeletalAnimation->m_invBindMatrices.get(), numPoseMatrices);
#if _SCE_TARGET_OS_PROSPERO
	ret = sce::Agc::Core::initialize(&m_skinningMatrixBuffer, &Agc::Core::BufferSpec().initAsRegularBuffer(pSkinningMatrices, sizeof(float) * 16, numPoseMatrices));
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return SCE_SAMPLE_UTIL_ERROR_FATAL;
	}
#endif
#if _SCE_TARGET_OS_ORBIS
	m_skinningMatrixBuffer.initAsRegularBuffer(pSkinningMatrices, sizeof(float) * 16, pSkeleton->numJoints);
#endif

	return SCE_OK;
}

int	SkeletalAnimationInstance::evaluateSkinningMatrices(float	timeInSec, VideoRingAllocator &graphicsRingMemory, const std::vector<JointTransformDesignation> &auxTransforms)
{
	int ret = SCE_OK; (void)ret;

	m_pSkeletalAnimation->resolve();

	// push empty pose onto the stack
	edgeAnimPoseStackPush(this);
	// get pose at the top of the stack
	EdgeAnimPoseInfo pose;
	edgeAnimPoseStackGetPose(this, &pose);

	// evaluate animation to pose
	const float evalTime = fmodf(timeInSec, m_pSkeletalAnimation->m_pEdgeAnimation->duration);
	edgeAnimEvaluate(m_pSkeletalAnimation->m_pEdgeAnimation, pSkeleton, &pose, evalTime);

	// apply auxiliary joint transforms if you have
	for (auto &auxTrans : auxTransforms) {
		const int idx = auxTrans.m_jointIndex;
		Vector3	jointScale(pose.jointArray[idx].scale);
		Quat	jointRotation(pose.jointArray[idx].rotation);
		Vector3	jointTranslation(pose.jointArray[idx].translation);
		jointScale = mulPerElem(auxTrans.m_scale, jointScale);
		jointRotation = Quat::rotation(auxTrans.m_rotation, Vectormath::Simd::RotationOrder::kXYZ) * jointRotation;
		jointTranslation += auxTrans.m_translation;
		storeXYZ(jointScale, pose.jointArray[idx].scale);
		storeXYZW(jointRotation, pose.jointArray[idx].rotation);
		storeXYZ(jointTranslation, pose.jointArray[idx].translation);
	}

	ret = convertPoseToSkinningMatrices(pose, graphicsRingMemory);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return ret;
	}

	// pop the stack
	edgeAnimPoseStackPop(this);

	return SCE_OK;
}

int	SkeletalAnimationInstance::evaluateAndBlendSkinningMatrices(float	timeInSec1, float	timeInSec2, float	ratio, VideoRingAllocator &graphicsRingMemory, const std::vector<JointTransformDesignation> &auxTransforms)
{
	int ret = SCE_OK; (void)ret;

	m_pSkeletalAnimation->resolve();

	SCE_SAMPLE_UTIL_ASSERT(ratio >= 0.f && ratio <= 1.f);
	if (ratio < 0.f || ratio > 1.f) {
		return SCE_SAMPLE_UTIL_ERROR_INVALID_PARAM;
	}

	// evaluate animation1 to pose1
	edgeAnimPoseStackPush(this);
	EdgeAnimPoseInfo pose1;
	edgeAnimPoseStackGetPose(this, &pose1);
	float evalTime1 = fmodf(timeInSec1, m_pSkeletalAnimation->m_pEdgeAnimation->duration);
	edgeAnimEvaluate(m_pSkeletalAnimation->m_pEdgeAnimation, pSkeleton, &pose1, evalTime1);

	// evaluate animation1 to pose2
	edgeAnimPoseStackPush(this);
	EdgeAnimPoseInfo pose2;
	edgeAnimPoseStackGetPose(this, &pose2);
	float evalTime2 = fmodf(timeInSec2, m_pSkeletalAnimation->m_pEdgeAnimation->duration);
	edgeAnimEvaluate(m_pSkeletalAnimation->m_pEdgeAnimation, pSkeleton, &pose2, evalTime2);

	// blend
	edgeAnimBlendPose(this, 1, 1, 0, EDGE_ANIM_BLENDOP_BLEND_LINEAR, ratio);
	edgeAnimPoseStackPop(this);

	// get pose at the top of the stack
	EdgeAnimPoseInfo pose;
	edgeAnimPoseStackGetPose(this, &pose);

	ret = convertPoseToSkinningMatrices(pose, graphicsRingMemory);
	SCE_SAMPLE_UTIL_ASSERT_EQUAL(ret, SCE_OK);
	if (ret != SCE_OK) {
		return ret;
	}

	// pop the stack
	edgeAnimPoseStackPop(this);

	return SCE_OK;
}

void	SkeletalAnimationInstance::drawJoints(Compat::DrawCommandBuffer &dcb, Transform3_arg	model, Transform3_arg	view, Matrix4_arg	proj)
{
	const unsigned int *pJointHashArray = EDGE_OFFSET_GET_POINTER(const unsigned int, pSkeleton->offsetJointNameHashArray);
	for (uint16_t i = 0; i < pSkeleton->numJoints; i++) {
		Point3	myPos = model * (Point3::origin() + reinterpret_cast<const Matrix4 *>(m_jointWorldMatrices.get())[i].getTranslation());
		auto parentIndex = edgeAnimSkeletonGetJointParent(pSkeleton, i);
		if (parentIndex >= 0) {
			// compute bone orientation and scale
			Point3	parentPos	= model * (Point3::origin() + reinterpret_cast<const Matrix4 *>(m_jointWorldMatrices.get())[parentIndex].getTranslation());
			Vector3	zAxis		= parentPos - myPos;
			Vector3	xAxis		= Vector3::xAxis();
			Vector3	yAxis		= normalize(cross(zAxis, xAxis));
			xAxis	= normalize(cross(yAxis, zAxis));
			const float coneRad = 0.02f;

			const unsigned int	boneHash = pJointHashArray[i];
			const Vector4	color(hashColor(boneHash), 1.f);
			Matrix4	modelMat(Matrix3(xAxis * coneRad, yAxis * coneRad, zAxis), parentPos - Point3::origin());
			SpriteUtil::drawCone(&dcb, modelMat, (Matrix4)view, proj, color, 50u);
			char	buf[256];
			sprintf_s(buf, "%d: %x", i, boneHash);
			Vector4 posInClip = proj * view * Vector4(myPos + (parentPos - myPos) * 0.5f);
			posInClip /= posInClip.getW();
			Vector2 uv(posInClip.getX() * 0.5f + 0.5f, -posInClip.getY() * 0.5f + 0.5f);
			SpriteUtil::drawDebugString(uv, 1.f / 80.f, color, buf);
		}
	}
}

}}} // namespace sce::SampleUtil::Graphics