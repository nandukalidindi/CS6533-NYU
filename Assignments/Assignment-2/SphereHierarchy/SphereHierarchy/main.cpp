
#include <glut.h>
#include "glsupport.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include <vector>

GLuint program;
GLuint vertexPositionVBO;
GLuint indexBO;
GLuint colorBufferObject;
GLuint normalBufferObject;
GLuint postionAttribute;
GLuint colorAttribute;
GLuint normalAttribute;
GLuint timeUniform;
GLuint modelViewMatrixUniform;
GLuint normalMatrixUniform;
GLuint projectionMatrixUniform;

int numIndices;

struct Entity {
    Cvec3 translation;
    Cvec3 rotation;
    Cvec3 scale;
    
    Matrix4 modelMatrix;
    Entity *parent;
};

struct VertexPN {
    Cvec3f p;
    Cvec3f n;
    VertexPN() {}
    VertexPN(float x, float y, float z, float nx, float ny, float nz) : p(x,y,z), n(nx, ny, nz) {}
    
    VertexPN& operator = (const GenericVertex& v) {
        p = v.pos;
        n = v.normal;
        return *this;
    }
};

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
//    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glUseProgram(program);
    
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glUniform1f(timeUniform, (float)timeSinceStart/1000.0f);
    
    //DRAW BODY
    Matrix4 bodyMatrix, bodyScaleMatrix, eyeMatrix, modelViewMatrix, axisRotateMatrix, sunMatrix;
    bodyMatrix = bodyMatrix.makeZRotation(45.0) * bodyMatrix.makeYRotation(timeSinceStart/10.0f);
//    bodyMatrix.makeZRotation(45.0 + timeSinceStart/10.0f);
//    Matrix4 yRotation;
//    yRotation = yRotation.makeYRotation(45.0);
//    bodyMatrix = bodyMatrix * yRotation;
    bodyScaleMatrix = bodyScaleMatrix.makeScale(Cvec3(2.0, 4.0, 1.0));
    
    eyeMatrix = eyeMatrix.makeTranslation(Cvec3(0.0, 0.0, 30.0));
    
    modelViewMatrix = inv(eyeMatrix) * bodyMatrix;// * bodyScaleMatrix;
    
    sunMatrix = modelViewMatrix;
    
    GLfloat glmatrix[16];
    modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glmatrix);
    
    Matrix4 normalMatrix = transpose(inv(modelViewMatrix));
    normalMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(normalMatrixUniform, 1, GL_FALSE, glmatrix);
    
    Matrix4 projectionMatrix;
    projectionMatrix = projectionMatrix.makeProjection(45, (1280.0/800.0), -0.5, -1000.0);
    GLfloat glmatrixProjection[16];
    projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glmatrixProjection);
    
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionVBO);
    glVertexAttribPointer(postionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
    glEnableVertexAttribArray(postionAttribute);
    
    glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
    glEnableVertexAttribArray(normalAttribute);
    
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferObject);
    glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(colorAttribute);

    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);    //
    
    Matrix4 mercuryMatrix, mercuryModelViewMatrix;
    
    mercuryMatrix = mercuryMatrix.makeZRotation(timeSinceStart/50.0f) *
                    mercuryMatrix.makeTranslation(Cvec3(10.0, 0.0, 0.0)) *
                    mercuryMatrix.makeZRotation(-45.0) *
                    mercuryMatrix.makeYRotation(timeSinceStart/10.0f) *
                    mercuryMatrix.makeScale(Cvec3(1.0/2.0, 1.0/2.0, 1.0/2.0));
    
    mercuryModelViewMatrix = inv(eyeMatrix) * mercuryMatrix;
    
    mercuryModelViewMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glmatrix);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    
    Matrix4 mercurySatellite;
    
    mercurySatellite = mercurySatellite.makeScale(Cvec3(2.0, 2.0, 2.0)) *
                       mercurySatellite.makeTranslation(Cvec3(2.0, 0.0, 0.0)) *
                       mercurySatellite.makeZRotation(timeSinceStart/10.0f) *
                       mercurySatellite.makeScale(Cvec3(1.0/4.0, 1.0/4.0, 1.0/4.0));
    
    mercuryModelViewMatrix = mercuryModelViewMatrix * mercurySatellite;
    
    mercuryModelViewMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glmatrix);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    
    
    Matrix4 earthMatrix, earthScaleMatrix, earthTranslationMatrix, earthRotationMatrix, moonMatrix,
            moonTranslationalMatrix, moonRotationMatrix, moonScaleMatrix, axishShiftMatrix;
    
    
    earthScaleMatrix = earthScaleMatrix.makeScale(Cvec3(1.0/2.0, 1.0/2.0, 1.0/2.0));
    earthTranslationMatrix = earthTranslationMatrix.makeTranslation(Cvec3(6.0, 0.0, 0.0));
    earthRotationMatrix = earthRotationMatrix.makeZRotation(45.0) * earthRotationMatrix.makeYRotation(timeSinceStart/10.0f);
    
    earthMatrix = earthTranslationMatrix * earthRotationMatrix * earthScaleMatrix;
    
