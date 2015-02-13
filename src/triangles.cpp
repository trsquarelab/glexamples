
#include <GL/glut.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>

#include "rcommon.h"
#include "rtransformation.h"
#include "rshader_object.h"

static chikku::RTransformation transformation1;
static chikku::RTransformation transformation2;
static chikku::RTransformation transformation3;

static chikku::RShaderObject shaderObject;

static float rotationAngle = 0.0f;

static const GLfloat verticies [] = {
                                    0.0f, 1.0f, 0.0f,
                                    -1.0f,-1.0f, 0.0f,
                                    1.0f,-1.0f, 0.0f
                                  };

static const GLfloat colors[] = {
                                0.0f,0.0f,1.0f,
                                0.0f,1.0f,0.0f,
                                1.0f,0.0f,0.0f
                              };

static const char *vShaderSource =  ""
                                    "uniform mat4 u_mvp_matrix;                             \n"
                                    "attribute vec4 a_position;                             \n"
                                    "attribute vec4 a_color;                                \n"
                                    "varying vec4 v_color;                                  \n"
                                    "                                                       \n"
                                    "void main()                                            \n"
                                    "{                                                      \n"
                                    "   v_color = a_color;                                  \n"
                                    "   gl_Position = u_mvp_matrix * a_position;            \n"
                                    "}                                                      \n"
                                    ;

static const char *fShaderSource =  ""
                                    "varying vec4 v_color;                                  \n"
                                    "                                                       \n"
                                    "void main()                                            \n"
                                    "{                                                      \n"
                                    "   gl_FragColor = v_color;                             \n"
                                    "}                                                      \n"
                                    ;

int initShader()
{
    if (!shaderObject.create(vShaderSource, fShaderSource)) {
        return -1;
    }

    return 0;
}

int init()
{
    int res = 0;

    glShadeModel(GL_SMOOTH);

    res = initShader();
    if (res < 0) {
        return res; 
    }

    return res;
}

void triangleUsingGlVertex()
{
    glPushMatrix();

    transformation1.rotate(rotationAngle, 0.0f, 0.0f, 1.0f);
    glLoadMatrixf(transformation1.matrix().data());
    transformation1.rotate(-rotationAngle, 0.0f, 0.0f, 1.0f);

    glBegin(GL_TRIANGLES);
    for (int i=0; i<3; ++i) {
        glColor3fv(colors + 3*i);
        glVertex3fv(verticies + 3*i);
    }
    glEnd();

    glPopMatrix();
}

void triangleUsingVertexAttribute()
{
    glPushMatrix();

    transformation2.rotate(rotationAngle, 1.0f, 0.0f, 0.0f);
    glLoadMatrixf(transformation2.matrix().data());
    transformation2.rotate(-rotationAngle, 1.0f, 0.0f, 0.0f);

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, verticies);

    glEnableClientState(GL_COLOR_ARRAY);
    glColorPointer(3, GL_FLOAT, 0, colors);

    glDrawArrays(GL_TRIANGLES, 0, (sizeof(verticies)/sizeof(verticies[0])) / 3);

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_COLOR_ARRAY);

    glPopMatrix();
}

void triangleUsingShader()
{
    glPushMatrix();
    glLoadIdentity();

    if (shaderObject.program() > 0) {
        glUseProgram(shaderObject.program());

        GLint clrLoc = glGetAttribLocation(shaderObject.program(), "a_color");
        GLint posLoc = glGetAttribLocation(shaderObject.program(), "a_position");
        GLint matLoc = glGetUniformLocation(shaderObject.program(), "u_mvp_matrix");

        transformation3.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

        glUniformMatrix4fv(matLoc, 1, GL_FALSE, transformation3.matrix().data());
        glVertexAttribPointer(clrLoc, 3, GL_FLOAT, GL_FALSE, 0, colors);
        glVertexAttribPointer(posLoc, 3, GL_FLOAT, GL_FALSE, 0, verticies);
        glEnableVertexAttribArray(clrLoc);
        glEnableVertexAttribArray(posLoc);
        glDrawArrays(GL_TRIANGLES, 0, (sizeof(verticies)/sizeof(verticies[0])) / 3);

        transformation3.rotate(-rotationAngle, 0.0f, 1.0f, 0.0f);

        glUseProgram(0);
    }

    glPopMatrix();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    triangleUsingGlVertex();
    triangleUsingVertexAttribute();
    triangleUsingShader();

    rotationAngle += 3.0f;
    if (rotationAngle > 360.0f) {
        rotationAngle = 0.0f;
    }

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    transformation1.perspective(45.0f, (float)w/(float)h, 0.0f, 100.0f);
    transformation1.translate(0, 0, -3);
    transformation1.translate(0, 1, -1);
    transformation1.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);

    transformation2.perspective(45.0f, (float)w/(float)h, 0.0f, 100.0f);
    transformation2.translate(0, 0, -3);
    transformation2.translate(0, 0, -1);
    transformation2.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);

    transformation3.perspective(45.0f, (float)w/(float)h, 0.0f, 100.0f);
    transformation3.translate(0, 0, -3);
    transformation3.translate(0, -1, -1);
    transformation3.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
}

void timer(int)
{
    glutPostRedisplay();
    glutTimerFunc(chikku::TimerDelay, timer, 0);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowPosition(chikku::WindowPositionX, chikku::WindowPositionY);
    glutInitWindowSize(chikku::WindowWidth, chikku::WindowHeight);
    glutInitDisplayMode(GLUT_DOUBLE);

    glutCreateWindow(argv[0]);

    glutReshapeFunc(reshape);
    glutDisplayFunc(display);
    glutTimerFunc(chikku::TimerDelay, timer, 0);

    if (init() < 0) {
        std::cerr << "initialization failed" << std::endl;
        return 1;
    }

    glutMainLoop();

    return 0;
}

