#version 330 core
out vec4 FragColor;
out vec3 Vec3Color;

in vec3 ourColor;
in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 color;

uniform sampler2D renderedTexture;
uniform float time;

uniform int effect;

in vec2 UV;

void main()
{

	if(effect == 0){
		FragColor = texture(texture1, TexCoord) * color;
		//FragColor = texture(texture1, UV) * color;
	}
	else if(effect == 1){
		//FragColor = texture(texture1, TexCoord) * color;
		Vec3Color = texture(renderedTexture,UV + 0.005*vec2(sin(time+1024.0*UV.x),cos(time+768.0*UV.y))).xyz;
		//Vec3Color = texture(renderedTexture,TexCoord).xyz;
		}
}