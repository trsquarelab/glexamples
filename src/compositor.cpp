
#include <string>
#include <cstring>
#include <cmath>
#include <sys/timeb.h>
#include <iostream>

#include "rscene.h"
#include "rtransformation.h"
#include "rcommon.h"
#include "rrectangle.h"
#include "rshader_object.h"
#include "rtexture_object.h"
#include "rsmart_pointer.h"

class RSceneObject
{
private:
    class State
    {
    public:
        State()
         : mRShape(-1.0f, -1.0f, 1.0f, 1.0f),
           mCompliment(0)
        {}

        chikku::RShaderObject mShader;
        
        chikku::RTextureObject mTexture;

        GLuint mBuffer[2];

        chikku::RRectangle mRShape;

        chikku::RShaderVariables mVars;

        unsigned char mCompliment;

        static const char * mVShaderSource;
        static const char * mFShaderSource;
    };

public:
    RSceneObject(std::string imgfilename, int w, int h)
     : mImageFilename(imgfilename),
       mRSceneCreated(false),
       mW(w),
       mH(h)
    {}
    
    ~RSceneObject()
    {
        destroy();
    }
    
    void setSize(int w, int h)
    {
        mW = w;
        mH = h;
        destroyRScene();
    }
    
    bool create()
    {
        return initGL();
    }

    void setRTransformation(chikku::RTransformation const & t)
    {
        mRTransformation = t;
    }
    
    GLuint getRScene()
    {
        if (!mRSceneCreated) {
            createRScene();
        }
        
        return mTexture;
    }
    
