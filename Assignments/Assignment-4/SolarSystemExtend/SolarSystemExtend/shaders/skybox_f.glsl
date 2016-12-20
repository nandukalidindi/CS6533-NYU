varying vec3 varyingNormal;
varying vec3 varyingPosition;
uniform mat4 normalMatrix;
uniform samplerCube environmentMap;
mat3 linearTranspose(mat4 m4) {
    return mat3(
                m4[0][0], m4[1][0], m4[2][0],
                m4[0][1], m4[1][1], m4[2][1],
                m4[0][2], m4[1][2], m4[2][2]);
}

void main() {
    vec3 reflectVector = reflect(normalize(varyingPosition), varyingNormal);
    reflectVector = linearTranspose(normalMatrix) * reflectVector;
    gl_FragColor = textureCube(environmentMap, reflectVector);
}