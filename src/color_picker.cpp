
#include <cstring>
#include <cmath>

#include <iostream>
#include <string>
#include <list>

#include "rscene.h"
#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rshader_object.h"
#include "rbmp_image_handler.h"
#include "rsmart_pointer.h"

class RSceneObject
{
private:
    enum State
    {
        None,
        Rotate
    };

public:
    RSceneObject()
     : mId(0),
       mTexture(0),
       mRotationAngle(0.0f),
       mState(Rotate)
    {
        mId = getId();
        init();
    }

    ~RSceneObject()
    {
        terminate();
    }

    chikku::RTransformation transformation() const
    {
        return mRTransformation;
    }

    void setRTransformation(const chikku::RTransformation & t)
    {
        mRTransformation = t;
    }

    int setTexture(const std::string & filename)
    {
        glGenTextures(1, &mTexture);

        chikku::RBMPImageHandler bmpImageHandler;
        if (!bmpImageHandler.load(filename.c_str())) {
            std::cerr << "loading image " << filename.c_str() << " failed" << std::endl;
            return -1;
        }

        glBindTexture(GL_TEXTURE_2D, mTexture);

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

    GLuint texture() const
    {
        return mTexture;
    }

    GLuint vertBuffer() const
    {
        return RSceneObject::mBuffer[0];
    }

    GLuint texBuffer() const
    {
        return RSceneObject::mBuffer[1];
    }

    chikku::RSolidSphere & shape()
    {
        return RSceneObject::mRShape;
    }

    const chikku::RSolidSphere & shape() const
    {
        return RSceneObject::mRShape;
    }

    float rotation() const
    {
        return mRotationAngle;
    }

    void update()
    {
        if (mState == Rotate) {
            mRTransformation.rotate(-mRotationAngle, 0.0f, 1.0f, 0.0f);
            mRotationAngle += 1.0f;
            if (mRotationAngle >= 360.0f) {
                mRotationAngle = 0.0f;
            }
            mRTransformation.rotate(mRotationAngle, 0.0f, 1.0f, 0.0f);
        }
    }

    void onMouseEvent(int button, int state, int x, int y)
    {
        if (state == GLUT_DOWN) {
            if (mState == Rotate) {
                mState = None;
            } else {
                mState = Rotate;
            }
        } else if (state == GLUT_UP) {
        }
    }

private:
    static unsigned int getId()
    {
        static unsigned int uniqueId = 0;
        return ++uniqueId;
    }

private:
    void terminate()
    {
        glDeleteTextures(1, &mTexture);
    }

    void init()
    {
        if (!RSceneObject::mInitialized) {
            glGenBuffers(2, RSceneObject::mBuffer);

            glBindBuffer(GL_ARRAY_BUFFER, RSceneObject::mBuffer[0]);
            glBufferData(GL_ARRAY_BUFFER, RSceneObject::mRShape.verticesSize(),
                         RSceneObject::mRShape.vertices(), GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, RSceneObject::mBuffer[1]);
            glBufferData(GL_ARRAY_BUFFER, RSceneObject::mRShape.texcoordsSize(),
                         RSceneObject::mRShape.texcoords(), GL_STATIC_DRAW);

            glBindBuffer(GL_ARRAY_BUFFER, 0);

            RSceneObject::mInitialized = true;
        }
    }

private:
    unsigned int mId;
    GLuint mTexture;
    chikku::RTransformation mRTransformation;
    float mRotationAngle;
    State mState;
    float mBlurStep;

public:
    static bool mInitialized;
    static GLuint mBuffer[2];
    static chikku::RSolidSphere mRShape;
};

bool RSceneObject::mInitialized = false;
GLuint RSceneObject::mBuffer[2];
chikku::RSolidSphere RSceneObject::mRShape = chikku::RSolidSphere(1.0f);

class ColorPickerRScene: public chikku::RScene
{
public:
    static ColorPickerRScene & instance()
    {
        static ColorPickerRScene scene("color picker");
        return scene;
    }
    
private:
    ColorPickerRScene(std::string const & title)
     : RScene(title)
    {}

    ~ColorPickerRScene()
    {
        terminate();
    }

