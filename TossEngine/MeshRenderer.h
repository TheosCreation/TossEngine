#pragma once
#include "Renderer.h"
#include "Mesh.h"
#include "SerializationUtils.h"

class TOSSENGINE_API MeshRenderer : public Renderer
{
public:
	MeshRenderer() = default;
	~MeshRenderer() = default;

    void onCreateLate() override;
    void onUpdate() override;
    void onUpdateInternal() override;
    bool SkeletonChanged(MeshPtr oldMesh, MeshPtr newMesh) const;

    virtual void OnInspectorGUI() override
	{
		// Display the material from the base Renderer component.
		Renderer::OnInspectorGUI();

        ResourceAssignableField(m_mesh, "Mesh");
	}

	void onShadowPass(uint index);

    void Render(UniformData data, RenderingPath renderPath) override;

	void SetMesh(MeshPtr mesh);
    void CreateBoneObjects();
    void ResetBonesToBindPose();
    bool HasValidBoneObjectsForMesh() const;
    void DestroyBoneObjects();
    MeshPtr GetMesh() const;
    GameObjectPtr GetBoneObjectByName(const std::string& boneName) const;
    float GetAlpha() const;
    Vector3 GetExtent() override;
private:
    void UploadSkinningMatrices(const ShaderPtr& shader) const;
    
private:
	MeshPtr m_mesh;

	ShaderPtr m_geometryShader;
	ShaderPtr m_shadowShader;
    
    std::vector<GameObjectPtr> m_boneObjects;
    GameObjectPtr m_boneRootObject;
    bool m_bonesCreated = false;
    
    SERIALIZABLE_MEMBERS(m_boneObjects, m_boneRootObject, m_mesh, m_material)
};

REGISTER_COMPONENT(MeshRenderer);
