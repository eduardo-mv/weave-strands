#version 460
layout(location = 0) in vec3 aPos; // the position variable has attribute position 0
  
layout(location = 0) out vec4 vertexColor; // specify a color output to the fragment shader
layout(location = 1) out vec2 vertexUV;

out gl_PerVertex {
    vec4 gl_Position;// makes gl_Position is part of interface
};

vec3 vertices[] = {
    vec3(-0.5f, -0.5f, 0.0f),
    vec3(0.5f, -0.5f, 0.0f),
    vec3(0.0f,  0.5f, 0.0f)
};

layout(std140, binding = 5) uniform Uniforms {
    vec4 testvcolor;
    vec4 testvcolor2;
    vec4 testvcolor3;
    vec4 what;
};

layout(std430, binding = 3) buffer MyBuffer
{
  vec4 color;
  vec4 waitAminute;
  float lotsOfFloats[];
};

uniform vec4 unif;

void main()
{
    gl_Position = vec4(aPos + color.xyz, 1.0); 
    //gl_Position = vec4(vertices[gl_VertexID]*1.1, 1.0);
    vertexColor = vec4(testvcolor.xyz + testvcolor3.xyz + color.xyz + what.xyz,1.0);
    vertexColor = unif;
    vertexUV = aPos.xy + vec2(0.5);
}