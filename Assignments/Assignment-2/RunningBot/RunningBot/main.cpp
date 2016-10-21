
#include <glut.h>
#include "glsupport.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include <vector>
#include <math.h>

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

float frameSpeed = 10.0f;

float xOffset = 0.5773, yOffset = 0.5773, zOffset = 0.5773;

float redOffset = 0.0, blueOffset = 0.0, greenOffset = 0.0;

int numIndices;

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
    
    void draw(Matrix4 &eyeMatrix, GLuint modelViewMatrixUniform, GLuint normalMatrixUniform) {
        if(parent == NULL) {
            modelViewMatrix = inv(eyeMatrix) * objectMatrix;
        } else {
            geometry.vertexVBO = parent->geometry.vertexVBO;
            geometry.colorVBO = parent->geometry.colorVBO;
            geometry.indexBO = parent->geometry.indexBO;
            geometry.numIndices = parent->geometry.numIndices;
            geometry.positionAttribute = parent->geometry.positionAttribute;
            geometry.colorAttribute = parent->geometry.colorAttribute;
            geometry.normalAttribute = parent->geometry.normalAttribute;
            
            modelViewMatrix = (parent->modelViewMatrix) * (objectMatrix);
        }
        
        geometry.draw();
        
        GLfloat glmatrix[16];
        modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
        glUniformMatrix4fv(modelViewMatrixUniform, 1, GL_FALSE, glmatrix);
        
        Matrix4 normalMatrix = transpose(inv(modelViewMatrix));
        normalMatrix.writeToColumnMajorMatrix(glmatrix);
        glUniformMatrix4fv(normalMatrixUniform, 1, GL_FALSE, glmatrix);
        
        glDrawElements(GL_TRIANGLES, geometry.numIndices, GL_UNSIGNED_SHORT, 0);
    }
};


