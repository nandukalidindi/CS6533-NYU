uniform vec3 colorUniform;     // vec3 - each coordinate for RED, GREEN and BLUE

void main() {
    // Start from BLACK and intensify/nullify from the input 
    gl_FragColor = vec4(colorUniform.x, colorUniform.y, colorUniform.z, 1.0) + vec4(0.0, 0.0, 0.0, 1.0);
}