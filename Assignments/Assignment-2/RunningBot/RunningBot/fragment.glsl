varying vec4 varyingColor;
varying vec4 varyingNormal;

uniform vec4 uColor;
uniform vec4 lightPosition;

void main() {
//    gl_FragColor = vec4(1.0, 0.5, 0.5, 1.0);
    gl_FragColor = varyingColor;
//    float diffuse = max(0.0, dot(varyingNormal, lightPosition));
//    vec4 intensity = uColor * diffuse * 3.0;
//    gl_FragColor = intensity;
//    float diffuse = max(0.0, dot(varyingNormal, vec4(0.0, 0.0, 30.0, 0.0)));
//    float diffuse = max(0.0, dot(varyingNormal, lightPosition));
//    vec4 intensity = varyingColor * uColor * diffuse * 3.0;
//    gl_FragColor = intensity;

}