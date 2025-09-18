// shaders/skybox.frag
#version 330 core
in vec3 vDir;
out vec4 FragColor;
uniform samplerCube env;
void main() {
    FragColor = texture(env, normalize(vDir));
}
