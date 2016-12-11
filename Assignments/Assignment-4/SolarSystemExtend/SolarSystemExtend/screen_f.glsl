//uniform sampler2D screenFramebuffer;
//varying vec2 texCoordVar;
//void main() {
//    gl_FragColor = texture2D( screenFramebuffer, texCoordVar);
//}

uniform sampler2D texture;
varying vec2 texCoordVar;
void main()
{
    gl_FragColor = vec4(1.0-texture2D(texture, texCoordVar).xyz, 1.0);
}

//uniform sampler2D texture;
//varying vec2 texCoordVar;
//void main()
//{
//    vec4 texColor = texture2D( texture, texCoordVar);
//    float brightness = (texColor.x+texColor.y+texColor.z)/3.0;
//    gl_FragColor = vec4(brightness, brightness, brightness, 1.0);
//}