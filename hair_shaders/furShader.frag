#version 430 core

uniform sampler2D mainTexture;

uniform vec3 cameraPosition;
uniform vec3 lightPos;
uniform vec3 lightColor;

in vec2 gTexCoord;
in vec3 gPosition;
in vec3 gTangent;

out vec4 color;

void main()
{
    // Calculate the direction vector from the light source to the vertex and normalize it.
    vec3 light = normalize(lightPos - gPosition);
    // Retrieve the surface color from the texture.
    vec3 texColor = vec3(texture(mainTexture, gTexCoord));
    // Calculate the diffuse intensity using cross product.
    float diffuseAmount = length(cross(light, gTangent));
    // If the diffuse intensity is less than 0.7, limit it to 0.7 to avoid over-brightness.
    if(diffuseAmount < 0.7)
        diffuseAmount = 0.7;
    // Calculate the diffuse color and multiply it by the surface color.
    vec3 diffuse = texColor * diffuseAmount;
    // Calculate the direction vector from the camera position to the vertex and normalize it.
    vec3 eye = normalize(gPosition - cameraPosition);
    // Define the specular exponent p and calculate the specular intensity
    float exponent = 50;
    vec3 H = normalize(-light + eye);
    float dotTH = dot(gTangent, H);
    float sinTH = sqrt(1.0 - dotTH* dotTH);
    float dirAtten = smoothstep(2.0f, 0, dotTH);
    vec3 specular = dirAtten * pow(sinTH, exponent) * texColor * 0.5;
    //Add the diffuse and specular color and set the alpha channel to 0.9 to get the final color.
    color = vec4(diffuse + specular, 0.9);

}