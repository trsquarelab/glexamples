
#include <GL/glut.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>

#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rshader_object.h"
#include "rtrans_animation.h"

static chikku::RTransformation transformationProjection;

static GLfloat matAmbient[]     = {0.2, 0.2, 0.2, 1.0};
static GLfloat matDiffuse[]     = {0.8, 0.8, 0.8, 1.0};
static GLfloat matSpecular[]    = {0.2, 0.2, 0.2, 1.0};
static GLfloat matEmissive[]    = {0.0, 0.0, 0.0, 1.0};
static GLfloat matShininess[]   = {20.0};

static GLfloat lightAmbient[]   = {0.2, 0.2, 0.2, 1.0};
static GLfloat lightDiffuse[]   = {1.0, 1.0, 1.0, 1.0};
static GLfloat lightSpecular[]  = {0.5, 0.5, 0.5, 1.0};
static GLfloat originalLightPosition[]  = {-40.0, 0.0, -2.0, 0.0};

chikku::RVector transformedLightPosition;

static GLfloat viewerPosition[] = {0.0, 0.0, 0.0, 1.0};

static chikku::RSolidSphere sphere(1.0f);

static chikku::RTransAnimation lightAnim;

static float rotationAngle = 0.0f;

static chikku::RShaderObject shaderObject;

static const char *vShaderSource =  ""
                                    "uniform mat4 u_mvp_matrix;                                 \n"
                                    "uniform mat4 u_normalmatrix;                               \n"
                                    "                                                           \n"
                                    "uniform vec3 u_lightambient;                               \n"
                                    "uniform vec3 u_lightdiffuse;                               \n"
                                    "uniform vec3 u_lightspecular;                              \n"
                                    "uniform vec3 u_lightpos;                                   \n"
                                    "                                                           \n"
                                    "uniform vec3 u_viewer;                                     \n"
                                    "                                                           \n"
                                    "uniform vec3 u_mat_ambient;                                \n"
                                    "uniform vec3 u_mat_diffuse;                                \n"
                                    "uniform vec3 u_mat_specular;                               \n"
                                    "uniform float u_mat_specexp;                               \n"
                                    "uniform vec3 u_mat_emissive;                               \n"
                                    "                                                           \n"
                                    "attribute vec3 a_position;                                 \n"
                                    "attribute vec3 a_normal;                                   \n"
                                    "                                                           \n"
                                    "varying vec3 v_lighting;                                   \n"
                                    "                                                           \n"
                                    DirectionLightVertexShaderPerVertex
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   vec4 tNormal = u_normalmatrix * vec4(a_normal, 1.0);    \n"
                                    "   vec3 tNordNormal = normalize(tNormal.xyz);              \n"
                                    "   vec3 tempAmbient = u_lightambient;                      \n"
                                    "   vec3 tempDiffuse = u_lightdiffuse;                      \n"
                                    "   vec3 tempSpecular = u_lightspecular;                    \n"
                                    "   vec3 ldir = u_lightpos - a_position;                    \n"
                                    "   vec3 halfWidth = normalize(normalize(ldir) +            \n"
                                    "                       u_viewer);                          \n"
                                    "                                                           \n"
                                    "   DirectionalLight(u_mat_specexp, u_lightpos.xyz,         \n"
                                    "                    halfWidth, tNordNormal, tempAmbient,   \n"
                                    "                    tempDiffuse, tempSpecular);            \n"
                                    "                                                           \n"
                                    "   v_lighting  = tempAmbient * u_mat_ambient;              \n"
                                    "   v_lighting += tempDiffuse * u_mat_diffuse;              \n"
                                    "   v_lighting += tempSpecular * u_mat_specular;            \n"
                                    "   v_lighting += u_mat_emissive;                           \n"
                                    "                                                           \n"
                                    "   gl_Position = u_mvp_matrix * vec4(a_position, 1.0);     \n"
                                    "}                                                          \n"
                                    ;

static const char *fShaderSource =  ""
                                    "varying vec3 v_lighting;                                   \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   gl_FragColor = vec4(v_lighting, 1.0);                   \n"
                                    "}                                                          \n"
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
    glMaterialfv(GL_FRONT, GL_AMBIENT, matAmbient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, matDiffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, matSpecular);
    glMaterialfv(GL_FRONT, GL_EMISSION, matEmissive);
    glMaterialfv(GL_FRONT, GL_SHININESS, matShininess);

    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmbient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDiffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightSpecular);

    glClearColor(0.0, 0.0, 0.0, 0.0);

    glShadeModel(GL_SMOOTH);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);

    glDepthFunc(GL_LEQUAL);
    glClearDepth(1.0);

    glCullFace(GL_BACK);

    glFrontFace(GL_CCW);

    if (initShader() < 0) {
        return -1;
    }

    lightAnim.setAngles(0.0f, 360.0f);
    lightAnim.setPoints(chikku::RVector(0.0f, 1.0f, 0.0f), chikku::RVector(0.0f, 1.0f, 0.0f));
    lightAnim.setType(chikku::RTransAnimation::Rotation);
    lightAnim.setRepeat(true);
    lightAnim.setDuration(5.0f);
    lightAnim.start();
    
    return 0;
}

