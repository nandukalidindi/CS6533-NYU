uniform vec3 colorUniform;     // vec3 - each coordinate for RED, GREEN and BLUEs

void main() {
    gl_FragColor = vec4(colorUniform.x, colorUniform.y, colorUniform.z, 1.0);
}