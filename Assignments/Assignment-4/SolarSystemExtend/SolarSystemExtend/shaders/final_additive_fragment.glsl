uniform sampler2D screenFramebuffer;
uniform sampler2D blurredHighlights;
varying vec2 texCoordVar;
//uniform float exposure;
void main() {
    const float gamma = 2.2;
    float exposure = 5.0;
    vec3 hdrColor = texture2D( screenFramebuffer, texCoordVar).rgb;
    vec3 mapped = vec3(1.0) - exp(-hdrColor * exposure);
    mapped = pow(mapped, vec3(gamma));
    mapped += texture2D(blurredHighlights, texCoordVar).rgb;
    gl_FragColor = vec4(mapped, 1.0);
}