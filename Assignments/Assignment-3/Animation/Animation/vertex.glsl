attribute vec4 position;
attribute vec4 normal;

uniform vec4 timeUniform;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;

varying vec3 varyingNormal;
varying vec3 varyingPosition;

void main() {
    varyingNormal = normalize((normalMatrix * normal).xyz);
    vec4 p = modelViewMatrix * position;
    varyingPosition = p.xyz;
    gl_Position = projectionMatrix * p;
}