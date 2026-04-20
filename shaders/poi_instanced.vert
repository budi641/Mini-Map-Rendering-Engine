#version 330 core
layout(location = 0) in vec2 a_unitQuad;
layout(location = 1) in vec2 a_worldPos;
layout(location = 2) in float a_sizeWorld;
layout(location = 3) in float a_headingDeg;
layout(location = 4) in float a_alpha;

uniform mat4 u_mvp;
out float v_alpha;

void main() {
    float rad = radians(a_headingDeg);
    mat2 rot = mat2(cos(rad), -sin(rad), sin(rad), cos(rad));
    vec2 worldPos = a_worldPos + rot * (a_unitQuad * a_sizeWorld);
    gl_Position = u_mvp * vec4(worldPos, 0.0, 1.0);
    v_alpha = a_alpha;
}
