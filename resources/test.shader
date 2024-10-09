#section vert

layout(location = 0) in uint index;
layout(location = 1) in vec2 position;
layout(location = 2) in float scale;

uniform mat4 projection;

void main()
{
    const vec2 quad_verts[6] = vec2[](
        vec2(-0.5, -0.5), 
        vec2(0.5, -0.5), 
        vec2(0.5, 0.5),
        vec2(-0.5, -0.5), 
        vec2(-0.5, 0.5), 
        vec2(0.5, 0.5)
    );
    vec4 scaled_pos = vec4((quad_verts[index] * scale) + position, 0.0, 1.0);
    gl_Position = projection * scaled_pos;
}

#section frag

out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0, 1.0, 0.0, 1.0);
}
