varying vec4 varyingColor;
varying vec4 varyingNormal;

uniform vec3 uColor;

void main() {
//    gl_FragColor = vec4(1.0, 0.5, 0.5, 1.0);
    gl_FragColor = varyingColor;
//    float diffuse = max(0.0, dot(varyingNormal, vec4(-0.5773, 0.5773, 0.5773, 0.0)));
//    vec3 intensity = vec3(1.0, 0.0, 0.0) * diffuse * 4.0;
//    gl_FragColor = vec4(intensity.xyz, 1.0);
}