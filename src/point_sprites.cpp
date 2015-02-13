
#include <cstring>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <sys/timeb.h>
#include <iostream>
#include <list>

#include "rscene.h"
#include "rtransformation.h"
#include "rcommon.h"
#include "rsolidsphere.h"
#include "rshader_object.h"
#include "rtexture_object.h"

class PointSpriteRScene: public chikku::RScene
{
public:
    static PointSpriteRScene & instance()
    {
        static PointSpriteRScene scene("point sprites");
        return scene;
    }

private:
    PointSpriteRScene(std::string const & title)
     : RScene(title)
    {}

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
        mModelView.translate(0.0, 0.0, -10.0);

        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
    }

    void onDisplay() const
    {
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        if (mShader.program() > 0) {

            glUseProgram(mShader.program());

            std::vector<ObjectsIter> expiredObjects;

            for (ObjectsIter iter = const_cast<PointSpriteRScene *>(this)->mObjects.begin(); iter != const_cast<PointSpriteRScene *>(this)->mObjects.end(); ++iter) {

                static const float StartTimeDeci = 0.0f;
                static const float EndTimeDeci = 5.0f;
                static const int IterationCount = 30;

                struct timeb currentTime;
                ftime(&currentTime);

                float diffSec = currentTime.time - iter->mStartTime.time;
                float diffMill = currentTime.millitm - iter->mStartTime.millitm;

                float currTimeDeci = diffSec + (diffMill / 1000.0f);
                static float Iteration[IterationCount];

                for (int i=0; i<IterationCount; ++i) {
                    Iteration[i] = EndTimeDeci / float(IterationCount) * i;
                }

                if (currTimeDeci-Iteration[IterationCount-1] <= EndTimeDeci) {
                    chikku::RTransformation transformation = mModelView.multiply(mProjection);
                    glUniformMatrix4fv(mVars["u_mvp_matrix"], 1, GL_FALSE, transformation.matrix().data());
                    glUniform1f(mVars["u_start_time"], StartTimeDeci);
                    glUniform1f(mVars["u_end_time"], EndTimeDeci);
                
                    float tt = std::tan(mFovy / 2.0f * M_PI / 180.0f);
                    chikku::RVector origin;
                    origin = mModelView.multiply(origin);
                
                    float h = tt * std::abs(origin.z());
                    float w = h * float(mW)/float(mH);

                    float px = w/((float)mW/2.0f) * iter->mCenter.x();
                    float py = h/((float)mH/2.0f) * iter->mCenter.y();

                    glUniform3f(mVars["u_center"], px-origin.x(), py-origin.y(), iter->mCenter.z());

                    glUniform4f(mVars["u_color"], iter->mR, iter->mG, iter->mB, iter->mA);

                    glBindBuffer(GL_ARRAY_BUFFER, mBuffer[0]);
                    glVertexAttribPointer(mVars["a_start_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
                    glBindBuffer(GL_ARRAY_BUFFER, mBuffer[1]);
                    glVertexAttribPointer(mVars["a_end_position"], 3, GL_FLOAT, GL_FALSE, 0, 0);
                    glBindBuffer(GL_ARRAY_BUFFER, 0);

                    glEnableVertexAttribArray(mVars["a_end_position"]);
                    glEnableVertexAttribArray(mVars["a_start_position"]);

                    glActiveTexture(GL_TEXTURE0);
                    mTexture.bind();
                    glUniform1i(mVars["u_sampler"], 0);

                    for (int i=0; i<IterationCount; ++i) {
                        if (currTimeDeci >= Iteration[i]) {
                            glUniform1f(mVars["u_current_time"], currTimeDeci-Iteration[i]);
                            glDrawArrays(GL_POINTS, 0, mStartPositions.size()/3);
                        } else {
                            break;
                        }
                    }

                } else {
                    expiredObjects.push_back(iter);
                }
            }
        
            for (size_t i=0; i<expiredObjects.size(); ++i) {
                const_cast<PointSpriteRScene *>(this)->mObjects.erase(expiredObjects[i]);
            }

            glUseProgram(0);
        }
        glutSwapBuffers();
    }

    void onMouseEvent(int button, int state, int x, int y)
    {
        if (state == GLUT_DOWN) {
            addObject(x - float(mW)/2.0f, float(mH)/2.0f - y);
        }
    }

    bool onInit()
    {
        createPoints();

        if (!initGL()) {
            return false;
        }

        addObject(0.0f, 0.0f);

        std::srand(std::time(0));

        return true;
    }

    void addObject(float cx, float cy)
    {
        RSceneObject object;
        
        object.mCenter.x() = cx;
        object.mCenter.y() = cy;
        ftime(&(object.mStartTime));

        object.mR = float(rand() % 255) / 255.0;
        object.mG = float(rand() % 255) / 255.0;
        object.mB = float(rand() % 255) / 255.0;
        object.mA = 1.0f;

        mObjects.push_back(object);
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

        if (!mShader.create(mVShaderSource, mFShaderSource)) {
            return false;
        }

        if (!retrieveVarLoc()) {
            return false;
        }

        if (!initBuffer()) {
            std::cout << "buffer creation failed" << std::endl;
            return false;
        }

        if (!mTexture.set("../data/image1.bmp")) {
            return false;
        }

        return true;
    }

    bool retrieveVarLoc()
    {
        mVars = mShader.getVariables();
        return !mVars.empty();
    }

    bool initBuffer()
    {
        glGenBuffers(2, mBuffer);

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer[0]);
        glBufferData(GL_ARRAY_BUFFER, mStartPositions.size() * sizeof(mStartPositions[0]),
                     mStartPositions.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, mBuffer[1]);
        glBufferData(GL_ARRAY_BUFFER, mEndPositions.size() * sizeof(mEndPositions[0]),
                     mEndPositions.data(), GL_STATIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, 0);

        return true;
    }

    void createPoints()
    {
        const float Interval = 2.0f * M_PI / 50.0;
        const float LastAngle = (2.0f * M_PI);
        const float Radius = 5.0f;

        for (float i=0.0f; i<LastAngle; i += Interval) {
            float x = Radius * cos(i);
            float y = Radius * sin(i);

            mStartPositions.push_back(0.0f);
            mStartPositions.push_back(0.0f);
            mStartPositions.push_back(0.0f);

            mEndPositions.push_back(x);
            mEndPositions.push_back(y);
            mEndPositions.push_back(0.0f);
        }
    }

private:
    class RSceneObject
    {
    public:
        struct timeb mStartTime;
        chikku::RVector mCenter;
        float mR;
        float mG;
        float mB;
        float mA;
    };

    typedef std::list<RSceneObject> Objects;
    typedef Objects::iterator ObjectsIter;
    typedef Objects::const_iterator ObjectsConstIter;
    
private:
    chikku::RTransformation mProjection;
    chikku::RTransformation mModelView;

    chikku::RShaderObject mShader;

    chikku::RTextureObject mTexture;

    chikku::RShaderVariables mVars;

    GLuint mBuffer[2];

    std::vector<float> mStartPositions;
    std::vector<float> mEndPositions;

    Objects mObjects;

private:
    static const float mNear;
    static const float mFar;
    static const float mFovy;

    static const char * mVShaderSource;
    static const char * mFShaderSource;
};

