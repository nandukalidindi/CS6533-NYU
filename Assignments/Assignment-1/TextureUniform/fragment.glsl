varying vec2 varyingTexturePosition;   

uniform vec2 texturePositionUniform;
uniform sampler2D textureUniform;

void main() {
    vec2 texCoord = vec2(varyingTexturePosition.x, varyingTexturePosition.y) + vec2(texturePositionUniform.x, texturePositionUniform.y);
    gl_FragColor = texture2D(textureUniform, texCoord);
}