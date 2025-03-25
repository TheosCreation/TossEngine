#version 460 core

in vec2 FragTexcoord;

uniform sampler2D Texture0;

// Out
out vec4 FinalColor;

void main()
{
    // Sample the color from the texture
    vec4 color = texture(Texture0, FragTexcoord);

    // Convert to grayscale using the luminosity method
    float grey = 0.3 * color.r + 0.11 * color.g + 0.059 * color.b;

    // Create a grayscale color vector
    vec4 greyScaleColor = vec4(grey, grey, grey, color.a);

    // Output the final color
    FinalColor = greyScaleColor;
}