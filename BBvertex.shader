#version 430


layout (location = 0) in BoundingBox BBInformation;
layout (location = 1) in vec3 normal;

out vec3 FragPos;
out vec3 Normal;
out vec3 LightPos;

uniform vec3 lightPos; // We now define the uniform in the vertex shader and pass the 'view space' lightpos to the fragment shader. lightPos is currently in world space.

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(position, 1.0f);
    FragPos = vec3(view * model * vec4(position, 1.0f));
    //Normal = mat3(transpose(inverse(view * model))) * normal;
	Normal = normal.xyz;
    LightPos = vec3(view * vec4(lightPos, 1.0)); // Transform world-space light position to view-space light position
}
