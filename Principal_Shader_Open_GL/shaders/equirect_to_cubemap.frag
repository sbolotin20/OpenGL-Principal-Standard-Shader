#version 330 core
out vec4 FragColor;
in vec3 localPos;  // Position on the cube face

uniform sampler2D equirectangularMap;

void main() {
    // Normalize to get direction from cube center
    vec3 direction = normalize(localPos);
    
    // Convert direction vector to spherical coordinates
    // Think of it as: where would this ray hit on a sphere?

    const float PI = 3.14159265358979323846;
    const float INV_PI = 1.0 / PI;
    const float INV_TWOPI = 1.0 / (2.0 * PI);
    
    // Step 1: Calculate spherical angles
    float theta = acos(direction.y); // angle from Y axis (0 at north pole, π at south pole)
    float phi = atan(direction.z, direction.x); // angle around Y axis (0 to 2π)
    
    // Step 2: Convert to UV coordinates [0,1]
    float u = fract(phi * INV_TWOPI + 0.5); // Normalize phi from [-π,π] to [0,1] - wraps around horizontally
    float v = theta * INV_PI + 0.5; // Normalize theta from [0,π] to [0,1] - top to bottom
    vec2 uv = vec2(u, v);
    
    // Step 3: Sample the equirectangular map
    vec3 color = texture(equirectangularMap, uv).rgb;
    FragColor = vec4(color, 1.0);
}