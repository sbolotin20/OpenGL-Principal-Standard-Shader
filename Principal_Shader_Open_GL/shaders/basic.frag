#version 330 core
out vec4 FragColor;
  
in vec2 texCoord; // the input variable from the vertex shader (same name and same type)  
in vec3 worldPos; // position of each pixel on [triangle] in world space
in vec3 fragTangent; 
in vec3 fragNormal;

// -- Lighting Uniforms (world space) --
uniform vec3 uLight_Position;    // point light position (moves each frame)
uniform vec3 uLight_Color;       // intensity/tint (linear RGB)
uniform vec3 uAmbient;           // small constant fill (0 ~ 0.1)
uniform vec3 uCamera_Position;   // camera position (to build view direction)
uniform int uLightType; // 0 = Directional, 1 = Point, 2 = Spot
uniform vec3 uDir_Direction; // normalized (for directional and spot light)
uniform float uSpotCosInner; // cos(innerAngle)
uniform float uSpotCosOuter; // cos(outerAngle) >= inner

// -- Material controls -- uniform floats are single value for entire surface
uniform float uRoughness;   // [0..1] 0=glossy, 1=rough (map to shininess)
uniform float uMetallic;    // [0..1] 0=dielectric, 1=metal
uniform vec3 uDielectricF0; // ~4% base reflectance for plastics 

// -- Base color (albedo) --
uniform sampler2D baseColorTex;   // Albedo/baseColor texture (linear or already converted) texture unit 0
uniform bool      useBaseColorTex;
uniform vec3      baseColorTint;  // rgb tint in linear space (0..1)

// -- Normal Mapping --
uniform sampler2D uNormalTex;
uniform bool uUseNormalTex;

// -- Roughness and Metallic Texture Maps -- different value at each pixel
uniform sampler2D roughnessMap;
uniform bool useRoughnessMap;
uniform sampler2D metallicMap;
uniform bool useMetallicMap;

// IBL
uniform samplerCube irradianceMap;
uniform bool useIBL;

float D_GGX(float NdotH, float roughness) {
    float alpha = roughness * roughness;
    float alpha2 = alpha * alpha;
    float NdotH2 = NdotH * NdotH;  // More efficient than pow(NdotH, 2)
    
    float denom = NdotH2 * (alpha2 - 1.0) + 1.0;
    denom = 3.14159265 * denom * denom;
    
    return alpha2 / max(denom, 0.001);  // Prevent division by zero
}

float G_SchlickGGX(float NdotV, float roughness) {
    // Geometry function for one direction (either L or V)
    float r = roughness + 1.0;
    float k = (r * r) / 8.0;  // for direct light
    float G1 = NdotV / max((NdotV * (1 - k) + k), 0.001);
    return G1;
}

float G_Smith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float G1_L = G_SchlickGGX(NdotL, roughness);  // Use NdotL here
    float G1_V = G_SchlickGGX(NdotV, roughness);  // Use NdotV here
    return G1_L * G1_V;  // Just multiply the two G terms
}

void main()
{
    // ========== SURFACE PROPERTIES ==========
    // Get base color from texture/tint
    vec3 texColor = useBaseColorTex ? texture(baseColorTex, texCoord).rgb : vec3(1.0);
    vec3 baseColor = texColor * baseColorTint;

    // ======== Roughness/Metallic Mapping ======
    float roughness = uRoughness; // Start with the uniform as a base/multiplier
    if (useRoughnessMap) {
        // Multiply by texture for spatial variation
        roughness *= texture(roughnessMap, texCoord).r;
    }
    float metallic = uMetallic;
    if (useMetallicMap) {
        metallic *= texture(metallicMap, texCoord).r;
    }
    
    // ========== NORMAL CALCULATION ==========
    vec3 N = normalize(fragNormal);  // Start with vertex normal, not hardcoded!
    
    if (uUseNormalTex) {
        // Decode normal map from [0,1] to [-1,1]
        vec3 normalSample = texture(uNormalTex, texCoord).rgb * 2.0 - 1.0;
        normalSample *= 0.5;
        normalSample = normalize(normalSample);
        
        // Build TBN matrix to transform from tangent to world space
        vec3 T = normalize(fragTangent);
        vec3 B = normalize(cross(N, T));
        mat3 TBN = mat3(T, B, N);
        
        // Apply normal map
        N = normalize(TBN * normalSample);
    }
    
    // ========== LIGHT CALCULATION ==========
    vec3 L;
    float att = 1.0;
    float spot = 1.0;
    
    // Calculate light direction and attenuation based on light type
    if (uLightType == 0) {
        // Directional light
        L = normalize(uDir_Direction);
    } else {
        // Point/Spot light
        vec3 lightVec = uLight_Position - worldPos;
        L = normalize(lightVec);
        float d = length(lightVec);
        att = 1.0 / (1.0 + 0.09*d + 0.032*d*d);
        
        if (uLightType == 2) {
            // Spot light cone
            float cosTheta = dot(-L, normalize(uDir_Direction));
            float t = clamp((cosTheta - uSpotCosOuter)/(uSpotCosInner - uSpotCosOuter), 0.0, 1.0);
            spot = t*t*(3.0 - 2.0*t);  // smoothstep
        }
    }
    
    // ========== VIEW VECTORS ==========
    vec3 V = normalize(uCamera_Position - worldPos);
    vec3 H = normalize(L + V);
    
    // ========== DOT PRODUCTS (used multiple times) ==========
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float VdotH = max(dot(V, H), 0.0);
    
    // ========== MATERIAL PROPERTIES ==========
    // F0: Base reflectance (dielectric ~4%, metals use base color)
    vec3 F0 = mix(uDielectricF0, baseColor, metallic);
    
    // ========== BRDF COMPONENTS ==========
    // Fresnel (F): Schlick approximation
    vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);
    
    // Distribution (D): GGX/Trowbridge-Reitz
    float D = D_GGX(NdotH, roughness);
    
    // Geometry (G): Smith with Schlick-GGX
    float G = G_Smith(N, V, L, roughness);
    
    // ========== LIGHTING CALCULATION ==========
    // Cook-Torrance BRDF
    vec3 numerator = D * G * F;
    float denominator = max(4.0 * NdotL * NdotV, 0.001);
    vec3 specular = numerator / denominator;
    
    // Energy conservation: kS = F, kD = 1 - kS
    vec3 kS = F;  // Specular contribution
    vec3 kD = vec3(1.0) - kS;  // This is (1.0 - F)
    kD *= 1.0 - metallic;     // Remove diffuse from metals
    vec3 irradiance = uLight_Color * att * spot * NdotL; // incoming light energy
    vec3 Lo = (kD * baseColor / 3.14159265 + specular) * irradiance;


    
    // ========== FINAL COLOR ==========
    vec3 ambient;
    if (useIBL) {
        vec3 irradiance = texture(irradianceMap, N).rgb;
        ambient = kD * irradiance * baseColor;
    } else {
        ambient = baseColor * uAmbient;
    }
    vec3 color = ambient + Lo;

    // ========== TONE MAPPING & GAMMA ==========
    vec3 mapped = color * 2.0;  // Exposure boost
    mapped = mapped / (mapped + vec3(1.0));  // Reinhard tone mapping
    mapped = pow(mapped, vec3(1.0/2.2));  // Gamma correction

    FragColor = vec4(mapped, 1.0);
    
    //FragColor = vec4(color, 1.0);
}