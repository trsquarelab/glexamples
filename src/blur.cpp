
#include <string>
#include <cstring>
#include <cmath>
#include <sys/timeb.h>
#include <iostream>

#include "rscene.h"
#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rsolidcube.h"
#include "rrectangle.h"
#include "rrectangle.h"
#include "rshader_object.h"
#include "rtexture_object.h"

class BlurRScene: public chikku::RScene
{
private:
    enum MenuItem
    {
        Original,
        Blur,
        OriginalOnBlur,
        BlurOnOriginal
    };

public:
    static chikku::RScene & instance()
    {
        static BlurRScene scene("blur");
        return scene;
    }

private:
    BlurRScene(std::string const &title)
     : chikku::RScene(title),
       mItemSelected(Blur)
    {}

    void createMenu()
    {
        glutAddMenuEntry("Blured", Blur);
        glutAddMenuEntry("Original", Original);
        glutAddMenuEntry("OriginalOnBlurred", OriginalOnBlur);
        glutAddMenuEntry("BlurOnOriginal", BlurOnOriginal);

        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }

    void onMenuItemClicked(int id)
    {
        switch (id) {
            case Original:
            mItemSelected = Original;   
            break;

            case Blur:
            mItemSelected = Blur;
            break;

            case OriginalOnBlur:
            mItemSelected = OriginalOnBlur;
            break;

            case BlurOnOriginal:
            mItemSelected = BlurOnOriginal;
            break;
        }
        glutPostRedisplay();
    }

    void onTimer()
    {
        redraw();
        addTimer();
    }

    void onReshape(int w, int h)
    {
        glMatrixMode(GL_PROJECTION);
        glViewport(0, 0, w, h);

        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();

        mProjection.reset();
        mModelView.reset();

        mProjection.perspective(mFovy, (float)w/(float)h, mNear, mFar);
        mModelView.translate(0.0, 0.0, -5.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();

        destroyRSceneTexture();
        mBlur.reset();
    }

    void onDisplay() const
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (mItemSelected) {
            case Original:
            drawOriginal();
            break;

            case Blur:
            drawBlur();
            break;

            case OriginalOnBlur:
            drawOriginal();
            drawBlur();
            break;

            case BlurOnOriginal:
            drawBlur();
            drawOriginal();
            break;
        }

        glutSwapBuffers();
    }

