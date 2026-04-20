#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;

uniform mat4 u_mvp;
uniform vec4 u_bounds;
uniform float u_alpha;
out vec2 v_uv;
out float v_alpha;

void main() {
    vec2 worldPos = mix(u_bounds.xy, u_bounds.zw, a_pos);
    gl_Position = u_mvp * vec4(worldPos, 0.0, 1.0);
    v_uv = a_uv;
    v_alpha = u_alpha;
}
