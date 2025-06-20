﻿#pragma once
#include "Renderer.h"
#include "Font.h"

class TOSSENGINE_API Text : public Renderer 
{
public:
    void onCreateLate() override;
    void OnInspectorGUI() override;
    void Render(UniformData data, RenderingPath renderPath) override;

    void SetText(const string text);
    string GetText() const;
    bool GetIsUi() const;
private:
    void RebuildMesh();
    VertexArrayObjectPtr m_vao;

    string m_text = "";
    int m_fontSize = 16;
    FontPtr m_font = nullptr;
    Vector3 m_color = Color::Black;
    bool m_isUi = true;

    SERIALIZABLE_MEMBERS(m_text, m_material, m_font, m_color, m_fontSize, m_isUi)
};

REGISTER_COMPONENT(Text);
