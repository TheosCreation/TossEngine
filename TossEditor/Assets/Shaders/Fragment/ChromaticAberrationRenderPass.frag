#version 460 core
#define SEQUENCE_LENGTH 24.0
#define FPS 12.

in vec2 FragTexcoord;

uniform vec2 Resolution;
uniform float Time;
uniform sampler2D Texture0;

// Out
out vec4 FinalColor;

void main()
{
	vec2 uv = FragTexcoord.xy;

	float amount = 0.0;
	
	amount = (1.0 + sin(Time*6.0)) * 0.5;
	amount *= 1.0 + sin(Time*16.0) * 0.5;
	amount *= 1.0 + sin(Time*19.0) * 0.5;
	amount *= 1.0 + sin(Time*27.0) * 0.5;
	amount = pow(amount, 3.0);

	amount *= 0.05;
	
    vec3 col;
    col.r = texture( Texture0, vec2(uv.x+amount,uv.y) ).r;
    col.g = texture( Texture0, uv ).g;
    col.b = texture( Texture0, vec2(uv.x-amount,uv.y) ).b;

	col *= (1.0 - amount * 0.5);
	
    FinalColor = vec4(col,1.0);
}