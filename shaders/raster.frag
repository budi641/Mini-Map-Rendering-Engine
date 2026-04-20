#version 330 core
in vec2 v_uv;
in float v_alpha;
out vec4 FragColor;
uniform sampler2D u_tex;

void main() {
    FragColor = texture(u_tex, v_uv) * vec4(1.0, 1.0, 1.0, 0.35 * v_alpha);
}
