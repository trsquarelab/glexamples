
#ifndef SHADEROBJECT_H
#define SHADEROBJECT_H

#include <map>
#include <iostream>

#include "rsmart_pointer.h"
#include "rshared_object.h"
#include "rcommon.h"

namespace chikku
{

class RShaderVariablesData
{
public:
    enum Type
    {
        Uniform,
        Attribute
    };

    class RLocation
    {
    public:
        RLocation(Type t=Uniform)
         : mType(t)
        {}

        GLint mLoc;
        Type mType;
    };

    typedef std::map<std::string, RLocation> VariableMap;
    typedef VariableMap::iterator VariableMapIter;
    typedef VariableMap::const_iterator VariableMapConstIter;

    VariableMap mMap;
};

class RShaderVariables: public RSharedObject<RShaderVariablesData>
{
public:
	typedef RShaderVariablesData::Type Type;
    typedef RShaderVariablesData::RLocation RLocation;
    typedef RShaderVariablesData::VariableMapIter VariableMapIter;
    typedef RShaderVariablesData::VariableMapConstIter VariableMapConstIter;
    		
    static const RShaderVariablesData::Type Uniform = RShaderVariablesData::Uniform;
    static const RShaderVariablesData::Type Attribute = RShaderVariablesData::Attribute;
    
public:
    bool empty() const
    {
    	return data_p().mMap.empty();
    }
    
    void clear()
    {
    	data_p().mMap.clear();
    }
    
    void add(std::string const &v, Type t=Uniform)
    {
        data_p().mMap.insert(std::pair<std::string, RLocation>(v, RLocation(t)));
    }

    GLint operator[](std::string const &v) const
    {
        return const_cast<RShaderVariables *>(this)->data_p().mMap[v].mLoc;
    }

    GLint operator[](std::string const &v)
    {
        return data_p().mMap[v].mLoc;
    }

    bool updateLoc(GLuint program)
    {
        bool res = true;
        glUseProgram(program);
        for (VariableMapIter iter=data_p().mMap.begin(); iter != data_p().mMap.end(); ++iter) {
            if (iter->second.mType == Uniform) {
                iter->second.mLoc = glGetUniformLocation(program, iter->first.c_str());
            } else {
                iter->second.mLoc = glGetAttribLocation(program, iter->first.c_str());
            }
            if (iter->second.mLoc < 0) {
                std::cerr << "retrieving location failed for " << iter->first << std::endl;
                res = false;
                break;
            }
        }
        glUseProgram(0);

        return res;
    }

private:
};

class RShaderObjectData
{
public:
	RShaderObjectData()
     : mCreated(false),
       mProgram(0),
       mVShader(0),
       mFShader(0),
       mGShader(0)
	{}

	bool create()
	{
		if (!mCreated) {
			mVShader = glCreateShader(GL_VERTEX_SHADER);
			if (!mVShader) {
				std::cerr << "vertex shader creation failed" << std::endl;
				return false;
			}
			mFShader = glCreateShader(GL_FRAGMENT_SHADER);
			if (!mFShader) {
				glDeleteShader(mVShader);
				std::cerr << "fragment shader creation failed" << std::endl;
				return false;
			}
			mProgram = glCreateProgram();
			if (!mProgram) {
				glDeleteShader(mVShader);
				glDeleteShader(mFShader);
				std::cerr << "program creation failed" << std::endl;
				return false;
			}
			mCreated = true;
		}
		
		return mCreated;
	}
	
    void createGS()
    {
		if (mCreated && mGShader == 0) {
			mGShader = glCreateShader(GL_GEOMETRY_SHADER);
			if (!mGShader) {
				glDeleteShader(mVShader);
				glDeleteShader(mFShader);
				std::cerr << "geometric shader creation failed" << std::endl;
			}
        }
    }

	~RShaderObjectData()
	{
		if (mCreated) {
			glDeleteProgram(mProgram);
			glDeleteShader(mVShader);
			glDeleteShader(mFShader);
            if (mGShader) {
			    glDeleteShader(mGShader);
            }
		}
	}

private:
	RShaderObjectData(RShaderObjectData const &);
	RShaderObjectData & operator=(RShaderObjectData const &);
	
public:
	bool mCreated;
    GLuint mProgram;
    GLuint mVShader;
    GLuint mFShader;
    GLuint mGShader;
};

class RShaderObject: public RSharedObject<RShaderObjectData>
{
public:
	RShaderObject()
	{}
	
	GLuint & program()
	{
		return data_p().mProgram;
	}

