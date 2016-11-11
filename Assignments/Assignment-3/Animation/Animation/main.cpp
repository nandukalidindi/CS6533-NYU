
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
GLuint textureAttributeFromVertexShader;
GLuint binormalAttributeFromVertexShader;
GLuint tangentAttributeFromVertexShader;

GLuint lightPositionUniformFromFragmentShader0;
GLuint lightPositionUniformFromFragmentShader1;

GLuint uColorUniformFromFragmentShader;
GLuint lightPositionUniformFromFragmentShader;
GLuint lightColorUniformFromFragmentShader;
GLuint specularLightColorUniformFromFragmentShader;

GLuint modelViewMatrixUniformFromVertexShader;
GLuint normalMatrixUniformFromVertexShader;
GLuint projectionMatrixUniformFromVertexShader;


GLuint diffuseTexture;
GLuint specularTexture;
GLuint normalTexture;

GLuint diffuseTextureUniformLocation;
GLuint specularTextureUniformLocation;
GLuint normalTextureUniformLocation;

Matrix4 eyeMatrix;

Cvec3 initialVector;
Cvec3 kVector;
float finalAngle;

float frameSpeed = 10.0f;
float lightXOffset = -0.5773, lightYOffset = 0.5773, lightZOffset = 0.5773;
float redOffset = 1.0, blueOffset = 1.0, greenOffset = 1.0;
float botX = 0.0, botY = 0.0, botZ = 0.0;
float botXDegree = 0.0, botYDegree = 0.0, botZDegree = 0.0;
int numIndices, timeSinceStart = 0.0;

struct VertexPN {
    Cvec3f p;
    Cvec3f n;
    Cvec2f t;
    Cvec3f b, tg;
    VertexPN() {}
    VertexPN(float x, float y, float z, float nx, float ny, float nz) : p(x,y,z), n(nx, ny, nz) {}
    
    VertexPN& operator = (const GenericVertex& v) {
        p = v.pos;
        n = v.normal;
        t = v.tex;
        b = v.binormal;
        tg = v.tangent;
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
        
        glEnableVertexAttribArray(textureAttributeFromVertexShader);
        glVertexAttribPointer(textureAttributeFromVertexShader, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, t));
        glEnableVertexAttribArray(textureAttributeFromVertexShader);
        
        glEnableVertexAttribArray(binormalAttributeFromVertexShader);
        glVertexAttribPointer(binormalAttributeFromVertexShader, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, b));
        glEnableVertexAttribArray(binormalAttributeFromVertexShader);
        
        glEnableVertexAttribArray(tangentAttributeFromVertexShader);
        glVertexAttribPointer(tangentAttributeFromVertexShader, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, tg));
        glEnableVertexAttribArray(tangentAttributeFromVertexShader);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        
        glUniform1i(diffuseTextureUniformLocation, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, diffuseTexture);
        
        glUniform1i(specularTextureUniformLocation, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, specularTexture);
        
        glUniform1i(specularTextureUniformLocation, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, normalTexture);
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
        
        Matrix4 normalizedMatrix = normalMatrix(modelViewMatrix);
        normalizedMatrix.writeToColumnMajorMatrix(glmatrix);
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

void calculateFaceTangent(const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, const Cvec2f &texCoord1, const Cvec2f &texCoord2,
                          const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal) {
    Cvec3f side0 = v1 - v2;
    Cvec3f side1 = v3 - v1;
    Cvec3f normal = cross(side1, side0);
    normalize(normal);
    float deltaV0 = texCoord1[1] - texCoord2[1];
    float deltaV1 = texCoord3[1] - texCoord1[1];
    tangent = side0 * deltaV1 - side1 * deltaV0;
    normalize(tangent);
    float deltaU0 = texCoord1[0] - texCoord2[0];
    float deltaU1 = texCoord3[0] - texCoord1[0];
    binormal = side0 * deltaU1 - side1 * deltaU0;
    normalize(binormal);
    Cvec3f tangentCross = cross(tangent, binormal);
    if (dot(tangentCross, normal) < 0.0f) {
        tangent = tangent * -1;
    }
}

