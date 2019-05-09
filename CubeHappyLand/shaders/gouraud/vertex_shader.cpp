#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
// position of this fragment
out vec3 FragPos;
// normal of this fragment
out vec3 Normal;
// model matrix
uniform mat4 model;
// view matrix to transform from global to view
uniform mat4 view;
// pojection matrix to project according to perspection
uniform mat4 projection;
void main()
{
	objectColor = aCol;
	FragPos = vec3(model * vec4(aPos, 1.0f));
	gl_Position = projection * view * FragPos;
	Noraml = mat3(transpose(inverse(model))) * aNormal;
}