    void drawOriginal() const
    {
        if (mOriginal.mShader.program() > 0) {
            glUseProgram(mOriginal.mShader.program());

            glActiveTexture(GL_TEXTURE0);

            chikku::RTransformation transformation = mModelView.multiply(mProjection);

            mOriginal.mTexture.bind();
            glUniform1i(mOriginal.mVars["u_sampler"], 0);

            glUniformMatrix4fv(mOriginal.mVars["u_mvp_matrix"], 1, GL_FALSE, transformation.matrix().data());
            glBindBuffer(GL_ARRAY_BUFFER, mOriginal.mBuffer[0]);
            glVertexAttribPointer(mOriginal.mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, mOriginal.mBuffer[1]);
            glVertexAttribPointer(mOriginal.mVars["a_texturecoord"], 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(mOriginal.mVars["a_position"]);
            glEnableVertexAttribArray(mOriginal.mVars["a_texturecoord"]);

            glDrawArrays(mOriginal.mRShape.primitives(), 0, mOriginal.mRShape.verticesCount());

            mOriginal.mTexture.unbind();

            glUseProgram(0);
        }
    }

    void drawBlur() const
    {
        if (!mBlur.mCreatedTexture) {
            const_cast<BlurRScene *>(this)->mBlur.mCreatedTexture = const_cast<BlurRScene *>(this)->createRSceneTexture();
            if (!mBlur.mCreatedTexture) {
                std::cerr << "scene texture creation failed" << std::endl;
                return;
            }
        }

        if (mBlur.mShader.program() > 0) {
            glUseProgram(mBlur.mShader.program());

            glActiveTexture(GL_TEXTURE0);

            glUniform1f(mBlur.mVars["u_blurstep"], mBlur.mStep);
            const_cast<BlurRScene *>(this)->mBlur.mStep = mBlur.mStep * mBlur.mMultiFactor;
            if (mBlur.mStep > 0.005 || mBlur.mStep < 0.0001) {
                const_cast<BlurRScene *>(this)->mBlur.mMultiFactor = 1.0f / mBlur.mMultiFactor;
            }
            

            glUniform1i(mBlur.mVars["u_sampler"], 0);

            glBindTexture(GL_TEXTURE_2D, mBlur.mTexture);

            glBindBuffer(GL_ARRAY_BUFFER, mBlur.mBuffer[0]);
            glVertexAttribPointer(mBlur.mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, mBlur.mBuffer[1]);
            glVertexAttribPointer(mBlur.mVars["a_texturecoord"], 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(mBlur.mVars["a_position"]);
            glEnableVertexAttribArray(mBlur.mVars["a_texturecoord"]);

            glDrawArrays(mBlur.mRShape.primitives(), 0, mBlur.mRShape.verticesCount());

            glBindTexture(GL_TEXTURE_2D, 0);

            glUseProgram(0);
        }
    }

    bool onInit()
    {
        if (!initGL()) {
            return false;
        }

        createMenu();

        return true;
    }

    bool initGL()
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_POINT_SPRITE);
        glEnable(GL_VERTEX_PROGRAM_POINT_SIZE);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBlendFunc(GL_SRC_ALPHA, GL_ONE);

        glTexEnvi(GL_POINT_SPRITE, GL_COORD_REPLACE, GL_TRUE);

        glDepthFunc(GL_LEQUAL);
        glClearDepth(1.0);

        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        if (!mOriginal.mShader.create(mOriginal.mVShaderSource, mOriginal.mFShaderSource)) {
            return false;
        }

        if (!mBlur.mShader.create(mBlur.mVShaderSource, mBlur.mFShaderSource)) {
            return false;
        }

        if (!mOriginal.mTexture.set("../data/image2.bmp")) {
            return false;
        }

        if (!initBuffer()) {
            std::cerr << "initializing buffer failed" << std::endl;
            return false;
        }

        if (!retrieveVarLoc()) {
            return false;
        }

        return true;
    }

