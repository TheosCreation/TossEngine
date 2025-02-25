#pragma once
#include "Renderer.h"

class Image : public Renderer
{
public:
	Image();
	~Image();

	virtual void onCreate() override;

	virtual void Render(UniformData data, RenderingPath renderPath) override;
	void SetSize(Vector2 size);

	void SetTexture(const TexturePtr& texture);


private:
	void updateVertices();

	VertexArrayObjectPtr m_vbo;
	Vector2 m_size = { 1, 1 };
	TexturePtr m_texture;
};

