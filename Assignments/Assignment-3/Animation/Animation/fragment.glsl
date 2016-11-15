varying vec3 varyingNormal;
varying vec3 varyingPosition;
uniform vec3 uColor;

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

uniform Light lights[3];


void main() {
    vec3 textureNormal = normalize((texture2D(normalTexture, varyingTexCoord).xyz * 2.0) -1.0);
    textureNormal = normalize(varyingTBNMatrix * textureNormal);
    vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
    vec3 specularColor = vec3(0.0, 0.0, 0.0);
    for(int i=0; i<3; i++) {
        float diffuse = max(0.0, dot(varyingNormal, lights[i].lightPosition));
        diffuseColor += (vec3(1.0, 1.0, 1.0) * diffuse);
        
        vec3 v = normalize(-varyingPosition);
        vec3 h = normalize(v + lights[i].lightPosition);
        float specular = pow(max(0.0, dot(h, varyingNormal)), 64.0);
        specularColor += vec3(1.0, 1.0, 1.0) * specular;
    }
    vec3 intensity = (texture2D(diffuseTexture, varyingTexCoord).xyz * diffuseColor) +
                     (specularColor * texture2D(specularTexture, varyingTexCoord).x);
    gl_FragColor = vec4(intensity.xyz, 1.0);
//    gl_FragColor = texture2D(diffuseTexture, varyingTexCoord);
}