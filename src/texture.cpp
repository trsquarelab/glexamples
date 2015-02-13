
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
static chikku::RTransformation transformationLight;

static const  std::string ImageFilename = std::string("../data/image2.bmp");

static GLfloat matAmbient[]     = {0.2, 0.2, 0.2, 1.0};
static GLfloat matDiffuse[]     = {0.8, 0.8, 0.8, 1.0};
static GLfloat matSpecular[]    = {0.2, 0.2, 0.2, 1.0};
static GLfloat matEmissive[]    = {0.0, 0.0, 0.0, 1.0};
static GLfloat matShininess[]   = {5.0};

static GLfloat lightAmbient[]   = {0.2, 0.2, 0.2, 1.0};
static GLfloat lightDiffuse[]   = {1.0, 1.0, 1.0, 1.0};
static GLfloat lightSpecular[]  = {0.5, 0.5, 0.5, 1.0};
static GLfloat originalLightPosition[]  = {-40.0, 0.0, -2.0, 0.0};

static GLfloat viewerPosition[] = {0.0, 0.0, 0.0, 1.0};

static chikku::RSolidSphere sphere(1.0f);

static float rotationAngle = 0.0f;

static chikku::RShaderObject shaderObject;

static GLuint textureH;

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
                                    "attribute vec2 a_texturecoord;                             \n"
                                    "                                                           \n"
                                    "varying vec3 v_lightspecular;                              \n"
                                    "varying vec3 v_mat_specular;                               \n"
                                    "varying float v_mat_specexp;                               \n"
                                    "varying vec3 v_diffuse;                                    \n"
                                    "varying vec3 v_ambient;                                    \n"
                                    "varying vec3 v_normal;                                     \n"
                                    "varying vec3 v_position;                                   \n"
                                    "varying vec3 v_lightpos;                                   \n"
                                    "varying vec2 v_texturecoord;                               \n"
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
                                    "   v_texturecoord = a_texturecoord;                        \n"
                                    "                                                           \n"
                                    "   gl_Position = u_mvp_matrix * vec4(a_position, 1.0);     \n"
                                    "}                                                          \n"
                                    ;

static const char *fShaderSourcePerPixel =
                                    "                                                           \n"
                                    "uniform vec3 u_viewer;                                     \n"
                                    "uniform sampler2D u_sampler;                               \n"
                                    "                                                           \n"
                                    "varying vec3 v_lightspecular;                              \n"
                                    "varying vec3 v_mat_specular;                               \n"
                                    "varying float v_mat_specexp;                               \n"
                                    "varying vec3 v_diffuse;                                    \n"
                                    "varying vec3 v_ambient;                                    \n"
                                    "varying vec3 v_normal;                                     \n"
                                    "varying vec3 v_position;                                   \n"
                                    "varying vec3 v_lightpos;                                   \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    vec3 n, halfV;                                         \n"
                                    "    float NdotL, NdotHV;                                   \n"
                                    "    vec3 lspec = v_lightspecular;                          \n"
                                    "    vec3 mspec = v_mat_specular;                           \n"
                                    "    float mexp  = v_mat_specexp;                           \n"
                                    "    vec3 color = v_ambient;                                \n"
                                    "    vec3 lightDir = normalize(v_lightpos - v_position);    \n"
                                    "    vec3 halfRVector = normalize(normalize(lightDir) +      \n"
                                    "                       u_viewer);                          \n"
                                    "                                                           \n"
                                    "    n = normalize(v_normal);                               \n"
                                    "    NdotL = max(dot(n, lightDir), 0.0);                    \n"
                                    "                                                           \n"
                                    "    if (NdotL > 0.0) {                                     \n"
                                    "        color += v_diffuse * NdotL;                        \n"
                                    "        halfV = normalize(halfRVector);                     \n"
                                    "        NdotHV = max(dot(n, halfV), 0.0);                  \n"
                                    "        color += mspec *                                   \n"
                                    "                 lspec *                                   \n"
                                    "                 pow(NdotHV, mexp);                        \n"
                                    "    }                                                      \n"
                                    "                                                           \n"
                                    "    vec4 texColor = texture2D(u_sampler, v_texturecoord);  \n"
                                    "                                                           \n"
                                    "    gl_FragColor = vec4(texColor.rgb * color, texColor.a); \n"
                                    "}                                                          \n"
                                    ;

int initTexture()
{
    chikku::RBMPImageHandler bmpImageHandler;
    if (!bmpImageHandler.load(ImageFilename.c_str())) {
        std::cerr << "loading image " << ImageFilename.c_str() << " failed" << std::endl;
        return -1;
    }

    textureH = 0;

    glGenTextures(1, &textureH);

    glBindTexture(GL_TEXTURE_2D, textureH);

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

int initShader()
{
    if (!shaderObject.create(vShaderSourcePerPixel, fShaderSourcePerPixel)) {
        std::cerr << "shader creation failed" << std::endl;
        return -1;
    }

    return 0;
}

int init()
{
    glClearColor(0.0, 0.0, 0.0, 0.0);

    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
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
    if (shaderObject.program() > 0) {
        glUseProgram(shaderObject.program());

        chikku::RTransformation modelRTransformation;
        modelRTransformation.translate(0, 0, -3);
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
        GLint mvp_matrixLoc = glGetUniformLocation(shaderObject.program(), "u_mvp_matrix");
        GLint samplerLoc = glGetUniformLocation(shaderObject.program(), "u_sampler");
        GLint normalLoc = glGetAttribLocation(shaderObject.program(), "a_normal");
        GLint positionLoc = glGetAttribLocation(shaderObject.program(), "a_position");
        GLint texturecoordLoc = glGetAttribLocation(shaderObject.program(), "a_texturecoord");

        chikku::RTransformation mat = modelRTransformation.inverse();
        mat = mat.transpose();

        glUniformMatrix4fv(normalmatrixLocv, 1, GL_FALSE, mat.matrix().data());
        glUniformMatrix4fv(mvp_matrixLoc, 1, GL_FALSE, transformation.matrix().data());
        glVertexAttribPointer(positionLoc, 3, GL_FLOAT, GL_FALSE, 0, sphere.vertices());
        glVertexAttribPointer(normalLoc, 3, GL_FLOAT, GL_FALSE, 0, sphere.normals());
        glVertexAttribPointer(texturecoordLoc, 2, GL_FLOAT, GL_FALSE, 0, sphere.texcoords());

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

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, textureH);
        glUniform1i(samplerLoc, 0);

        glEnableVertexAttribArray(positionLoc);
        glEnableVertexAttribArray(normalLoc);
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

