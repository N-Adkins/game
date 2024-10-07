#section vert

layout(location = 0) in int index;
layout(location = 1) in vec2 position;
layout(location = 2) in float scale;

void main()
{
    vec2 quad_verts[3];
    quad_verts[0] = vec2(-0.5, -0.5); // Bottom left
    quad_verts[1] = vec2(0.5, -0.5);  // Bottom right
    quad_verts[2] = vec2(0.5, 0.5);   // Top right
    //quad_verts[3] = vec2(-0.5, 0.5);  // Top left
    
    gl_Position = vec4((quad_verts[index] * scale) + position, 0.0, 1.0);
}

#section frag

out vec3 FragColor;

void main()
{
    FragColor = vec3(1.0, 0.0, 0.0);
}
