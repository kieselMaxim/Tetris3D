#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout(location = 2) in vec3 aNormal;

layout (std140) uniform Matrices
{
    mat4 projection;
    mat4 view;
};

out vec2 TexCoord;

uniform mat4 model;
uniform mat4 transform;

uniform int effect;

//lightning
out vec3 FragPos;
out vec3 Normal;


void main()
{
    if(effect == 0){
        //position und rotation wird durch model und transform Matrix verändert
        FragPos = vec3(model* transform * vec4(aPos, 1.0));
        Normal = aNormal;

        //die projection und view matrix sind für die Kammerabewegung zuständig
        gl_Position = projection * view * vec4(FragPos, 1.0);
        TexCoord = vec2(aTexCoord.x, aTexCoord.y);
    }
    else if(effect == 1){
        //für das post processing werden die Texturcoordinaten und die Positonen der vertices des Quads hier festgelegt
        TexCoord = aTexCoord;
        gl_Position = vec4(aPos.x, aPos.y, 0.0, 1.0); 
    }

}