void loadObjFile(const std::string &fileName, std::vector<VertexPN> &outVertices, std::vector<unsigned short> &outIndices) {
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
    std::string err;
    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &err, fileName.c_str(), NULL, true);
    if(ret) {
        for(int i=0; i < shapes.size(); i++) {
            for(int j=0; j < shapes[i].mesh.indices.size(); j++) {
                unsigned int vertexOffset = shapes[i].mesh.indices[j].vertex_index * 3;
                unsigned int normalOffset = shapes[i].mesh.indices[j].normal_index * 3;
                unsigned int texOffset = shapes[i].mesh.indices[j].texcoord_index * 2;
                VertexPN v;
                v.p[0] = attrib.vertices[vertexOffset];
                v.p[1] = attrib.vertices[vertexOffset+1];
                v.p[2] = attrib.vertices[vertexOffset+2];
                v.n[0] = attrib.normals[normalOffset];
                v.n[1] = attrib.normals[normalOffset+1];
                v.n[2] = attrib.normals[normalOffset+2];
                v.t[0] = attrib.texcoords[texOffset];
                v.t[1] = 1.0-attrib.texcoords[texOffset+1];
                outVertices.push_back(v);
                outIndices.push_back(outVertices.size()-1);
            }
        }
        
        for(int i=0; i < outVertices.size(); i += 3) {
            Cvec3f tangent;
            Cvec3f binormal;
            calculateFaceTangent(outVertices[i].p, outVertices[i+1].p, outVertices[i+2].p,
                                 outVertices[i].t, outVertices[i+1].t, outVertices[i+2].t, tangent, binormal);
            outVertices[i].tg = tangent;
            outVertices[i+1].tg = tangent;
            outVertices[i+2].tg = tangent;
            outVertices[i].b = binormal;
            outVertices[i+1].b = binormal;
            outVertices[i+2].b = binormal;
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
    //    glUniform3f(lightPositionUniformFromFragmentShader, lightXOffset, lightYOffset, lightZOffset);
    glUniform3f(uColorUniformFromFragmentShader, redOffset, greenOffset, blueOffset);
    //    glUniform3f(lightColorUniformFromFragmentShader, 1.0, 0.0, 0.0);
    //    glUniform3f(specularLightColorUniformFromFragmentShader, 1.0, 0.0, 0.0);
    
//    glUniform3f(lightPositionUniformFromFragmentShader0, 3.0*sin(timeSinceStart/1000.0f), 3.0*cos(timeSinceStart/1000.0f), 0.0);
//    glUniform3f(lightPositionUniformFromFragmentShader1, 3.0*sin(timeSinceStart/1000.0f), 3.0*cos(timeSinceStart/1000.0f), 0.0);
    
    glUniform3f(lightPositionUniformFromFragmentShader0, 1.5, 1.5, 0.5);
    glUniform3f(lightPositionUniformFromFragmentShader1, -1.5, 1.5, 0.5);

    
    // ------------------------------- EYE -------------------------------
    //    eyeMatrix = quatToMatrix(Quat::makeYRotation(40.0)) *
    //                quatToMatrix(Quat::makeYRotation(botYDegree)) *
    //                quatToMatrix(Quat::makeXRotation(botXDegree)) *
    //                quatToMatrix(Quat::makeZRotation(botZDegree));
    eyeMatrix = quatToMatrix(Quat::makeKRotation(kVector, finalAngle)) *
    //    quatToMatrix(Quat::makeYRotation(timeSinceStart/50.0)) *
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
    
    Matrix4 invShiftMatrix = Matrix4::makeTranslation(Cvec3(0.0, 20.0, 0.0));
    
    Matrix4 trunkMatrix =  /*Matrix4::makeTranslation(Cvec3(5*sin(timeSinceStart/1000.0f), 10 * sin(timeSinceStart/1000.0f), 0.0)) */
    //                          quatToMatrix(slerp(quat1, quat2, sin(timeSinceStart/1000.0f))) *
    Matrix4::makeScale(Cvec3(1.5, 1.5, 1.5));
    //    Matrix4::makeScale(Cvec3(80.0, 80.0, 80.0));
    
    trunkMatrix = invShiftMatrix * trunkMatrix * inv(invShiftMatrix);
    
    drawBodyParts(genericBufferBinder, trunkMatrix, NULL);
    
    
    // Disabled all vertex attributes
    glDisableVertexAttribArray(postionAttributeFromVertexShader);
    glDisableVertexAttribArray(colorAttributeFromVertexShader);
    glDisableVertexAttribArray(normalAttributeFromVertexShader);
    glDisableVertexAttribArray(textureAttributeFromVertexShader);
    glDisableVertexAttribArray(binormalAttributeFromVertexShader);
    glDisableVertexAttribArray(tangentAttributeFromVertexShader);
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
    textureAttributeFromVertexShader = glGetAttribLocation(program, "texCoord");
    binormalAttributeFromVertexShader = glGetAttribLocation(program, "binormal");
    tangentAttributeFromVertexShader = glGetAttribLocation(program, "tangent");
    
    // Normal Uniforms
    uColorUniformFromFragmentShader = glGetUniformLocation(program, "uColor");
    //    lightPositionUniformFromFragmentShader = glGetUniformLocation(program, "lightPosition");
    //    lightColorUniformFromFragmentShader = glGetUniformLocation(program, "lightColor");
    //    specularLightColorUniformFromFragmentShader = glGetUniformLocation(program, "specularLightColor");
    
    diffuseTexture = loadGLTexture("/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/monk_texture.tga");
    diffuseTextureUniformLocation = glGetUniformLocation(program, "diffuseTexture");
    
    specularTexture = loadGLTexture("/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/monk_specular1.tga");
    specularTextureUniformLocation = glGetUniformLocation(program, "specularTexture");
    
    normalTexture = loadGLTexture("/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/monk_normal.tga");
    normalTextureUniformLocation = glGetUniformLocation(program, "normalTexture");
    
    lightPositionUniformFromFragmentShader0 = glGetUniformLocation(program, "lights[0].lightPosition");
    lightPositionUniformFromFragmentShader1 = glGetUniformLocation(program, "lights[1].lightPosition");
    
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
    loadObjFile("/Users/kaybus/Documents/nandukalidindi-github/CS6533-NYU/Assignments/Assignment-3/Animation/Animation/monk.obj", vtx, idx);
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