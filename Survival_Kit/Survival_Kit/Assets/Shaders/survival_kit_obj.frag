#version 420 core
//
//struct Material 
//{
//    vec3 Ka;            // Ambient reflectivity
//    vec3 Kd;            // Diffuse reflectivity
//    vec3 Ks;            // Specular reflectivity
//    float shininess;    // Specular shininess factor
//};
//
struct Light 
{
    vec3 position;      // Position of the light source in the world space
    vec3 La;            // Ambient light intensity
    vec3 Ld;            // Diffuse light intensity
    vec3 Ls;            // Specular light intensity
};

in vec3 Position;       // In view space
in vec3 Normal;         // In view space
in vec3 Color;         

uniform Light light;
//uniform Material material;
uniform mat4 V;         // View transform matrix

layout(location=0) out vec4 FragColor;


//vec3 BlinnPhong(vec3 position, vec3 normal, Light light, Material material, mat4 view)
//{
//    // Transform light position from world to view space
//    vec3 LightInViewSpace = vec3(view * vec4(light.position, 1.0));
//
//    // Return 0 if light in view space is same as position
//    if (LightInViewSpace == position) {
//        return vec3(0.0);
//    }
//
//    // Compute vector to light normal
//    vec3 vectorL = normalize(LightInViewSpace - position); 
//    
//    // Compute view vector (camera is at vec3(0,0,0) if position is in view space
//    vec3 vectorV = normalize(-position);
//
//    // Compute half vector of vector to light and view vector
//    vec3 vectorH = normalize(vectorV + vectorL);
//    
//    // Compute ambient light contribution
//    vec3 ambient = light.La * material.Ka;
//
//    // Compute diffuse light contribution
//    float radiantEnergy = max(dot(normal, vectorL), 0.0);
//    vec3 diffuse = light.Ld * material.Kd * radiantEnergy;
//
//    // Compute specular light contribution
//    vec3 specular = vec3(0.0);
//    if (0.0 < radiantEnergy) {
//        specular = light.Ls * material.Ks * pow(max(dot(normal, vectorH), 0.0), material.shininess);  
//    }
//
//    // Compute final illumination
//    vec3 illumination = ambient + diffuse + specular; 
//    
//    return illumination;
//}


void main() 
{
    //FragColor = vec4(BlinnPhong(Position, normalize(Normal), light[0], material, V), 1.0f);

    // Temporary default material and lighting values
    // Material
    vec3 material_Ka = vec3(0.3, 0.5, 0.9);  // Ambient
    vec3 material_Kd = vec3(0.3, 0.5, 0.9);  // Diffuse
    vec3 material_Ks = vec3(0.8, 0.8, 0.8);  // Specular
    float shininess = 100.0;

//    // Light Hardcoded
//    vec3 light_Pos = vec3(0.0, 5.0, 0.0);
//    vec3 light_La = vec3(0.4, 0.4, 0.4);     // Ambient
//    vec3 light_Ld = vec3(1.0, 1.0, 1.0);     // Diffuse
//    vec3 light_Ls = vec3(1.0, 1.0, 1.0);     // Specular

    // Light
    vec3 light_Pos = light.position;
    vec3 light_La = light.La;     // Ambient
    vec3 light_Ld = light.Ld;     // Diffuse
    vec3 light_Ls = light.Ls;     // Specular

    
    // Temporary default illumination computation

    vec3 lightInViewSpace = vec3(V * vec4(light_Pos, 1.0));

    if(lightInViewSpace == Position) {
        FragColor = vec4(0.0, 0.0, 0.0, 1.0);
        return;
    } 

    vec3 vectorL = normalize(lightInViewSpace - Position);
    vec3 vectorV = normalize(-Position);
    vec3 vectorH = normalize(vectorV + vectorL);

    vec3 ambient = light_La * material_Ka;
    
    float radiantEnergy = max(dot(Normal, vectorL), 0.0);
    vec3 diffuse = light_Ld * material_Kd * radiantEnergy;

    vec3 specular = vec3(0.0);
    if(0.0 < radiantEnergy) {
        specular = light_Ls * material_Ks * pow(max(dot(Normal, vectorH), 0.0), shininess);
    }

    vec3 illumination = ambient + diffuse + specular;
    FragColor = vec4(illumination, 1.0);

//    vec3 n = normalize(Normal);
//    FragColor = vec4(n * 0.5 + 0.5, 1.0);
}