
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

float wHeight = 1280.0, wWidth = 720.0;

GLuint vertexPositionBO,
       orbitPositionBO,
       ringPositionBO,
       oribitIndexBO,
       indexBO,
       ringIndexBO;

GLuint postionAttributeFromVertexShader,
       colorAttributeFromVertexShader,
       normalAttributeFromVertexShader,
       textureAttributeFromVertexShader,
       binormalAttributeFromVertexShader,
       tangentAttributeFromVertexShader;

GLuint lightPositionUniformFromFragmentShader0,
       lightPositionUniformFromFragmentShader1,
       lightPositionUniformFromFragmentShader2,
       lightPositionUniformFromFragmentShader3,
       lightPositionUniformFromFragmentShader4,
       lightColorUniformFromFragmentShader0,
       specularLightColorUniformFromFragmentShader0;

GLuint modelViewMatrixUniformFromVertexShader,
       normalMatrixUniformFromVertexShader,
       projectionMatrixUniformFromVertexShader;

GLuint diffuseTextureUniformLocation,
       specularTextureUniformLocation,
       normalTextureUniformLocation;

Matrix4 eyeMatrix;

Cvec3 initialVector,
      kVector;

float finalAngle, initialAngle, previousAngle = 0.0;

int numIndices,
    oribitNumIndices,
    ringNumIndices,
    timeSinceStart = 0.0;

struct TextureBinder {
    GLuint diffuseTexture;
    GLuint specularTexture;
    GLuint normalTexture;
};

TextureBinder sunTexBinder,
mercuryTexBinder,
venusTexBinder,
earthTexBinder,
marsTexBinder,
jupiterTexBinder,
saturnTexBinder,
uranusTexBinder,
neptuneTexBinder,
plutoTexBinder;

TextureBinder moonTexBinder;

TextureBinder uranusRingTexBinder,
saturnRingTexBinder;

TextureBinder orbitTexBinder;

struct CelestialBodyTexture {
    string diffuse;
    string specular;
    string normal;
};

CelestialBodyTexture celestialBodyTextures[] = {
    { "sun.jpg", "sun_SPEC.png", "sun_NRM.png" },
    
    { "mercury.jpg", "mercury_SPEC.png", "mercury_NRM.png" },
    
    { "venus.jpg", "venus_SPEC.png", "venus_NRM.png" },
    
    { "earth.jpg", "earth_SPEC.png", "earth_NRM.png" },
    
    { "mars.jpg", "mars_SPEC.png", "mars_NRM.png" },
    
    { "jupiter.jpg", "jupiter_SPEC.png", "jupiter_NRM.png" },
    
    { "saturn.jpg", "saturn_SPEC.png", "saturn_NRM.png" },
    
    { "uranus.jpg", "uranus_SPEC.png", "uranus_NRM.png" },
    
    { "neptune.jpg", "neptune_SPEC.png", "neptune_NRM.png" },
    
    { "pluto.jpg", "pluto_SPEC.png", "pluto_NRM.png" }
};

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
    GLuint textureAttribute;
    GLuint binormalAttribute;
    GLuint tangentAttribute;
    
    GLuint modelViewMatrixUniform;
    GLuint normalMatrixUniform;
    GLuint diffuseTextureUniform;
    GLuint specularTextureUniform;
    GLuint normalTextureUniform;
    
    TextureBinder texBinder;
    
    int numIndices;
    
    void draw() {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
        glVertexAttribPointer(positionAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
        glEnableVertexAttribArray(positionAttribute);
        
        glVertexAttribPointer(normalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
        glEnableVertexAttribArray(normalAttribute);
        
        glEnableVertexAttribArray(textureAttribute);
        glVertexAttribPointer(textureAttribute, 2, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, t));
        
        glEnableVertexAttribArray(binormalAttribute);
        glVertexAttribPointer(binormalAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, b));
        
        glEnableVertexAttribArray(tangentAttribute);
        glVertexAttribPointer(tangentAttribute, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, tg));
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObject);
        
        glUniform1i(diffuseTextureUniform, 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texBinder.diffuseTexture);
        
        glUniform1i(specularTextureUniform, 1);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, texBinder.specularTexture);
        
        glUniform1i(normalTextureUniform, 2);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, texBinder.normalTexture);
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

struct PlanetProperty {
    float revolutionRate;
    float intialAngle;
    Cvec3 size;
    Cvec3 radius;
    Cvec3 orbit;
    CelestialBodyTexture images;
    TextureBinder texture;
    Entity *planetEntity;
};

