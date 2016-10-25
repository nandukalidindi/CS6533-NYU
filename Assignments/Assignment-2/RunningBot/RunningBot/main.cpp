
#include <glut.h>
#include "glsupport.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include <vector>
#include <math.h>
#include "quat.h"

GLuint program;
GLuint vertexPositionVBO;
GLuint indexBO;
GLuint colorBufferObject;
GLuint normalBufferObject;
GLuint postionAttribute;
GLuint colorAttribute;
GLuint normalAttribute;
GLuint timeUniform;
GLuint uColorUniform;
GLuint lightPositionUniform;
GLuint modelViewMatrixUniform;
GLuint normalMatrixUniform;
GLuint projectionMatrixUniform;

Matrix4 eyeMatrix;

float frameSpeed = 10.0f;

float xOffset = 0.5773, yOffset = 0.5773, zOffset = 10.0;

float redOffset = 1.0, blueOffset = 1.0, greenOffset = 1.0;

float botX = 0.0, botY = 0.0, botZ = 0.0;

float botXDegree = 0.0, botYDegree = 0.0, botZDegree = 0.0;

int numIndices, timeSinceStart = 0.0;

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

struct Geometry {
    GLuint vertexVBO;
    GLuint colorVBO;
    GLuint indexBO;
    GLuint positionAttribute;
    GLuint colorAttribute;
    GLuint normalAttribute;
    GLuint modelViewMatrixUniform;
    GLuint normalMatrixUniform;
    int numIndices;
    
    void draw() {
        glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
        glVertexAttribPointer(postionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN),
                              (void*)offsetof(VertexPN, p));
        glEnableVertexAttribArray(postionAttribute);
        
        glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN),
                              (void*)offsetof(VertexPN, n));
        glEnableVertexAttribArray(normalAttribute);
        
        glBindBuffer(GL_ARRAY_BUFFER, colorVBO);
        glVertexAttribPointer(colorAttribute, 4, GL_FLOAT, GL_FALSE, 0, 0);
        glEnableVertexAttribArray(colorAttribute);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
    }
};

struct Entity {
    Matrix4 objectMatrix;
    Geometry geometry;
    Matrix4 modelViewMatrix;
    Entity *parent;
    
    void draw(Matrix4 &eyeMatrix) {
        if(parent == NULL)
            modelViewMatrix = inv(eyeMatrix) * objectMatrix;
        else
            modelViewMatrix = (parent->modelViewMatrix) * (objectMatrix);
        
        geometry.draw();
        
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
        
        glDrawElements(GL_TRIANGLES, geometry.numIndices, GL_UNSIGNED_SHORT, 0);
    }
};

Entity *drawBodyParts(Geometry geometry, Matrix4 objectMatrix, Entity *parent) {
    Entity *partEntity = new Entity;
    partEntity->parent = parent;
    partEntity->geometry = geometry;
    partEntity->objectMatrix = objectMatrix;
    partEntity->draw(eyeMatrix);
    return partEntity;
}


