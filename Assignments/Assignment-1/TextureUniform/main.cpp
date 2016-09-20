
#include <glut.h>
#include "glsupport.h"
#include <stdio.h>

using namespace std;

float xTextureOffset = 0.0;
float yTextureOffset = 0.0;

GLuint program;

// Texture Mappers
GLuint batmanTexture;
GLuint supermanTexture;
GLuint versusTexture;

// Uniform Mappers
GLuint texturePositionUniformFromFragmentShader;
GLuint positionUniformFromVertexShader;

// Buffer Objects
GLuint vertexBufferObject;
GLuint textureBufferObject;

// Attribute Mappers
GLuint postionAttributeFromVertexShader;
GLuint textureAttributeFromVertexShader;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    
    glUniform2f(texturePositionUniformFromFragmentShader, xTextureOffset, yTextureOffset);
    
    glUseProgram((program));
    
    // Triangle Render
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(postionAttributeFromVertexShader, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(postionAttributeFromVertexShader);
    
    // Texture Coordinate maps over the trianlges
    glBindBuffer(GL_ARRAY_BUFFER, textureBufferObject);
    glVertexAttribPointer(textureAttributeFromVertexShader, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(textureAttributeFromVertexShader);
    
    // Draw leftmost texture
    glBindTexture(GL_TEXTURE_2D, batmanTexture);
    glUniform2f(positionUniformFromVertexShader, -0.5, 0.0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Draw middle texture
    glBindTexture(GL_TEXTURE_2D, versusTexture);
    glUniform2f(positionUniformFromVertexShader, 0.0, 0.0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    // Draw rightmost texture
    glBindTexture(GL_TEXTURE_2D, supermanTexture);
    glUniform2f(positionUniformFromVertexShader, 0.5, 0.0);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(postionAttributeFromVertexShader);
    glutSwapBuffers();
}

void init() {
    program = glCreateProgram();
    
    // Load all textures. *Copy the images and shaders into the executable folder while running the project
    batmanTexture = loadGLTexture("batman.png");
    supermanTexture = loadGLTexture("superman.png");
    versusTexture = loadGLTexture("versus.png");
    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    
    glUseProgram(program);
    
    // Attributes
    postionAttributeFromVertexShader = glGetAttribLocation(program, "positionAttribute");
    textureAttributeFromVertexShader = glGetAttribLocation(program, "texturePositionAttribute");
    
    // Uniforms
    texturePositionUniformFromFragmentShader = glGetUniformLocation(program, "texturePositionUniform");
    positionUniformFromVertexShader = glGetUniformLocation(program, "positionUniform");
    
    // Vertices for triangles
    glGenBuffers(1, &vertexBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    GLfloat vertices[12] = {
        -0.5f, -0.5f,
        0.5f, 0.5f,
        0.5f, -0.5f,
        
        -0.5f, -0.5f,
        -0.5f, 0.5f,
        0.5f, 0.5f
    };
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    
    // Coordinates for texture image over triangles
    glGenBuffers(1, &textureBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, textureBufferObject);
    GLfloat textures[12] = {
        0.0f, 1.0f,
        1.0f, 0.0f,
        1.0f, 1.0f,
        
        0.0f, 1.0f,
        0.0f, 0.0f,
        1.0f, 0.0f,
    };
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), textures, GL_STATIC_DRAW);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y) {
    switch(key) {
        case 'a':
            xTextureOffset += 0.02;
        break;
        case 'd':
            xTextureOffset -= 0.02;
        break;
        case 'w':
            yTextureOffset += 0.02;
        break;
        case 's':
            yTextureOffset -= 0.02;
        break;
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 500);
    glutCreateWindow("Texture Uniform");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    glutKeyboardFunc(keyboard);
    
    init();
    glutMainLoop();
    return 0;
}