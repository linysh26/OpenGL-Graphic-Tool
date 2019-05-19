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
## About light model: Blinn, Phong and Gouraud
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
## Lighting Texture
Generally different material will present different effect of light's reflection, such as metal with a shiny reflecting effect and wooden surface always with a weak reflection, which means that it's neccessary for us to prepare different texture for different materials.
Usually two kinds of textures is better to be given:
+ diffuse texture
+ specular texture
which is exactly two parts of lighting models. Diffuse texture discribe colors of the fragment for diffuse component and is always with a normal strength for reflection like vec(0.5). Note that we also attach diffuse texture for ambient component and, considerably, with a weak reflection like vec(0.1). And for specular, apparently it will have a very strong reflection usually vec(1.0) for the specular texture.
And now fragment's shader will be:
```c++
#version 330 core
in VS_OUT {
	vec3 FragPos;
	vec3 Normal;
	vec2 TexCoords;
}fs_in;
out vec4 FragColor;
uniform vec3 viewPos;
struct Light {
	vec3 color;
	vec3 position;
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;
};
struct Material {
	sampler2D diffuse;
	sampler2D specular;
	float shininess;
};
uniform Light light;
uniform Material material;
void main()
{
// calculate ambient with diffuse texture (surely you can also use an ambient texture)
	vec3 ambient = light.ambient * light.color * texture(material.diffuse, fs_in.TexCoords).rgb;
// calculate normal and light direction
	vec3 normal = normalize(fs_in.Normal);
	vec3 lightDirection = normalize(light.position - fs_in.FragPos);
// calculate diffuse component with diffuse texture
	float diffuseStrength = max(dot(normal, lightDirection), 0.0f);
	vec3 diffuse = light.diffuse * (diffuseStrength * light.color * texture(material.diffuse, fs_in.TexCoords).rgb);
// calculate specular component with specular texture
	vec3 viewDirection = normalize(viewPos - fs_in.FragPos);
	vec3 halfwayDirection = normalize(lightDirection + viewDirection);
	float specularStrength = pow(max(dot(normal, halfwayDirection), 0.0f), material.shininess);
	vec3 specular = light.specular * (specularStrength * texture(material.specular, fs_in.TexCoords).rgb * light.color);
// final fragment's color
  vec3 result =  ambient + diffuse + specular;
  FragColor = vec4(result, 1.0f);
}
```
## About Shadow
### Introduction
First a frame buffer will be genreated with an attachment of depth texture, in which depth information will be renderred from the perspective of the position of source light with a depth shader with vertex shader code as:
```c++
#version 330 core
layout (location = 0) in vec3 position;

uniform mat4 lightSpaceMatrix;
uniform mat4 model;

void main()
{
    gl_Position = lightSpaceMatrix * model * vec4(position, 1.0f);
}
```
and fragment shader code:
```c++
#version 330 core

void main()
{             
    // gl_FragDepth = gl_FragCoord.z;
}
```
which does nothing 'cause depth information will be set automatically. Then we've got a depth texture which reflects depth of the closest fragment seen from position of source light, and we can use it to render our shadow simply with a function checking whether current fragment's depth is deeper than the closest one see from light to it:
```
...
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check if deeper or not
    shadow = currentDepth > closestDepth?1.0f:0.0f;
    return shadow;
}
void main(){
    // calculate ambient, diffuse and specular
    ...
    // since ambient will always be considerred
    vec3 result = ambient + (1 - shadow) * (diffuse + specular);
    FragColor = vec4(result, 1.0f);
}
```
### Optimization
Shadow performs disappointing 'cause there are some problems:
+ Shadow Acne
+ Serrated
#### Shadow Acne

#### Serrated
Since several pixels of fragments sampled with one pixel of the depth texture, it always results in a serrated effect of the shadow renderred. However, it could be reduced by setting a higher resolution of the depth texture or making the spetrum much more closer to the scene going to be renderred. But sometimes it's not available to do so due to some requires of efficiency or the very scene gonna to  be renderred. 
One of a way is called PCF (Percentage-Closer Filtering) which does a multiple sampling and calculates an average result, but still not satisfies us a lot. Anyway, it can fit most of our requirement in renderring shadow.
And now the shadow calculation function will like:
```c++
...
float ShadowCalculation(vec4 fragPosLightSpace)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    // Transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // Get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(shadowMap, projCoords.xy).r; 
    // Get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // Calculate bias (based on depth map resolution and slope)
    vec3 normal = normalize(fs_in.Normal);
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    float bias = max(0.05 * (1.0 - dot(normal, lightDir)), 0.005);
    // Check whether current frag pos is in shadow
    // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
    // PCF
    float shadow = 0.0;
    vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
    for(int x = -1; x <= 1; ++x)
    {
        for(int y = -1; y <= 1; ++y)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r; 
            shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
        }    
    }
    shadow /= 9.0;
    
    // Keep the shadow at 0.0 when outside the far_plane region of the light's frustum.
    if(projCoords.z > 1.0)
        shadow = 0.0;
        
    return shadow;
}
...
```
