#version 330 core
out vec4 FragColor;
out vec3 Vec3Color;

in vec2 TexCoord;

uniform sampler2D texture1;
uniform vec4 color;

uniform sampler2D screenTexture;

uniform int effect;
uniform int postProcessing;

in vec3 Normal;
in vec3 FragPos;

uniform vec3 lightPos;
uniform vec3 lightColor;


void main()
{

	if(effect == 0){
		//ambient Licht
		float ambientStrength = 0.1;
		vec3 ambient = ambientStrength * lightColor * (vec3(texture(texture1, TexCoord) * color));

		//diffuse Licht
		vec3 norm = normalize(Normal);
		vec3 lightDir = normalize(lightPos - FragPos);
		float diff = max(dot(norm, lightDir), 0.0);
		vec3 diffuse = diff * lightColor * (vec3(texture(texture1, TexCoord)*color));

		//ambient und diffuse werden addiert
		FragColor = vec4(ambient + diffuse, 1.0);
		return;
	}

	//post processing
	if (effect == 1) {
		if (postProcessing == 1) {
			//Textur des Views wird auf eine Textur geladen
			vec3 col = texture(screenTexture, TexCoord).rgb;
			FragColor = vec4(col, 1.0);
			return;
		}
		if (postProcessing == 2) {
			//Inverse Farben
			FragColor = vec4(vec3(1.0 - texture(screenTexture, TexCoord)), 1.0);
			return;
		}
	}

}
