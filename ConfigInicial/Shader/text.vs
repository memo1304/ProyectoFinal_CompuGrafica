#version 330 core
layout (location = 0) in vec2 aPos;
uniform mat4 projection;
uniform vec2 scale; // <-- NUEVO
void main() {
    gl_Position = projection * vec4(aPos * scale, 0.0, 1.0);
}