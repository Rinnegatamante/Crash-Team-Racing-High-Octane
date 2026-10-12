#version 140

uniform vec4 uCrop;
out vec2 v_uv;

void main()
{
    vec2 corner = vec2(float(gl_VertexID & 1), float((gl_VertexID >> 1) & 1));
    v_uv = uCrop.xy + corner * uCrop.zw;
    gl_Position = vec4(corner * 2.0 - 1.0, 0.0, 1.0);
}
