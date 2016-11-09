varying vec4 varyingNormal;

uniform vec4 lightPosition;

void main() {
    float diffuse = max(0.0, dot(varyingNormal, lightPosition));
    vec3 intensity = vec3(1.0, 1.0, 1.0) * diffuse * 3.0;
    gl_FragColor = vec4(intensity.xyz, 1.0);
}