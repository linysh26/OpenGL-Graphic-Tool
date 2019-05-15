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
## About our light model: Blinn, Phong and Gouraud
### Introduction
These kinds of light model are a light model consists three parts:
+ ambient
+ diffuse
+ specular
### Differences between Blinn, Phong and Gouraud
We have implemented three kinds of lighting model, Blinn, Phong and Gouraud. The most difference from Blinn and Phong with Gouraud is that **Blinn(Phong) calculating light's influence on each pixel with normal interpolated by each vertex while Gouraud calculating light's influence on vertices, with which fragment's color is interpolated**. And the only difference between Blinn and Phong is the way of handling specular part, as **Phong calculates specular with reflect direction and our viewing direction while Blinn calculates using a halfway angle and the normal of the very fragment**. We can easily figure out this difference with their shader programs.
Blinn and Phong's fragment shader code:  
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
    // blinn
    // float specularStrength = 0.5;
    // vec3 viewDir = normalize(viewPos - FragPos);
    // vec3 halfwayDirection = normalize(lightDirection + viewDirection);
    // float specularStrength = pow(max(dot(fs_in.normal, halfwayDirection), 0.0f), material.shininess);
    // vec3 specular = light.specular * (specularStrength * texture(material.specular, fs_in.TexCoords).rgb * light.color);
        
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
