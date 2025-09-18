// shaders/skybox.vert
#version 330 core
layout(location=0) in vec3 aPos;
out vec3 vDir;
uniform mat4 view;
uniform mat4 projection;
void main() {
    vDir = aPos;
    mat4 V = mat4(mat3(view));        // remove translation
    vec4 p = projection * V * vec4(aPos, 1.0);
    gl_Position = p.xyww;             // push to far plane
}