float calculateTimeAngle(float anglePerRev, float timeSinceStart) {
    float timeCrunch = timeSinceStart/anglePerRev;
    float finalAngle = 0.0;
    int revolution = floor(timeCrunch);
    if(revolution%2 == 0)
        finalAngle = ((timeCrunch) - floor(timeCrunch))*anglePerRev;
    else
        finalAngle = (ceil(timeCrunch) - timeCrunch)*anglePerRev;
    return finalAngle;
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glUseProgram(program);
    
    timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glUniform1f(timeUniform, (float)timeSinceStart/1000.0f);
    glUniform4f(lightPositionUniform, xOffset, yOffset, zOffset, 0.0);
    glUniform4f(uColorUniform, redOffset, greenOffset, blueOffset, 1.0);
    
    // ------------------------------- EYE -------------------------------
    eyeMatrix = quatToMatrix(Quat::makeYRotation(40.0)) *
                quatToMatrix(Quat::makeYRotation(botYDegree)) *
                quatToMatrix(Quat::makeXRotation(botXDegree)) *
                quatToMatrix(Quat::makeZRotation(botZDegree));
    eyeMatrix = eyeMatrix * eyeMatrix.makeTranslation(Cvec3(0.0, 0.0, 30.0));
    // ------------------------------- EYE -------------------------------
    
    // ------------------------------- TRUNK -------------------------------
    Geometry trunkGeometry;
    trunkGeometry.vertexVBO = vertexPositionVBO;
    trunkGeometry.indexBO = indexBO;
    trunkGeometry.colorVBO = colorBufferObject;
    trunkGeometry.numIndices = numIndices;
    trunkGeometry.positionAttribute = postionAttribute;
    trunkGeometry.colorAttribute = colorAttribute;
    trunkGeometry.normalAttribute = normalAttribute;
    
    Matrix4 trunkMatrix = Matrix4::makeTranslation(Cvec3(botX, botY, botZ));
//                          quatToMatrix(Quat::makeXRotation(botXDegree)) *
//                          quatToMatrix(Quat::makeYRotation(botYDegree)) *
//                          quatToMatrix(Quat::makeZRotation(botZDegree));
    trunkMatrix = trunkMatrix * Matrix4::makeScale(Cvec3(2.0, 3.0, 1.0));
    Entity *trunkEntity = drawBodyParts(trunkGeometry, trunkMatrix, NULL);
    // ------------------------------- TRUNK -------------------------------
    
    // ------------------------------- HEAD -------------------------------
    Matrix4 axisShiftMatrix;
    
    Matrix4 headMatrix = Matrix4::makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                        Matrix4::makeTranslation(Cvec3(0.0, 4.5, 0.0)) *
                        quatToMatrix(Quat::makeYRotation(45-calculateTimeAngle(90, timeSinceStart/frameSpeed))) *
                        Matrix4::makeScale(Cvec3(1.0, 1.2, 1.0));
    
    Entity *headEntity = drawBodyParts(trunkGeometry, headMatrix, trunkEntity);
    // ------------------------------- HEAD -------------------------------
    
    // ------------------------------ EYES -------------------------------
    for(int i=0; i<2; i++) {
        Matrix4 botEyeMatrix = Matrix4::makeScale(Cvec3(1.0/1.0, 1.0/1.2, 1.0/1.0)) *
                               Matrix4::makeTranslation(Cvec3(0.7 - (1.4*i), 0.4, 1.0)) *
                               Matrix4::makeScale(Cvec3(1.0/5.0, 1.0/5.0, 1.0/5.0));
        
        drawBodyParts(trunkGeometry, botEyeMatrix, headEntity);
    }
    // ------------------------------- EYES -------------------------------
    
    // ------------------------------- LEFT THIGH -------------------------------
    Matrix4 leftThighMatrix = Matrix4::makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                              Matrix4::makeTranslation(Cvec3(1.5, -6.0, 0.0)) *
                              quatToMatrix(Quat::makeXRotation(calculateTimeAngle(90,timeSinceStart/frameSpeed)-45)) *
                              Matrix4::makeScale(Cvec3(1.0/1.5, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    leftThighMatrix = axisShiftMatrix * leftThighMatrix * inv(axisShiftMatrix);
    
    Entity *leftThighEntity = drawBodyParts(trunkGeometry, leftThighMatrix, trunkEntity);
    // ------------------------------- LEFT THIGH -------------------------------
    
    // ------------------------------- LEFT KNEE -------------------------------
    Matrix4 leftKneeMatrix = Matrix4::makeScale(Cvec3(1.5, 1.0/1.5, 1.0)) *
                             Matrix4::makeTranslation(Cvec3(0.001, -3.0, 0.0)) *
                             quatToMatrix(Quat::makeXRotation(45-calculateTimeAngle(45, timeSinceStart/frameSpeed)))*
                             Matrix4::makeScale(Cvec3(1.0/2.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    leftKneeMatrix = axisShiftMatrix * leftKneeMatrix * inv(axisShiftMatrix);
    
    Entity *leftKneeEntity = drawBodyParts(trunkGeometry, leftKneeMatrix, leftThighEntity);
    // ------------------------------- LEFT KNEE -------------------------------
    
    // ------------------------------- LEFT FOOT FINGERS -------------------------------
    for(int i=0; i<3; i++) {
        Matrix4 leftFootFingerMatrix =  Matrix4::makeScale(Cvec3(2.0, 1/1.5, 1.0)) *
                                        Matrix4::makeTranslation(Cvec3(0.4-(i*0.4), -1.8, 0.8)) *
                                        Matrix4::makeScale(Cvec3(1.0/8.0, 1.0/8.0, 1.0/2.0));
        drawBodyParts(trunkGeometry, leftFootFingerMatrix, leftKneeEntity);
    }
    // ------------------------------- LEFT FOOT FINGERS -------------------------------
    
    // ------------------------------- RIGHT THIGH -------------------------------
    Matrix4 rightThighMatrix = Matrix4::makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                               Matrix4::makeTranslation(Cvec3(-1.5, -6.0, 0.0)) *
                               quatToMatrix(Quat::makeXRotation(45-calculateTimeAngle(90,timeSinceStart/frameSpeed)))*
                               Matrix4::makeScale(Cvec3(1.0/1.5, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    rightThighMatrix = axisShiftMatrix * rightThighMatrix * inv(axisShiftMatrix);
    
    Entity *rightThighEntity = drawBodyParts(trunkGeometry, rightThighMatrix, trunkEntity);
    // ------------------------------- RIGHT THIGH -------------------------------
    
    // ------------------------------- RIGHT KNEE -------------------------------
    Matrix4 rightKneeMatrix = Matrix4::makeScale(Cvec3(1.5, 1.0/1.5, 1.0)) *
                              Matrix4::makeTranslation(Cvec3(0.001, -3.0, 0.0)) *
                              quatToMatrix(Quat::makeXRotation(45-calculateTimeAngle(45,timeSinceStart/frameSpeed)))*
                              Matrix4::makeScale(Cvec3(1.0/2.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    rightKneeMatrix = axisShiftMatrix * rightKneeMatrix * inv(axisShiftMatrix);

    Entity *rightKneeEntity = drawBodyParts(trunkGeometry, rightKneeMatrix, rightThighEntity);
    // ------------------------------- RIGHT KNEE -------------------------------
    
    // ------------------------------- RIGHT FOOT FINGERS -------------------------------
    for(int i=0; i<3; i++) {
        Matrix4 rightFootFingerMatrix =  Matrix4::makeScale(Cvec3(2.0, 1/1.5, 1.0)) *
                                         Matrix4::makeTranslation(Cvec3(0.4-(0.4*i), -1.8, 0.8)) *
                                         Matrix4::makeScale(Cvec3(1.0/8.0, 1.0/8.0, 1.0/2.0));
        drawBodyParts(trunkGeometry, rightFootFingerMatrix, rightKneeEntity);
    }
    // ------------------------------- RIGHT FOOT FINGERS -------------------------------
    
    // ------------------------------- LEFT ARM -------------------------------
    Matrix4 leftArmMatrix = Matrix4::makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                            Matrix4::makeTranslation(Cvec3(-2.7, 5.0, 0.0)) *
                            quatToMatrix(Quat::makeXRotation(180.0) *
                                         Quat::makeXRotation(calculateTimeAngle(90, timeSinceStart/frameSpeed)-45)) *
                            Matrix4::makeScale(Cvec3(1.0/1.8, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    
    leftArmMatrix = axisShiftMatrix * leftArmMatrix * inv(axisShiftMatrix);
    
    Entity *leftArmEntity = drawBodyParts(trunkGeometry, leftArmMatrix, trunkEntity);
    // ------------------------------- LEFT ARM -------------------------------

    // ------------------------------- LEFT ELBOW -------------------------------
    Matrix4 leftElbowMatrix = Matrix4::makeScale(Cvec3(1.8, 1.0/1.5, 1.0)) *
                              Matrix4::makeTranslation(Cvec3(0.001, 3.0, 0.0)) *
                              quatToMatrix(Quat::makeXRotation(-45.0)) *
                              Matrix4::makeScale(Cvec3(1.0/2.0, 1.5, 1.0));
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    leftElbowMatrix = axisShiftMatrix * leftElbowMatrix * inv(axisShiftMatrix);
    Entity *leftElbowEntity = drawBodyParts(trunkGeometry, leftElbowMatrix, leftArmEntity);
    // ------------------------------- LEFT ELBOW -------------------------------
    
    // ------------------------------- LEFT HAND FINGERS -------------------------------
    for(int i=0; i<4; i++) {
        Matrix4 leftFingerMatrix = Matrix4::makeScale(Cvec3(2.0, 1.0/1.5, 1.0)) *
                                   Matrix4::makeTranslation(Cvec3(0.0, 1.6, 0.7-(0.5*i))) *
                                   Matrix4::makeScale(Cvec3(1.0/5.0, 1.0/2.0, 1.0/5.0));
        
        drawBodyParts(trunkGeometry, leftFingerMatrix, leftElbowEntity);
    }
    // ------------------------------- LEFT HAND FINGERS -------------------------------

    // ------------------------------- RIGHT ARM -------------------------------
    Matrix4 rightArmMatrix = Matrix4::makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                             Matrix4::makeTranslation(Cvec3(2.7, 5.0, 0.0)) *
                             quatToMatrix(Quat::makeXRotation(180.0) *
                                          Quat::makeXRotation(45-calculateTimeAngle(90,timeSinceStart/frameSpeed))) *
                             Matrix4::makeScale(Cvec3(1.0/1.8, 1.5, 1.0));
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    rightArmMatrix = axisShiftMatrix * rightArmMatrix * inv(axisShiftMatrix);
    Entity *rightArmEntity = drawBodyParts(trunkGeometry, rightArmMatrix, trunkEntity);
    // ------------------------------- RIGHT ARM -------------------------------

    // ------------------------------- RIGHT ELBOW -------------------------------
    Matrix4 rightElbowMatrix = Matrix4::makeScale(Cvec3(1.8, 1.0/1.5, 1.0)) *
                               Matrix4::makeTranslation(Cvec3(0.001, 3.0, 0.0)) *
                               quatToMatrix(Quat::makeXRotation(-45.0)) *
                               Matrix4::makeScale(Cvec3(1.0/2.0, 1.5, 1.0));
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    rightElbowMatrix = axisShiftMatrix * rightElbowMatrix * inv(axisShiftMatrix);
    Entity *rightElbowEntity = drawBodyParts(trunkGeometry, rightElbowMatrix, rightArmEntity);
    // ------------------------------- RIGHT ELBOW -------------------------------
    
    // ------------------------------- RIGHT HAND FINGERS -------------------------------
    for(int i=0; i<4; i++) {
        Matrix4 rightFingerMatrix = Matrix4::makeScale(Cvec3(2.0, 1.0/1.5, 1.0)) *
                                    Matrix4::makeTranslation(Cvec3(0.0, 1.6, 0.7-(0.5*i))) *
                                    Matrix4::makeScale(Cvec3(1.0/5.0, 1.0/2.0, 1.0/5.0));
        
        drawBodyParts(trunkGeometry, rightFingerMatrix, rightElbowEntity);
    }
    // ------------------------------- RIGHT HAND FINGERS -------------------------------
    
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
    readAndCompileShader(program, "/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-2/RunningBot/RunningBot/vertex.glsl", "/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-2/RunningBot/RunningBot/fragment.glsl");
    
    glUseProgram(program);
    postionAttribute = glGetAttribLocation(program, "position");
    colorAttribute = glGetAttribLocation(program, "color");
    normalAttribute = glGetAttribLocation(program, "normal");
    timeUniform = glGetUniformLocation(program, "timeUniform");
    uColorUniform = glGetUniformLocation(program, "uColor");
    lightPositionUniform = glGetUniformLocation(program, "lightPosition");
    modelViewMatrixUniform = glGetUniformLocation(program, "modelViewMatrix");
    normalMatrixUniform = glGetUniformLocation(program, "normalMatrix");
    projectionMatrixUniform = glGetUniformLocation(program, "projectionMatrix");
    
    
    int ibLen, vbLen;
    
    getSphereVbIbLen(10, 10, vbLen, ibLen);
    std::vector<VertexPN> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);
    makeSphere(1.3, 10, 10, vtx.begin(), idx.begin());
    
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
        0.609f,  0.115f,  0.436f, 1.0f
    };
    
    glBufferData(GL_ARRAY_BUFFER, sizeof(cubeColors), cubeColors, GL_STATIC_DRAW);
    
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'w':
            botZ += 0.75;
//            zOffset += 0.5;
            break;
        case 'a':
            botX -= 0.75;
            break;
        case 'd':
            botX += 0.75;
            break;
        case 's':
            botZ -= 0.75;
            break;
        case 'x':
            botXDegree += 5.0;
            break;
        case 'y':
            botYDegree += 5.0;
            break;
        case 'z':
            botZDegree += 5.0;
            break;
        case 'r':
            if (redOffset <= 1.0)
                redOffset += 0.02;
            break;
        case 'g':
            if (greenOffset <= 1.0)
                greenOffset += 0.02;
            break;
        case 'b':
            if (blueOffset <= 1.0)
                blueOffset += 0.02;
            break;
        case 'v':
            botX = botY = botZ = botXDegree = botYDegree = botZDegree = 0.0;
            break;
        case 'c':
            redOffset = 0.0;
            blueOffset = 0.0;
            greenOffset = 0.0;
            break;
        case 'f':
            frameSpeed += 5.0;
            break;
        case 'e':
            if(frameSpeed > 5.0)
                frameSpeed -= 5.0;
            break;
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 800);
    glutCreateWindow("Running Bot");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    glutKeyboardFunc(keyboard);
    
    init();
    glutMainLoop();
    return 0;
}