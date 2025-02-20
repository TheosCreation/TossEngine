#version 460 core

uniform vec3 uColor; // Color uniform
uniform float alpha = 1.0f;

// Out
out vec4 FinalColor;

void main()
{
    // Calculate the final color with the specified alpha
    FinalColor = vec4(uColor, alpha);
}