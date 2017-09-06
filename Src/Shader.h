#pragma once
/**
 *
 * @file Shader.h
 *
 */

#include<GL/glew.h>
#include<string>
#include<memory>

namespace Shader {

	class Program;
	typedef std::shared_ptr<Program> ProgramPtr; //プロジェクトポインタ型
/**
 *シェーだープログラム
 */
class Program {
public:
	static ProgramPtr Create(const char* vsFilename, const char* fsFilename);

	bool UniformBlockBinding(const char* blockName, GLuint bindingPoint);
	void  UseProgram();
	void BindTexture(GLenum unit, GLenum type, GLuint texture);

private:
	Program() = default;
	~Program();
	Program(const Program&) = delete;
	Program& operator = (const Program&) = delete;

private:
	GLuint program = 0;
	GLint samplerLocation = -1;
	int samplerCount = 0;
	std::string name;
};

	GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename);
}

