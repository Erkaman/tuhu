#include "shader_program.hpp"

#include "uniform_location_store.hpp"
#include "shader_program_builder.hpp"
#include "resource_manager.hpp"

#include "camera.hpp"
#include "log.hpp"
#include "str.hpp"
#include "common.hpp"

#include "math/color.hpp"
#include "math/matrix4f.hpp"
#include "math/vector4f.hpp"
#include "math/vector3f.hpp"

#include "file.hpp"


using namespace std;

ShaderProgram::ShaderProgram():m_uniformLocationStore(nullptr) {

}

ShaderProgram::~ShaderProgram() {
    MY_DELETE(m_uniformLocationStore);

    glDeleteProgram(m_alreadyBoundProgram);
}

static std::string GetShaderContents(const std::string& shaderPath) {

    std::string resourcePath = ResourceManager::GetInstance().FindResource(shaderPath);

    File f(resourcePath, FileModeReading);

    if(f.HasError()) {
	LOG_E("Could not read the shader %s: %s", resourcePath.c_str(), f.GetError().c_str() );
    }

    return f.GetFileContents();
}


ShaderProgram::ShaderProgram(const std::string& shaderName){


    m_alreadyBoundProgram = false;

    string geometryShaderPath = shaderName + "_gs.glsl";
    string geometryShaderSource;
    if(	!File::Exists(geometryShaderSource)) {
	geometryShaderSource = ""; // do not load a geometry shader, because it does not exist.
    } else {



	geometryShaderSource = GetShaderContents(geometryShaderPath);
    }

    CompileShaderProgram(

	GetShaderContents(shaderName + "_vs.glsl"),
	GetShaderContents(shaderName + "_fs.glsl"),
	geometryShaderSource,
	GetFileDirectory(shaderName + "_vs.glsl"));


}

ShaderProgram::ShaderProgram(const std::string& vertexShaderSource, const std::string& fragmentShaderSource) {
    CompileShaderProgram(

	vertexShaderSource,
	fragmentShaderSource,
	"",
	"");

}


void ShaderProgram::CompileShaderProgram(const string& vertexShaderSource, const string& fragmentShaderSource,const string& geometryShaderSource, const std::string& path) {
    // link shader program.
    ShaderProgramBuilder shaderBuilder(vertexShaderSource, fragmentShaderSource, geometryShaderSource, path);
    m_shaderProgram = shaderBuilder.GetLinkedShaderProgram();

    m_uniformLocationStore = new UniformLocationStore(m_shaderProgram);


}

void ShaderProgram::Bind() {
    if(m_alreadyBoundProgram)
	return;

    GL_C(glUseProgram(m_shaderProgram));

    m_alreadyBoundProgram = true;
}

void ShaderProgram::Unbind() {
    if(!m_shaderProgram) {
	return;
    }

    GL_C(glUseProgram(0));
    m_alreadyBoundProgram = false;
}

void ShaderProgram::SetUniform(const std::string& uniformName, const Color& color) {
    if (m_uniformLocationStore->UniformExists(uniformName)) {
	const GLuint location =m_uniformLocationStore->GetUniformLocation(uniformName);
	GL_C(glUniform4f(location, color.r, color.g, color.b, color.a));
    } else {
	SetUniformWarn(uniformName);
    }
}

void ShaderProgram::SetUniformWarn(const std::string& uniformName) {
    // we will only issue the warning once for every uniform. Otherwise it will completely spam the log.
    for(const std::string& warned : m_warnedUniforms) {
	if(warned == uniformName) {
	    return;
	}
    }

    LOG_W("Uniform %s could not be found(the GLSL compiler may have optimized it away)", uniformName.c_str() );
    m_warnedUniforms.push_back(uniformName);
}

void ShaderProgram::SetUniform(const std::string& uniformName, const Matrix4f& matrix) {
    if (m_uniformLocationStore->UniformExists(uniformName)) {
	const GLuint location =m_uniformLocationStore->GetUniformLocation(uniformName);
	const GLfloat* arr = reinterpret_cast<const GLfloat*>(&matrix);
	GL_C(glUniformMatrix4fv(location, 1, true, arr));
    } else {
	SetUniformWarn(uniformName);
    }
}


void ShaderProgram::SetUniform(const std::string& uniformName, const int val) {
    if (m_uniformLocationStore->UniformExists(uniformName)) {
	const GLuint location =m_uniformLocationStore->GetUniformLocation(uniformName);
	GL_C(glUniform1i(location, val));
    } else {
	SetUniformWarn(uniformName);
    }
}


void ShaderProgram::SetUniform(const std::string& uniformName, const Vector4f& v) {
    if (m_uniformLocationStore->UniformExists(uniformName)) {
	const GLuint location =m_uniformLocationStore->GetUniformLocation(uniformName);
	GL_C(glUniform4f(location, v.x, v.y, v.z, v.w));
    } else {
	SetUniformWarn(uniformName);
    }
}

void ShaderProgram::SetUniform(const std::string& uniformName, const Vector3f& v) {
    if (m_uniformLocationStore->UniformExists(uniformName)) {
	const GLuint location =m_uniformLocationStore->GetUniformLocation(uniformName);
	GL_C(glUniform3f(location, v.x, v.y, v.z));
    } else {
	SetUniformWarn(uniformName);
    }
}

void ShaderProgram::SetPhongUniforms(const Matrix4f& modelMatrix, const Camera& camera, const Vector4f& lightPosition) {

    const Matrix4f modelViewMatrix = camera.GetModelViewMatrix(
	modelMatrix);

    const Matrix4f mvp = camera.GetMvp(modelViewMatrix);

    SetUniform("mvp", mvp);
    SetUniform("modelViewMatrix", modelViewMatrix);
    SetUniform("normalMatrix", Matrix4f::GetNormalMatrix(modelViewMatrix));
    SetUniform("viewSpaceLightPosition", Vector3f(camera.GetViewMatrix() * lightPosition) );
}

void ShaderProgram::SetUniform(const std::string& uniformName, const float val) {
    if (m_uniformLocationStore->UniformExists(uniformName)) {
	const GLuint location =m_uniformLocationStore->GetUniformLocation(uniformName);
	GL_C(glUniform1f(location, val));
    } else {
	SetUniformWarn(uniformName);
    }
}
