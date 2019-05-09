# OpenGL-Graphic-Tool
This is a tool implemented by c++'s OpenGL which can draw triangles, lines, circles and 3D cube
## LearnOpenGL-Turorial
[Here is the link for this nice tutorial](https://learnopengl-cn.github.io)
## GLFW
[download GLFW and include it in your project](https://learnopengl-cn.github.io/01%20Getting%20started/02%20Creating%20a%20window/#glfw)
## GLAD
[create your glad header and include it in your project](https://learnopengl-cn.github.io/01%20Getting%20started/02%20Creating%20a%20window/#glad)
## ImGUI
[download and include ImGUI in your project](https://github.com/ocornut/imgui)
## About our light model: Phong and Gouraud
### Introduction
Phong is a light model consists three parts:
+ ambient
+ diffuse
+ specular
which is also parts of Gouraud.
### Differences between Phong and Gouraud
We have implemented two kinds of lighting model, one is Phong Lighting, another one is Gouraud Lighting. The most difference between these two kinds of light shader is that Phong calculating light's influence on each pixel with normal interpolated by each vertex while Gouraud calculating light's influence on vertices, with which fragment's color is interpolated. And we can easily figure out this difference with their shader programs.
Phong's fragment shader code:  
```c++
#version 330 core
out vec4 FragColor;

in vec3 Normal;  
in vec3 FragPos;  
  
uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

void main()
{
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
        
    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
} 
```
Gouraud's vertex shader's code:  
```c++
#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out vec3 color;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 lightPos; 
uniform vec3 viewPos; 
uniform vec3 lightColor;
uniform vec3 objectColor;

uniform float ambientFactor;
uniform float diffuseFactor;
uniform float specularFactor;
uniform float shininess;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;  
    
    gl_Position = projection * view * vec4(FragPos, 1.0);
    // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
    
    // specular
    float specularStrength = 0.5;
    vec3 viewDir = normalize(viewPos - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
    vec3 specular = specularStrength * spec * lightColor;  
    vec3 color = (ambient + diffuse + specular) * objectColor;
} 
```
It is obvious that Phong calculate light in fragment shader while Gouraud in vertex shader instead, which shows that Phong is calculating light using normals interpolated while Gouraud outputs the interpolation of vertices' color calculated in vertex shader using corresponding normals.