float calculateTimeAngle(float anglePerRev, float timeSinceStart) {
    float timeCrunch = timeSinceStart/anglePerRev;
    float finalAngle = 0.0;
    int revolution = floor(timeCrunch);
    if(revolution%2 == 0) {
        finalAngle = ((timeCrunch) - floor(timeCrunch))*anglePerRev;
    } else {
        finalAngle = (ceil(timeCrunch) - timeCrunch)*anglePerRev;
    }
    return finalAngle;
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glUseProgram(program);
    
    int timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glUniform1f(timeUniform, (float)timeSinceStart/1000.0f);
    
    glUniform4f(lightPositionUniform, xOffset, yOffset, zOffset, 0.0);
    
    glUniform4f(uColorUniform, redOffset, greenOffset, blueOffset, 1.0);
    
    Entity *trunkEntity = new Entity;
    trunkEntity->parent = NULL;
    Geometry trunkGeometry;
    
    //Eye Matrix
    Matrix4 eyeMatrix;
    //    eyeMatrix = eyeMatrix.makeYRotation(calculateTimeAngle(90.0, timeSinceStart/10.0f)-45);
    eyeMatrix = eyeMatrix.makeYRotation(40.0) /* eyeMatrix.makeZRotation(timeSinceStart/10.0f)*/;
    eyeMatrix = eyeMatrix * eyeMatrix.makeTranslation(Cvec3(0.0, 0.0, 30.0));
    
    trunkGeometry.vertexVBO = vertexPositionVBO;
    trunkGeometry.indexBO = indexBO;
    trunkGeometry.colorVBO = colorBufferObject;
    trunkGeometry.numIndices = numIndices;
    trunkGeometry.positionAttribute = postionAttribute;
    trunkGeometry.colorAttribute = colorAttribute;
    trunkGeometry.normalAttribute = normalAttribute;
    
    trunkEntity->geometry = trunkGeometry;
    
    Matrix4 trunkMatrix, trunkScaleMatrix;
    
    trunkScaleMatrix = trunkScaleMatrix.makeScale(Cvec3(2.0, 3.0, 1.0));

    trunkEntity->objectMatrix = (trunkMatrix * trunkScaleMatrix);
    
    trunkEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    //Model view matrix
    Matrix4 trunkModelViewMatrix, modelViewMatrix;
    
    //Draw Trunk 2 3 4 5 7 Sec 2
    
    Matrix4 projectionMatrix;
    projectionMatrix = projectionMatrix.makeProjection(45, (1280.0/800.0), -0.5, -1000.0);
    GLfloat glmatrixProjection[16];
    projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
    glUniformMatrix4fv(projectionMatrixUniform, 1, GL_FALSE, glmatrixProjection);
    
    // HEAD
    
    Entity *headEntity = new Entity;
    
    Matrix4 headMatrix, axisShiftMatrix, headModelViewMatrix;
    
    headMatrix = headMatrix.makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                 headMatrix.makeTranslation(Cvec3(0.0, 4.5, 0.0)) *
                 headMatrix.makeYRotation(timeSinceStart/10.0f) *
                 headMatrix.makeScale(Cvec3(1.0, 1.0, 1.0));
    
    headEntity->objectMatrix = headMatrix;
    headEntity->parent = trunkEntity;
    
    headEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    
    // LEFT THIGH AND KNEE
    Matrix4 leftThighMatrix, leftKneeMatrix, rightThighMatrix, rightKneeMatrix, leftThighModelViewMatrix, leftKneeModelViewMatrix,
            rightThighModelViewMatrix, rightKneeModelViewMatrix;
    
    leftThighMatrix = leftThighMatrix.makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                      leftThighMatrix.makeTranslation(Cvec3(1.5, -6.0, 0.0)) *
                      rightThighMatrix.makeXRotation(calculateTimeAngle(90, timeSinceStart/frameSpeed)-45) *
                      leftThighMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    leftThighMatrix = axisShiftMatrix * leftThighMatrix * inv(axisShiftMatrix);
    
    Entity *leftThighEntity = new Entity;
    
    leftThighEntity->parent = trunkEntity;
    leftThighEntity->objectMatrix = leftThighMatrix;
    leftThighEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    
    // Left KNEE
    leftKneeMatrix = leftKneeMatrix.makeScale(Cvec3(3.0, 1.0/1.5, 1.0)) *
                     leftKneeMatrix.makeTranslation(Cvec3(0.0, -3.0, 0.0)) *
                     rightThighMatrix.makeXRotation(45 - calculateTimeAngle(45, timeSinceStart/frameSpeed))*
                     leftKneeMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    leftKneeMatrix = axisShiftMatrix * leftKneeMatrix * inv(axisShiftMatrix);
    
    
    Entity *leftKneeEntity = new Entity;
    
    leftKneeEntity->parent = leftThighEntity;
    leftKneeEntity->objectMatrix = leftKneeMatrix;
    
    leftKneeEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    
    // RIGHT THIGH AND KNEE
    rightThighMatrix = rightThighMatrix.makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                       rightThighMatrix.makeTranslation(Cvec3(-1.5, -6.0, 0.0)) *
                       rightThighMatrix.makeXRotation(45-calculateTimeAngle(90, timeSinceStart/frameSpeed))*
                       rightThighMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    rightThighMatrix = axisShiftMatrix * rightThighMatrix * inv(axisShiftMatrix);
    
    Entity *rightThighEntity = new Entity;
    
    rightThighEntity->parent = trunkEntity;
    rightThighEntity->objectMatrix = rightThighMatrix;
    rightThighEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    
    //Right KNEE
    rightKneeMatrix = rightKneeMatrix.makeScale(Cvec3(3.0, 1.0/1.5, 1.0)) *
                      rightKneeMatrix.makeTranslation(Cvec3(0.0, -3.0, 0.0)) *
                      rightKneeMatrix.makeXRotation(45 - calculateTimeAngle(45, timeSinceStart/frameSpeed))*
                      rightKneeMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, 1.0, 0.0));
    
    rightKneeMatrix = axisShiftMatrix * rightKneeMatrix * inv(axisShiftMatrix);
    
    Entity *rightKneeEntity = new Entity;
    
    rightKneeEntity->parent = rightThighEntity;
    rightKneeEntity->objectMatrix = rightKneeMatrix;
    rightKneeEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    
