
#ifndef SPHERE_H
#define SPHERE_H

#include <vector>
#include <cmath>

#include "rshape.h"

namespace chikku
{

class RSolidSphere: public RShape
{
public:
    RSolidSphere(float radius = 1.0f)
     : mRadius(radius)
    {
        create(radius);
    }

    float radius() const
    {
        return mRadius;
    }

private:
    void create(float r)
    {
        r = r/2.0;
        float pb2 = M_PI/2.0;
        float inc = M_PI / 12.0;
      
        for (float theta=-pb2; theta <= (pb2 - inc); theta += inc) {
            for (float phi=0.0; phi < (2.0 * M_PI); phi += inc) {
                float nx = std::cos(theta) * std::cos(phi);
                float ny = std::sin(theta);
                float s = std::asin(nx) / M_PI + 0.5;
                float t = std::asin(ny) / M_PI + 0.5;

                addNormal(nx, ny, std::cos(theta) * std::sin(phi));
                addVertex(r * std::cos(theta) * std::cos(phi), r * std::sin(theta), r * std::cos(theta) * std::sin(phi));
                addTexCoord(s, t);
          
                nx = std::cos(theta+inc) * std::cos(phi);
                ny = std::sin(theta+inc);
                s = std::asin(nx) / M_PI + 0.5;
                t = std::asin(ny) / M_PI + 0.5;
                addNormal(nx, ny, std::cos(theta+inc) * std::sin(phi));
                addVertex(r * std::cos(theta+inc) * std::cos(phi), r * std::sin(theta+inc), r * std::cos(theta+inc) * std::sin(phi));
                addTexCoord(s, t);
          
                nx = std::cos(theta+inc) * std::cos(phi+inc);
                ny = std::sin(theta+inc);
                s = std::asin(nx) / M_PI + 0.5;
                t = std::asin(ny) / M_PI + 0.5;
                addNormal(nx, ny, std::cos(theta+inc) * std::sin(phi+inc));
                addVertex(r * std::cos(theta+inc) * std::cos(phi+inc), r * std::sin(theta+inc), r * std::cos(theta+inc) * std::sin(phi+inc));
                addTexCoord(s, t);
          
                nx = std::cos(theta) * std::cos(phi);
                ny = std::sin(theta);
                s = std::asin(nx) / M_PI + 0.5;
                t = std::asin(ny) / M_PI + 0.5;
                addNormal(nx, ny, std::cos(theta) * std::sin(phi));
                addVertex(r * std::cos(theta) * std::cos(phi), r * std::sin(theta), r * std::cos(theta) * std::sin(phi));
                addTexCoord(s, t);
          
                nx = std::cos(theta+inc) * std::cos(phi+inc);
                ny = std::sin(theta+inc);
                s = std::asin(nx) / M_PI + 0.5;
                t = std::asin(ny) / M_PI + 0.5;
                addNormal(nx, ny, std::cos(theta+inc) * std::sin(phi+inc));
                addVertex(r * std::cos(theta+inc) * std::cos(phi+inc), r * std::sin(theta+inc), r * std::cos(theta+inc) * std::sin(phi+inc));
                addTexCoord(s, t);
          
                nx = std::cos(theta) * std::cos(phi+inc);
                ny = std::sin(theta);
                s = std::asin(nx) / M_PI + 0.5;
                t = std::asin(ny) / M_PI + 0.5;
                addNormal(nx, ny, std::cos(theta) * std::sin(phi+inc));
                addVertex(r * std::cos(theta) * std::cos(phi+inc), r * std::sin(theta), r * std::cos(theta) * std::sin(phi+inc));
                addTexCoord(s, t);
            }
        }
    }
  
private:
    float mRadius;
};
  
}

#endif



