#version 460 core

in vec2 FragTexcoord;

uniform sampler2D Texture0;

// Out
out vec4 FinalColor;

void main()
{
    // Sample the color from the texture
    vec4 color = texture(Texture0, FragTexcoord);

    // Convert to grayscale by averaging the RGB components
    float grey = (color.r + color.g + color.b) / 3.0;

    // Create a grayscale color vector
    vec4 greyScaleColor = vec4(grey, grey, grey, color.a);

    // Output the final color (grayscale in this case)
    FinalColor = greyScaleColor;
}