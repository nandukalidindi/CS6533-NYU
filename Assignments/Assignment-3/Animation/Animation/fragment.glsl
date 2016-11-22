varying vec3 varyingNormal;
varying vec3 varyingPosition;

varying vec2 varyingTexCoord;
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D normalTexture;

varying mat3 varyingTBNMatrix;

struct Light {
    vec3 lightPosition;
    vec3 lightColor;
    vec3 specularLightColor;
};

uniform Light lights[4];

float attenuate(float dist, float a, float b) {
    return 1.0 / (1.0 + a*dist + b*dist*dist);
}


void main() {
    vec3 textureNormal = normalize((texture2D(normalTexture, varyingTexCoord).xyz * 2.0) -1.0);
    textureNormal = normalize(varyingTBNMatrix * textureNormal);
    
    vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
    vec3 specularColor = vec3(0.0, 0.0, 0.0);
    for(int i=0; i<4; i++) {
        vec3 lightDirection = -normalize(varyingPosition-lights[i].lightPosition);
        
        float diffuse = max(0.0, dot(textureNormal, lightDirection));
        float attenuation = attenuate(distance(varyingPosition, lightDirection) / 50.0, 50.7, 50.0);
        diffuseColor += (lights[0].lightColor * diffuse);
        
        
        vec3 v = normalize(-varyingPosition);
        vec3 h = normalize(v + lightDirection);
        float specular = pow(max(0.0, dot(h, textureNormal)), 64.0);
        specularColor += (lights[0].specularLightColor) * specular;
    }
    vec3 intensity = (texture2D(diffuseTexture, varyingTexCoord).xyz * diffuseColor) +
                     (texture2D(specularTexture, varyingTexCoord).x * specularColor);
    gl_FragColor = vec4(intensity.xyz, 1.0);
}