    bool initBuffer()
    {
        glGenBuffers(2, mOriginal.mBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, mOriginal.mBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, mOriginal.mRShape.verticesSize(),
                         mOriginal.mRShape.vertices(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mOriginal.mBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, mOriginal.mRShape.texcoordsSize(),
                         mOriginal.mRShape.texcoords(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(2, mBlur.mBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, mBlur.mBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, mBlur.mRShape.verticesSize(),
                         mBlur.mRShape.vertices(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mBlur.mBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, mBlur.mRShape.texcoordsSize(),
                         mBlur.mRShape.texcoords(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return true;
    }

    bool retrieveVarLoc()
    {
    
        mOriginal.mVars = mOriginal.mShader.getVariables();

        if (mOriginal.mVars.empty()) {
            std::cerr << "variable retrieval failed" << std::endl;
            return false;
        }

        mBlur.mVars = mBlur.mShader.getVariables();

        if (mBlur.mVars.empty()) {
            std::cerr << "variable retrieval failed" << std::endl;
            return false;
        }

        return true;
    }

    bool createRSceneTexture()
    {
        glGenFramebuffers(1, &mBlur.mFrameBuffer);
        glGenRenderbuffers(1, &mBlur.mDepthRenderbuffer);
        glGenTextures(1, &mBlur.mTexture);

        glBindTexture(GL_TEXTURE_2D, mBlur.mTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mW, mH,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindRenderbuffer(GL_RENDERBUFFER, mBlur.mDepthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mW, mH);

        glBindFramebuffer(GL_FRAMEBUFFER, mBlur.mFrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mBlur.mTexture, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, mBlur.mDepthRenderbuffer);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glBindRenderbuffer(GL_FRAMEBUFFER, mBlur.mFrameBuffer);

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        drawOriginal();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return true;
    }

    void destroyRSceneTexture()
    {
        glDeleteRenderbuffers(1, &mBlur.mDepthRenderbuffer);
        glDeleteFramebuffers(1, &mBlur.mFrameBuffer);
        glDeleteTextures(1, &mBlur.mTexture);
    }

private:
    class OriginalState
    {
    public:
        OriginalState()
    	 : mRShape(-1.0f, -1.0f, 1.0f, 1.0f)
        {}

        chikku::RShaderObject mShader;

        chikku::RTextureObject mTexture;

        GLuint mBuffer[2];

        chikku::RRectangle mRShape;

        chikku::RShaderVariables mVars;

        static const char * mVShaderSource;
        static const char * mFShaderSource;
    };

    class BlurState
    {
    public:
        BlurState()
         : mRShape(-1.0f, -1.0f, 1.0f, 1.0f),
           mCreatedTexture(false),
           mStep(0.0001f),
           mMultiFactor(1.03f)
        {}

        void reset()
        {
            mCreatedTexture = false;
        }

    public:
        chikku::RShaderObject mShader;

        GLuint mBuffer[2];

        GLuint mTexture;
        GLuint mFrameBuffer;
        GLuint mDepthRenderbuffer;

        chikku::RShaderVariables mVars;

        chikku::RRectangle mRShape;
 
        bool mCreatedTexture;

        float mStep;
        float mMultiFactor;

        static const char * mVShaderSource;
        static const char * mFShaderSource;
    };

private:
    chikku::RTransformation mProjection;
    chikku::RTransformation mModelView;

    OriginalState mOriginal;
    BlurState mBlur;

    MenuItem mItemSelected;

private:
    static const float mNear;
    static const float mFar;
    static const float mFovy;
};

const float BlurRScene::mNear = 0.0f;
const float BlurRScene::mFar = 20.0f;
const float BlurRScene::mFovy = 60.0f;

const char * BlurRScene::OriginalState::mVShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
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

const char * BlurRScene::OriginalState::mFShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
                                    "uniform sampler2D u_sampler;                               \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    gl_FragColor = texture2D(u_sampler, v_texturecoord);   \n"
                                    "}                                                          \n"
                                    ;


const char * BlurRScene::BlurState::mVShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
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
                                    "   gl_Position = vec4(a_position, 1.0);                    \n"
                                    "}                                                          \n"
                                    ;

const char * BlurRScene::BlurState::mFShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
                                    "uniform sampler2D u_sampler;                               \n"
                                    "uniform float u_blurstep;                                  \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    vec4 sample0;                                          \n"
                                    "    vec4 sample1;                                          \n"
                                    "    vec4 sample2;                                          \n"
                                    "    vec4 sample3;                                          \n"
                                    "                                                           \n"
                                    "    float step = u_blurstep;                               \n"
                                    "                                                           \n"
                                    "    sample0 = texture2D(u_sampler,                         \n"
                                    "                        vec2(v_texturecoord.x - step,      \n"
                                    "                        v_texturecoord.y - step));         \n"
                                    "                                                           \n"
                                    "    sample1 = texture2D(u_sampler,                         \n"
                                    "                        vec2(v_texturecoord.x + step,      \n"
                                    "                        v_texturecoord.y - step));         \n"
                                    "                                                           \n"
                                    "    sample2 = texture2D(u_sampler,                         \n"
                                    "                        vec2(v_texturecoord.x - step,      \n"
                                    "                        v_texturecoord.y + step));         \n"
                                    "                                                           \n"
                                    "    sample3 = texture2D(u_sampler,                         \n"
                                    "                        vec2(v_texturecoord.x + step,      \n"
                                    "                        v_texturecoord.y + step));         \n"
                                    "                                                           \n"
                                    "    gl_FragColor = (sample0 + sample1 + sample2 + sample3) \n"
                                    "                               / 4.0;                      \n"
                                    "}                                                          \n"
                                    ;

chikku::RScene & chikku::RScene::instance()
{
    return BlurRScene::instance();
}

int main(int argc, char ** argv)
{
    chikku::RScene *scene = chikku::RScene::create(argc, argv);
    if (!scene) {
        std::cerr << "scene initialization failed" << std::endl;
        return 1;
    }

    return scene->exec();
}

