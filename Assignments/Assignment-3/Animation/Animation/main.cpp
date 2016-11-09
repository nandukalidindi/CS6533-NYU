
#include <glut.h>
#include "glsupport.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include <vector>
#include <math.h>
#include "quat.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"

GLuint program;

GLuint vertexPositionVBO;
GLuint indexBO;
GLuint colorBufferObject;
GLuint normalBufferObject;

GLuint postionAttributeFromVertexShader;
GLuint colorAttributeFromVertexShader;
GLuint normalAttributeFromVertexShader;

GLuint uColorUniformFromFragmentShader;
GLuint lightPositionUniformFromFragmentShader;
GLuint modelViewMatrixUniformFromVertexShader;
GLuint normalMatrixUniformFromVertexShader;
GLuint projectionMatrixUniformFromVertexShader;

Matrix4 eyeMatrix;

Cvec3 initialVector;
Cvec3 kVector;
float finalAngle;

float frameSpeed = 10.0f;
float lightXOffset = -0.5773, lightYOffset = 0.5773, lightZOffset = 10.0;
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

/**
 * Structure to hold all the attribute, uniform, buffer object locations and bind
 * them to the buffers accordingly
 *
 * Structure: BufferBinder
 */
struct BufferBinder {
    GLuint vertexBufferObject;
    GLuint colorBufferObject;
    GLuint indexBufferObject;
    GLuint positionAttribute;
    GLuint colorAttribute;
    GLuint normalAttribute;
    GLuint modelViewMatrixUniform;
    GLuint normalMatrixUniform;
    int numIndices;
    
