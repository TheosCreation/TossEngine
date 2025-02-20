// Structure representing a generic light source with color and specular strength.
struct Light {
    vec3 Color;           // The color of the light.
    float SpecularStrength; // The strength of the specular reflection from the light.
};

// Structure representing a directional light with direction.
struct DirectionalLight {
    Light Base;          // Base properties of the light.
    vec3 Direction;     // Direction vector of the light, typically normalized.
};

// Structure representing a point light with position and attenuation properties.
struct PointLight {
    Light Base;                  // Base properties of the light.
    vec3 Position;              // Position of the point light in world space.
    float AttenuationConstant;   // Constant term for attenuation.
    float AttenuationLinear;     // Linear term for distance-based attenuation.
    float AttenuationExponent;    // Quadratic term for distance-based attenuation.
};

// Structure representing a spotlight with position, direction, and cutoff angles.
struct SpotLight {
    Light Base;                 // Base properties of the light.
    vec3 Position;              // Position of the spotlight in world space.
    vec3 Direction;             // Direction vector of the spotlight, typically normalized.
    float CutOff;               // Inner cutoff angle for the spotlight.
    float OuterCutOff;          // Outer cutoff angle for the spotlight.
    float AttenuationConstant;   // Constant term for attenuation.
    float AttenuationLinear;     // Linear term for distance-based attenuation.
    float AttenuationExponent;    // Quadratic term for distance-based attenuation.
};

/**
 * @brief Calculates the combined lighting effect from a light source.
 * 
 * This function computes the diffuse and specular components of lighting
 * based on the light's properties, the direction to the light, the view direction,
 * the normal vector of the surface, the attenuation, and the object's shininess.
 * 
 * @param baseLight The base properties of the light.
 * @param lightDir The direction vector from the surface point to the light source.
 * @param viewDir The direction vector from the surface point to the camera.
 * @param normal The normal vector at the surface point.
 * @param attenuation The attenuation factor based on distance.
 * @param objectShininess The shininess factor of the surface, affecting specular highlight size.
 * @return The resulting color contribution from the light.
 */
vec3 CalculateLight(Light baseLight, vec3 lightDir, vec3 viewDir, vec3 normal, float attenuation, float objectShininess)
{
    // Diffuse shading: calculates the contribution from the diffuse reflection.
    float DiffuseStrength = max(dot(normal, -lightDir), 0.0);
    vec3 Diffuse = DiffuseStrength * baseLight.Color;

    // Specular shading: calculates the contribution from the specular reflection.
    vec3 HalfwayDir = normalize(-lightDir - viewDir); // Halfway vector between light and view direction.
    float SpecularReflection = pow(max(dot(normal, HalfwayDir), 0.0), objectShininess); // Blinn-Phong specular reflection.
    vec3 Specular = baseLight.SpecularStrength * SpecularReflection * baseLight.Color;

    // Combine diffuse and specular results, applying attenuation.
    vec3 result = (Diffuse + Specular) / attenuation;
    return result; // Return the final lighting result.
}

/**
 * @brief Calculates the lighting contribution from a directional light.
 * 
 * This function computes the lighting effect from a directional light source
 * by normalizing its direction and calling the CalculateLight function.
 * 
 * @param light The directional light source.
 * @param viewDir The direction vector from the fragment to the camera.
 * @param objectShininess The shininess factor of the surface.
 * @param fragNormal The normal vector at the fragment's position.
 * @return The resulting color contribution from the directional light.
 */
vec3 CalculateDirectionalLight(DirectionalLight light, vec3 viewDir, float objectShininess, vec3 fragNormal)
{
    vec3 LightDir = normalize(light.Direction); // Normalize the directional light vector.
    return CalculateLight(light.Base, LightDir, viewDir, fragNormal, 1.0, objectShininess); // Call CalculateLight with a fixed attenuation of 1.0.
}

/**
 * @brief Calculates the lighting contribution from a point light.
 * 
 * This function computes the lighting effect from a point light source
 * based on its position and distance from the fragment. It calculates the
 * attenuation based on distance and calls the CalculateLight function.
 * 
 * @param light The point light source.
 * @param viewDir The direction vector from the fragment to the camera.
 * @param objectShininess The shininess factor of the surface.
 * @param fragNormal The normal vector at the fragment's position.
 * @param fragPos The world position of the fragment.
 * @return The resulting color contribution from the point light.
 */
vec3 CalculatePointLight(PointLight light, vec3 viewDir, float objectShininess, vec3 fragNormal, vec3 fragPos)
{
    vec3 LightDir = normalize(fragPos - light.Position); // Calculate direction to the point light.
    float Distance = length(light.Position - fragPos); // Calculate distance to the light.
    // Calculate attenuation based on the distance.
    float Attenuation = light.AttenuationConstant + (light.AttenuationLinear * Distance) + (light.AttenuationExponent * Distance * Distance);
    return CalculateLight(light.Base, LightDir, viewDir, fragNormal, Attenuation, objectShininess); // Call CalculateLight with calculated attenuation.
}

/**
 * @brief Calculates the lighting contribution from a spotlight.
 * 
 * This function computes the lighting effect from a spotlight by considering
 * the direction and cutoff angles. It calculates attenuation based on distance
 * and calls the CalculateLight function, scaling the result by the spotlight's intensity.
 * 
 * @param light The spotlight source.
 * @param viewDir The direction vector from the fragment to the camera.
 * @param objectShininess The shininess factor of the surface.
 * @param fragNormal The normal vector at the fragment's position.
 * @param fragPos The world position of the fragment.
 * @return The resulting color contribution from the spotlight.
 */
vec3 CalculateSpotLight(SpotLight light, vec3 viewDir, float objectShininess, vec3 fragNormal, vec3 fragPos)
{
    vec3 LightDir = normalize(fragPos - light.Position); // Calculate direction to the spotlight.
    float theta = dot(LightDir, normalize(light.Direction)); // Calculate the angle between the light direction and the fragment.
    float epsilon = light.CutOff - light.OuterCutOff; // Calculate the range of cutoff angles.
    // Calculate intensity based on the angle and clamp it to [0, 1].
    float intensity = clamp((theta - light.OuterCutOff) / epsilon, 0.0, 1.0);  
    float Distance = length(light.Position - fragPos); // Calculate distance to the light.
    // Calculate attenuation based on the distance.
    float Attenuation = light.AttenuationConstant + (light.AttenuationLinear * Distance) + (light.AttenuationExponent * Distance * Distance);
    return CalculateLight(light.Base, LightDir, viewDir, fragNormal, Attenuation, objectShininess) * intensity; // Call CalculateLight and scale by intensity.
}