	void use()
	{
		glUseProgram(program());
	}
	
	void unuse()
	{
		glUseProgram(0);
	}
	
	const GLuint & program() const
	{
		return data_p().mProgram;
	}

    bool attachAndLink()
    {
        if (glIsProgram(program()) == GL_FALSE) {
            std::cerr << "not a valid program object " << std::endl;
            return false;
        }

        if (glIsShader(data_p().mVShader) == GL_FALSE) {
            std::cerr << "not a valid shader object " << std::endl;
            return false;
        }

        if (glIsShader(data_p().mFShader) == GL_FALSE) {
            std::cerr << "not a valid shader object " << std::endl;
            return false;
        }

        if (data_p().mGShader) {
            if (glIsShader(data_p().mGShader) == GL_FALSE) {
                std::cerr << "not a valid shader object " << std::endl;
                return false;
            }
            glAttachShader(program(), data_p().mGShader);
        }

        glAttachShader(program(), data_p().mVShader);
        glAttachShader(program(), data_p().mFShader);

        GLint lr = GL_TRUE;
        glGetProgramiv(program(), GL_ATTACHED_SHADERS, &lr);
        if (lr != 2) {
            std::cerr << "attached shader count is " << lr << std::endl;
        }

        glLinkProgram(program());

        glGetProgramiv(program(), GL_LINK_STATUS, &lr);
        if (lr != GL_TRUE) {
            char infoLog[1024] = {'\0'};
            glGetShaderInfoLog(program(), sizeof(infoLog)-1, 0, infoLog);
            std::cerr << "error : " << infoLog << std::endl;
            return false;
        }

        return true;
    } 

    bool create(const char * vSource, const char * fSource, const char * gSource = 0)
    {
    	if (!this->data_p().create()) {
    		return false;
    	}
    	
        bool res = compileShader(data_p().mVShader, vSource);
        if (!res) {
            std::cerr << "vertex shader creation failed" << std::endl;
            return false;
        }

        res = compileShader(data_p().mFShader, fSource);
        if (!res) {
            std::cerr << "fragment shader creation failed" << std::endl;
            return false;
        }

        if (gSource) {
            data_p().createGS();
            res = compileShader(data_p().mGShader, gSource);
            if (!res) {
                std::cerr << "geometric shader creation failed" << std::endl;
                return false;
            }
        }

        if (!attachAndLink()) {
            std::cerr << "attaching failed" << std::endl;
            return false;
        }

        return true;
    }

    RShaderVariables getVariables()
    {
    	RShaderVariables vars;
    
    	GLint maxVariableLength;
    	GLint numberOfVariables;
    	
    	use();
    	
    	glGetProgramiv(program(), GL_ACTIVE_UNIFORMS, &numberOfVariables);
    	glGetProgramiv(program(), GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxVariableLength);
    	
        SmartArrayPointer<char> variableName = new char[maxVariableLength];

    	for(int index = 0; index < numberOfVariables; index++) {
    		GLint size;
    		GLenum type;
    		
    		glGetActiveUniform(program(), index, maxVariableLength,
    						   0, &size, &type, variableName.data());
    		
    		
    		vars.add(variableName.data(), RShaderVariables::Uniform);

    	}

    	glGetProgramiv(program(), GL_ACTIVE_ATTRIBUTES, &numberOfVariables);
    	glGetProgramiv(program(), GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxVariableLength);
    	
    	variableName = new char[maxVariableLength];
    	
    	for(int index = 0; index < numberOfVariables; index++) {
    		GLint size;
    		GLenum type;
    		
    		glGetActiveAttrib(program(), index, maxVariableLength,
    						   0, &size, &type, variableName.data());
    		
    		
    		vars.add(variableName.data(), RShaderVariables::Attribute);
    	}
    	
    	unuse();
    	
    	if (!vars.updateLoc(program())) {
    		return RShaderVariables();
    	}
    	
    	return vars;
    }
    
private:
    
    
    bool compileShader(GLuint &shader, const char *source)
    {
        GLint lengths[2] = {std::strlen(source), 0};
        glShaderSource(shader, 1, &source, lengths);
    
        glCompileShader(shader);
    
        GLint result = GL_TRUE;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
        
        if (result != GL_TRUE) {
            char infoLog[1024] = {'\0'};
            glGetShaderInfoLog(shader, sizeof(infoLog)-1, 0, infoLog);
            std::cerr << "error : " << infoLog << std::endl;
            return false;
        }
    
        return true;
    }
};

}

#endif

