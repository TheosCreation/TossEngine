
float CalculateShadow(mat4 VPLight, sampler2D Texture_ShadowMap, vec3 FragPos) {
    vec4 FragPos_LightSpace = VPLight * vec4(FragPos, 1.0f);
    vec3 NDC_Space = FragPos_LightSpace.xyz / FragPos_LightSpace.w;
    vec3 ProjCoordinates = 0.5f * NDC_Space + 0.5f; // Convert to [0, 1] range

    float ShadowBias = 0.005f;
    float CurrentDepth = ProjCoordinates.z - ShadowBias;

    // Sample the depth from the shadow map
    //float LightClosestDepth = texture(Texture_ShadowMap, ProjCoordinates.xy).r;

    // Determine if the fragment is in shadow
    //float Shadow = CurrentDepth > LightClosestDepth ? 1.0f : 0.0f;

    //PCF
    vec2 TexelSize = 1.0f / textureSize(Texture_ShadowMap, 0);
    float Shadow = 0.0f;
    int Count = 0;
    for(int Row = -3; Row <= 3; Row++)
    {
        for(int Col = -3; Col <= 3; Col++)
        {
            vec2 TextureCoordOffset = ProjCoordinates.xy + vec2(Row, Col) * TexelSize;
            float PCF_Depth = texture(Texture_ShadowMap, TextureCoordOffset).x;
            Shadow += CurrentDepth > PCF_Depth ? 1.0f : 0.0f;
            Count++;
        }
    }
    Shadow /= float(Count);
    return Shadow;
}