    bool onInit()
    {
        glClearColor(0.0, 0.0, 0.0, 0.0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);

        glDepthFunc(GL_LEQUAL);
        glClearDepth(1.0);

        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        if (mOriginal.mShader.create(mOriginal.mVShaderSource, mOriginal.mFShaderSource) < 0) {
            return false;
        }
        mOriginal.mVars = mOriginal.mShader.getVariables();

        if (mChoose.mShader.create(mChoose.mVShaderSource, mChoose.mFShaderSource) < 0) {
            return false;
        }
        mChoose.mVars = mChoose.mShader.getVariables();

        const std::string TextureFilemames[] =
                                        {
                                            std::string("../data/image1.bmp"),
                                            std::string("../data/image2.bmp"),
                                            std::string("../data/image3.bmp"),
                                            std::string("../data/image4.bmp")
                                        };

        const float XPos[] = {-1.5f, 1.5f, -1.5f,  1.5f};
        const float YPos[] = { 1.5f, 1.5f, -1.5f, -1.5f};
        const float ZPos[] = { 0.0f, 0.0f, -0.0f, -0.0f};

        for (unsigned int i=0; i<sizeof(TextureFilemames)/sizeof(TextureFilemames[0]); ++i) {
            RSceneObject *item = new RSceneObject();
            if (item->setTexture(TextureFilemames[i]) < 0) {
                return -1;
            }

            chikku::RTransformation t;
            t.translate(0, 0, -5);
            t.scale(1.0f/3.0f, 1.0f/3.0f, 1.0f/3.0f);
            t.translate(XPos[i], YPos[i], ZPos[i]);
            item->setRTransformation(t);

            mItems.push_back(item);
        }

        return true;
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

        const float near = 1.0f;
        const float far = 20.0f;
        const float fovy = 45.0f;

        mProjection.perspective(fovy, (float)w/(float)h, near, far);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void onDisplay() const
    {
        drawRScene();
    }

    void onMouseEvent(int button, int state, int x, int y)
    {
        RSceneObject * item = getObject(x, y);
        if (item) {
            item->onMouseEvent(button, state, x, y);
        }
    }

private:
    void drawRScene() const
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (RSceneItemsConstIter iter = mItems.begin(); iter != mItems.end(); ++iter) {
            drawOriginal(*iter);
        }

        glutSwapBuffers();
    }

    void drawOriginal(const RSceneObject * item) const
    {
        if (mOriginal.mShader.program() > 0) {
            glUseProgram(mOriginal.mShader.program());

            glActiveTexture(GL_TEXTURE0);

            chikku::RTransformation transformation = item->transformation().multiply(mProjection);

            glBindTexture(GL_TEXTURE_2D, item->texture());
            glUniform1i(mOriginal.mVars["u_sampler"], 0);

            glUniformMatrix4fv(mOriginal.mVars["u_mvp_matrix"], 1, GL_FALSE, transformation.matrix().data());
            glBindBuffer(GL_ARRAY_BUFFER, item->vertBuffer());
            glVertexAttribPointer(mOriginal.mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, item->texBuffer());
            glVertexAttribPointer(mOriginal.mVars["a_texturecoord"], 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(mOriginal.mVars["a_position"]);
            glEnableVertexAttribArray(mOriginal.mVars["a_texturecoord"]);

            glDrawArrays(GL_TRIANGLES, 0, item->shape().verticesCount());

            const_cast<RSceneObject *>(item)->update();

            glBindTexture(GL_TEXTURE_2D, 0);

            glUseProgram(0);
        }
    }

    class ChooseData
    {
    public:
        ChooseData(unsigned char r=0, unsigned char g=0,
                   unsigned char b=0, RSceneObject * o=0)
         : mR(r),
           mG(g),
           mB(b),
           mObject(o)
        {}

        unsigned char mR;
        unsigned char mG;
        unsigned char mB;
        RSceneObject * mObject;
    };

    RSceneObject * getObject(int px, int py)
    {
        RSceneObject * clickedItem = 0;

        if (!mChoose.mCreated) {
            glGenFramebuffers(1, &mChoose.mFrameBuffer);
            glGenRenderbuffers(1, &mChoose.mDepthRenderbuffer);
            glGenTextures(1, &mChoose.mTexture);

            glBindTexture(GL_TEXTURE_2D, mChoose.mTexture);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mW, mH,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            glBindRenderbuffer(GL_RENDERBUFFER, mChoose.mDepthRenderbuffer);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mW, mH);

            glBindFramebuffer(GL_FRAMEBUFFER, mChoose.mFrameBuffer);
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                                   mChoose.mTexture, 0);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                      GL_RENDERBUFFER, mChoose.mDepthRenderbuffer);

            glBindTexture(GL_TEXTURE_2D, 0);
            glBindRenderbuffer(GL_RENDERBUFFER, 0);
            glBindFramebuffer(GL_FRAMEBUFFER, 0);