    void dropRScene()
    {
        destroyRScene();
    }
    
private:
    bool initGL()
    {
        if (!mState.mShader.create(mState.mVShaderSource, mState.mFShaderSource)) {
            return false;
        }

        if (!mState.mTexture.set(mImageFilename)) {
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
        glGenBuffers(2, mState.mBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, mState.mBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, mState.mRShape.verticesSize(),
                         mState.mRShape.vertices(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mState.mBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, mState.mRShape.texcoordsSize(),
                         mState.mRShape.texcoords(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return true;
    }

    bool retrieveVarLoc()
    {
    	mState.mVars = mState.mShader.getVariables();
    	
    	return !mState.mVars.empty();
    }
    
    void destroy()
    {
        destroyRScene();
        
        glDeleteBuffers(2, mState.mBuffer);

    }
    
    void draw() const
    {
        if (mState.mShader.program() > 0) {
            glUseProgram(mState.mShader.program());

            glActiveTexture(GL_TEXTURE0);

            mState.mTexture.bind();
            glUniform1i(mState.mVars["u_sampler"], 0);
            glUniform1i(mState.mVars["u_compliment"], mState.mCompliment);

            glUniformMatrix4fv(mState.mVars["u_mvp_matrix"], 1, GL_FALSE, mRTransformation.matrix().data());
            glBindBuffer(GL_ARRAY_BUFFER, mState.mBuffer[0]);
            glVertexAttribPointer(mState.mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, mState.mBuffer[1]);
            glVertexAttribPointer(mState.mVars["a_texturecoord"], 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glEnableVertexAttribArray(mState.mVars["a_position"]);
            glEnableVertexAttribArray(mState.mVars["a_texturecoord"]);

            glDrawArrays(mState.mRShape.primitives(), 0, mState.mRShape.verticesCount());

            glBindTexture(GL_TEXTURE_2D, 0);

            glUseProgram(0);
        }
    }
    
    bool createRScene()
    {
        glGenFramebuffers(1, &mFrameBuffer);
        glGenRenderbuffers(1, &mDepthRenderbuffer);
        glGenTextures(1, &mTexture);

        glBindTexture(GL_TEXTURE_2D, mTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mW, mH,
                         0, GL_RGBA, GL_UNSIGNED_BYTE, 0);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindRenderbuffer(GL_RENDERBUFFER, mDepthRenderbuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, mW, mH);

        glBindFramebuffer(GL_FRAMEBUFFER, mFrameBuffer);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                               mTexture, 0);

        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                                  GL_RENDERBUFFER, mDepthRenderbuffer);

        glBindTexture(GL_TEXTURE_2D, 0);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        glBindRenderbuffer(GL_FRAMEBUFFER, mFrameBuffer);

        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        draw();

        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        mRSceneCreated = true;
        
        return true;
    }

    bool destroyRScene()
    {
        if (mRSceneCreated) {
            glDeleteRenderbuffers(1, &mDepthRenderbuffer);
            glDeleteFramebuffers(1, &mFrameBuffer);
            glDeleteTextures(1, &mTexture);

            mRSceneCreated = false;
        }
        return true;
    }
    
private:
    std::string mImageFilename;
    State mState;
    GLuint mTexture;
    GLuint mFrameBuffer;
    GLuint mDepthRenderbuffer;
    bool mRSceneCreated;
    chikku::RTransformation mRTransformation;
    int mW;
    int mH;
};

const char * RSceneObject::State::mVShaderSource =
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

const char * RSceneObject::State::mFShaderSource =
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

class RSceneObjectList
{
public:
    typedef std::vector<chikku::RSmartPointer<RSceneObject> > RSceneObjects;
    typedef RSceneObjects::iterator RSceneObjectsIter;
    typedef RSceneObjects::const_iterator RSceneObjectsConstIter;
    
public:
    RSceneObjectList()
     : mCurrentItem(0)
    {}
    
    RSceneObjects & objects()
    {
        return mObjects;
    }
    
    void add(RSceneObject *o)
    {
        mObjects.push_back(o);
    }
    
    RSceneObject * getCurrentItem()
    {
        return mObjects[mCurrentItem];
    }

    RSceneObject * get(int index)
    {
        return mObjects[index];
    }

    void moveNext()
    {
        if (!mObjects.empty()) {
            ++mCurrentItem;
            if (mCurrentItem == mObjects.size()) {
                mCurrentItem = 0;
            }
        }
    }
    
    RSceneObject * getNextItem()
    {
        int index = 0;
        if (mCurrentItem+1 < mObjects.size()) {
            index = mCurrentItem + 1;
        }
        return mObjects[index];
    }

    RSceneObject * getPreviousItem()
    {
        int index = (int)mCurrentItem;
        index -= 1;
        if (index < 0) {
            index = mObjects.size()-1;
        }
        return mObjects[index];
    }

private:
    RSceneObjectList(RSceneObjectList const &);
    RSceneObjectList& operator=(RSceneObjectList const &);
    
private:
    RSceneObjects mObjects;
    unsigned int mCurrentItem;
};

class Compositor
{
public:
    Compositor(RSceneObjectList * objects)
     : mObjects(objects),
       mWaitTime(2.0f),
       mRShape(-1.0f, -1.0f, 1.0f, 1.0f),
       mComposing(false)
    {}

    virtual ~Compositor()
    {
        glDeleteBuffers(2, mBuffer);
    }
    
    void setWaitTime(int ms) 
    {
        mWaitTime = ms;
    }

    virtual bool init()
    {
        glClearColor(0.0, 0.0, 0.0, 1.0);
        glClearDepth(1.0);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_CULL_FACE);
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);

        glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
        glDepthFunc(GL_LEQUAL);
        glCullFace(GL_BACK);
        glFrontFace(GL_CCW);

        if (!mShader.create(mVShaderSource, mFShaderSource)) {
            return false;
        }

        if (!initBuffer()) {
            std::cerr << "initializing buffer failed" << std::endl;
            return false;
        }

        if (!retrieveVarLoc()) {
            return false;
        }

        ftime(&mStartTime);

        return true;
    }
    
    void draw() const
    {
        if (mShader.program() > 0) {
            
            struct timeb currentTime;
            ftime(&currentTime);

            float diffSec = currentTime.time - mStartTime.time;
            float diffMill = currentTime.millitm - mStartTime.millitm;

            float currTimeSec = diffSec + (diffMill / 1000.0f);
            
            if (currTimeSec > mWaitTime) {
                if (!mComposing) {
                    const_cast<Compositor *>(this)->onComposingStart();
                    const_cast<Compositor *>(this)->mComposing = true;
                }
                
                const_cast<Compositor *>(this)->onComposingStep();
                
            } else {
                const_cast<Compositor *>(this)->onDrawing();
            }
        }
    }

protected:
    virtual void onComposingStart() = 0;
    virtual void onComposingStep() = 0;
    
    virtual void onDrawing()
    {
        drawFrame(mObjects->getCurrentItem());
    } 

    virtual void stopComposing()
    {
        ftime(&mStartTime);
        mComposing = false;
        mObjects->moveNext();
    }

    void drawFrame(RSceneObject *object1, float alpha=1.0f,
                   chikku::RTransformation const & t=chikku::RTransformation(), 
                   RSceneObject *object2=0)
    {
        if (mShader.program() > 0) {
        
            GLuint tex1 = object1->getRScene();
            GLuint tex2 = 0;
            if (object2) {
                tex2 = object2->getRScene();
            }
            
            glUseProgram(mShader.program());

            glUniformMatrix4fv(mVars["u_mvp_matrix"], 1, GL_FALSE, t.matrix().data());

            glBindBuffer(GL_ARRAY_BUFFER, mBuffer[0]);
            glVertexAttribPointer(mVars["a_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, mBuffer[1]);
            glVertexAttribPointer(mVars["a_texturecoord"], 2, GL_FLOAT, GL_FALSE, 0, 0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);

            glUniform1f(mVars["u_alpha"], alpha);
            glUniform1i(mVars["u_multitexture"], object2 != 0);

            if (object2) {
                glActiveTexture(GL_TEXTURE1);
                glBindTexture(GL_TEXTURE_2D, tex2);
                glUniform1i(mVars["u_sampler2"], 1);
            }
            
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, tex1);
            glUniform1i(mVars["u_sampler1"], 0);            

            glEnableVertexAttribArray(mVars["a_position"]);
            glEnableVertexAttribArray(mVars["a_texturecoord"]);

            glDrawArrays(mRShape.primitives(), 0, mRShape.verticesCount());

            glBindTexture(GL_TEXTURE_2D, 0);

            glUseProgram(0);
        }
    }

private:
    bool initBuffer()
    {
        glGenBuffers(2, mBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, mRShape.verticesSize(),
                         mRShape.vertices(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, mRShape.texcoordsSize(),
                         mRShape.texcoords(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return true;
    }

    bool retrieveVarLoc()
    {
        mVars = mShader.getVariables();
        return !mVars.empty();
    }

protected:
    RSceneObjectList * mObjects;

    float mWaitTime;
    struct timeb mStartTime;
    
    chikku::RShaderObject mShader;

    GLuint mBuffer[2];

    chikku::RRectangle mRShape;

    chikku::RShaderVariables mVars;
    
    bool mComposing;
    
private:
    static const char * mVShaderSource;
    static const char * mFShaderSource;
};

const char * Compositor::mVShaderSource =
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

const char * Compositor::mFShaderSource =
                                    "                                                           \n"
                                    "#version 120                                               \n"
                                    "                                                           \n"
                                    "uniform sampler2D u_sampler1;                              \n"
                                    "uniform sampler2D u_sampler2;                              \n"
                                    "uniform float u_alpha;                                     \n"
                                    "uniform int u_multitexture;                                \n"
                                    "                                                           \n"
                                    "varying vec2 v_texturecoord;                               \n"
                                    "                                                           \n"
                                    "void main()                                                \n"
                                    "{                                                          \n"
                                    "    vec4 clr = texture2D(u_sampler1, v_texturecoord);      \n"
                                    "    if (u_multitexture == 1) {                             \n"    
                                    "        if (u_alpha < 1.0) {                               \n"
                                    "            vec4 old = texture2D(u_sampler2,               \n"
                                    "                                 v_texturecoord);          \n"
                                    "            clr = clr * u_alpha + old * (1.0-u_alpha);     \n"
                                    "        }                                                  \n"
                                    "    } else {                                               \n"
                                    "        clr = clr * u_alpha;                               \n"
                                    "    }                                                      \n"
                                    "                                                           \n"
                                    "    gl_FragColor = clr;                                    \n"
                                    "}                                                          \n"
                                    ;

class FadingCompositor: public Compositor
{
public:    
    FadingCompositor(RSceneObjectList * objects)
     : Compositor(objects),
       mAlpha(1.0f)
    {}
    
    void onComposingStart()
    {
        mAlpha = 0.0f;
    }

    void onComposingStep()
    {
        drawFrame(mObjects->getNextItem(), mAlpha, chikku::RTransformation(), mObjects->getCurrentItem());
        mAlpha += 0.03;
        
        if (mAlpha >= 1.0f) {
            stopComposing();
        }
    }
    
private:
    float mAlpha;
};

class RotatingCompositor: public Compositor
{
public:    
    RotatingCompositor(RSceneObjectList * objects)
     : Compositor(objects),
       mAngle(0.0f)
    {}
    
    void onComposingStart()
    {
        mAngle = 0.0f;
    }

    void onComposingStep()
    {
        const float MidAngle = 180.0f;
        
        if (mAngle <= MidAngle) {
            chikku::RTransformation t;
            t.rotate(mAngle, 1.0, 1.0, 1.0);
            drawFrame(mObjects->getCurrentItem(), 1.0f, t);
        } else if (mAngle < MidAngle*2.0f) {
            chikku::RTransformation t;
            t.rotate(MidAngle*2.0f-mAngle, 1.0, 1.0, 1.0);
            drawFrame(mObjects->getNextItem(), 1.0f, t);
        } 
        
        mAngle += 5.0f;

        if (mAngle >= MidAngle*2.0f) {
            stopComposing();
        }
    }
    
private:
    float mAngle;
};

class ZoomingCompositor: public Compositor
{
public:    
    ZoomingCompositor(RSceneObjectList * objects)
     : Compositor(objects),
       mZoomFactor(0.0f)
    {}
    
    void onComposingStart()
    {
        mZoomFactor = 0.0f;
    }

    void onComposingStep()
    {
        chikku::RTransformation tn;
        tn.scale(mZoomFactor, mZoomFactor, mZoomFactor);
        drawFrame(mObjects->getNextItem(), mZoomFactor, tn);
        
        chikku::RTransformation to;
        float czf = 1.0-mZoomFactor; 
        to.scale(czf, czf, czf);
        drawFrame(mObjects->getCurrentItem(), czf, to);
                        
        mZoomFactor += 0.03f;
        
        if (mZoomFactor >= 1.0f) {
            stopComposing();
        }
    }
    
private:
    float mZoomFactor;
};

class SlidingCompositor: public Compositor
{
public:    
	SlidingCompositor(RSceneObjectList * objects)
     : Compositor(objects),
       mSlideFactor(0.0f)
    {}
    
    void onComposingStart()
    {
    	mSlideFactor = 0.0f;
    }

    void onComposingStep()
    {
        chikku::RTransformation tn;
        tn.translate(mSlideFactor, 0.0f, 0.0f);
        drawFrame(mObjects->getCurrentItem(), 1.0f, tn);
        
        const float MaxSlide = 2.0f;
        
        chikku::RTransformation to;
        float czf = mSlideFactor-MaxSlide; 
        to.translate(czf, 0.0f, 0.0f);
        drawFrame(mObjects->getNextItem(), 1.0f, to);
                        
        mSlideFactor += 0.05f;
        
        if (mSlideFactor >= MaxSlide) {
            stopComposing();
        }
    }
    
private:
    float mSlideFactor;
};

class CompositorRScene: public chikku::RScene
{

public:
    static chikku::RScene & instance()
    {
        static CompositorRScene scene("compositor");
        return scene;
    }

private:
    enum Type
    {
        Fading,
        Rotating,
        Zooming,
        Sliding
    };
    
private:
    CompositorRScene(std::string const &title)
     : chikku::RScene(title)
    {}

    void createMenu()
    {
        glutAddMenuEntry("Fading", Fading);
        glutAddMenuEntry("Rotating", Rotating);
        glutAddMenuEntry("Zooming", Zooming);
        glutAddMenuEntry("Sliding", Sliding);

        glutAttachMenu(GLUT_RIGHT_BUTTON);
    }

    void onMenuItemClicked(int id)
    {
        Compositor * newCompositor = 0;

        switch (id) {
            case Fading:
            newCompositor = new FadingCompositor(&mObjects);
            break;

            case Rotating:
            newCompositor = new RotatingCompositor(&mObjects);
            break;
            
            case Zooming:
            newCompositor = new ZoomingCompositor(&mObjects);
            break;
            
            case Sliding:
            newCompositor = new SlidingCompositor(&mObjects);
            break;
        }
        
        if (newCompositor) {
            newCompositor->init();    
            mCompositor = newCompositor;
        }
    }

    void onTimer()
    {
        redraw();
        addTimer();
    }

    void onReshape(int w, int h)
    {
        glViewport(0, 0, w, h);

        mProjection.reset();
        mModelView.reset();

        mProjection.perspective(mFovy, (float)w/(float)h, mNear, mFar);
        mModelView.translate(0.0, 0.0, -4.0);
        
        for (RSceneObjectList::RSceneObjectsIter iter = mObjects.objects().begin();
                iter != mObjects.objects().end(); ++iter) {
            RSceneObject * object = *iter;
            object->setSize(mW, mH);
            object->setRTransformation(mModelView.multiply(mProjection));
        }
            
    }

    void onDisplay() const
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        mCompositor->draw();
        
        glutSwapBuffers();
    }

    bool onInit()
    {
        std::string imageFiles[] = {"../data/image1.bmp",
                "../data/image2.bmp",
                "../data/image3.bmp",
                "../data/image4.bmp",};
        
        for (unsigned int i=0; i<sizeof(imageFiles)/sizeof(imageFiles[0]); ++i) {
            RSceneObject * object = new RSceneObject(imageFiles[i], mW, mH);
            mObjects.add(object);
            if (!object->create()) {
                return false;
            }
        }

        mCompositor = new RotatingCompositor(&mObjects);
        
        if (!mCompositor->init()) {
            return false;
        }
        
        createMenu();

        return true;
    }


private:
    chikku::RTransformation mProjection;
    chikku::RTransformation mModelView;

    RSceneObjectList mObjects;
    chikku::RSmartPointer<Compositor> mCompositor;
    
private:
    static const float mNear;
    static const float mFar;
    static const float mFovy;
};

const float CompositorRScene::mNear = 0.0f;
const float CompositorRScene::mFar = 20.0f;
const float CompositorRScene::mFovy = 60.0f;

chikku::RScene & chikku::RScene::instance()
{
    return CompositorRScene::instance();
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
