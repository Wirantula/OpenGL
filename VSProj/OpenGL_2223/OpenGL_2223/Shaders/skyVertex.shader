#version 330 core
layout(location = 0) in vec3 aPos;

out vec3 worldPosition;

uniform mat4 world, view, projection;

void main()
{
	gl_Position = projection * view * world * vec4(aPos, 1.0);
	worldPosition = mat3(world) * aPos;
}