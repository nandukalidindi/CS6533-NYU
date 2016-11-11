varying vec3 varyingNormal;
varying vec3 varyingPosition;
uniform vec3 uColor;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 specularLightColor;


void main() {
    float diffuse = max(0.0, dot(varyingNormal, lightPosition));
    vec3 diffuseColor = vec3(1.0, 0.0, 0.0) * diffuse;
    
    vec3 v = normalize(-varyingPosition);
    vec3 h = normalize(v + lightPosition);
    float specular = pow(max(0.0, dot(h, varyingNormal)), 64.0);
    vec3 specularHighlight = vec3(1.0, 0.0, 0.0) * specular;

    vec3 intensity = (uColor * diffuseColor) + specularHighlight;
    gl_FragColor = vec4(intensity.xyz, 1.0);
}