void setLightPos()
{
    glPushMatrix();

    glLoadIdentity();

    chikku::RTransformation transformationLight;

    lightAnim.step();

    transformationLight = transformationProjection;
    
    transformationLight.translate(0, 0, -3);
    
    transformationLight = 	lightAnim.transformation().multiply(transformationLight);
    
    transformedLightPosition = transformationLight.multiply(originalLightPosition[0],
                                     originalLightPosition[1],
                                     originalLightPosition[2]);

    glLightfv(GL_LIGHT0, GL_POSITION, transformedLightPosition.data());

    glPopMatrix();
}

void drawWithGlutRSolidSphere()
{
    glPushMatrix();

    chikku::RTransformation modelRTransformation;
    modelRTransformation.translate(0, 0, -3);
    modelRTransformation.translate(0, 1, -2);
    modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
    modelRTransformation.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

    chikku::RTransformation transformation = modelRTransformation.multiply(transformationProjection);

    glLoadMatrixf(transformation.matrix().data());

    glutSolidSphere(1.0f, 20.0f, 50.0f);

    glPopMatrix();
}

void drawWithCustomRSolidSphere()
{
    glPushMatrix();

    chikku::RTransformation modelRTransformation;
    modelRTransformation.translate(0, 0, -3);
    modelRTransformation.translate(0, 0, -2);
    modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
    modelRTransformation.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

    chikku::RTransformation transformation = modelRTransformation.multiply(transformationProjection);
    glLoadMatrixf(transformation.matrix().data());

    glEnableClientState(GL_VERTEX_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, sphere.vertices());

    glEnableClientState(GL_NORMAL_ARRAY);
    glNormalPointer(GL_FLOAT, 0, sphere.normals());

    glDrawArrays(GL_TRIANGLES, 0, sphere.verticesCount());

    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_NORMAL_ARRAY);

    glPopMatrix();
}

void drawWithShaderRSolidSphere()
{
    glPushMatrix();

    if (shaderObject.program() > 0) {

        glUseProgram(shaderObject.program());

        chikku::RTransformation modelRTransformation;
        modelRTransformation.translate(0, 0, -3);
        modelRTransformation.translate(0, -1, -2);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

        chikku::RTransformation transformation = modelRTransformation.multiply(transformationProjection);

        GLint normalmatrixLocv = glGetUniformLocation(shaderObject.program(), "u_normalmatrix");
        GLint lightambientLocv = glGetUniformLocation(shaderObject.program(), "u_lightambient");
        GLint lightdiffuseLocv = glGetUniformLocation(shaderObject.program(), "u_lightdiffuse");
        GLint lightspecularLocv = glGetUniformLocation(shaderObject.program(), "u_lightspecular");
        GLint lightposLocv = glGetUniformLocation(shaderObject.program(), "u_lightpos");
        GLint viewerLocv = glGetUniformLocation(shaderObject.program(), "u_viewer");
        GLint mat_ambientLocv = glGetUniformLocation(shaderObject.program(), "u_mat_ambient");
        GLint mat_diffuseLocv = glGetUniformLocation(shaderObject.program(), "u_mat_diffuse");
        GLint mat_specularLocv = glGetUniformLocation(shaderObject.program(), "u_mat_specular");
        GLint mat_specexpLocv = glGetUniformLocation(shaderObject.program(), "u_mat_specexp");
        GLint mat_emissiveLocv = glGetUniformLocation(shaderObject.program(), "u_mat_emissive");
        GLint normalLoc = glGetAttribLocation(shaderObject.program(), "a_normal");
        GLint positionLoc = glGetAttribLocation(shaderObject.program(), "a_position");
        GLint mvp_matrixLoc = glGetUniformLocation(shaderObject.program(), "u_mvp_matrix");

        chikku::RTransformation mat = modelRTransformation.inverse();
        mat = mat.transpose();

        glUniformMatrix4fv(normalmatrixLocv, 1, GL_FALSE, mat.matrix().data());
        glUniformMatrix4fv(mvp_matrixLoc, 1, GL_FALSE, transformation.matrix().data());
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, sphere.vertices());
        glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, sphere.normals());

        glUniform3fv(lightambientLocv, 1, lightAmbient);
        glUniform3fv(lightdiffuseLocv, 1, lightDiffuse);
        glUniform3fv(lightspecularLocv, 1, lightSpecular);
        glUniform3fv(lightposLocv, 1, transformedLightPosition.data());
        glUniform3fv(viewerLocv, 1, viewerPosition);
        glUniform3fv(mat_ambientLocv, 1, matAmbient);
        glUniform3fv(mat_diffuseLocv, 1, matDiffuse);
        glUniform3fv(mat_specularLocv, 1, matSpecular);
        glUniform3fv(mat_emissiveLocv, 1, matEmissive);
        glUniform1fv(mat_specexpLocv, 1, matShininess);

        glEnableVertexAttribArray(positionLoc);
        glEnableVertexAttribArray(normalLoc);

        glDrawArrays(GL_TRIANGLES, 0, sphere.verticesCount());

        glUseProgram(0);
    }

    glPopMatrix();
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

    setLightPos();

    drawWithGlutRSolidSphere();
    drawWithCustomRSolidSphere();
    drawWithShaderRSolidSphere();

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

