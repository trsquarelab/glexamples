
#ifndef BUFFEROBJECT_H
#define BUFFEROBJECT_H

#include <iostream>

#include "rcommon.h"
#include "rsmart_pointer.h"

namespace chikku
{

class RBufferObjectData
{
public:
    RBufferObjectData()
     : mCreated(false),
       mCount(1),
       mBuffer(0)
    {}

    bool create()
    {
        if (!mCreated) {
            mBuffer = new GLuint[mCount];
            glGenBuffers(mCount, mBuffer);
            mCreated = true;
        }
        
        return mCreated;
    }

    ~RBufferObjectData()
    {
        if (mCreated) {
            glDeleteBuffers(mCount, mBuffer);
        }
    }

private:
    RBufferObjectData(RBufferObjectData const &);
    RBufferObjectData & operator=(RBufferObjectData const &);
    
public:
    bool mCreated;
    int mCount;
    SmartArrayPointer<GLuint> mBuffer;
};

class RBufferObject: public RSharedObject<RBufferObjectData>
{
public:
    RBufferObject(int count = 1)
    {
        data_p().mCount = count;
    }
    
    GLuint & handle(int index=0)
    {
        return data_p().mBuffer[index];
    }

    const GLuint & handle(int index=0) const
    {
        return data_p().mBuffer[index];
    }

    void bind(int index=0) const
    {
        glBindBuffer(GL_ARRAY_BUFFER, handle(index));
    }
    
    void unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    bool set(GLsizeiptr size, const GLvoid *data, int index=0, GLenum target=GL_ARRAY_BUFFER, GLenum usage=GL_STATIC_DRAW)
    {
        if (!this->data_p().create()) {
            return false;
        }
        
        if (glIsBuffer(handle(index) == GL_FALSE)) {
            std::cerr << "not a valid buffer" << std::endl;
            return false;
        }

        bind(index);
        glBufferData(target, size, data, usage);
        unbind();
        
        return true;
    }
};

}

#endif
