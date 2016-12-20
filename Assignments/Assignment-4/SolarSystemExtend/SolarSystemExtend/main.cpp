#include <glut.h>
#include "glsupport.h"
#include "matrix4.h"
#include "geometrymaker.h"
#include <vector>
#include <math.h>
#include "quat.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "planet_properties.h"

GLuint program;

bool isBlackAndWhite = false,
isInvertColor = false,
isBlackAndWhiteAndInvertColor = false,
isFxaa = false,
isGlare = false,
isHDR = false,
isBlur = false,
isBlurWithInvertColors = false;


struct postProcessor {
    GLuint program,
           framebufferUniform,
           positionBuffer,
           positionAttribute,
           uvBuffer,
           texCoordAttribute,
           frameBuffer,
           frameBufferTexture,
           depthBufferTexture;
};

GLuint horizontalBlurProgram,
       horizontalBlurFramebufferUniform,
       horizontalBlurPositionBuffer,
       horizontalBlurPositionAttribute,
       horizontalBlurUVBuffer,
       horizontalBlurTexCoordAttribute,
       horizontalBlurFrameBuffer,
       horizontalBlurFrameBufferTexture,
       horizontalBlurDepthBufferTexture;

GLuint verticalBlurProgram,
       verticalBlurFramebufferUniform,
       verticalBlurPositionBuffer,
       verticalBlurPositionAttribute,
       verticalBlurUVBuffer,
       verticalBlurTexCoordAttribute,
       verticalBlurFrameBuffer,
       verticalBlurFrameBufferTexture,
       verticalBlurDepthBufferTexture;

GLuint luminanceClampProgram,
       luminanceClampFramebufferUniform,
       luminanceClampPositionBuffer,
       luminanceClampPositionAttribute,
       luminanceClampUVBuffer,
       luminanceClampTexCoordAttribute,
       luminanceClampFrameBuffer,
       luminanceClampFrameBufferTexture,
       luminanceClampDepthBufferTexture;

GLuint finalAdditiveProgram,
       finalAdditiveFramebufferUniform,
       finalAdditivePositionBuffer,
       finalAdditivePositionAttribute,
       finalAdditiveUVBuffer,
       finalAdditiveTexCoordAttribute,
       finalAdditiveFrameBuffer,
       finalAdditiveFrameBufferTexture,
       finalAdditiveDepthBufferTexture;

GLuint invertColorProgram,
       invertColorFramebufferUniform,
       invertColorPositionBuffer,
       invertColorPositionAttribute,
       invertColorUVBuffer,
       invertColorTexCoordAttribute,
       invertColorFrameBuffer,
       invertColorFrameBufferTexture,
       invertColorDepthBufferTexture;

GLuint blackAndWhiteProgram,
       blackAndWhiteFramebufferUniform,
       blackAndWhitePositionBuffer,
       blackAndWhitePositionAttribute,
       blackAndWhiteUVBuffer,
       blackAndWhiteTexCoordAttribute,
       blackAndWhiteFrameBuffer,
       blackAndWhiteFrameBufferTexture,
       blackAndWhiteDepthBufferTexture;

GLuint fxaaProgram,
       fxaaFramebufferUniform,
       fxaaPositionBuffer,
       fxaaPositionAttribute,
       fxaaUVBuffer,
       fxaaTexCoordAttribute,
       fxaaFrameBuffer,
       fxaaFrameBufferTexture,
       fxaaDepthBufferTexture;

GLuint hdrProgram,
       hdrFramebufferUniform,
       hdrPositionBuffer,
       hdrPositionAttribute,
       hdrUVBuffer,
       hdrTexCoordAttribute,
       hdrFrameBuffer,
       hdrFrameBufferTexture,
       hdrDepthBufferTexture;

GLuint environmentMapProgram;
GLuint cubeMap;

