attribute vec4 positionAttribute;
uniform vec2 positionUniform;

void main() {
    gl_Position = vec4(positionUniform.x, positionUniform.y, 0.0, 0.0) + positionAttribute;
}