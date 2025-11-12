#version 460
layout(location = 0) out vec4 FragColor;
  
layout(location = 0) in vec4 vertexColor; // the input variable from the vertex shader (same name and same type)  
layout(location = 1) in vec2 vertexUV;

layout(binding = 0) uniform sampler2D img;

void main()
{
    FragColor = vertexColor + texture(img, vertexUV);
} 