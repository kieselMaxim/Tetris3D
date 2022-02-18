#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec2 TexCoord;
out vec2 UV;

uniform mat4 model;
uniform mat4 transform;

void main()
{
    gl_Position = projection * view * model * transform * vec4(aPos, 1.0);
    TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    UV = (aPos.xy+vec2(1,1))/2.0;
}