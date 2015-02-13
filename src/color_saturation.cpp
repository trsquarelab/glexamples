
#include <string>
#include <cstring>
#include <cmath>
#include <sys/timeb.h>
#include <iostream>

#include "rscene.h"
#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rrectangle.h"
#include "rshader_object.h"
#include "rtexture_object.h"

class ColorSaturationRScene: public chikku::RScene
{
private:
    enum MenuItem
    {
        Original,
        OriginalCompliment,
        GrayScale,
        GrayScaleCompliment
    };

private:
    class StateCommon
    {
    public:
        StateCommon()
    	 : mRShape(-1.0f, -1.0f, 1.0f, 1.0f),
           mCompliment(0)
    	{}

        chikku::RShaderObject mShader;

        chikku::RShaderVariables mVars;

        chikku::RTextureObject mTexture;

        GLuint mBuffer[2];

        chikku::RRectangle mRShape;

        unsigned char mCompliment;
    };

    class OriginalState: public StateCommon
    {
    public:
    	OriginalState()
    	{}
    	
        static const char * mVShaderSource;
        static const char * mFShaderSource;
    };

    class GrayState: public StateCommon
    {
    public:
    	GrayState()
    	{}
    	
        static const char * mVShaderSource;
        static const char * mFShaderSource;
    };

public:
    static chikku::RScene & instance()
    {
        static ColorSaturationRScene scene("color saturation");
        return scene;
    }

private:
    ColorSaturationRScene(std::string const &title)
     : chikku::RScene(title),
       mItemSelected(Original)
    {}

    void createMenu()
    {
        glutAddMenuEntry("Original", Original);
        glutAddMenuEntry("Original Compliment", OriginalCompliment);
        glutAddMenuEntry("Gray Scale", GrayScale);
        glutAddMenuEntry("Gray Scale Compliment", GrayScaleCompliment);

        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }

    void onMenuItemClicked(int id)
    {
        switch (id) {
            case Original:
            mItemSelected = Original;   
            mOriginal.mCompliment = 0;
            break;

            case GrayScale:
            mItemSelected = GrayScale;
            mGray.mCompliment = 0;
            break;

            case OriginalCompliment:
            mItemSelected = Original;   
            mOriginal.mCompliment = 1;
            break;

            case GrayScaleCompliment:
            mItemSelected = GrayScale;
            mGray.mCompliment = 1;
            break;
        }
        glutPostRedisplay();
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
    }

    void onDisplay() const
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        switch (mItemSelected) {
            case Original:
            case OriginalCompliment:
            draw(mOriginal);
            break;

            case GrayScale:
            case GrayScaleCompliment:
            draw(mGray);
            break;
        }

        glutSwapBuffers();
    }

    void draw(const StateCommon & state) const
    {
        if (state.mShader.program() > 0) {
            glUseProgram(state.mShader.program());

            glActiveTexture(GL_TEXTURE0);

            chikku::RTransformation transformation = mModelView.multiply(mProjection);

            state.mTexture.bind();
            glUniform1i(state.mVars["u_sampler"], 0);
            glUniform1i(state.mVars["u_compliment"], state.mCompliment);

            glUniformMatrix4fv(state.mVars["u_mvp_matrix"], 1, GL_FALSE, transformation.matrix().data());
            glBindBuffer(GL_ARRAY_BUFFER, state.mBuffer[0]);
            glVertexAttribPointer(state.mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, state.mBuffer[1]);
            glVertexAttribPointer(state.mVars["a_texturecoord"], 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(state.mVars["a_position"]);
            glEnableVertexAttribArray(state.mVars["a_texturecoord"]);

            glDrawArrays(state.mRShape.primitives(), 0, state.mRShape.verticesCount());

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

        if (!mGray.mShader.create(mGray.mVShaderSource, mGray.mFShaderSource)) {
            return false;
        }



        if (!mOriginal.mTexture.set("../data/image2.bmp")) {
            return false;
        }

        if (!mGray.mTexture.set("../data/image2.bmp")) {
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

        glGenBuffers(2, mGray.mBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, mGray.mBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, mGray.mRShape.verticesSize(),
        		mGray.mRShape.vertices(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mGray.mBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, mGray.mRShape.texcoordsSize(),
        		mGray.mRShape.texcoords(), GL_STATIC_DRAW);

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

        mGray.mVars = mGray.mShader.getVariables();

        if (mGray.mVars.empty()) {
            std::cerr << "variable retrieval failed" << std::endl;
            return false;
        }

        return true;
    }

private:
    chikku::RTransformation mProjection;
    chikku::RTransformation mModelView;

    OriginalState mOriginal;
    GrayState mGray;

    MenuItem mItemSelected;

private:
    static const float mNear;
    static const float mFar;
    static const float mFovy;
};

const float ColorSaturationRScene::mNear = 0.0f;
const float ColorSaturationRScene::mFar = 20.0f;
const float ColorSaturationRScene::mFovy = 60.0f;

const char * ColorSaturationRScene::OriginalState::mVShaderSource =
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

const char * ColorSaturationRScene::OriginalState::mFShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
                                    "uniform sampler2D u_sampler;                               \n"
                                    "uniform int u_compliment;                                  \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    vec4 c = texture2D(u_sampler, v_texturecoord);         \n"
                                    "    if (u_compliment == 1) {                               \n"
                                    "       c.r = 1.0-c.r; c.g=1.0-c.g;c.b=1.0-c.b;             \n"
                                    "    }                                                      \n"
                                    "    gl_FragColor = c;                                      \n"
                                    "}                                                          \n"
                                    ;

const char * ColorSaturationRScene::GrayState::mVShaderSource =
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

const char * ColorSaturationRScene::GrayState::mFShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
                                    "uniform sampler2D u_sampler;                               \n"
                                    "uniform int u_compliment;                                  \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    vec4 clr = texture2D(u_sampler, v_texturecoord);       \n"
		                            "    float l = clr.r*0.299 + clr.g*0.587 + clr.b*0.114;     \n"
		                            "    l = clamp(l, 0.0, 1.0);                                \n"
                                    "    if (u_compliment == 1) {                               \n"
                                    "        l = 1.0 - l;                                       \n"
                                    "    }                                                      \n"
                                    "    gl_FragColor = vec4(l, l, l, clr.a);                   \n"
                                    "}                                                          \n"
                                    ;


chikku::RScene & chikku::RScene::instance()
{
    return ColorSaturationRScene::instance();
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

