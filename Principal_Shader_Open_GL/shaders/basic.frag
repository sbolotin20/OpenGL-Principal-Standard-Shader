#version 330 core
out vec4 FragColor;
  
in vec2 texCoord; // the input variable from the vertex shader (same name and same type)  
in vec3 worldPos; // position of each pixel on [triangle] in world space

// -- Lighting Uniforms --
uniform vec3 uLight_Direction; // assume normalized, lighting in world space, points from surface -> light 
uniform vec3 uLight_Position; // world space position of the point light
uniform vec3 uLight_Color; // intensity/tint
uniform vec3 uAmbient; // small constant fill

// -- Texture Uniforms --
uniform sampler2D baseColorTex;   // texture unit 0
uniform bool      useBaseColorTex;
uniform vec3      baseColorTint;  // rgb tint in linear space (0..1)

void main()
{
    // lighting
    vec3 N = vec3(0.0, 0.0, 1.0); // facing +Z
    N = normalize(N);
    vec3 L = normalize(uLight_Position - worldPos); // L = light direction vector pointing from surface point -> light 
    float dot_product = max(dot(N, L), 0.0); // how much the light is aligned with the surface normal
    
    // texture
    vec3 texColor = vec3(1.0);
    if (useBaseColorTex) {
        texColor = texture(baseColorTex, texCoord).rgb;
    } 

    // FragColor calculations
    vec3 baseColor = texColor * baseColorTint; // multiplying ambient and diffuse by baseColor tints the light by object's material color
    vec3 ambient = baseColor * uAmbient;
    vec3 diffuse = max(dot(N, L), 0.0) * uLight_Color * baseColor; // brightens depending on angle between N and L
    vec3 finalColor = ambient + diffuse;
    FragColor = vec4(finalColor, 1.0);
    
} 