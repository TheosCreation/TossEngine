#version 460 core

in vec2 FragTexcoord;

uniform vec2 Resolution;
uniform float Time;
uniform sampler2D Texture0;
uniform sampler2D Texture1;

// Out
out vec4 FinalColor;

void main()
{
    vec2 uv = FragTexcoord.xy;
    vec2 rainCoord = vec2(uv.x * 2.0, uv.y * 0.1 + Time * 0.125);
    vec3 raintex = texture(Texture1, rainCoord).rgb / 8.0;
    vec2 where = (uv.xy - raintex.xy);
    vec3 texchur1 = texture(Texture0, vec2(where.x, where.y)).rgb;

    FinalColor = vec4(texchur1, 1.0);
}
