#version 460 core

in vec3 FragPos;
in vec2 FragTexcoord;
in vec3 FragNormal;

// Out
out vec4 FinalColor;

void main()
{
    vec4 redColor = vec4(1.0, 0.0, 0.0, 1);

    // Set the final color output
    FinalColor = redColor;
}