//    axishShiftMatrix = axishShiftMatrix.makeZRotation(45.0);
    
    earthMatrix = axishShiftMatrix * earthMatrix * inv(axishShiftMatrix);
    
    modelViewMatrix = modelViewMatrix * earthMatrix;
    
    modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glmatrix);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
//
    moonTranslationalMatrix = moonTranslationalMatrix.makeTranslation(Cvec3(4.0, 0.0, 0.0));
    moonScaleMatrix = moonScaleMatrix.makeScale(Cvec3(1.0/2.0, 1.0/2.0, 1.0/2.0));
    
    moonRotationMatrix = moonRotationMatrix.makeZRotation(45.0) * moonRotationMatrix.makeYRotation(timeSinceStart/10.0f);
    
    modelViewMatrix = modelViewMatrix * moonTranslationalMatrix * moonRotationMatrix * moonScaleMatrix;
    
    modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glmatrix);
    glDrawElements(GL_TRIANGLES, numIndices, GL_UNSIGNED_SHORT, 0);
    
    glDisableVertexAttribArray(postionAttribute);
    glDisableVertexAttribArray(colorAttribute);
    glDisableVertexAttribArray(normalAttribute);
    glutSwapBuffers();
}

void init() {
    glClearDepth(0.0f);
    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);
    
    program = glCreateProgram();
    readAndCompileShader(program, "/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-2/SphereHierarchy/SphereHierarchy/vertex.glsl", "/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-2/SphereHierarchy/SphereHierarchy/fragment.glsl");
    
    glUseProgram(program);
    postionAttribute = glGetAttribLocation(program, "position");
    colorAttribute = glGetAttribLocation(program, "color");
    normalAttribute = glGetAttribLocation(program, "normal");
    timeUniform = glGetUniformLocation(program, "timeUniform");
    modelViewMatrixUniform = glGetUniformLocation(program, "modelViewMatrix");
    normalMatrixUniform = glGetUniformLocation(program, "normalMatrix");
    projectionMatrixUniform = glGetUniformLocation(program, "projectionMatrix");
    
    
    int ibLen, vbLen;
    
//    getCubeVbIbLen(vbLen, ibLen);
//    std::vector<VertexPN> vtx(vbLen);
//    std::vector<unsigned short> idx(ibLen);
//    makeCube(2, vtx.begin(), idx.begin());
    
    getSphereVbIbLen(16, 16, vbLen, ibLen);
    std::vector<VertexPN> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);
    makeSphere(2, 16, 16, vtx.begin(), idx.begin());
    
    glGenBuffers(1, &vertexPositionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
    
    numIndices = ibLen;
    
    
    glGenBuffers(1, &indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), idx.data(), GL_STATIC_DRAW);

    
    glGenBuffers(1, &colorBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferObject);
    
    GLfloat cubeColors[] = {
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f,
        0.583f,  0.771f,  0.014f, 1.0f,
        0.609f,  0.115f,  0.436f, 1.0f,
        0.327f,  0.483f,  0.844f, 1.0f,
        0.822f,  0.569f,  0.201f, 1.0f,
        0.435f,  0.602f,  0.223f, 1.0f,
        0.310f,  0.747f,  0.185f, 1.0f,
        0.597f,  0.770f,  0.761f, 1.0f,
        0.559f,  0.436f,  0.730f, 1.0f,
        0.359f,  0.583f,  0.152f, 1.0f,
        0.483f,  0.596f,  0.789f, 1.0f,
        0.559f,  0.861f,  0.639f, 1.0f,
        0.195f,  0.548f,  0.859f, 1.0f,
        0.014f,  0.184f,  0.576f, 1.0f,
        0.771f,  0.328f,  0.970f, 1.0f,
        0.406f,  0.615f,  0.116f, 1.0f,
        0.676f,  0.977f,  0.133f, 1.0f,
        0.971f,  0.572f,  0.833f, 1.0f,
        0.140f,  0.616f,  0.489f, 1.0f,
        0.997f,  0.513f,  0.064f, 1.0f,
        0.945f,  0.719f,  0.592f, 1.0f,
        0.543f,  0.021f,  0.978f, 1.0f,
        0.279f,  0.317f,  0.505f, 1.0f,
        0.167f,  0.620f,  0.077f, 1.0f,
        0.347f,  0.857f,  0.137f, 1.0f,
        0.055f,  0.953f,  0.042f, 1.0f,
        0.714f,  0.505f,  0.345f, 1.0f,
        0.783f,  0.290f,  0.734f, 1.0f,
        0.722f,  0.645f,  0.174f, 1.0f,
        0.302f,  0.455f,  0.848f, 1.0f,
        0.225f,  0.587f,  0.040f, 1.0f,
        0.517f,  0.713f,  0.338f, 1.0f,
        0.053f,  0.959f,  0.120f, 1.0f,
        0.393f,  0.621f,  0.362f, 1.0f,
        0.673f,  0.211f,  0.457f, 1.0f,
        0.820f,  0.883f,  0.371f, 1.0f,
        0.982f,  0.099f,  0.879f, 1.0f
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
    
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 800);
    glutCreateWindow("CS-6533");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    init();
    glutMainLoop();
    return 0;
}