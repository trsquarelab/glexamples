
#include <GL/glut.h>
#include <iostream>
#include <cstring>
#include <string>
#include <cmath>

#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rshader_object.h"

static chikku::RTransformation transformationProjection;
static chikku::RTransformation transformationLight;

static GLfloat matAmbient[]     = {0.2, 0.2, 0.2, 1.0};
static GLfloat matDiffuse[]     = {0.8, 0.8, 0.8, 1.0};
static GLfloat matSpecular[]    = {0.2, 0.2, 0.2, 1.0};
static GLfloat matEmissive[]    = {0.0, 0.0, 0.0, 1.0};
static GLfloat matShininess[]   = {20.0};

static GLfloat lightAmbient[]   = {0.2, 0.2, 0.2, 1.0};
static GLfloat lightDiffuse[]   = {1.0, 1.0, 1.0, 1.0};
static GLfloat lightSpecular[]  = {0.5, 0.5, 0.5, 1.0};
static GLfloat originalLightPosition[]  = {-40.0, 0.0, -2.0, 0.0};

static GLfloat viewerPosition[] = {0.0, 0.0, 0.0, 1.0};

static chikku::RSolidSphere sphere(1.0f);

static float rotationAngle = 0.0f;

static chikku::RShaderObject shaderPerVertex;
static chikku::RShaderObject shaderPerPixel;

chikku::RVector transformedLightPosition;

static const char *vShaderSourcePerPixel =
                                    "uniform mat4 u_mvp_matrix;                                 \n"
                                    "uniform mat4 u_normalmatrix;                               \n"
                                    "                                                           \n"
                                    "uniform vec3 u_lightambient;                               \n"
                                    "uniform vec3 u_lightdiffuse;                               \n"
                                    "uniform vec3 u_lightpos;                                   \n"
                                    "uniform vec3 u_lightspecular;                              \n"
                                    "                                                           \n"
                                    "uniform vec3 u_mat_ambient;                                \n"
                                    "uniform vec3 u_mat_diffuse;                                \n"
                                    "uniform vec3 u_mat_emissive;                               \n"
                                    "uniform vec3 u_mat_specular;                               \n"
                                    "uniform float u_mat_specexp;                               \n"
                                    "                                                           \n"
                                    "attribute vec3 a_position;                                 \n"
                                    "attribute vec3 a_normal;                                   \n"
                                    "                                                           \n"
                                    "varying vec3 v_lightspecular;                              \n"
                                    "varying vec3 v_mat_specular;                               \n"
                                    "varying float v_mat_specexp;                               \n"
                                    "varying vec3 v_diffuse;                                    \n"
                                    "varying vec3 v_ambient;                                    \n"
                                    "varying vec3 v_normal;                                     \n"
                                    "varying vec3 v_position;                                   \n"
                                    "varying vec3 v_lightpos;                                   \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   v_lightspecular = u_lightspecular;                      \n"
                                    "   v_mat_specular = u_mat_specular;                        \n"
                                    "   v_mat_specexp = u_mat_specexp;                          \n"
                                    "   vec4 tNormal = u_normalmatrix * vec4(a_normal, 1.0);    \n"
                                    "   v_normal = normalize(tNormal.xyz);                      \n"
                                    "   v_position = a_position;                                \n"
                                    "   v_lightpos = u_lightpos;                                \n"
                                    "   v_ambient = u_lightambient *                            \n"
                                    "                   u_mat_ambient;                          \n"
                                    "   v_diffuse = u_lightdiffuse *                            \n"
                                    "                   u_mat_diffuse;                          \n"
                                    "                                                           \n"
                                    "   gl_Position = u_mvp_matrix * vec4(a_position, 1.0);     \n"
                                    "}                                                          \n"
                                    ;

static const char *fShaderSourcePerPixel =
                                    "                                                           \n"
                                    "uniform vec3 u_viewer;                                     \n"
                                    "                                                           \n"
                                    "varying vec3 v_lightspecular;                              \n"
                                    "varying vec3 v_mat_specular;                               \n"
                                    "varying float v_mat_specexp;                               \n"
                                    "varying vec3 v_diffuse;                                    \n"
                                    "varying vec3 v_ambient;                                    \n"
                                    "varying vec3 v_normal;                                     \n"
                                    "varying vec3 v_position;                                   \n"
                                    "varying vec3 v_lightpos;                                   \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    vec3 n;                                                \n"
                                    "    float NdotL, NdotHV;                                   \n"
                                    "    vec3 lspec = v_lightspecular;                          \n"
                                    "    vec3 mspec = v_mat_specular;                           \n"
                                    "    float mexp  = v_mat_specexp;                           \n"
                                    "    vec3 color = v_ambient;                                \n"
                                    "    vec3 lightDir = normalize(v_lightpos - v_position);    \n"
                                    "    vec3 halfRVector = normalize(lightDir + u_viewer);     \n"
                                    "                                                           \n"
                                    "    n = normalize(v_normal);                               \n"
                                    "    NdotL = max(dot(n, lightDir), 0.0);                    \n"
                                    "                                                           \n"
                                    "    if (NdotL > 0.0) {                                     \n"
                                    "        color += v_diffuse * NdotL;                        \n"
                                    "        NdotHV = max(dot(n, halfRVector), 0.0);            \n"
                                    "        color += mspec *                                   \n"
                                    "                 lspec *                                   \n"
                                    "                 pow(NdotHV, mexp);                        \n"
                                    "    }                                                      \n"
                                    "    gl_FragColor = vec4(color, 1.0);                       \n"
                                    "}                                                          \n"
                                    ;