const float PointSpriteRScene::mNear = 0.0f;
const float PointSpriteRScene::mFar = 20.0f;
const float PointSpriteRScene::mFovy = 60.0f;

const char * PointSpriteRScene::mVShaderSource = 
                                    "#version 120                                                   \n"
                                    "                                                               \n"
                                    "uniform mat4 u_mvp_matrix;                                     \n"
                                    "uniform vec3 u_center;                                         \n"
                                    "uniform float u_start_time;                                    \n"
                                    "uniform float u_current_time;                                  \n"
                                    "uniform float u_end_time;                                      \n"
                                    "                                                               \n"
                                    "attribute vec3 a_start_position;                               \n"
                                    "attribute vec3 a_end_position;                                 \n"
                                    "                                                               \n"
                                    "varying float v_alpha;                                         \n"
                                    "                                                               \n"
                                    "void main()                                                    \n"
                                    "{                                                              \n"
                                    "    float t = (u_current_time - u_start_time) /                \n"
                                    "              (u_end_time - u_start_time);                     \n"
                                    "    t = t * t;                                                 \n"
                                    "                                                               \n"
                                    "    vec3 position = a_start_position +                         \n"
                                    "                    t * (a_end_position - a_start_position);   \n"
                                    "    position += u_center;                                      \n"
                                    "    v_alpha = (1.0 - t);                                       \n"
                                    "    v_alpha = clamp(v_alpha, 0.0, 1.0);                        \n"
                                    "    gl_Position = u_mvp_matrix * vec4(position, 1.0);          \n"
                                    "    gl_PointSize = 3.0;                                        \n"
                                    "}                                                              \n"
                                    ;

const char * PointSpriteRScene::mFShaderSource = 
                                    "#version 120                                                   \n"
                                    "                                                               \n"
                                    "uniform sampler2D u_sampler;                                   \n"
                                    "uniform vec4 u_color;                                          \n"
                                    "                                                               \n"
                                    "varying float v_alpha;                                         \n"
                                    "                                                               \n"
                                    "void main()                                                    \n"
                                    "{                                                              \n"
                                    "    gl_FragColor = u_color * vec4(                             \n"
                                    "                      texture2D(u_sampler, gl_PointCoord).rgb, \n" 
                                    "                      v_alpha);                                \n"
                                    "}                                                              \n"
                                    ;

chikku::RScene & chikku::RScene::instance()
{
    return PointSpriteRScene::instance();
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

