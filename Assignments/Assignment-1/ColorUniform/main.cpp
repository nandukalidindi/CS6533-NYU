
#include <glut.h>
#include "glsupport.h"
#include <stdio.h>

using namespace std;

float redOffset = 0.0;
float greenOffset = 0.0;
float blueOffset = 0.0;

GLuint program;

// Uniform Mappers
GLuint positionUniformFromVertexShader;
GLuint colorUniformFromFragmentShader;

// Buffer Objects
GLuint vertexBufferObject;

// Attribute Mappers
GLuint postionAttributeFromVertexShader;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(program);
    
    glUniform3f(colorUniformFromFragmentShader, redOffset, greenOffset, blueOffset);
    
    // Draw Triangles
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(postionAttributeFromVertexShader, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(postionAttributeFromVertexShader);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(postionAttributeFromVertexShader);
    glutSwapBuffers();
}

void init() {
    program = glCreateProgram();

    readAndCompileShader(program, "vertex.glsl", "fragment.glsl");
    
    glUseProgram(program);
    // Load attribute variables
    postionAttributeFromVertexShader = glGetAttribLocation(program, "positionAttribute");
    
    // Load uniform variables
    positionUniformFromVertexShader = glGetUniformLocation(program, "positionUniform");
    colorUniformFromFragmentShader = glGetUniformLocation(program, "colorUniform");
    
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
            if (redOffset <= 1.0)
                redOffset += 0.05;
            break;
        case 'a':
            if (greenOffset <= 1.0)
                greenOffset += 0.05;
            break;
        case 'd':
            if (blueOffset <= 1.0)
                blueOffset += 0.05;
            break;
        case 's':
            if (redOffset >= 0 )
                redOffset -= 0.05;
            if (greenOffset >= 0)
                greenOffset -= 0.05;
            if (blueOffset >= 0)
                blueOffset -= 0.05;
            break;
    }
}

void mouse(int button, int state, int x, int y) {
    float newPositionX = (float)x/500.0f - 1.0f;
    float newPositionY = (1.0-(float)y/250.0);
    glUniform2f(positionUniformFromVertexShader, newPositionX, newPositionY);
}

void mouseMove(int x, int y) {
    float newPositionX = (float)x/500.0f - 1.0f;
    float newPositionY = (1.0-(float)y/250.0);
    glUniform2f(positionUniformFromVertexShader, newPositionX, newPositionY);
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1000, 500);
    glutCreateWindow("Color Uniform");
    
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