GLuint vertexPositionBO,
       orbitPositionBO,
       ringPositionBO,
       oribitIndexBO,
       indexBO,
       ringIndexBO,
       cubePositionBO,
       cubeIndexBO;

GLuint postionAttributeFromVertexShader,
       colorAttributeFromVertexShader,
       normalAttributeFromVertexShader,
       textureAttributeFromVertexShader,
       binormalAttributeFromVertexShader,
       tangentAttributeFromVertexShader;

GLuint postionAttributeFromVertexShaderE,
       normalAttributeFromVertexShaderE,
       modelViewMatrixUniformFromVertexShaderE,
       normalMatrixUniformFromVertexShaderE,
       projectionMatrixUniformFromVertexShaderE;

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
       normalTextureUniformLocation,
       environmentMapUniformLocation;

Matrix4 eyeMatrix;

Cvec3 initialVector,
      kVector;

float finalAngle, initialAngle, previousAngle = 0.0;

int numIndices,
    oribitNumIndices,
    ringNumIndices,
    cubeNumIndices,
    timeSinceStart = 0.0;

TextureBinder moonTexBinder;

TextureBinder uranusRingTexBinder,
saturnRingTexBinder;

TextureBinder orbitTexBinder;

TextureBinder environmentTexBinder;

Entity *drawBodyParts(BufferBinder bufferBinder, Matrix4 objectMatrix, Entity *parent) {
    Entity *partEntity = new Entity;
    partEntity->parent = parent;
    partEntity->bufferBinder = bufferBinder;
    partEntity->objectMatrix = objectMatrix;
    partEntity->draw(eyeMatrix);
    return partEntity;
}

void generateFrameBuffer(GLuint &frameBuffer, GLuint &frameBufferTexture, GLuint &depthBufferTexture, bool isHDR) {
    glGenFramebuffers(1, &frameBuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    
    glGenTextures(1, &frameBufferTexture);
    glBindTexture(GL_TEXTURE_2D, frameBufferTexture);
    
    if (isHDR)
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGBA16F_ARB, wHeight, wWidth, 0, GL_RGB, GL_FLOAT, NULL);
    else
        glTexImage2D(GL_TEXTURE_2D, 0,GL_RGB, wHeight, wWidth, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, frameBufferTexture, 0);
    
    
    glGenTextures(1, &depthBufferTexture);
    glBindTexture(GL_TEXTURE_2D, depthBufferTexture);
    glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT, wHeight, wWidth, 0,GL_DEPTH_COMPONENT,
                 GL_UNSIGNED_BYTE, NULL);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, wHeight, wWidth);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D,
                           depthBufferTexture, 0);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderSceneUsingFramebuffer(GLuint &shaderProgram, GLuint &frameBufferUniform, GLuint positionBuffer, GLuint uvBuffer, vector<GLuint> &frameBufferTexture, GLuint positionAttribute, GLuint texCoordAttribute, int textureCount) {
    vector<GLuint> textureArray = {GL_TEXTURE0, GL_TEXTURE1, GL_TEXTURE2};
    
    glUseProgram(shaderProgram);
    
    for (int i = 0; i< textureCount; i++) {
        glUniform1i(frameBufferUniform, i);
        glActiveTexture(textureArray[i]);
        glBindTexture(GL_TEXTURE_2D, frameBufferTexture[i]);
    }
    
    
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    glVertexAttribPointer(positionAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(positionAttribute);
    
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    glVertexAttribPointer(texCoordAttribute, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(texCoordAttribute);
    
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(positionAttribute);
    glDisableVertexAttribArray(texCoordAttribute);
}

void renderSkybox() {
    glUseProgram(environmentMapProgram);
    
    glBindBuffer(GL_ARRAY_BUFFER, cubePositionBO);
    glVertexAttribPointer(postionAttributeFromVertexShaderE, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, p));
    
    glEnableVertexAttribArray(postionAttributeFromVertexShaderE);
    
    glVertexAttribPointer(normalAttributeFromVertexShaderE, 3, GL_FLOAT, GL_FALSE, sizeof(VertexPN), (void*)offsetof(VertexPN, n));
    glEnableVertexAttribArray(normalAttributeFromVertexShaderE);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBO);
    
    glUniform1i(environmentMapUniformLocation, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);
    
    Matrix4 environmentMatrix = Matrix4::makeScale(Cvec3(50.0, 50.0, 50.0));
    Matrix4 modelViewMatrix = inv(eyeMatrix) * environmentMatrix;
    
    GLfloat glmatrix[16];
    modelViewMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(modelViewMatrixUniformFromVertexShaderE, 1, GL_FALSE, glmatrix);
    
    Matrix4 normalizedMatrix = normalMatrix(modelViewMatrix);
    normalizedMatrix.writeToColumnMajorMatrix(glmatrix);
    glUniformMatrix4fv(normalMatrixUniformFromVertexShaderE, 1, GL_FALSE, glmatrix);
    
    Matrix4 projectionMatrix;
    projectionMatrix = projectionMatrix.makeProjection(45, (wHeight/800.0), -0.5, -1000.0);
    GLfloat glmatrixProjection[16];
    projectionMatrix.writeToColumnMajorMatrix(glmatrixProjection);
    glUniformMatrix4fv(projectionMatrixUniformFromVertexShaderE, 1, GL_FALSE, glmatrixProjection);
    
    glDrawElements(GL_TRIANGLES, cubeNumIndices, GL_UNSIGNED_SHORT, 0);
    
    glDisableVertexAttribArray(postionAttributeFromVertexShaderE);
    glDisableVertexAttribArray(normalAttributeFromVertexShaderE);
}


