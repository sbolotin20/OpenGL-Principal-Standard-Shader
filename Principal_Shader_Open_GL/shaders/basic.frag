#version 330 core
out vec4 FragColor;
  
in vec2 texCoord; // the input variable from the vertex shader (same name and same type)  

uniform sampler2D baseColorTex;   // texture unit 0
uniform bool      useBaseColorTex;
uniform vec3      baseColorTint;  // rgb tint in linear space (0..1)

void main()
{
    vec3 texColor = vec3(1.0);
    if (useBaseColorTex) {
        texColor = texture(baseColorTex, texCoord).rgb;
    } 
    vec3 baseColor = texColor * baseColorTint;
    FragColor = vec4(baseColor, 1.0);
    
} 