#include "shader_program_builder.hpp"


#include "file.hpp"
#include "log.hpp"
#include "resource_manager.hpp"

#include <vector>
#include "string_util.hpp"

using namespace std;

GLuint CreateShaderFromString(const string& str, GLenum shaderType);
string FormatCompilerErrorOutput(const GLuint shader, const string& shaderStr);

string GetShaderLogInfo(GLuint shaderProgram);
string GetProgramLogInfo(GLuint shaderProgram);

bool GetCompileStatus(GLuint shaderProgram);

string ParseShader(const std::string& shaderSource, const std::string& path);

ShaderProgramBuilder::ShaderProgramBuilder(const string& vertexShaderSource, const string& fragmentShaderSource, const string& geometryShaderSource, const std::string& path, void (*beforeLinkingHook)(GLuint)) {

    m_compiledVertexShader = BuildAndCompileShader(vertexShaderSource, GL_VERTEX_SHADER, path);
    m_compiledFragmentShader = BuildAndCompileShader(fragmentShaderSource,  GL_FRAGMENT_SHADER, path);

    if(geometryShaderSource != "") {

	    m_compiledGeometryShader = BuildAndCompileShader(geometryShaderSource,  GL_GEOMETRY_SHADER, path);
	m_hasGeometryShader = true;
    } else {
	m_hasGeometryShader = false;
    }

    Attach();

    // here put hook.

    if(beforeLinkingHook != NULL) {
	beforeLinkingHook(m_shaderProgram);
    }

    Link();
}

GLuint ShaderProgramBuilder::BuildAndCompileShader(const string& shaderSource, const GLenum shaderType, const std::string& path){


    string shaderContents = ParseShader(shaderSource, path);

//    LOG_I("shaderContents: %s", shaderSource.c_str() );

    return CreateShaderFromString(shaderContents, shaderType);
}


GLuint CreateShaderFromString(const string& str, const GLenum shaderType) {

    GLuint shader;
    // this one is quite old. and it doesn't seem to work.
    GL_C(shader =    glCreateShader(shaderType));

    if(shader == 0) {
		LOG_E("Could not create shader of type %d", shaderType);
    }


    const char* c_str = str.c_str();
    const GLint len = str.size();
    GL_C(glShaderSource(shader, 1, &c_str, &len));
    GL_C(glCompileShader(shader));


    if (!GetCompileStatus(shader)) {
	// compilation failed
	LOG_W("Could not compile shader \n%s", /*shaderPath.c_str(),*/ FormatCompilerErrorOutput(shader, str).c_str());
	exit(1);
    }


    return shader;
}

// split shader source code into lines.
std::vector<std::string> SplitShaderSource(const std::string& str)
{
    std::vector<std::string> strings;

    std::string::size_type pos = 0;
    std::string::size_type prev = 0;
    const string& delimiter = "\n";

    while ((pos = str.find(delimiter, prev)) != std::string::npos)
    {
	string token = str.substr(prev, pos - prev+1); // include the newline in the line.

	strings.push_back(token);

        prev = pos + 1;
    }

    // To get the last substring (or only, if delimiter is not found)
    strings.push_back(str.substr(prev));

    return strings;
}


string FormatCompilerErrorOutput(const GLuint shader, const string& shaderStr)  {
    string result = "";

#ifdef __APPLE__
    vector<string> errors =  StringUtil::SplitString(GetShaderLogInfo(shader), "\n");

    vector<string> shaderLines = SplitShaderSource(shaderStr);

    // iterate over the errors, one after one.
    for(string error: errors) {

	if(error == "")
	    continue;

	size_t firstColonIndex = error.find(":");
	size_t secondColonIndex = error.find(":", firstColonIndex+1);
	size_t thirdColonIndex = error.find(":", secondColonIndex+1);

	int lineNumber = std::stoi(error.substr(secondColonIndex+1, thirdColonIndex-(secondColonIndex+1)));

	// print the error.
	result.append(error);
	result.append("\nLine where the error occurred:\n");
	result.append(shaderLines[lineNumber-1]);

	// the affected line.
    }
#else
    result = GetShaderLogInfo(shader);

#endif

    return result;

}

