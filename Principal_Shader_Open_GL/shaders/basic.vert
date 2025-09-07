#version 330 core
layout (location = 0) in vec3 aPos; // the position variable has attribute position 0
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord; // the texture variable has attribute position 2
layout (location = 3) in vec3 aTangent;
  
out vec2 texCoord; // specify a texture output to the fragment shader
out vec3 worldPos;

out vec3 fragTangent; 
out vec3 fragNormal;

uniform mat4 modelMatrix; // positions/rotates/scales objects in world (vertex pos -> world pos)
uniform mat4 viewMatrix; // postions everything relative to camera (world pos -> camera space pos)
uniform mat4 projectionMatrix; // creates perspective (near things big, fal things small - camera space -> screen space)


void main()
{
    // model transforms vertex to world -> view transforms world to camera -> projection transforms to screen
    gl_Position =  projectionMatrix * viewMatrix * modelMatrix * vec4(aPos, 1.0);

    //gl_Position = vec4(aPos, 1.0); // see how we directly give a vec3 to vec4's constructor
    texCoord = vec2(aTexCoord);
    // A point light needs each pixel’s position in world space so the fragment shader can compute a unique light direction per pixel
    worldPos = (modelMatrix * (vec4(aPos, 1.0))).xyz;

    mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
    fragTangent = normalize(normalMatrix * aTangent);
    fragNormal = normalize(normalMatrix * aNormal); 
}