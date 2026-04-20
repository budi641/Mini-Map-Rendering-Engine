#version 330 core
in float v_alpha;
out vec4 FragColor;

void main() {
    FragColor = vec4(0.98, 0.48, 0.06, 0.92 * v_alpha);
}
