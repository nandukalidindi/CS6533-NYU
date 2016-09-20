attribute vec4 positionAttribute;
attribute vec2 texturePositionAttribute;

varying vec2 varyingTexturePosition;

uniform vec2 positionUniform;

void main() {
    varyingTexturePosition = texturePositionAttribute;
    gl_Position = vec4(positionUniform.x, positionUniform.y, 0.0, 0.0) + positionAttribute;
    
}