//    // LEFT ARM AND ELBOW
    Matrix4 leftArmMatrix, leftElbowMatrix, rightArmMatrix, rightElbowMatrix, leftArmModelViewMatrix, leftElbowModelViewMatrix,
            rightArmModelViewMatrix, rightElbowModelViewMatrix;
    
    leftArmMatrix = leftArmMatrix.makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                    leftArmMatrix.makeTranslation(Cvec3(-2.5, 5.0, 0.0)) *
                    leftArmMatrix.makeXRotation(180.0) *
                    leftArmMatrix.makeXRotation(calculateTimeAngle(90, timeSinceStart/frameSpeed)-45) *
                    leftArmMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    
    leftArmMatrix = axisShiftMatrix * leftArmMatrix * inv(axisShiftMatrix);
    
    Entity *leftArmEntity = new Entity;
    
    leftArmEntity->parent = trunkEntity;
    leftArmEntity->objectMatrix = leftArmMatrix;
    leftArmEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);

    // Left ELBOW
    leftElbowMatrix = leftElbowMatrix.makeScale(Cvec3(3.0, 1.0/1.5, 1.0)) *
                      leftElbowMatrix.makeTranslation(Cvec3(0.0, 3.0, 0.0)) *
                      leftElbowMatrix.makeXRotation(-45.0) *
                      leftElbowMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    
    leftElbowMatrix = axisShiftMatrix * leftElbowMatrix * inv(axisShiftMatrix);
    
    Entity *leftElbowEntity = new Entity;
    
    leftElbowEntity->parent = leftArmEntity;
    leftElbowEntity->objectMatrix = leftElbowMatrix;
    leftElbowEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);

    // Right ARM
    rightArmMatrix = rightArmMatrix.makeScale(Cvec3(1.0/2.0, 1.0/3.0, 1.0)) *
                     rightArmMatrix.makeTranslation(Cvec3(2.5, 5.0, 0.0)) *
                     rightArmMatrix.makeXRotation(180.0) *
                     rightArmMatrix.makeXRotation(45 - calculateTimeAngle(90, timeSinceStart/frameSpeed)) *
                     rightArmMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    
    rightArmMatrix = axisShiftMatrix * rightArmMatrix * inv(axisShiftMatrix);
    
    Entity *rightArmEntity = new Entity;
    
    rightArmEntity->parent = trunkEntity;
    rightArmEntity->objectMatrix = rightArmMatrix;
    rightArmEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);

    // Right ELBOW
    rightElbowMatrix = rightElbowMatrix.makeScale(Cvec3(3.0, 1.0/1.5, 1.0)) *
                       rightElbowMatrix.makeTranslation(Cvec3(0.0, 3.0, 0.0)) *
                       rightElbowMatrix.makeXRotation(-45.0)*
                       rightElbowMatrix.makeScale(Cvec3(1.0/3.0, 1.5, 1.0));
    
    axisShiftMatrix = axisShiftMatrix.makeTranslation(Cvec3(0.0, -1.0, 0.0));
    
    rightElbowMatrix = axisShiftMatrix * rightElbowMatrix * inv(axisShiftMatrix);
    
    Entity *rightElbowEntity = new Entity;
    
    rightElbowEntity->parent = rightArmEntity;
    rightElbowEntity->objectMatrix = rightElbowMatrix;
    rightElbowEntity->draw(eyeMatrix, modelViewMatrixUniform, normalMatrixUniform);
    
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
    
    getCubeVbIbLen(vbLen, ibLen);
    std::vector<VertexPN> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);
    makeCube(2, vtx.begin(), idx.begin());
    
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
            zOffset += 0.5;
            break;
        case 'a':
            xOffset += 0.5;
            break;
        case 'd':
            yOffset += 0.5;
            break;
        case 's':
            xOffset = 0.5773;
            yOffset = 0.5773;
            zOffset = 0.5773;
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
        case 'c':
            redOffset = 0.0;
            blueOffset = 0.0;
            greenOffset = 0.0;
            break;
    }
}

void mouse(int button, int state, int x, int y) {
    if(state == 0) {
        xOffset = x;
        yOffset = y;
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1280, 800);
    glutCreateWindow("CS-6533");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    
    init();
    glutMainLoop();
    return 0;
}