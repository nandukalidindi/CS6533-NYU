uniform sampler2D screenFramebuffer;
varying vec2 texCoordVar;
//uniform float exposure;
void main() {
    const float gamma = 2.2;
    float exposure = 5.0;
    vec3 hdrColor = texture2D( screenFramebuffer, texCoordVar).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    float luminance = dot(mapped, vec3(0.299, 0.587, 0.144));
    mapped = mapped * smoothstep(0.8, 1.0, luminance) * 2.0;
    mapped = pow(mapped, vec3(gamma));
    gl_FragColor = vec4(mapped, 1.0);
}