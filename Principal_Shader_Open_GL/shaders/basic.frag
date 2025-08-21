#version 330 core
out vec4 FragColor;
  
in vec2 texCoord; // the input variable from the vertex shader (same name and same type)  
in vec3 worldPos; // position of each pixel on [triangle] in world space

// -- Lighting Uniforms (world space) --
uniform vec3 uLight_Direction;   // assume normalized, lighting in world space, points from surface -> light 
uniform vec3 uLight_Position;    // point light position (moves each frame)
uniform vec3 uLight_Color;       // intensity/tint (linear RGB)
uniform vec3 uAmbient;           // small constant fill (0 ~ 0.1)
uniform vec3 uCamera_Position;   // camera position (to build view direction)
uniform vec3 uSpecularColor;

// -- Material controls --
uniform float uRoughness;   // [0..1] 0=glossy, 1=rough (map to shininess)
uniform float uMetallic;    // [0..1] 0=dielectric, 1=metal
uniform vec3 uDielectricF0; // ~4% base reflectance for plastics 

// -- Base color (albedo) --
uniform sampler2D baseColorTex;   // Albedo/baseColor texture (linear or already converted) texture unit 0
uniform bool      useBaseColorTex;
uniform vec3      baseColorTint;  // rgb tint in linear space (0..1)

void main()
{
    // 1. Surface frame: constant normal (facing +Z)
    vec3 N = normalize(vec3(0.0, 0.0, 1.0)); 

    // 2. Lighting/view directions (in world space, unit length)
    vec3 L = normalize(uLight_Position - worldPos); // L = light direction vector pointing from surface point -> light 
    vec3 V = normalize(uCamera_Position - worldPos); // V = camera position vector pointing from surface point -> camera
    vec3 H = normalize(L + V); // H = half-vector between light and view
    
    // 3. Roughness -> shininess (Blinn-Phong exponent) High shininess = tight lobe = low roughness
    float shininess = mix(8.0, 2048.0, pow(1.0 - uRoughness, 2.0)); 
    
    // 4. Base color (albedo). Texture is optional; multiply by tint
    vec3 texColor = vec3(1.0);
    if (useBaseColorTex) {
        texColor = texture(baseColorTex, texCoord).rgb;
    } 
    vec3 baseColor = texColor * baseColorTint; 
    
    // 5. Specular base reflectance at normal indicidence (F0)
    vec3 F0 = mix(uDielectricF0, baseColor, uMetallic); // specular base color - as uMetallic -> 1, specular color becomes base color
    
    // 6. Fresnel (Schlick approximation): more reflection at grazing angles
    float VoH = max(dot(V, H), 0.0);
    vec3 F = F0 + (1.0 - F0) * pow(1 - VoH, 5.0);

    // 7. Lambert diffuse factor and Blinn-Phong spec power
    float NdotL = max(dot(N, L), 0.0);
    float specPow = pow(max(dot(N, H), 0.0), shininess);

    // 8. Energy-aware split:
    //    - Diffuse color vanishes as metallic -> 1 (metals have no diffuse)
    //    - (1-F) reduces diffuse at grazing angles to conserve energy
    vec3 diffuseColor = baseColor * (1.0 - uMetallic); 
    vec3 diffuse = (1.0 - F) * diffuseColor * uLight_Color * NdotL;
    vec3 specular = F * specPow * uLight_Color; 

    // Optional distance attentuation for point lights
    float d = length(uLight_Position - worldPos);
    float att = 1.0 / (1.0 + 0.09*d + 0.032*d*d); // most common falloff model
    diffuse *=  att; // ensures brightness fades as light moves away
    specular *= att; // ensures brightness fades as light moves away
    
    // 9. Ambient term (small) tints the surface uniformly
    vec3 ambient = baseColor * uAmbient;

    // 10. Final composition (linear space)
    vec3 finalColor = ambient + diffuse + specular;
    FragColor = vec4(finalColor, 1.0);
    
} 