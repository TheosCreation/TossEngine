// Structure representing a generic light source with color and specular strength.
struct Light {
    float Intensity;
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
    float Radius;               // Radius for the max distance to cutoff the lighting calculations
};

// Structure representing a spotlight with position, direction, and cutoff angles.
struct SpotLight {
    Light Base;                 // Base properties of the light.
    vec3 Position;              // Position of the spotlight in world space.
    vec3 Direction;             // Direction vector of the spotlight, typically normalized.
    float CutOff;               // Inner cutoff angle for the spotlight.
    float OuterCutOff;          // Outer cutoff angle for the spotlight.
    float Range;          		// Range of the spotlight.
};

vec3 CalculateLight(Light baseLight, vec3 lightDir, vec3 viewDir, vec3 normal, float attenuation, float objectShininess)
{
    float diffuseStrength = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = diffuseStrength * baseLight.Color * baseLight.Intensity;

    float clampedShininess = max(objectShininess, 1.0);
    vec3 halfwayDir = normalize(lightDir + viewDir);
    float specularReflection = pow(max(dot(normal, halfwayDir), 0.0), clampedShininess);
    vec3 specular = baseLight.SpecularStrength * specularReflection * baseLight.Color * baseLight.Intensity;

    return (diffuse + specular) * attenuation;
}

vec3 CalculateDirectionalLight(DirectionalLight light, vec3 viewDir, float objectShininess, vec3 fragNormal)
{
    vec3 lightDir = normalize(-light.Direction);
    return CalculateLight(light.Base, lightDir, viewDir, fragNormal, 1.0, objectShininess);
}

vec3 CalculatePointLight(PointLight light, vec3 viewDir, float objectShininess, vec3 fragNormal, vec3 fragPos)
{
    vec3 lightOffset = light.Position - fragPos;
    float distance = length(lightOffset);

    if (distance >= light.Radius)
    {
        return vec3(0.0);
    }

    vec3 lightDir = normalize(lightOffset);

    // Normalized distance in [0,1]
    float normalizedDistance = distance / light.Radius;

    // Unity-like smooth range fade
    float attenuation = 1.0 - normalizedDistance * normalizedDistance;
    attenuation = attenuation * attenuation;

    return CalculateLight(light.Base, lightDir, viewDir, fragNormal, attenuation, objectShininess);
}

vec3 CalculateSpotLight(SpotLight light, vec3 viewDir, float objectShininess, vec3 fragNormal, vec3 fragPos)
{
    vec3 lightOffset = light.Position - fragPos;
    float distance = length(lightOffset);

    if (distance >= light.Range)
    {
        return vec3(0.0);
    }

    vec3 lightDir = normalize(lightOffset);

    float normalizedDistance = distance / light.Range;
    float distanceAttenuation = 1.0 - normalizedDistance * normalizedDistance;
    distanceAttenuation = distanceAttenuation * distanceAttenuation;

    float theta = dot(normalize(-lightDir), normalize(light.Direction));
    float epsilon = max(light.CutOff - light.OuterCutOff, 0.0001);
    float angularAttenuation = clamp((theta - light.OuterCutOff) / epsilon, 0.0, 1.0);
    angularAttenuation = angularAttenuation * angularAttenuation;

    float attenuation = distanceAttenuation * angularAttenuation;

    return CalculateLight(light.Base, lightDir, viewDir, fragNormal, attenuation, objectShininess);
}