PlanetProperty planetProperties[] = {
    { 01.00, 00.00, Cvec3(1.80, 1.80, 1.80), Cvec3(00.0, 0.0, 0.0), Cvec3(0.0, 0.0, 0.0), celestialBodyTextures[0] },
    
    { 47.89, -80.0, Cvec3(0.20, 0.20, 0.20), Cvec3(03.0, 0.0, 0.0), Cvec3(1.0/2.0, 1.0/2.0, 1.0/2.0), celestialBodyTextures[1] },
    
    { 35.03, -70.0, Cvec3(0.30, 0.30, 0.30), Cvec3(04.5, 0.0, 0.0), Cvec3(1.0/1.35, 1.0/1.35, 1.0/1.35), celestialBodyTextures[2] },
    
    { 29.79, 45.00, Cvec3(0.50, 0.50, 0.50), Cvec3(06.0, 0.0, 0.0), Cvec3(1.0, 1.0, 1.0), celestialBodyTextures[3] },
    
    { 24.13, -30.0, Cvec3(0.40, 0.40, 0.40), Cvec3(08.0, 0.0, 0.0), Cvec3(1.35, 1.0, 1.35), celestialBodyTextures[4] },
    
    { 13.06, -40.0, Cvec3(1.00, 1.00, 1.00), Cvec3(11.0, 0.0, 0.0), Cvec3(1.85, 1.0, 1.85), celestialBodyTextures[5] },
    
    { 09.64, -45.0, Cvec3(0.80, 0.80, 0.80), Cvec3(14.5, 0.0, 0.0), Cvec3(2.40, 1.0, 2.40), celestialBodyTextures[6] },
    
    { 06.81, 45.00, Cvec3(0.30, 0.30, 0.30), Cvec3(17.0, 0.0, 0.0), Cvec3(2.825, 1.0, 2.825), celestialBodyTextures[7] },
    
    { 05.43, 60.00, Cvec3(0.25, 0.25, 0.25), Cvec3(18.0, 0.0, 0.0), Cvec3(3.0, 1.0, 3.0), celestialBodyTextures[8] },
    
    { 04.00, 70.00, Cvec3(0.15, 0.15, 0.15), Cvec3(19.0, 0.0, 0.0), Cvec3(3.165, 1.0, 3.165), celestialBodyTextures[9] }
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

void calculateFaceTangent(const Cvec3f &v1, const Cvec3f &v2, const Cvec3f &v3, const Cvec2f &texCoord1, const Cvec2f &texCoord2, const Cvec2f &texCoord3, Cvec3f &tangent, Cvec3f &binormal) {
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
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    timeSinceStart = glutGet(GLUT_ELAPSED_TIME);
    
    Cvec4 lightPosition0 = inv(eyeMatrix) * Cvec4(-8.0, 0.0, 8.0, 1.0),
          lightPosition1 = inv(eyeMatrix) * Cvec4(8.0, 0.0, 8.0, 1.0),
          lightPosition2 = inv(eyeMatrix) * Cvec4(8.0, 0.0, -8.0, 1.0),
          lightPosition3 = inv(eyeMatrix) * Cvec4(-8.0, 0.0, -8.0, 1.0),
          lightPosition4 = inv(eyeMatrix) * Cvec4(0.0, 8.0, 0.0, 1.0);
    
    glUniform3f(lightPositionUniformFromFragmentShader0, lightPosition0[0], lightPosition0[1], lightPosition0[2]);
    glUniform3f(lightPositionUniformFromFragmentShader1, lightPosition1[0], lightPosition1[1], lightPosition1[2]);
    glUniform3f(lightPositionUniformFromFragmentShader2, lightPosition2[0], lightPosition2[1], lightPosition2[2]);
    glUniform3f(lightPositionUniformFromFragmentShader3, lightPosition3[0], lightPosition3[1], lightPosition3[2]);
    glUniform3f(lightPositionUniformFromFragmentShader4, lightPosition4[0], lightPosition4[1], lightPosition4[2]);
    
    glUniform3f(lightColorUniformFromFragmentShader0, 1.0, 1.0, 1.0);
    glUniform3f(specularLightColorUniformFromFragmentShader0, 1.0, 1.0, 1.0);

    
    // ------------------------------- EYE -------------------------------
    eyeMatrix = quatToMatrix(Quat::makeXRotation(-20.0)) *
                quatToMatrix(Quat::makeKRotation(kVector, finalAngle)) *
                Matrix4::makeTranslation(Cvec3(0.0, -2.5, 35.0)) *
                Matrix4::makeZRotation(-25.0);
    // ------------------------------- EYE -------------------------------
    
    // Initialising a Genric bufferBinder object as the same buffers are used to
    // render all the objects in hierarchy
    BufferBinder genericBufferBinder;
    genericBufferBinder.vertexBufferObject = vertexPositionBO;
    genericBufferBinder.indexBufferObject = indexBO;
    genericBufferBinder.numIndices = numIndices;
    genericBufferBinder.positionAttribute = postionAttributeFromVertexShader;
    genericBufferBinder.normalAttribute = normalAttributeFromVertexShader;
    genericBufferBinder.textureAttribute = textureAttributeFromVertexShader;
    genericBufferBinder.binormalAttribute = binormalAttributeFromVertexShader;
    genericBufferBinder.tangentAttribute = tangentAttributeFromVertexShader;
    
    genericBufferBinder.diffuseTextureUniform = diffuseTextureUniformLocation;
    genericBufferBinder.specularTextureUniform = specularTextureUniformLocation;
    genericBufferBinder.normalTextureUniform = normalTextureUniformLocation;
    
    Matrix4 planetMatrix;
    for(int i=0; i<=9; i++) {
        genericBufferBinder.texBinder = planetProperties[i].texture;
        planetMatrix = quatToMatrix(Quat::makeYRotation(timeSinceStart * planetProperties[i].revolutionRate/500.0)) *
                       Matrix4::makeTranslation(planetProperties[i].radius) *
                       Matrix4::makeScale(planetProperties[i].size) *
                       quatToMatrix(Quat::makeXRotation(planetProperties[i].intialAngle)) *
                       quatToMatrix(Quat::makeYRotation(timeSinceStart/10.0));
        planetProperties[i].planetEntity = drawBodyParts(genericBufferBinder, planetMatrix, NULL);
    }
    
    genericBufferBinder.texBinder = moonTexBinder;
    Matrix4 moonMatrix = quatToMatrix(Quat::makeYRotation((timeSinceStart * 25.00)/500.0f)) *
                         Matrix4::makeTranslation(Cvec3(2.0, 0.0, 0.0)) *
                         Matrix4::makeScale(Cvec3(0.4, 0.4, 0.4)) *
                         quatToMatrix(Quat::makeYRotation(timeSinceStart/10.0f));
    
    drawBodyParts(genericBufferBinder, moonMatrix, planetProperties[3].planetEntity);
    
    genericBufferBinder.vertexBufferObject = orbitPositionBO;
    genericBufferBinder.indexBufferObject = oribitIndexBO;
    genericBufferBinder.numIndices = oribitNumIndices;
    
    genericBufferBinder.texBinder = orbitTexBinder;
    for(int i=1; i<=9; i++) {
        Matrix4 planetOrbit = Matrix4::makeScale(planetProperties[i].orbit);
        drawBodyParts(genericBufferBinder, planetOrbit, NULL);
    }
    
    genericBufferBinder.vertexBufferObject = ringPositionBO;
    genericBufferBinder.indexBufferObject = ringIndexBO;
    genericBufferBinder.numIndices = ringNumIndices;
    
    genericBufferBinder.texBinder = saturnRingTexBinder;
    Matrix4 saturnRing = Matrix4::makeScale(Cvec3(2.0, 2.0, 2.0));
    drawBodyParts(genericBufferBinder, saturnRing, planetProperties[6].planetEntity);
    
    genericBufferBinder.texBinder = uranusRingTexBinder;
    Matrix4 uranusRing = Matrix4::makeScale(Cvec3(2.0, 2.0, 2.0));
    drawBodyParts(genericBufferBinder, uranusRing, planetProperties[7].planetEntity);
    
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
    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    
    glUseProgram(program);
    
    // Shader Atrributes
    postionAttributeFromVertexShader = glGetAttribLocation(program, "position");
    colorAttributeFromVertexShader = glGetAttribLocation(program, "color");
    normalAttributeFromVertexShader = glGetAttribLocation(program, "normal");
    textureAttributeFromVertexShader = glGetAttribLocation(program, "texCoord");
    binormalAttributeFromVertexShader = glGetAttribLocation(program, "binormal");
    tangentAttributeFromVertexShader = glGetAttribLocation(program, "tangent");
    
    diffuseTextureUniformLocation = glGetUniformLocation(program, "diffuseTexture");
    specularTextureUniformLocation = glGetUniformLocation(program, "specularTexture");
    normalTextureUniformLocation = glGetUniformLocation(program, "normalTexture");
    
    lightPositionUniformFromFragmentShader0 = glGetUniformLocation(program, "lights[0].lightPosition");
    lightPositionUniformFromFragmentShader1 = glGetUniformLocation(program, "lights[1].lightPosition");
    lightPositionUniformFromFragmentShader2 = glGetUniformLocation(program, "lights[2].lightPosition");
    lightPositionUniformFromFragmentShader3 = glGetUniformLocation(program, "lights[3].lightPosition");
    lightPositionUniformFromFragmentShader4 = glGetUniformLocation(program, "lights[4].lightPosition");
    
    lightColorUniformFromFragmentShader0 = glGetUniformLocation(program, "lights[0].lightColor");
    specularLightColorUniformFromFragmentShader0 = glGetUniformLocation(program, "lights[0].specularLightColor");
    
    //Matrix Uniforms
    modelViewMatrixUniformFromVertexShader = glGetUniformLocation(program, "modelViewMatrix");
    normalMatrixUniformFromVertexShader = glGetUniformLocation(program, "normalMatrix");
    projectionMatrixUniformFromVertexShader = glGetUniformLocation(program, "projectionMatrix");
    
    for(int i=0; i<=9; i++) {
        planetProperties[i].texture.diffuseTexture = loadGLTexture(planetProperties[i].images.diffuse.c_str());
        planetProperties[i].texture.specularTexture = loadGLTexture(planetProperties[i].images.specular.c_str());
        planetProperties[i].texture.normalTexture = loadGLTexture(planetProperties[i].images.normal.c_str());
    }
    
    saturnRingTexBinder.diffuseTexture = loadGLTexture("saturnring.jpg");
    
    uranusRingTexBinder.diffuseTexture = loadGLTexture("uranusringcolour.jpg");
    
    moonTexBinder.diffuseTexture = loadGLTexture("moon.jpg");
    
    orbitTexBinder.diffuseTexture = loadGLTexture("orbit_texture.jpg");
    
    int ibLen, vbLen;
    getSphereVbIbLen(20, 20, vbLen, ibLen);
    std::vector<VertexPN> vtx(vbLen);
    std::vector<unsigned short> idx(ibLen);
    makeSphere(1.3, 20, 20, vtx.begin(), idx.begin());
    numIndices = ibLen;
    
    // Bind the respective vertex, color and index buffers
    glGenBuffers(1, &vertexPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexPositionBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx.size(), vtx.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &indexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx.size(), idx.data(), GL_STATIC_DRAW);
    
    std::vector<VertexPN> vtx1;
    std::vector<unsigned short> idx1;
    loadObjFile("torus_final.obj", vtx1, idx1);
    
    glGenBuffers(1, &orbitPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, orbitPositionBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx1.size(), vtx1.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &oribitIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, oribitIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx1.size(), idx1.data(), GL_STATIC_DRAW);
    oribitNumIndices = (int)idx1.size();
    
    std::vector<VertexPN> vtx2;
    std::vector<unsigned short> idx2;
    loadObjFile("planet_ring.obj", vtx2, idx2);
    
    glGenBuffers(1, &ringPositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, ringPositionBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx2.size(), vtx2.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &ringIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ringIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx2.size(), idx2.data(), GL_STATIC_DRAW);
    ringNumIndices = (int)idx2.size();
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

void shiftFrame(int &x, int &y) {
    float xShiftHalf = wHeight/2.0, yShiftHalf = wWidth/2.0;
    if(x >=0 && x <=xShiftHalf && y >=0 && y<=yShiftHalf) {
        x = -abs(xShiftHalf-x);
        y = abs(yShiftHalf-y);
    } else if (x >= xShiftHalf && x <= wWidth && y >=0 && y<=yShiftHalf) {
        x = abs(xShiftHalf-x);
        y = abs(yShiftHalf-y);
    } else if (x >=0 && x <=xShiftHalf && y >=yShiftHalf && y <= wHeight) {
        x = -abs(xShiftHalf-x);
        y = -abs(yShiftHalf-y);
    } else if (x >=xShiftHalf && x <= wWidth && y >= yShiftHalf && y <= wHeight) {
        x = abs(xShiftHalf-x);
        y = -abs(yShiftHalf-y);
    }
}

void mouse(int button, int state, int x, int y) {
    if(state == 0) {
        shiftFrame(x, y);
        initialVector = normalize(Cvec3(x, y, 30.0));
    }
    if(state == 1) {
//        previousAngle = finalAngle;
    }
    
}

void mouseMove(int x, int y) {
    shiftFrame(x, y);
    Cvec3 finalVector = normalize(Cvec3(x, y, 30.0));
    finalAngle = -1 * acos(dot(initialVector, finalVector)) * 57.2958 + previousAngle;
    
    Cvec3 crossProduct = cross(initialVector, finalVector);
    if (crossProduct[0] != 0 && crossProduct[1] != 0 && crossProduct[2] != 0)
        kVector = normalize(crossProduct);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(wHeight, wWidth);
    glutCreateWindow("Solar System");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    glutMouseFunc(mouse);
    glutMotionFunc(mouseMove);
    
    init();
    glutMainLoop();
    return 0;
}