    void draw() {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glVertexAttribPointer(postionAttributeFromVertexShader, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
        glEnableVertexAttribArray(postionAttributeFromVertexShader);
        
        glVertexAttribPointer(normalAttributeFromVertexShader, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
        glEnableVertexAttribArray(normalAttributeFromVertexShader);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
    }
};

/**
 * Structure to perform the Hierarchical operations on children objects and to issue
 * a draw call via drawElements
 *
 * Structure: Entity
 */

struct Entity {
    Matrix4 objectMatrix;
    BufferBinder bufferBinder;
    Matrix4 modelViewMatrix;
    Entity *parent;
    
    void draw(Matrix4 &eyeMatrix) {
        if(parent == NULL)
            modelViewMatrix = inv(eyeMatrix) * objectMatrix;
        else
            modelViewMatrix = (parent->modelViewMatrix) * (objectMatrix);
        
        bufferBinder.draw();
        
        GLfloat glmatrix[16];
        modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
        glUniformMatrix4fv(modelViewMatrixUniformFromVertexShader, 1, GL_FALSE, glmatrix);
        
        Matrix4 normalMatrix = transpose(inv(modelViewMatrix));
        normalMatrix.writeToColumnMajorMatrix(glmatrix);
        glUniformMatrix4fv(normalMatrixUniformFromVertexShader, 1, GL_FALSE, glmatrix);
        
        Matrix4 projectionMatrix;
        projectionMatrix = projectionMatrix.makeProjection(45, (1280.0/800.0), -0.5, -1000.0);
        GLfloat glmatrixProjection[16];
        projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
        glUniformMatrix4fv(projectionMatrixUniformFromVertexShader, 1, GL_FALSE, glmatrixProjection);
        
        glDrawElements(GL_TRIANGLES, bufferBinder.numIndices, GL_UNSIGNED_SHORT, 0);
    }
};

/**
 * Function to issue a draw call to the respective entity object by assigning all the
 * required attributes
 *
 * Function: drawBodyParts
 *           bufferBinder - Structure to bind the resepective attributes and buffer objects
 *           objectMatrix - Object matrix with respect to the object frame
 *           parent - Immediate hierarchical parent
 */
Entity *drawBodyParts(BufferBinder bufferBinder, Matrix4 objectMatrix, Entity *parent) {
    Entity *partEntity = new Entity;
    partEntity->parent = parent;
    partEntity->bufferBinder = bufferBinder;
    partEntity->objectMatrix = objectMatrix;
    partEntity->draw(eyeMatrix);
    return partEntity;
}

/**
 * Function to restrict the motion of a body part to an angle and to perform the rotation
 * cycles continuously rather than a sudden complete switch of the offset angle
 *
 * Function: calculateTimeAngle
 * anglePerRev - Offset angle after which the motion repeats smoothly
 * timeSinceStart - Variable which is applied as an angle
 */
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

void loadObjFile(const std::string &fileName, std::vector<VertexPN> &outVertices, std::vector<unsigned
                 short> &outIndices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
    if(ret) {
        for(int i=0; i < attrib.vertices.size(); i+=3) {
            VertexPN v;
            v.p[0] = attrib.vertices[i];
            v.p[1] = attrib.vertices[i+1];
            v.p[2] = attrib.vertices[i+2];
            v.n[0] = attrib.normals[i];
            v.n[1] = attrib.normals[i+1];
            v.n[2] = attrib.normals[i+2];
            outVertices.push_back(v);
        }
        for(int i=0; i < shapes.size(); i++) {
            for(int j=0; j < shapes[i].mesh.indices.size(); j++) {
                outIndices.push_back(shapes[i].mesh.indices[j].vertex_index);
            }
        }
    } else {
        std::cout << err << std::endl;
        assert(false);
    }
}

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glUseProgram(program);
    
    timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    glUniform4f(lightPositionUniformFromFragmentShader, lightXOffset, lightYOffset, lightZOffset, 0.0);
    glUniform4f(uColorUniformFromFragmentShader, redOffset, greenOffset, blueOffset, 1.0);
    
    // ------------------------------- EYE -------------------------------
    //    eyeMatrix = quatToMatrix(Quat::makeYRotation(40.0)) *
    //                quatToMatrix(Quat::makeYRotation(botYDegree)) *
    //                quatToMatrix(Quat::makeXRotation(botXDegree)) *
    //                quatToMatrix(Quat::makeZRotation(botZDegree));
    eyeMatrix = quatToMatrix(Quat::makeKRotation(kVector, finalAngle)) *
    Matrix4::makeTranslation(Cvec3(0.0, 0.0, 30.0));
    // ------------------------------- EYE -------------------------------
    
    // Initialising a Genric bufferBinder object as the same buffers are used to
    // render all the objects in hierarchy
    BufferBinder genericBufferBinder;
    genericBufferBinder.vertexBufferObject = vertexPositionVBO;
    genericBufferBinder.colorBufferObject = colorBufferObject;
    genericBufferBinder.indexBufferObject = indexBO;
    genericBufferBinder.numIndices = numIndices;
    genericBufferBinder.positionAttribute = postionAttributeFromVertexShader;
    genericBufferBinder.colorAttribute = colorAttributeFromVertexShader;
    genericBufferBinder.normalAttribute = normalAttributeFromVertexShader;
    
    // ------------------------------- TRUNK -------------------------------
    
    Quat quat1 = Quat::makeYRotation(45.0);
    Quat quat2 = Quat::makeYRotation(180.0);
    
    Matrix4 invShiftMatrix = Matrix4::makeTranslation(Cvec3(0.0, 7.5/80.0, 0.0));
    
    Matrix4 trunkMatrix =  /*Matrix4::makeTranslation(Cvec3(5*sin(timeSinceStart/1000.0f), 10 * sin(timeSinceStart/1000.0f), 0.0)) */
    //                          quatToMatrix(slerp(quat1, quat2, sin(timeSinceStart/1000.0f))) *
    Matrix4::makeScale(Cvec3(80.0, 80.0, 80.0));
    
    trunkMatrix = invShiftMatrix * trunkMatrix * inv(invShiftMatrix);
    
    drawBodyParts(genericBufferBinder, trunkMatrix, NULL);
    
    
    // Disabled all vertex attributes
    glDisableVertexAttribArray(postionAttributeFromVertexShader);
    glDisableVertexAttribArray(colorAttributeFromVertexShader);
    glDisableVertexAttribArray(normalAttributeFromVertexShader);
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
    readAndCompileShader(program, "/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/vertex.glsl", "/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/fragment.glsl");
    
    glUseProgram(program);
    
    // Shader Atrributes
    postionAttributeFromVertexShader = glGetAttribLocation(program, "position");
    colorAttributeFromVertexShader = glGetAttribLocation(program, "color");
    normalAttributeFromVertexShader = glGetAttribLocation(program, "normal");
    
    // Normal Uniforms
    uColorUniformFromFragmentShader = glGetUniformLocation(program, "uColor");
    lightPositionUniformFromFragmentShader = glGetUniformLocation(program, "lightPosition");
    
    //Matrix Uniforms
    modelViewMatrixUniformFromVertexShader = glGetUniformLocation(program, "modelViewMatrix");
    normalMatrixUniformFromVertexShader = glGetUniformLocation(program, "normalMatrix");
    projectionMatrixUniformFromVertexShader = glGetUniformLocation(program, "projectionMatrix");
    
    
    // Initialize Sphere
    //    int ibLen, vbLen;
    //    getSphereVbIbLen(12, 12, vbLen, ibLen);
    //    std::vector<VertexPN> vtx(vbLen);
    //    std::vector<unsigned short> idx(ibLen);
    //    makeSphere(1.3, 12, 12, vtx.begin(), idx.begin());
    //    numIndices = ibLen;
    
    std::vector<VertexPN> vtx;
    std::vector<unsigned short> idx;
    loadObjFile("/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/lucy.obj", vtx, idx);
    
    numIndices = idx.size();
    
    // Bind the respective vertex, color and index buffers
    glGenBuffers(1, &vertexPositionVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), idx.data(), GL_STATIC_DRAW);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
            // ------------------------------- BOT MOVEMENT -------------------------------
        case 'w':
            botZ += 0.75;
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
        case 'X':
            botXDegree -= 5.0;
            break;
        case 'y':
            botYDegree += 5.0;
            break;
        case 'Y':
            botYDegree -= 5.0;
            break;
        case 'z':
            botZDegree += 5.0;
            break;
        case 'Z':
            botZDegree -= 5.0;
            break;
        case 'v':
            botX = botY = botZ = botXDegree = botYDegree = botZDegree = 0.0;
            break;
            // ------------------------------- BOT MOVEMENT -------------------------------
            
