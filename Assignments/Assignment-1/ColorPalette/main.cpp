
#include <glut.h>
#include "glsupport.h"
#include <stdio.h>
#include <math.h>

using namespace std;

float redOffset = 0.0;
float greenOffset = 0.0;
float blueOffset = 0.0;

GLuint program;

static float averageClickProximity = 10.0;

static int paletteCombinations = 29;

// Matrix which has information on what color of each square gets in the palette
float offsetMatrix[29][3] = {
    {1.00, 0.00, 0.00}, {0.00, 1.00, 0.00}, {0.00, 0.00, 1.00}, {1.00, 1.00, 0.00}, {0.00, 1.00, 1.00}, {1.00, 0.00, 1.00},
    {0.75, 0.50, 0.50}, {0.50, 0.75, 0.50}, {0.50, 0.50, 0.75}, {0.75, 0.75, 0.50}, {0.50, 0.75, 0.75}, {0.75, 0.50, 0.75},
    {0.50, 0.25, 0.25}, {0.25, 0.50, 0.25}, {0.25, 0.25, 0.50}, {0.50, 0.50, 0.25}, {0.25, 0.50, 0.50}, {0.50, 0.25, 0.50},
    {0.25, 0.00, 0.00}, {0.00, 0.25, 0.00}, {0.00, 0.00, 0.25}, {0.25, 0.25, 0.00}, {0.00, 0.25, 0.25}, {0.25, 0.00, 0.25},
                        {1.00, 1.00, 1.00}, {0.75, 0.75, 0.75}, {0.50, 0.50, 0.50}, {0.25, 0.25, 0.25}
};

// Matrix which has information on where to place each square of the palette
float locationMatrix[29][2] = {
    {-0.90, 0.90}, {-0.85, 0.90}, {-0.80, 0.90}, {-0.75, 0.90}, {-0.70, 0.90}, {-0.65, 0.90},
    {-0.90, 0.85}, {-0.85, 0.85}, {-0.80, 0.85}, {-0.75, 0.85}, {-0.70, 0.85}, {-0.65, 0.85},
    {-0.90, 0.80}, {-0.85, 0.80}, {-0.80, 0.80}, {-0.75, 0.80}, {-0.70, 0.80}, {-0.65, 0.80},
    {-0.90, 0.75}, {-0.85, 0.75}, {-0.80, 0.75}, {-0.75, 0.75}, {-0.70, 0.75}, {-0.65, 0.75},
                   {-0.85, 0.70}, {-0.80, 0.70}, {-0.75, 0.70}, {-0.70, 0.70}
};
// Uniform Mappers
GLuint positionUniformFromVertexShader;
GLuint colorUniformFromFragmentShader;

// Buffer Objects
GLuint vertexBufferObject;
GLuint largeBufferObject;


// Attribute Mappers
GLuint postionAttributeFromVertexShader;

void display(void) {
    glClear(GL_COLOR_BUFFER_BIT);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glUseProgram(program);
    
    
    // Initialize the buffer and position attribute
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObject);
    glVertexAttribPointer(postionAttributeFromVertexShader, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(postionAttributeFromVertexShader);
    
    // Draw all palatte triangles
    for(int i=0; i<paletteCombinations; i++) {
        glUniform3f(colorUniformFromFragmentShader, offsetMatrix[i][0], offsetMatrix[i][1], offsetMatrix[i][2]);
        glUniform2f(positionUniformFromVertexShader, locationMatrix[i][0], locationMatrix[i][1]);
        glDrawArrays(GL_TRIANGLES, 0, 6);
    }
    
    glDisableVertexAttribArray(postionAttributeFromVertexShader);
    
    // Initialize the buffer again to draw another triangle as it requires different manipulations
    glBindBuffer(GL_ARRAY_BUFFER, largeBufferObject);
    glVertexAttribPointer(postionAttributeFromVertexShader, 2, GL_FLOAT, GL_FALSE, 0, 0);
    glEnableVertexAttribArray(postionAttributeFromVertexShader);
    
    glUniform3f(colorUniformFromFragmentShader, redOffset, greenOffset, blueOffset);
    glUniform2f(positionUniformFromVertexShader, 0.0, 0.0);
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
        -0.02f, 0.02f,
        -0.02f, -0.02f,
        0.02f, -0.02f,
        
        -0.02f, 0.02f,
        0.02f, -0.02f,
        0.02f, 0.02f
    };
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), vertices, GL_STATIC_DRAW);
    
    glGenBuffers(1, &largeBufferObject);
    glBindBuffer(GL_ARRAY_BUFFER, largeBufferObject);
    GLfloat largeVertices[12] = {
        -0.5f, 0.5f,
        -0.5f, -0.5f,
        0.5f, -0.5f,
        
        -0.5f, 0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f
    };
    glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), largeVertices, GL_STATIC_DRAW);
}

void reshape(int w, int h) {
    glViewport(0, 0, w, h);
}

void idle(void) {
    glutPostRedisplay();
}

// Intensification logic
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

// Euclidian distance between the mouse click position and center of squares
float euclidianDistance(int x, int y, float x1, float y1) {
    return sqrtf(powf((x-x1), 2) + powf((y-y1), 2));
}

// Find out which square is clicked and return the number which corresponds to a color
int nearTo(int x, int y) {
    int xFrameOffset, yFrameOffset;
    for(int i=0; i<paletteCombinations; i++) {
        xFrameOffset = (locationMatrix[i][0]+1)*512;
        yFrameOffset = (-(locationMatrix[i][1])+1)*360;
        if(euclidianDistance(x, y, xFrameOffset, yFrameOffset) <= averageClickProximity) {
            return i;
        }
    }
    return 100;
}

// Get the square on which the mouse click happened and apply the color to the square
void mouse(int button, int state, int x, int y) {
    if(state == 0) {
        int k = nearTo(x, y);
        if (k >= paletteCombinations) {
            redOffset = 0.0;
            greenOffset = 0.0;
            blueOffset = 0.0;
        } else {
            redOffset = offsetMatrix[k][0];
            greenOffset = offsetMatrix[k][1];
            blueOffset = offsetMatrix[k][2];
        }
    }
}

int main(int argc, char **argv) {
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1024, 720);
    glutCreateWindow("Color Palette");
    
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);
    
    glutKeyboardFunc(keyboard);
    glutMouseFunc(mouse);
    
    init();
    glutMainLoop();
    return 0;
}