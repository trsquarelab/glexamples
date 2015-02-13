
#ifndef TEXTUREOBJECT_H
#define TEXTUREOBJECT_H

#include <iostream>

#include "rbmp_image_handler.h"
#include "rcommon.h"
#include "rsmart_pointer.h"

namespace chikku
{

class RTextureObjectData
{
public:
    RTextureObjectData()
     : mCreated(false),
       mCount(1),
       mTexture(0)
    {}

    bool create()
    {
        if (!mCreated) {
            mTexture = new GLuint[mCount];
            glGenTextures(mCount, mTexture);
            mCreated = true;
        }
        return mCreated;
    }
    
    ~RTextureObjectData()
    {
        if (mCreated) {
            glDeleteTextures(mCount, mTexture);
        }
    }

private:
    RTextureObjectData(RTextureObjectData const &);
    RTextureObjectData & operator=(RTextureObjectData const &);
    
public:
    bool mCreated;
    int mCount;
    SmartArrayPointer<GLuint> mTexture;
};

class RTextureObject: public RSharedObject<RTextureObjectData>
{
public:
    RTextureObject(int count = 1)
    {
        data_p().mCount = count;
    }

    GLuint & handle(int index=0)
    {
        return data_p().mTexture[index];
    }

    const GLuint & handle(int index=0) const
    {
        return data_p().mTexture[index];
    }

    void bind(int index=0) const
    {
        glBindTexture(GL_TEXTURE_2D, handle(index));
    }
    
    void unbind(int index=0) const
    {
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    bool set(const std::string & filename, int index=0)
    {
        if (!this->data_p().create()) {
            return false;
        }

        chikku::RBMPImageHandler bmpImageHandler;
        if (!bmpImageHandler.load(filename.c_str())) {
            std::cerr << "loading image " << filename.c_str() << " failed" << std::endl;
            return false;
        }

        return set(bmpImageHandler.defaultSize().width(),
                      bmpImageHandler.defaultSize().height(),
                      bmpImageHandler.data(),
                      index);
    }
    
    bool set(GLsizei width, GLsizei height, const GLvoid *pixels,
             int index=0, int level=0, GLint format=GL_RGB,
             GLint border=0, GLenum type = GL_UNSIGNED_BYTE)
    {
        if (!this->data_p().create()) {
            return false;
        }

        bind(index);

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, level, format,
                      width, height, border, format, type, pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        unbind();
        
        return true;
    }
};

}

#endif