            // ------------------------------- COLOR SHADING -------------------------------
        case 'r':
            if (redOffset <= 1.0)
                redOffset += 0.02;
            break;
        case 'R':
            if (redOffset >= 0.02)
                redOffset -= 0.02;
            break;
        case 'g':
            if (greenOffset <= 1.0)
                greenOffset += 0.02;
            break;
        case 'G':
            if (greenOffset >= 0.02)
                greenOffset -= 0.02;
            break;
        case 'b':
            if (blueOffset <= 1.0)
                blueOffset += 0.02;
            break;
        case 'B':
            if (blueOffset >= 0.02)
                blueOffset -= 0.02;
            break;
            // ------------------------------- COLOR SHADING -------------------------------
            
        case 'c':
            redOffset = 0.0;
            blueOffset = 0.0;
            greenOffset = 0.0;
            lightXOffset = -0.5773;
            lightYOffset = 0.5773;
            lightZOffset = 10.0;
            break;
        case 'C':
            redOffset = 1.0;
            blueOffset = 1.0;
            greenOffset = 1.0;
            break;
            
            // ------------------------------- FRAME SPEED -------------------------------
        case 'f':
            frameSpeed += 5.0;
            break;
        case 'F':
            if(frameSpeed > 5.0)
                frameSpeed -= 5.0;
            break;
            // ------------------------------- FRAME SPEED -------------------------------
            
            // ------------------------------- LIGHT LOCATION -------------------------------
        case 'k':
            lightZOffset += 2.0;
            break;
        case 'K':
            lightZOffset -= 2.0;
            break;
        case 'l':
            lightXOffset += 2.0;
            break;
        case 'L':
            lightXOffset -= 2.0;
            break;
        case 'i':
            lightYOffset += 2.0;
            break;
        case 'I':
            lightYOffset -= 2.0;
            break;
            // ------------------------------- LIGHT LOCATION -------------------------------
    }
}

void shiftFrame(int &x, int &y) {
    if(x >=0 && x <=250 && y >=0 && y<=250) {
        x = -abs(250-x);
        y = abs(250-y);
    } else if (x >= 250 && x <= 500 && y >=0 && y<=250) {
        x = abs(250-x);
        y = abs(250-y);
    } else if (x >=0 && x <=250 && y >=250 && y <= 500) {
        x = -abs(250-x);
        y = -abs(250-y);
    } else if (x >=250 && x <= 500 && y >= 250 && y <= 500) {
        x = abs(250-x);
        y = -abs(250-y);
    }
}

void mouse(int button, int state, int x, int y) {
    if(state == 0) {
        shiftFrame(x, y);
        initialVector = normalize(Cvec3(x, y-7.5, 30.0));
    }
}

void mouseMove(int x, int y) {
    shiftFrame(x, y);
    Cvec3 finalVector = normalize(Cvec3(x, y-7.5, 30.0));
    finalAngle = -1 * acos(dot(initialVector, finalVector)) * 57.2958;
    
    kVector = normalize(cross(initialVector, finalVector));
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(500, 500);
    glutCreateWindow("Running Bot");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);
    
    init();
    glutMainLoop();
    return 0;
}