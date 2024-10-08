#section vert

layout(location = 0) in uint index;
layout(location = 1) in vec2 position;
layout(location = 2) in float scale;

void main()
{
    const vec2 quad_verts[3] = vec2[](vec2(-0.5, -0.5), vec2(0.5, -0.5), vec2(0.5, 0.5));
    gl_Position = vec4((quad_verts[index] * scale) + position, 0.0, 1.0);
}

#section frag

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
