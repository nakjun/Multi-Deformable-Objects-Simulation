#version 430
in vec3 FragPos;
in vec3 Normal;
in vec3 LightPos;   // Extra in variable, since we need the light position in view space we calculate this in the vertex shader

out vec4 color;

uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    /*
    // Ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;    
    
     // Diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(LightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // Specular
    float specularStrength = 0.5f;
    vec3 viewDir = normalize(-FragPos); 
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor; 
    
    vec3 result = (ambient + diffuse + specular) * objectColor;
    color = vec4(result, 1.0f);*/
	
	vec3 color22 = Normal.xyz;

	color = vec4(color22,1.0f);
}