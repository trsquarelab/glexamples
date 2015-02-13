
#ifndef SHAPE_H
#define SHAPE_H

#include "rcommon.h"
#include "rshared_object.h"

namespace chikku
{

class RShapeData
{
public:
    void clear()
    {
        mVertices.clear();
        mNormals.clear();
        mTexCoords.clear();
    }

public:
    std::vector<float> mVertices;
    std::vector<float> mNormals;
    std::vector<float> mTexCoords;
};

class RShape: public RDetachableRSharedObject<RShapeData>
{
public:
    const float * vertices() const
    {
        return &(this->data_p().mVertices[0]);
    }

    const float * normals() const
    {
        return &(data_p().mNormals[0]);
    }

    const float * texcoords() const
    {
        return &(data_p().mTexCoords[0]);
    }

    int verticesCount() const
    {
        return data_p().mVertices.size() / 3;
    }

    int normalsCount() const
    {
        return data_p().mNormals.size() / 3;
    }

    int texcoordsCount() const
    {
        return data_p().mTexCoords.size() / 2;
    }

    int verticesSize() const
    {
        return data_p().mVertices.size() * sizeof(float);
    }

    int normalsSize() const
    {
        return data_p().mNormals.size() * sizeof(float);
    }

    int texcoordsSize() const
    {
        return data_p().mTexCoords.size() * sizeof(float);
    }

    virtual GLenum primitives() const
    {
        return GL_TRIANGLES;
    }

protected:
    void clear()
    {
        data_p().clear();
    }

    void addNormal(float x, float y, float z)
    {
        data_p().mNormals.push_back(x);
        data_p().mNormals.push_back(y);
        data_p().mNormals.push_back(z);
    }

    void addVertex(float x, float y, float z)
    {
        data_p().mVertices.push_back(x);
        data_p().mVertices.push_back(y);
        data_p().mVertices.push_back(z);
    }

    void addTexCoord(float x, float y)
    {
        data_p().mTexCoords.push_back(x);
        data_p().mTexCoords.push_back(y);
    }
};

}

#endif