static const char *vShaderSourcePerVertex =
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

static const char *fShaderSourcePerVertex =
                                    "varying vec3 v_lighting;                                   \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   gl_FragColor = vec4(v_lighting, 1.0);                   \n"
                                    "}                                                          \n"
                                    ;

int initShader()
{
    if (!shaderPerVertex.create(vShaderSourcePerVertex, fShaderSourcePerVertex)) {
        std::cerr << "per vertex shader creation failed" << std::endl;
        return -1;
    }

    if (!shaderPerPixel.create(vShaderSourcePerPixel, fShaderSourcePerPixel)) {
        std::cerr << "per pixel shader creation failed" << std::endl;
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

    return 0;
}

void setLightPos()
{
    static float lightRotationAngle = 0.0f;
    glPushMatrix();

    glLoadIdentity();

    transformationLight.push();

    transformationLight.rotate(lightRotationAngle, 0.0f, 1.0f, 0.0f);
    transformationLight.translate(0, 0, -3);
    transformedLightPosition = transformationLight.multiply(originalLightPosition[0],
                                     originalLightPosition[1],
                                     originalLightPosition[2]);

    glLightfv(GL_LIGHT0, GL_POSITION, transformedLightPosition.data());

    transformationLight.pop();

    lightRotationAngle += 0.5f;
    if (lightRotationAngle > 360.0f) {
        lightRotationAngle = 0.0f;
    }

    glPopMatrix();
}

void drawPerPixel()
{
    if (shaderPerPixel.program() > 0) {
        glUseProgram(shaderPerPixel.program());

        chikku::RTransformation modelRTransformation;
        modelRTransformation.translate(0, 0, -3);
        modelRTransformation.translate(0, 1, -2);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.rotate(rotationAngle, 0.0f, 1.0f, 0.0f);

        chikku::RTransformation transformation = modelRTransformation.multiply(transformationProjection);

        GLint normalmatrixLocv = glGetUniformLocation(shaderPerPixel.program(), "u_normalmatrix");
        GLint lightambientLocv = glGetUniformLocation(shaderPerPixel.program(), "u_lightambient");
        GLint lightdiffuseLocv = glGetUniformLocation(shaderPerPixel.program(), "u_lightdiffuse");
        GLint lightspecularLocv = glGetUniformLocation(shaderPerPixel.program(), "u_lightspecular");
        GLint lightposLocv = glGetUniformLocation(shaderPerPixel.program(), "u_lightpos");
        GLint viewerLocv = glGetUniformLocation(shaderPerPixel.program(), "u_viewer");
        GLint mat_ambientLocv = glGetUniformLocation(shaderPerPixel.program(), "u_mat_ambient");
        GLint mat_diffuseLocv = glGetUniformLocation(shaderPerPixel.program(), "u_mat_diffuse");
        GLint mat_specularLocv = glGetUniformLocation(shaderPerPixel.program(), "u_mat_specular");
        GLint mat_specexpLocv = glGetUniformLocation(shaderPerPixel.program(), "u_mat_specexp");
        GLint mat_emissiveLocv = glGetUniformLocation(shaderPerPixel.program(), "u_mat_emissive");
        GLint normalLoc = glGetAttribLocation(shaderPerPixel.program(), "a_normal");
        GLint positionLoc = glGetAttribLocation(shaderPerPixel.program(), "a_position");
        GLint mvp_matrixLoc = glGetUniformLocation(shaderPerPixel.program(), "u_mvp_matrix");

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
}

void drawPerVertex()
{
    if (shaderPerVertex.program() > 0) {

        glUseProgram(shaderPerVertex.program());

        chikku::RTransformation modelRTransformation;
        modelRTransformation.translate(0, 0, -3);
        modelRTransformation.translate(0, -1, -2);
        modelRTransformation.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
        modelRTransformation.rotate(rotationAngle, 1.0f, 0.0f, 0.0f);

        chikku::RTransformation transformation = modelRTransformation.multiply(transformationProjection);

        GLint normalmatrixLocv = glGetUniformLocation(shaderPerVertex.program(), "u_normalmatrix");
        GLint lightambientLocv = glGetUniformLocation(shaderPerVertex.program(), "u_lightambient");
        GLint lightdiffuseLocv = glGetUniformLocation(shaderPerVertex.program(), "u_lightdiffuse");
        GLint lightspecularLocv = glGetUniformLocation(shaderPerVertex.program(), "u_lightspecular");
        GLint lightposLocv = glGetUniformLocation(shaderPerVertex.program(), "u_lightpos");
        GLint viewerLocv = glGetUniformLocation(shaderPerVertex.program(), "u_viewer");
        GLint mat_ambientLocv = glGetUniformLocation(shaderPerVertex.program(), "u_mat_ambient");
        GLint mat_diffuseLocv = glGetUniformLocation(shaderPerVertex.program(), "u_mat_diffuse");
        GLint mat_specularLocv = glGetUniformLocation(shaderPerVertex.program(), "u_mat_specular");
        GLint mat_specexpLocv = glGetUniformLocation(shaderPerVertex.program(), "u_mat_specexp");
        GLint mat_emissiveLocv = glGetUniformLocation(shaderPerVertex.program(), "u_mat_emissive");
        GLint normalLoc = glGetAttribLocation(shaderPerVertex.program(), "a_normal");
        GLint positionLoc = glGetAttribLocation(shaderPerVertex.program(), "a_position");
        GLint mvp_matrixLoc = glGetUniformLocation(shaderPerVertex.program(), "u_mvp_matrix");

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

    transformationLight = transformationProjection;

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

    drawPerVertex();
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

