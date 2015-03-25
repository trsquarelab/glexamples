
#include <GL/glut.h>
#include <iostream>
#include <string>

class TextureRegion {
private:
    int mTextureWidth;
    int mTextureHeight;
    int mX;
    int mY;
    int mWidth;
    int mHeight;
    float mMatrix[3*3];
    
public:
    TextureRegion()
    {}
    TextureRegion(int textureWidth, int textureHeight, int x, int y, int width, int height) {
        mTextureWidth = textureWidth;
        mTextureHeight = textureHeight;
        
        if (x < 0) {
            x = 0;
        }
        if (x < 0) {
            x = 0;
        }
        
        if (width > textureWidth) {
            width = textureWidth;
        }
        
        if (height > textureHeight) {
            height = textureHeight;
        }

        mX = x;
        mY = y;
        mWidth = width;
        mHeight = height;

        createMatrix();            
    }
    const float * getMatrix() {
        return mMatrix;
    }
private:
    void createMatrix() {
        // translation factor
        float tx = (float)(mX - 0) / (float)mTextureWidth; 
        float ty = (float)(mY - 0) / (float)mTextureHeight;
        
        // scaling factor
        float sx = (float)mWidth / (float)mTextureWidth; 
        float sy = (float)mHeight / (float)mTextureHeight;

        // 3x3 matrix
        float mat[] = {
                sx, 0,   0,
                0,   sy, 0,
                tx, ty, 1.0f
        };
        std::copy(mat, mat+9, mMatrix);
    }
};

bool init();
void display();
void reshape(int w, int h);
int loadShader(int shaderType, std::string pSource);
int createProgram(std::string vertexSource, std::string fragmentSource);

static const std::string VertexShader =
    "attribute vec4 aPosition;\n"
    "attribute vec2 aTexCoord;\n"
    "varying vec2 vTexCoord;\n"
    "void main() {\n"
    "    vTexCoord = aTexCoord;\n"
    "    gl_Position = aPosition;\n"
    "}\n";

static const std::string FragmentShader =
    "#ifdef GL_ES\n"
    "precision mediump float;\n"
    "#endif\n"
    "varying vec2 vTexCoord;\n"
    "uniform mat3 uTextureMatrix;\n"
    "uniform vec4 uColor;\n"
    "uniform sampler2D uTexture;\n"
    "void main() {\n"
    "    vec3 texCoord = uTextureMatrix * vec3(vTexCoord.x, vTexCoord.y, 1.0f);\n"
    "    gl_FragColor = uColor * texture2D(uTexture, texCoord.xy);\n"
    "}\n";

int mProgram;

GLuint mTextureId;

int mPosLoc;
int mTexCoordLoc;
int mTexMatrixLoc;
int mSampleLoc;
int mColorLoc;

float mVerticesBuffer[8];
float mTexcoordBuffer[8];

TextureRegion mTextureRegion;

int loadShader(int shaderType, std::string pSource)
{
    int shader = glCreateShader(shaderType);
    if (shader != 0) {
        const char *vsSource  = pSource.c_str();
        glShaderSource(shader, 1, &vsSource, 0);
        glCompileShader(shader);
        // not interested in checking status
    }

    return shader;
}

int createProgram(std::string vertexSource, std::string fragmentSource)
{
    int vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (vertexShader == 0) {
        return 0;
    }

    int pixelShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
    if (pixelShader == 0) {
        return 0;
    }

    int program = glCreateProgram();
    if (program != 0) {
        glAttachShader(program, vertexShader);
        glAttachShader(program, pixelShader);
        glLinkProgram(program);
        // not interested in checking status
    }
    return program;
}

bool init()
{
    glClearColor(0.0f, 1.0f, 1.0f, 1.0f);

    mProgram = createProgram(VertexShader, FragmentShader);

    // retrieve locations
    glUseProgram(mProgram);

    mPosLoc = glGetAttribLocation(mProgram, "aPosition");
    mTexCoordLoc = glGetAttribLocation(mProgram, "aTexCoord");
    mSampleLoc = glGetUniformLocation(mProgram, "uTexture");
    mColorLoc = glGetUniformLocation(mProgram, "uColor");
    mTexMatrixLoc = glGetUniformLocation(mProgram, "uTextureMatrix");

    GLubyte texdata[] = {
        0xFF, 0x00, 0x00, 0xFF,
        0x00, 0xFF, 0x00, 0xFF,
        0x00, 0x00, 0xFF, 0xFF,
        0xFF, 0xFF, 0x00, 0xFF
    };

    glGenTextures(1, &mTextureId);

    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, texdata);

    // first color
    mTextureRegion = TextureRegion(2, 2, 1, 1, 1, 1);

    glEnable(GL_TEXTURE_2D);

    // create vertices buffer
    float vertices[] = {
        -0.75f, -0.75f,
        0.75f, -0.75f,
        0.75f, 0.75f,
        -0.75f, 0.75f
    };

    std::copy(vertices, vertices+8, mVerticesBuffer);

    // create texcoord buffer
    float texcoords[] = {
        0, 1,
        1, 1,
        1, 0,
        0, 0
    };

    std::copy(texcoords, texcoords+8, mTexcoordBuffer);

    glEnable(GL_BLEND);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    return true;
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glUseProgram(mProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, mTextureId);
    glUniform1i(mSampleLoc, 0);

    glUniform4f(mColorLoc, 1.0f, 1.0f, 1.0f, 1.0f);

    glVertexAttribPointer(mPosLoc, 2, GL_FLOAT, false, 0, mVerticesBuffer);
    glVertexAttribPointer(mTexCoordLoc, 2, GL_FLOAT, false, 0, mTexcoordBuffer);

    glUniformMatrix3fv(mTexMatrixLoc, 1, false, mTextureRegion.getMatrix());

    glEnableVertexAttribArray(mPosLoc);
    glEnableVertexAttribArray(mTexCoordLoc);

    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);

    glutSwapBuffers();
}

void reshape(int w, int h)
{
    glViewport(0, 0, w, h);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowPosition(100, 100);
    glutInitWindowSize(320, 420);

    glutCreateWindow("Texture region example");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);

    if (init()) {
        glutMainLoop();
    } else {
        std::cerr << "Initialization failed" << std::endl;
        return 1;
    }

    return 0;
}

