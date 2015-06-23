#pragma once

#include <vector>
#include <string>
#include "gl_common.hpp"

class UniformLocationStore;
struct UniformLocationStoreDeleter
{
    void operator()(UniformLocationStore *p);
};

class ShaderProgram {

private:




    std::unique_ptr<UniformLocationStore> uniformLocationStore;

    std::vector<std::string> warnedUniforms;

    std::string shaderProgramName;

    bool alreadyBoundProgram;

    GLuint shaderProgram;

    void CompileShaderProgram(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);

    ShaderProgram();

public:

    ShaderProgram(const std::string& shaderName);
    ~ShaderProgram();

    inline void Bind();

    inline void Unbind();

    inline void Dispose();


};