void ShaderProgramBuilder::Attach()  {

    GL_C(m_shaderProgram = glCreateProgram());

    // attach.
    GL_C(glAttachShader(m_shaderProgram, m_compiledFragmentShader));
    GL_C(glAttachShader(m_shaderProgram, m_compiledVertexShader));
    if(m_hasGeometryShader) {
	GL_C(glAttachShader(m_shaderProgram, m_compiledGeometryShader));
    }

    // clean up.
    GL_C(glDeleteShader( m_compiledFragmentShader));
    GL_C(glDeleteShader(m_compiledVertexShader));
	if (m_hasGeometryShader) {

    GL_C(glDeleteShader(m_compiledGeometryShader));
	}
}

void ShaderProgramBuilder::BindAttribLocation(const GLuint attribIndex, const string& attribName) {

    GL_C(glBindAttribLocation(m_shaderProgram, attribIndex, attribName.c_str()));
}

void ShaderProgramBuilder::Link() {

    GL_C(glLinkProgram(m_shaderProgram));

    GLint linkOk;
    glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &linkOk);

    if (linkOk == GL_FALSE) {
	LOG_E("Error linking program: %s", GetProgramLogInfo(m_shaderProgram).c_str());
    }
}

GLuint ShaderProgramBuilder::GetLinkedShaderProgram() {
    return m_shaderProgram;
}

bool GetCompileStatus(GLuint shaderProgram)  {
    GLint status;
    GL_C(glGetShaderiv(shaderProgram,  GL_COMPILE_STATUS, &status));
    return status == GL_TRUE;
}

string GetShaderLogInfo(GLuint shaderProgram) {
    GLint len;
    GL_C(glGetShaderiv(shaderProgram,  GL_INFO_LOG_LENGTH, &len));

    char* infoLog = new char[len];
    GLsizei actualLen;
    glGetShaderInfoLog(shaderProgram, len, &actualLen, infoLog);
    string logInfoStr(infoLog, actualLen);
    delete infoLog;
    return logInfoStr;
}

string GetProgramLogInfo(GLuint shaderProgram) {

    GLint len;
    GL_C(glGetProgramiv(shaderProgram,  GL_INFO_LOG_LENGTH, &len));

    char* infoLog = new char[len];
    GLsizei actualLen;
    glGetProgramInfoLog(shaderProgram, len, &actualLen, infoLog);
    string logInfoStr(infoLog, actualLen);
    delete infoLog;
    return logInfoStr;
}

static std::string GetShaderContents(const std::string& shaderPath) {
    File *f = File::Load(shaderPath, FileModeReading);

    if(!f) {

	PrintErrorExit();
    }

    return f->GetFileContents();
}

string ParseShader(const std::string& shaderSource, const std::string& path) {

    string parsedShader = "";

    parsedShader +=  "#version 330\n";

    if(HighQuality)
	parsedShader +=  "#define HIGH_QUALITY\n";

    // File::GetFileContents(
    vector<string> shaderLines = StringUtil::SplitString(shaderSource, "\n");

    for(const string& line: shaderLines) {

	if(StringUtil::BeginsWith(line, "#include")) {

		const size_t firstQuoteIndex = line.find("\"");
		const size_t secondQuoteIndex = line.find("\"", firstQuoteIndex + 1);


		string includePath = line.substr(firstQuoteIndex + 1, secondQuoteIndex - firstQuoteIndex - 1);
		//string shaderDir = GetFileDirectory(shaderPath);

		string includeStr;


		string defaultPath = File::AppendPaths(path, includePath);
		if(File::Exists(defaultPath) ){
		    includeStr = GetShaderContents(defaultPath);
		} else {
		    includeStr = GetShaderContents(includePath);
		}

		parsedShader += includeStr + "\n";
	} else {
	    parsedShader += line + "\n";
	}
    }

    return parsedShader;
}