            mChoose.mCreated = true;
        }

        glBindFramebuffer(GL_FRAMEBUFFER, mChoose.mFrameBuffer);

        if (mChoose.mShader.program() > 0) {
            static const unsigned char Increment = 1;

            unsigned char r = 0;
            unsigned char g = 0;
            unsigned char b = 0;

            typedef std::list<ChooseData> ObjectMap;
            typedef ObjectMap::iterator ObjectMapIter;

            ObjectMap objects;
            for (RSceneItemsIter iter = mItems.begin(); iter != mItems.end(); ++iter) {
                r += Increment;
                if (r >= 255) {
                    r = 0;
                    g += Increment;
                }

                if (g >= 255) {
                    g = 0;
                    b += Increment;
                }

                objects.push_back(ChooseData(r, g, b, *iter));
            }

            glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            glUseProgram(mChoose.mShader.program());

            for (ObjectMapIter iter = objects.begin(); iter != objects.end(); ++iter) {
                RSceneObject * item = iter->mObject;

                chikku::RTransformation transformation = item->transformation().multiply(mProjection);

                glUniformMatrix4fv(mChoose.mVars["u_mvp_matrix"], 1, GL_FALSE, transformation.matrix().data());
                glUniform4f(mChoose.mVars["u_color"], (float)iter->mR/255.0f, (float)iter->mG/255.0f, (float)iter->mB/255.0f, 1.0f);

                glBindBuffer(GL_ARRAY_BUFFER, item->vertBuffer());
                glVertexAttribPointer(mChoose.mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
                glBindBuffer(GL_ARRAY_BUFFER, 0);

                glEnableVertexAttribArray(mChoose.mVars["a_position"]);

                glDrawArrays(GL_TRIANGLES, 0, item->shape().verticesCount());
            }

            glFinish();

            if (px != 0) {
                unsigned char data[4] = {0.0f};
                glReadPixels(px, mH-py, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, data);
                for (ObjectMapIter iter = objects.begin(); iter != objects.end(); ++iter) {
                    if (iter->mR == data[0] &&
                        iter->mG == data[1] &&
                        iter->mB == data[2]) {
                        clickedItem = iter->mObject;
                        break;
                    }
                }
            }
            glUseProgram(0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        return clickedItem;
    }

    void terminate()
    {
        mItems.clear();
    }

private:
    typedef std::vector<chikku::RSmartPointer<RSceneObject> > RSceneItems;
    typedef RSceneItems::iterator RSceneItemsIter;
    typedef RSceneItems::const_iterator RSceneItemsConstIter;

    RSceneItems mItems;
    chikku::RTransformation mProjection;

    class StateOriginal
    {
    public:
        chikku::RShaderObject mShader;
        chikku::RShaderVariables mVars;

        static const char *mVShaderSource;
        static const char *mFShaderSource;
    };

    class StateChoose
    {
    public:
        StateChoose()
         : mFrameBuffer(0),
           mDepthRenderbuffer(0),
           mTexture(0),
           mCreated(false)
        {}

        chikku::RShaderObject mShader;
        chikku::RShaderVariables mVars;
        GLuint mFrameBuffer;
        GLuint mDepthRenderbuffer;
        GLuint mTexture;
        bool mCreated;

        static const char *mVShaderSource;
        static const char *mFShaderSource;
    };

    StateOriginal mOriginal;
    StateChoose mChoose;
};

const char * ColorPickerRScene::StateOriginal::mVShaderSource =
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

const char * ColorPickerRScene::StateOriginal::mFShaderSource =
                                    "uniform sampler2D u_sampler;                               \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    gl_FragColor = texture2D(u_sampler, v_texturecoord);   \n"
                                    "}                                                          \n"
                                ;

const char * ColorPickerRScene::StateChoose::mVShaderSource =
                                    "uniform mat4 u_mvp_matrix;                                 \n"
                                    "                                                           \n"
                                    "attribute vec3 a_position;                                 \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "   gl_Position = u_mvp_matrix * vec4(a_position, 1.0);     \n"
                                    "}                                                          \n"
                                ;

const char * ColorPickerRScene::StateChoose::mFShaderSource =
                                    "                                                           \n"
                                    "uniform vec4 u_color;                                      \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    gl_FragColor = u_color;                                \n"
                                    "}                                                          \n"
                                ;

chikku::RScene & chikku::RScene::instance()
{
    return ColorPickerRScene::instance();
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

