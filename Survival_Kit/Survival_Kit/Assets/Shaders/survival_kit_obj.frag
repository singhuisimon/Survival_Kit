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
//struct Light 
//{
//    vec3 position;      // Position of the light source in the world space
//    vec3 La;            // Ambient light intensity
//    vec3 Ld;            // Diffuse light intensity
//    vec3 Ls;            // Specular light intensity
//};

in vec3 Position;       // In view space
in vec3 Normal;         // In view space

//uniform Light light[1];
//uniform Material material;
//uniform mat4 V;         // View transform matrix

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

    // Temporary default ambient value
    vec3 light_la = vec3(0.4f, 0.4f, 0.4f);
    vec3 material_ka = vec3(0.9f, 0.5f, 0.3f);
    vec3 ambient = light_la * material_ka;

    FragColor = vec4(ambient, 1.0f);
}