void renderScene() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.0, 0.0, 0.0, 0.0);
    
    glUseProgram(program);
    
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
    Matrix4::makeTranslation(Cvec3(0.0, 0.0, 35.0)) *
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
    
    genericBufferBinder.modelViewMatrixUniform = modelViewMatrixUniformFromVertexShader;
    genericBufferBinder.normalMatrixUniform = normalMatrixUniformFromVertexShader;
    genericBufferBinder.projectionMatrixUniform = projectionMatrixUniformFromVertexShader;
    
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
    
    glDisableVertexAttribArray(postionAttributeFromVertexShader);
    glDisableVertexAttribArray(colorAttributeFromVertexShader);
    glDisableVertexAttribArray(normalAttributeFromVertexShader);
    glDisableVertexAttribArray(textureAttributeFromVertexShader);
    glDisableVertexAttribArray(binormalAttributeFromVertexShader);
    glDisableVertexAttribArray(tangentAttributeFromVertexShader);
}

void glare() {
    //******************************************** BIND FRAMEBUFFER A ***********************************************
    generateFrameBuffer(luminanceClampFrameBuffer, luminanceClampFrameBufferTexture, luminanceClampDepthBufferTexture, true);
    
    glBindFramebuffer(GL_FRAMEBUFFER, luminanceClampFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    //******************************************** SOLAR SYSTEM SCENE ***********************************************
    renderScene();
    
    //*********************************************** UNIVERSE SKYBOX ***********************************************
    renderSkybox();
    
    //******************************************** BIND FRAMEBUFFER B ***********************************************
    generateFrameBuffer(horizontalBlurFrameBuffer, horizontalBlurFrameBufferTexture, horizontalBlurDepthBufferTexture, false);
    
    glBindFramebuffer(GL_FRAMEBUFFER, horizontalBlurFrameBuffer);
    
    //******************************************** RENDER A USING CLAMP *********************************************
    vector<GLuint> luminanceTexture = { luminanceClampFrameBufferTexture };
    renderSceneUsingFramebuffer(luminanceClampProgram, luminanceClampFramebufferUniform, luminanceClampPositionBuffer, luminanceClampUVBuffer, luminanceTexture, luminanceClampPositionAttribute, luminanceClampTexCoordAttribute, 1);

    //******************************************** BIND FRAMEBUFFER C ***********************************************
    generateFrameBuffer(verticalBlurFrameBuffer, verticalBlurFrameBufferTexture, verticalBlurDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, verticalBlurFrameBuffer);

    //******************************************** RENDER B USING HBLUR *********************************************
    vector<GLuint> horizontalTexture = { horizontalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(horizontalBlurProgram, horizontalBlurFramebufferUniform, horizontalBlurPositionBuffer, horizontalBlurUVBuffer, horizontalTexture, horizontalBlurPositionAttribute, horizontalBlurTexCoordAttribute, 1);
    
    //******************************************** BIND FRAMEBUFFER B ***********************************************
    generateFrameBuffer(horizontalBlurFrameBuffer, horizontalBlurFrameBufferTexture, horizontalBlurDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, horizontalBlurFrameBuffer);
    
    //******************************************** RENDER C USING VBLUR *********************************************
    vector<GLuint> verticalTexture = { verticalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(verticalBlurProgram, verticalBlurFramebufferUniform, verticalBlurPositionBuffer, verticalBlurUVBuffer, verticalTexture, verticalBlurPositionAttribute, verticalBlurTexCoordAttribute, 1);
    
    //************************************ RENDER A AND B USING ADDITIVE ********************************************
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> finalTextures = { luminanceClampFrameBufferTexture, horizontalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(finalAdditiveProgram, finalAdditiveFramebufferUniform, finalAdditivePositionBuffer, finalAdditiveUVBuffer, finalTextures, finalAdditivePositionAttribute, finalAdditiveTexCoordAttribute, 2);
    
    glutSwapBuffers();
    
}

void blur() {
    generateFrameBuffer(horizontalBlurFrameBuffer, horizontalBlurFrameBufferTexture, horizontalBlurDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, horizontalBlurFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    generateFrameBuffer(verticalBlurFrameBuffer, verticalBlurFrameBufferTexture, verticalBlurDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, verticalBlurFrameBuffer);
    
    vector<GLuint> horizontalTexture = { horizontalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(horizontalBlurProgram, horizontalBlurFramebufferUniform, horizontalBlurPositionBuffer, horizontalBlurUVBuffer, horizontalTexture, horizontalBlurPositionAttribute, horizontalBlurTexCoordAttribute, 1);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> verticalTexture = { verticalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(verticalBlurProgram, verticalBlurFramebufferUniform, verticalBlurPositionBuffer, verticalBlurUVBuffer, verticalTexture, verticalBlurPositionAttribute, verticalBlurTexCoordAttribute, 1);
    
    glutSwapBuffers();
}

void invertColor() {
    generateFrameBuffer(invertColorFrameBuffer, invertColorFrameBufferTexture, invertColorDepthBufferTexture, false);
    
    glBindFramebuffer(GL_FRAMEBUFFER, invertColorFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> invertColorTexture = { invertColorFrameBufferTexture };
    renderSceneUsingFramebuffer(invertColorProgram, invertColorFramebufferUniform, invertColorPositionBuffer, invertColorUVBuffer, invertColorTexture, invertColorPositionAttribute, invertColorTexCoordAttribute, 1);
    
    glutSwapBuffers();
}

void blackAndWhite() {
    generateFrameBuffer(blackAndWhiteFrameBuffer, blackAndWhiteFrameBufferTexture, blackAndWhiteDepthBufferTexture, false);
    
    glBindFramebuffer(GL_FRAMEBUFFER, blackAndWhiteFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> blackAndWhiteTexture = { blackAndWhiteFrameBufferTexture };
    renderSceneUsingFramebuffer(blackAndWhiteProgram, blackAndWhiteFramebufferUniform, blackAndWhitePositionBuffer, blackAndWhiteUVBuffer, blackAndWhiteTexture, blackAndWhitePositionAttribute, blackAndWhiteTexCoordAttribute, 1);
    
    glutSwapBuffers();
}

void blackAndWhiteWithInvertColor() {
    generateFrameBuffer(blackAndWhiteFrameBuffer, blackAndWhiteFrameBufferTexture, blackAndWhiteDepthBufferTexture, false);
    
    glBindFramebuffer(GL_FRAMEBUFFER, blackAndWhiteFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    generateFrameBuffer(invertColorFrameBuffer, invertColorFrameBufferTexture, invertColorDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, invertColorFrameBuffer);
    
    vector<GLuint> blackAndWhiteTexture = { blackAndWhiteFrameBufferTexture };
    renderSceneUsingFramebuffer(blackAndWhiteProgram, blackAndWhiteFramebufferUniform, blackAndWhitePositionBuffer, blackAndWhiteUVBuffer, blackAndWhiteTexture, blackAndWhitePositionAttribute, blackAndWhiteTexCoordAttribute, 1);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> invertColorTexture = { invertColorFrameBufferTexture };
    renderSceneUsingFramebuffer(invertColorProgram, invertColorFramebufferUniform, invertColorPositionBuffer, invertColorUVBuffer, invertColorTexture, invertColorPositionAttribute, invertColorTexCoordAttribute, 1);
    
    glutSwapBuffers();
}

void hdr() {
    generateFrameBuffer(hdrFrameBuffer, hdrFrameBufferTexture, hdrDepthBufferTexture, false);
    
    glBindFramebuffer(GL_FRAMEBUFFER, hdrFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> hdrTexture = { hdrFrameBufferTexture };
    
    renderSceneUsingFramebuffer(hdrProgram, hdrFramebufferUniform, hdrPositionBuffer, hdrUVBuffer, hdrTexture, hdrPositionAttribute, hdrTexCoordAttribute, 1);
    glutSwapBuffers();
}

void fxaa() {
    generateFrameBuffer(fxaaFrameBuffer, fxaaFrameBufferTexture, fxaaDepthBufferTexture, false);
    
    glBindFramebuffer(GL_FRAMEBUFFER, fxaaFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> fxaaTexture = { fxaaFrameBufferTexture };
    renderSceneUsingFramebuffer(fxaaProgram, fxaaFramebufferUniform, fxaaPositionBuffer, fxaaUVBuffer, fxaaTexture, fxaaPositionAttribute, fxaaTexCoordAttribute, 1);
    
    glutSwapBuffers();
}

void blurWithInvertcolors() {
    generateFrameBuffer(horizontalBlurFrameBuffer, horizontalBlurFrameBufferTexture, horizontalBlurDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, horizontalBlurFrameBuffer);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    
    renderSkybox();
    
    generateFrameBuffer(verticalBlurFrameBuffer, verticalBlurFrameBufferTexture, verticalBlurDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, verticalBlurFrameBuffer);
    
    vector<GLuint> horizontalTexture = { horizontalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(horizontalBlurProgram, horizontalBlurFramebufferUniform, horizontalBlurPositionBuffer, horizontalBlurUVBuffer, horizontalTexture, horizontalBlurPositionAttribute, horizontalBlurTexCoordAttribute, 1);
    
    generateFrameBuffer(invertColorFrameBuffer, invertColorFrameBufferTexture, invertColorDepthBufferTexture, false);
    glBindFramebuffer(GL_FRAMEBUFFER, invertColorFrameBuffer);
    
    vector<GLuint> verticalTexture = { verticalBlurFrameBufferTexture };
    renderSceneUsingFramebuffer(verticalBlurProgram, verticalBlurFramebufferUniform, verticalBlurPositionBuffer, verticalBlurUVBuffer, verticalTexture, verticalBlurPositionAttribute, verticalBlurTexCoordAttribute, 1);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    vector<GLuint> invertColorTexture = { invertColorFrameBufferTexture };
    renderSceneUsingFramebuffer(invertColorProgram, invertColorFramebufferUniform, invertColorPositionBuffer, invertColorUVBuffer, invertColorTexture, invertColorPositionAttribute, invertColorTexCoordAttribute, 1);
    
    glutSwapBuffers();

}

void plainScene() {
    glViewport(0, 0, wHeight, wWidth);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    renderScene();
    renderSkybox();
    glutSwapBuffers();
}

void display(void) {
    if (isBlur)
        blur();
    else if (isGlare)
        glare();
    else if (isInvertColor)
        invertColor();
    else if (isBlackAndWhite)
        blackAndWhite();
    else if (isBlackAndWhiteAndInvertColor)
        blackAndWhiteWithInvertColor();
    else if (isFxaa)
        fxaa();
    else if (isHDR)
        hdr();
    else if (isBlurWithInvertColors)
        blurWithInvertcolors();
    else
        plainScene();
}


GLuint loadShaders(string vertexShader, string fragmentShader, GLuint &positionAttribute, GLuint &texCoordAttribute, GLuint &frameBufferUniform, GLuint &positionBuffer, GLuint &uvBuffer) {
    GLuint shaderProgram = glCreateProgram();
    readAndCompileShader(shaderProgram, vertexShader.c_str(), fragmentShader.c_str());
    
    positionAttribute = glGetAttribLocation(shaderProgram, "position");
    texCoordAttribute = glGetAttribLocation(shaderProgram, "texCoord");
    frameBufferUniform = glGetUniformLocation(shaderProgram, "screenFramebuffer");
    
    glGenBuffers(1, &positionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, positionBuffer);
    GLfloat framePositions[] = {
        1.0f, 1.0f,
        1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, -1.0f,
        -1.0f, 1.0f,
        1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(framePositions), framePositions, GL_STATIC_DRAW);
    
    glGenBuffers(1, &uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, uvBuffer);
    GLfloat frameUVs[] = {
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0,
        0.0f, 0.0f,
        0.0f, 1.0f,
        1.0f, 1.0f
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(frameUVs), frameUVs, GL_STATIC_DRAW);
    
    return shaderProgram;
}

void init() {
    glClearDepth(0.0f);
    glCullFace(GL_BACK);
//    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_GREATER);
    glReadBuffer(GL_BACK);
    
    program = glCreateProgram();
    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    
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
    
    for(int i=0; i<10; i++) {
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
    
    int cubeIbLen, cubeVblen;
    getCubeVbIbLen(cubeVblen, cubeIbLen);
    std::vector<VertexPN> vtx3(cubeVblen);
    std::vector<unsigned short> idx3(cubeIbLen);
    makeCube(5, vtx3.begin(), idx3.begin());
    
    glGenBuffers(1, &cubePositionBO);
    glBindBuffer(GL_ARRAY_BUFFER, cubePositionBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(VertexPN) * vtx3.size(), vtx3.data(), GL_STATIC_DRAW);
    
    glGenBuffers(1, &cubeIndexBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cubeIndexBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(unsigned short) * idx3.size(), idx3.data(), GL_STATIC_DRAW);
    cubeNumIndices = (int)idx3.size();
    
    environmentMapProgram = glCreateProgram();
    readAndCompileShader(environmentMapProgram, "skybox_v.glsl", "skybox_f.glsl");
    
    postionAttributeFromVertexShaderE = glGetAttribLocation(environmentMapProgram, "position");
    normalAttributeFromVertexShaderE = glGetAttribLocation(environmentMapProgram, "normal");
    
    environmentMapUniformLocation = glGetUniformLocation(environmentMapProgram, "environmentMap");
    
    modelViewMatrixUniformFromVertexShaderE = glGetUniformLocation(environmentMapProgram, "modelViewMatrix");
    normalMatrixUniformFromVertexShaderE = glGetUniformLocation(environmentMapProgram, "normalMatrix");
    projectionMatrixUniformFromVertexShaderE = glGetUniformLocation(environmentMapProgram, "projectionMatrix");
    
    std::vector<std::string> cubemapFiles;
    cubemapFiles.push_back("nebulaLF.png");
    cubemapFiles.push_back("nebulaRT.png");
    cubemapFiles.push_back("nebulaUP180.png");
    cubemapFiles.push_back("nebulaDN180.png");
    cubemapFiles.push_back("nebulaBK.png");
    cubemapFiles.push_back("nebulaFT.png");
    cubeMap = loadGLCubemap(cubemapFiles);
    environmentTexBinder.environmentMap = cubeMap;        

    luminanceClampProgram = loadShaders("luminance_clamp_vertex.glsl", "luminance_clamp_fragment.glsl", luminanceClampPositionAttribute, luminanceClampTexCoordAttribute, luminanceClampFramebufferUniform, luminanceClampPositionBuffer, luminanceClampUVBuffer);
    
    horizontalBlurProgram = loadShaders("h_blur_v.glsl", "h_blur_f.glsl", horizontalBlurPositionAttribute, horizontalBlurTexCoordAttribute, horizontalBlurFramebufferUniform, horizontalBlurPositionBuffer, horizontalBlurUVBuffer);
    
    verticalBlurProgram = loadShaders("v_blur_v.glsl", "v_blur_f.glsl", verticalBlurPositionAttribute, verticalBlurTexCoordAttribute, verticalBlurFramebufferUniform, verticalBlurPositionBuffer, verticalBlurUVBuffer);
    
    finalAdditiveProgram = loadShaders("final_additive_vertex.glsl", "final_additive_fragment.glsl", finalAdditivePositionAttribute, finalAdditiveTexCoordAttribute, finalAdditiveFramebufferUniform, finalAdditivePositionBuffer, finalAdditiveUVBuffer);
    
    invertColorProgram = loadShaders("invert_color_vertex.glsl", "invert_color_fragment.glsl", invertColorPositionAttribute, invertColorTexCoordAttribute, invertColorFramebufferUniform, invertColorPositionBuffer, invertColorUVBuffer);
    
    blackAndWhiteProgram = loadShaders("black_and_white_vertex.glsl", "black_and_white_fragment.glsl", blackAndWhitePositionAttribute, blackAndWhiteTexCoordAttribute, blackAndWhiteFramebufferUniform, blackAndWhitePositionBuffer, blackAndWhiteUVBuffer);
    
    fxaaProgram = loadShaders("fxaa_vertex.glsl", "fxaa_fragment.glsl", fxaaPositionAttribute, fxaaTexCoordAttribute, fxaaFramebufferUniform, fxaaPositionBuffer, fxaaUVBuffer);
    
    hdrProgram = loadShaders("hdr_vertex.glsl", "hdr_fragment.glsl", hdrPositionAttribute, hdrTexCoordAttribute, hdrFramebufferUniform, hdrPositionBuffer, hdrUVBuffer);
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

void resetAllFlags() {
    isBlackAndWhite = false,
    isInvertColor = false,
    isBlackAndWhiteAndInvertColor = false,
    isFxaa = false,
    isGlare = false,
    isHDR = false,
    isBlur = false;
    isBlurWithInvertColors = false;
}

void keyboard(unsigned char key, int x, int y) {
    resetAllFlags();
    switch(key) {
        case '1':
            isBlackAndWhite = true;
            break;
        case '2':
            isInvertColor = true;
            break;
        case '3':
            isBlackAndWhiteAndInvertColor = true;
            break;
        case '4':
            isFxaa = true;
            break;
        case '5':
            isGlare = true;
            break;
        case '6':
            isHDR = true;
            break;
        case '7':
            isBlur = true;
            break;
        case '8':
            isBlurWithInvertColors = true;
            break;
        default:
            break;
    }
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
    
    glutKeyboardFunc(keyboard);
    
    init();
    glutMainLoop();
    return 0;
}