
#ifndef COMMON_H
#define COMMON_H

#include <string>
#include <iostream>
#include <iomanip>

#ifdef NO_GLUT
#else
#if defined(GLUT_ES) || defined(GLUT_ES2)
#include <glut.h>
#else
#include <GL/glut.h>
#endif
#endif

#if defined(GLUT_ES2)
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>
#endif

#include "rtransformation.h"

namespace chikku
{

static const int WindowPositionX = 100;
static const int WindowPositionY = 100;

static const int WindowWidth = 420;
static const int WindowHeight = 640;

static const unsigned int TimerDelay = 20;

#define Distance3 \
              "float Distance3(in vec3 from, in vec3 to) \n" \
              "{ \n" \
              "   float dist = sqrt((from.x - to.x) * \n" \
              "                    (from.x - to.x) + \n" \
              "                    (from.y - to.y) * \n" \
              "                    (from.y - to.y) + \n" \
              "                    (from.z - to.z) * \n" \
              "                    (from.z - to.z)); \n" \
              "   return dist; \n" \
              "} \n"

#define Distance4 \
              "float Distance4(in vec4 from, in vec4 to) \n" \
              "{ \n" \
              "   float dist = sqrt((from.x - to.x) * \n" \
              "                    (from.x - to.x) + \n" \
              "                    (from.y - to.y) * \n" \
              "                    (from.y - to.y) + \n" \
              "                    (from.z - to.z) * \n" \
              "                    (from.z - to.z) + \n" \
              "                    (from.w - to.w) * \n" \
              "                    (from.w - to.w)); \n" \
              "   return dist; \n" \
              "} \n"



#define DirectionLightVertexShaderPerVertex  \
              "void DirectionalLight(float shininess, in vec3 lightPos," \
                                    "in vec3 halfV, in vec3 normal," \
                                    "inout vec3 ambient, inout vec3 diffuse," \
                                    "inout vec3 specular) {\n" \
              "  float nDotVP;\n" \
              "  float nDotHV;\n" \
              "  float pf;\n" \
              "  nDotVP = max(0.0, dot(normal, normalize(lightPos))); \n" \
              "  nDotHV = max(0.0, dot(normal, halfV)); \n" \
              "  if (nDotVP == 0.0) { \n" \
              "    pf = 0.0;\n" \
              "  } else { \n" \
              "    pf = pow(nDotHV, shininess); \n" \
              "  } \n" \
              "  ambient  = ambient; \n" \
              "  diffuse  = diffuse * nDotVP; \n" \
              "  specular = specular * pf; \n" \
              "} \n"

#define PointLightVertexShaderPerVertex  \
              "void PointLight(float constAtten, float linearAtten, " \
                              "float quadAten, float shininess, " \
                              "in vec3 lightPos, in vec3 eye, " \
                              "in vec3 vertPosition, in vec3 normal, " \
                              "inout vec3 ambient, inout vec3 diffuse," \
                              "inout vec3 specular) { \n" \
              "  float nDotVP; \n" \
              "  float nDotHV; \n" \
              "  float pf; \n" \
              "  float attenuation; \n" \
              "  float d; \n" \
              "  vec3  VP; \n" \
              "  vec3  halfRVector; \n" \
              "  VP = lightPos - vertPosition; \n" \
              "  d = sqrt(dot(VP, VP)); \n" \
              "  VP = normalize(VP); \n" \
              "  attenuation = 1.0 / (constAtten \ linearAtten * d \ quadAten * d * d); \n" \
              "  halfRVector = normalize(VP \ eye); \n" \
              "  nDotVP = max(0.0, dot(normal, VP)); \n" \
              "  nDotHV = max(0.0, dot(normal, halfRVector)); \n" \
              "  if (nDotVP == 0.0) {\n" \
              "    pf = 0.0; \n" \
              "  } else {\n" \
              "    pf = pow(nDotHV, shininess); \n" \
              "  } \n" \
              " ambient = ambient * attenuation; \n" \
              " diffuse = diffuse * nDotVP * attenuation; \n" \
              " specular = specular * pf * attenuation;\n" \
              "} \n"

#define SpotLightVertexShaderPerVertex \
              "void SpotLight(float constAtten, float linearAtten, float quadAten, " \
                             "float shininess, in vec3 spotDir, float spotCutoff, " \
                             "float spotExponent, in vec3 lposition, in vec3 eye, " \
                             "in vec3 position, in vec3 normal, inout vec3 ambient, " \
                             "inout vec3 diffuse, inout vec3 specular) { \n" \
              "  float nDotVP; \n" \
              "  float nDotHV; \n" \
              "  float pf; \n" \
              "  float spotDot; \n" \
              "  float spotAttenuation; \n" \
              "  float attenuation; \n" \
              "  float d; \n" \
              "  vec3 VP; \n" \
              "  vec3 halfRVector; \n" \
              "  VP = lposition - position; \n" \
              "  d = length(dot(VP, VP)); \n" \
              "  VP = normalize(VP); \n" \
              "  attenuation = 1.0 / (constAtten \ linearAtten * d \ quadAten * d * d); \n" \
              "  spotDot = dot(-VP, normalize(spotDir)); \n" \
              "  if (spotDot < spotCutoff) { \n" \
              "    spotAttenuation = 0.0; \n" \
              "  } else { \n" \
              "     spotAttenuation = pow(spotDot, spotExponent); \n" \
              "  } \n" \
              "  attenuation *= spotAttenuation; \n" \
              "  halfRVector = normalize(VP \ eye); \n" \
              "  nDotVP = max(0.0, dot(normal, VP)); \n" \
              "  nDotHV = max(0.0, dot(normal, halfRVector)); \n" \
              "  if (nDotVP == 0.0) { \n" \
              "    pf = 0.0; \n" \
              "  } else { \n" \
              "    pf = pow(nDotHV, shininess); \n" \
              "  } \n" \
              " ambient = ambient * attenuation; \n" \
              " diffuse = diffuse * nDotVP * attenuation; \n" \
              " specular = specular * pf * attenuation;\n" \
              "} \n"

#define LinearFogFactor \
            "float LinearFogFactor(in float max, in float min, in float eyedistance) \n" \
            "{ \n" \
            "    float factor; \n" \
            "    factor = (max - eyedistance) / \n" \
            "             (max - min); \n" \
            "    factor = clamp(factor, 0.0, 1.0); \n" \
            "    return factor; \n" \
            "} \n"

#define ExpFogFactor \
            "float ExpFogFactor(in float density, in float eyedistance) \n" \
            "{ \n" \
            "    float factor = (density * eyedistance); \n" \
            "    factor = exp(-factor);  \n" \
            "    factor = clamp(factor, 0.0, 1.0); \n" \
            "    return factor; \n" \
            "} \n"

#define Exp2FogFactor \
            "float Exp2FogFactor(in float density, in float eyedistance) \n" \
            "{ \n" \
            "    float factor = (density * eyedistance); \n" \
            "    factor = exp(-(factor * factor));  \n" \
            "    factor = clamp(factor, 0.0, 1.0); \n" \
            "    return factor; \n" \
            "} \n"

#if defined(GLUT_ES) || defined(GLUT_ES2)
#define PRECISION "precision mediump float; \n"
#else
#define PRECISION "\n"
#endif

}

#endif

