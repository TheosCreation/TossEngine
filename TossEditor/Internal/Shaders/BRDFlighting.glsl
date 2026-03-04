// Structure representing a generic light source with color and specular strength.
struct Light {
    vec3 Color;                 // The color of the light.
    float SpecularStrength;     // The strength of the specular reflection from the light.
};

// Structure representing a directional light with direction.
struct DirectionalLight {
    Light Base;                 // Base properties of the light.
    vec3 Direction;             // Direction vector of the light, typically normalized.
};

// Structure representing a point light with position and attenuation properties.
struct PointLight {
    Light Base;                 // Base properties of the light.
    vec3 Position;              // Position of the point light in world space.
    float AttenuationConstant;  // Constant term for attenuation.
    float AttenuationLinear;    // Linear term for distance-based attenuation.
    float AttenuationExponent;  // Quadratic term for distance-based attenuation.
    float Radius;               // Radius for the max distance to cutoff the lighting calculations
};

// Structure representing a spotlight with position, direction, and cutoff angles.
struct SpotLight {
    Light Base;                 // Base properties of the light.
    vec3 Position;              // Position of the spotlight in world space.
    vec3 Direction;             // Direction vector of the spotlight, typically normalized.
    float CutOff;               // Inner cutoff angle for the spotlight.
    float OuterCutOff;          // Outer cutoff angle for the spotlight.
    float AttenuationConstant;  // Constant term for attenuation.
    float AttenuationLinear;    // Linear term for distance-based attenuation.
    float AttenuationExponent;  // Quadratic term for distance-based attenuation.
};

// Constants for BRDF
const float PI = 3.141592653589793;
const float EPSILON = 0.0001;

// GGX/Trowbridge-Reitz Normal Distribution Function (NDF)
float DistributionGGX(vec3 N, vec3 H, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH * NdotH;

    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return a2 / max(denom, EPSILON);
}

// Fresnel-Schlick approximation
vec3 FresnelSchlick(float cosTheta, vec3 F0)
{
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

// Smith's Schlick-GGX Geometry Function
float GeometrySchlickGGX(float NdotV, float roughness)
{
    float r = (roughness + 1.0);
    float k = (r * r) / 8.0;

    float denom = NdotV * (1.0 - k) + k;
    return NdotV / denom;
}

// Smith's Geometry Function
float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness)
{
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx1 = GeometrySchlickGGX(NdotV, roughness);
    float ggx2 = GeometrySchlickGGX(NdotL, roughness);
    return ggx1 * ggx2;
}

/**
 * @brief Calculates the BRDF lighting effect from a light source.
 * 
 * This function computes the diffuse and specular components of lighting
 * using the Cook-Torrance BRDF model.
 * 
 * @param baseLight The base properties of the light.
 * @param lightDir The direction vector from the surface point to the light source.
 * @param viewDir The direction vector from the surface point to the camera.
 * @param normal The normal vector at the surface point.
 * @param attenuation The attenuation factor based on distance.
 * @param roughness The roughness of the surface.
 * @param metallic The metallic property of the surface.
 * @param F0 The base reflectivity of the surface.
 * @return The resulting color contribution from the light.
 */
vec3 CalculateBRDF(Light baseLight, vec3 lightDir, vec3 viewDir, vec3 normal, float attenuation, float roughness, float metallic, vec3 F0)
{
    vec3 H = normalize(viewDir + lightDir); // Halfway vector
    float NdotV = max(dot(normal, viewDir), 0.0);
    float NdotL = max(dot(normal, lightDir), 0.0);

    // Diffuse term (Lambertian reflectance)
    vec3 diffuse = baseLight.Color * (1.0 - metallic) / PI;

    // Cook-Torrance specular term
    float D = DistributionGGX(normal, H, roughness);
    vec3 F = FresnelSchlick(max(dot(H, viewDir), 0.0), F0);
    float G = GeometrySmith(normal, viewDir, lightDir, roughness);

    vec3 numerator = D * F * G;
    float denominator = 4.0 * NdotV * NdotL + EPSILON;
    vec3 specular = numerator / max(denominator, EPSILON);

    // Combine diffuse and specular terms
    vec3 result = (diffuse + specular) * baseLight.Color * NdotL / attenuation;
    return result;
}

/**
 * @brief Calculates the lighting contribution from a directional light using BRDF.
 */
vec3 CalculateDirectionalLightBRDF(DirectionalLight light, vec3 viewDir, float roughness, float metallic, vec3 F0, vec3 fragNormal)
{
    vec3 LightDir = normalize(light.Direction);
    return CalculateBRDF(light.Base, LightDir, viewDir, fragNormal, 1.0, roughness, metallic, F0);
}

/**
 * @brief Calculates the lighting contribution from a point light using BRDF.
 */
vec3 CalculatePointLightBRDF(PointLight light, vec3 viewDir, float roughness, float metallic, vec3 F0, vec3 fragNormal, vec3 fragPos)
{
    vec3 LightDir = normalize(fragPos - light.Position);
    float Distance = length(light.Position - fragPos);

    if (Distance < light.Radius)
    {
        float Attenuation = light.AttenuationConstant + (light.AttenuationLinear * Distance) + (light.AttenuationExponent * Distance * Distance);
        return CalculateBRDF(light.Base, LightDir, viewDir, fragNormal, Attenuation, roughness, metallic, F0);
    }

    return vec3(0, 0, 0);
}

/**
 * @brief Calculates the lighting contribution from a spotlight using BRDF.
 */
vec3 CalculateSpotLightBRDF(SpotLight light, vec3 viewDir, float roughness, float metallic, vec3 F0, vec3 fragNormal, vec3 fragPos)
{
    vec3 LightDir = normalize(fragPos - light.Position);
    float theta = dot(LightDir, normalize(light.Direction));
    float epsilon = light.CutOff - light.OuterCutOff;
    float intensity = clamp((theta - light.OuterCutOff) / epsilon, 0.0, 1.0);
    float Distance = length(light.Position - fragPos);
    float Attenuation = light.AttenuationConstant + (light.AttenuationLinear * Distance) + (light.AttenuationExponent * Distance * Distance);
    return CalculateBRDF(light.Base, LightDir, viewDir, fragNormal, Attenuation, roughness, metallic, F0) * intensity;
}