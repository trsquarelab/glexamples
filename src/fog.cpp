
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


static const  std::string ImageFilename = std::string("../data/image2.bmp");
static const GLfloat viewerPosition[] = {0.0, 0.0, 0.0, 1.0};
static const GLfloat FogColor[] = {1.0, 1.0, 0.0, 1.0};
static const GLfloat FogMaxDistance = 8.0f;
static const GLfloat FogMinDistance = 1.0f;
static const GLfloat FogDensity = 0.30f;


static const float RotationAngleCenter = 0.0f;
static const float RotationAngleDifferent = 0.0f;

static chikku::RSolidSphere sphere(1.0f);
static float rotationAngle = RotationAngleCenter;
static chikku::RTransformation transformationProjection;

static chikku::RShaderObject shaderObject;

static GLuint textureH[1];

static GLuint bufferH[2];

static const char *vShaderSource =
                                    "uniform mat4 u_p_matrix;                                   \n"
                                    "uniform mat4 u_m_matrix;                                   \n"
                                    "uniform vec3 u_viewerpos;                                  \n"
                                    "                                                           \n"
                                    "attribute vec3 a_position;                                 \n"
                                    "attribute vec2 a_texturecoord;                             \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "varying float v_eyedist;                                   \n"
                                    "                                                           \n"
                                    Distance3
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   v_texturecoord = a_texturecoord;                        \n"
                                    "                                                           \n"
                                    "   vec4 viewPos = u_m_matrix * vec4(a_position, 1.0);      \n"
                                    "                                                           \n"
                                    "   v_eyedist = Distance3(viewPos.xyz, u_viewerpos.xyz);    \n"
                                    "                                                           \n"
                                    "   gl_Position = u_p_matrix * u_m_matrix *                 \n"
                                    "                       vec4(a_position, 1.0);              \n"
                                    "}                                                          \n"
                                    ;

static const char *fShaderSource =
                                    "                                                           \n"
                                    "uniform int u_fogmode;                                     \n"
                                    "uniform vec4 u_fogcolor;                                   \n"
                                    "uniform float u_fogdensity;                                \n"
                                    "uniform float u_fogmaxdist;                                \n"
                                    "uniform float u_fogmindist;                                \n"
                                    "uniform sampler2D u_sampler;                               \n"
                                    "                                                           \n"
                                    "varying float v_eyedist;                                   \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    LinearFogFactor
                                    ExpFogFactor
                                    Exp2FogFactor
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    float fogFactor;                                       \n"
                                    "    if (u_fogmode == 0) {                                  \n"
                                    "        fogFactor = LinearFogFactor(u_fogmaxdist,          \n"
                                    "                                      u_fogmindist,        \n"
                                    "                                      v_eyedist);          \n"
                                    "    } else if (u_fogmode == 1) {                           \n"
                                    "        fogFactor = ExpFogFactor(u_fogdensity, v_eyedist); \n"
                                    "    } else {                                               \n"
                                    "        fogFactor = Exp2FogFactor(u_fogdensity, v_eyedist);\n"
                                    "    }                                                      \n"
                                    "                                                           \n"
                                    "    vec4 fogColor = fogFactor * u_fogcolor;                \n"
                                    "    vec4 baseColor = texture2D(u_sampler, v_texturecoord); \n"
                                    "                                                           \n"
                                    "    gl_FragColor = baseColor * fogFactor +                 \n"
                                    "               fogColor * (1.0 - fogFactor);               \n"
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
    glGenTextures(1, textureH);
    if (loadImage(ImageFilename, textureH[0]) < 0) {
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
        modelRTransformation.translate(0, 0, -5);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.translate(0, -15, 0);
        modelRTransformation.rotate(rotationAngle, 1.0f, 0.0f, 0.0f);
        modelRTransformation.translate(0, 15, 0);

        GLint p_matrixLoc = glGetUniformLocation(shaderObject.program(), "u_p_matrix");
        GLint m_matrixLoc = glGetUniformLocation(shaderObject.program(), "u_m_matrix");
        GLint samplerLoc = glGetUniformLocation(shaderObject.program(), "u_sampler");
        GLint viewerposLocv = glGetUniformLocation(shaderObject.program(), "u_viewerpos");
        GLint fogmode = glGetUniformLocation(shaderObject.program(), "u_fogmode");
        GLint fogcolor = glGetUniformLocation(shaderObject.program(), "u_fogcolor");
        GLint fogmaxdist = glGetUniformLocation(shaderObject.program(), "u_fogmaxdist");
        GLint fogmindist = glGetUniformLocation(shaderObject.program(), "u_fogmindist");
        GLint fogdensity = glGetUniformLocation(shaderObject.program(), "u_fogdensity");

        GLint positionLoc = glGetAttribLocation(shaderObject.program(), "a_position");
        GLint texturecoordLoc = glGetAttribLocation(shaderObject.program(), "a_texturecoord");

        glUniformMatrix4fv(p_matrixLoc, 1, GL_FALSE, transformationProjection.matrix().data());
        glUniformMatrix4fv(m_matrixLoc, 1, GL_FALSE, modelRTransformation.matrix().data());
        glUniform3fv(viewerposLocv, 1, viewerPosition);
        glUniform1i(fogmode, 0);
        glUniform4fv(fogcolor, 1, FogColor);
        glUniform1f(fogmaxdist, FogMaxDistance);
        glUniform1f(fogmindist, FogMinDistance);
        glUniform1f(fogdensity, FogDensity);

        glBindBuffer(GL_ARRAY_BUFFER, bufferH[0]);
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, bufferH[1]);
        glVertexAttribPointer(texturecoordLoc, 2, GL_FLOAT, GL_FALSE, 0, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureH[0]);
        glUniform1i(samplerLoc, 0);

        glEnableVertexAttribArray(positionLoc);
        glEnableVertexAttribArray(texturecoordLoc);

        glDrawArrays(GL_TRIANGLES, 0, sphere.verticesCount());

        modelRTransformation.reset();
        modelRTransformation.translate(0, 0, -5);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.translate(-1, -15, 0);
        modelRTransformation.rotate(rotationAngle, 1.0f, 0.0f, 0.0f);
        modelRTransformation.translate(-1, 15, 0);

        glUniformMatrix4fv(m_matrixLoc, 1, GL_FALSE, modelRTransformation.matrix().data());
        glUniform1i(fogmode, 1);
        glDrawArrays(GL_TRIANGLES, 0, sphere.verticesCount());

        modelRTransformation.reset();
        modelRTransformation.translate(0, 0, -5);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.translate(1, -15, 0);
        modelRTransformation.rotate(rotationAngle, 1.0f, 0.0f, 0.0f);
        modelRTransformation.translate(1, 15, 0);

        glUniformMatrix4fv(m_matrixLoc, 1, GL_FALSE, modelRTransformation.matrix().data());
        glUniform1i(fogmode, 2);
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
    static float angleUpdation = 1.0f;
    glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    drawPerPixel();

    rotationAngle += angleUpdation;
    if (rotationAngle > RotationAngleCenter+45.0f) {
        angleUpdation *= -1.0f;
    }
    if (rotationAngle < RotationAngleCenter-45.0f) {
        angleUpdation *= -1.0f;
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

