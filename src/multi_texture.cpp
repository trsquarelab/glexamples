
#include <GL/glut.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>

#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rshader_object.h"
#include "rbmp_image_handler.h"

static chikku::RTransformation transformationProjection;

static const  std::string ImageFilenameTop = std::string("../data/image1.bmp");
static const  std::string ImageFilenameBase = std::string("../data/image2.bmp");

static chikku::RSolidSphere sphere(1.0f);

static float rotationAngle = 0.0f;

static chikku::RShaderObject shaderObject;

static GLuint textureH[2];

static GLuint bufferH[2];

static const char *vShaderSource =
                                    "uniform mat4 u_mvp_matrix;                                 \n"
                                    "                                                           \n"
                                    "attribute vec3 a_position;                                 \n"
                                    "attribute vec2 a_texturecoord;                             \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   v_texturecoord = a_texturecoord;                        \n"
                                    "                                                           \n"
                                    "   gl_Position = u_mvp_matrix * vec4(a_position, 1.0);     \n"
                                    "}                                                          \n"
                                    ;

static const char *fShaderSource =
                                    "                                                           \n"
                                    "uniform sampler2D u_base;                                  \n"
                                    "uniform sampler2D u_top;                                   \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "                                                           \n"
                                    "    vec4 baseColor = texture2D(u_base, v_texturecoord);    \n"
                                    "    vec4 topColor = texture2D(u_top, v_texturecoord);      \n"
                                    "                                                           \n"
                                    "    gl_FragColor = baseColor * (topColor + 0.25);          \n"
                                    "}                                                          \n"
                                    ;

int loadImage(std::string const & filename, GLuint texture)
{
    chikku::RBMPImageHandler bmpImageHandler;
    if (!bmpImageHandler.load(filename.c_str())) {
        std::cerr << "loading image " << filename.c_str() << " failed" << std::endl;
        return -1;
    }

    glBindTexture(GL_TEXTURE_2D, texture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB,
                 bmpImageHandler.defaultSize().width(), 
                 bmpImageHandler.defaultSize().height(), 
                 0, GL_RGB, GL_UNSIGNED_BYTE,
                 bmpImageHandler.data());

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
    glGenerateMipmap(GL_TEXTURE_2D);

    glBindTexture(GL_TEXTURE_2D, 0);

    return 0;
}

int initTexture()
{
    glGenTextures(2, textureH);
    if (loadImage(ImageFilenameBase, textureH[0]) < 0) {
        return -1;
    }

    if (loadImage(ImageFilenameTop, textureH[1]) < 0) {
        return -1;
    }

    return 0;
}

int initShader()
{
    if (!shaderObject.create(vShaderSource, fShaderSource)) {
        std::cerr << "shader creation failed" << std::endl;
        return -1;
    }

    return 0;
}

int initBuffer()
{
    glGenBuffers(2, bufferH);

    glBindBuffer(GL_ARRAY_BUFFER, bufferH[0]);
    glBufferData(GL_ARRAY_BUFFER, sphere.verticesSize(), sphere.vertices(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, bufferH[1]);
    glBufferData(GL_ARRAY_BUFFER, sphere.texcoordsSize(), sphere.texcoords(), GL_STATIC_DRAW);

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    return 0;
}

int init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    glCullFace(GL_BACK);

    glFrontFace(GL_CCW);

    if (initTexture() < 0) {
        return -1;
    }

    if (initShader() < 0) {
        return -1;
    }

    if (initBuffer() < 0) {
        return -1;
    }

    return 0;
}

void drawPerPixel()
{
    if (shaderObject.program() > 0) {
        glUseProgram(shaderObject.program());

        chikku::RTransformation modelRTransformation;
        modelRTransformation.translate(0, 0, -3);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

        chikku::RTransformation transformation = modelRTransformation.multiply(transformationProjection);

        GLint mvp_matrixLoc = glGetUniformLocation(shaderObject.program(), "u_mvp_matrix");
        GLint baseLoc = glGetUniformLocation(shaderObject.program(), "u_base");
        GLint topLoc = glGetUniformLocation(shaderObject.program(), "u_top");
        GLint positionLoc = glGetAttribLocation(shaderObject.program(), "a_position");
        GLint texturecoordLoc = glGetAttribLocation(shaderObject.program(), "a_texturecoord");

        chikku::RTransformation mat = modelRTransformation.inverse();
        mat = mat.transpose();

        glUniformMatrix4fv(mvp_matrixLoc, 1, GL_FALSE, transformation.matrix().data());

        glBindBuffer(GL_ARRAY_BUFFER, bufferH[0]);
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferH[1]);
        glVertexAttribPointer(texturecoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureH[0]);
        glUniform1i(baseLoc, 0);

        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, textureH[1]);
        glUniform1i(topLoc, 1);

        glEnableVertexAttribArray(positionLoc);
        glEnableVertexAttribArray(texturecoordLoc);

        glDrawArrays(GL_TRIANGLES, 0, sphere.verticesCount());

        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
    }
}

void reshape(int w, int h)
{
    glMatrixMode(GL_PROJECTION);
    glViewport(0, 0, w, h);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    const float near = 1.0f;
    const float far = 20.0f;
    const float fovy = 45.0f;

    transformationProjection.perspective(fovy, (float)w/(float)h, near, far);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

void timer(int)
{
    glutPostRedisplay();
    glutTimerFunc(chikku::TimerDelay, timer, 0);
}

void display()
{
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawPerPixel();

    rotationAngle += 2.0f;
    if (rotationAngle > 360.0f) {
        rotationAngle = 0.0f;
    }

    glutSwapBuffers();
}

int main(int argc, char ** argv)
{
    glutInit(&argc, argv);
    glutInitWindowPosition(chikku::WindowPositionX, chikku::WindowPositionY);
    glutInitWindowSize(chikku::WindowWidth, chikku::WindowHeight);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

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

