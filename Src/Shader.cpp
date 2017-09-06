/**
 *@file Shader.cpp
 */

#include "Shader.h"
#include <vector>
#include <iostream>
#include <cstdint>
#include <stdio.h>
#include <sys/stat.h>

namespace Shader{

	/**
	 *
	 *
	 *
	 *
	 */
	ProgramPtr Program::Create(const char* vsFilename, const char* fsFilename) {
		struct Impl : Program {
			Impl(){}
			~Impl(){}
		};
		ProgramPtr p = std::make_shared<Impl>();
		if (!p) {
			std::cerr << "ERROR:プログラム" << vsFilename << "の作成に失敗" << std::endl;
			return{};
		}
		p->program = CreateProgramFromFile(vsFilename, fsFilename);
		if (!p->program) {
			return{};
		}
		//サンプラーの数と位置を取得する
		GLint activeUniforms;
		glGetProgramiv(p->program, GL_ACTIVE_UNIFORMS, &activeUniforms);
		for (int i = 0;i < activeUniforms;++i) {
			GLint size;
			GLenum type;
			GLchar name[128];
			glGetActiveUniform(p->program, i, sizeof(name), nullptr, &size, &type, name);
			if (type == GL_SAMPLER_2D) {
				p->samplerCount = size;
				p->samplerLocation = glGetUniformLocation(p->program, name);
				if (p->samplerLocation < 0) {
					std::cerr << "ERROR:プログラム " << vsFilename << "の作成に失敗" << std::endl;
					return{};
				}
				break;
			}
		}

		//頂点シェーダーファイル名
		p->name = vsFilename;
		p->name.resize(p->name.size() - 4);

		return p;
	}


	Program::~Program()
	{
		if (program) {
			glDeleteProgram(program);
		}
	}

	/**
	 *Uniformブロックをバインディングポイントに割り当てる
	 *@param blockName 割り当てるUniformブロックの名前　
	 *@param bindingPoint 割り当てる先のバインディングポイント
	 *
	 *@return true 割当成功
	 *
	 *@return false　割当失敗
	 */
	bool Program::UniformBlockBinding(const char* blockName, GLuint bindingPoint) {
		const GLuint blockIndex = glGetUniformBlockIndex(program, blockName);
		if (blockIndex == GL_INVALID_INDEX) {
			std::cerr << "ERROR(" << name << "):Uniformブロック" << blockName << "見つかりません" << std::endl;
			return false;
		}
		glUniformBlockBinding(program, blockIndex, bindingPoint);
		const GLenum result = glGetError();
		if (result != GL_NO_ERROR) {
			std::cerr << "ERROR(" << name << "):Uniformブロック" << blockName << "のバインドに失敗" << std::endl;
			return false;
		}
		return true;
	}

	/**
	 * 描画用プログラムに設定する
	 */
	void Program::UseProgram() {
		glUseProgram(program);
		for (GLint i = 0;i < samplerCount; ++i) {
			glUniform1i(samplerLocation + i, i);
		}
	}

	/** 
	* テクスチャをテクスチャ・イメージ・ユニットに割り当てる
	*	@param unit 割り当てる先のテクスチャイメージユニット番号
	* @param type	 割り当てるテクスチャの種類
	* @param texture 割り当てるテクスチャオブジェクト
	*/
	void Program::BindTexture(GLenum unit, GLenum type, GLuint texture) {
		if (unit >= GL_TEXTURE0 && unit < static_cast<GLenum>(GL_TEXTURE0 + samplerCount)) {
			glActiveTexture(unit);
			glBindTexture(type, texture);
		}
	}

	/**
	*シェーダーコードをコンパイルする
	*
	*@param typeシェーダの種類
	*@param string シェーダーコードへのポインタ
	*
	*@return 作成したシェーダオブジェクト
	*/
	GLuint CompileShader(GLenum type, const GLchar* string) {
		GLuint sharder = glCreateShader(type);
		glShaderSource(sharder, 1, &string, nullptr);
		glCompileShader(sharder);
		GLint compiled = 0;
		glGetShaderiv(sharder, GL_COMPILE_STATUS, &compiled);
		if (!compiled) {
			GLint infoLen = 0;
			glGetShaderiv(sharder, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				std::vector<char> buf;
				buf.resize(infoLen);
				if (static_cast<int>(buf.size()) >= infoLen) {
					glGetShaderInfoLog(sharder, infoLen, NULL, buf.data());
					std::cerr << "ERROR: シェーダーのコンパイルに失敗\n" << buf.data() << std::endl;
				}
			}
			glDeleteShader(sharder);
			return 0;
		}
		return sharder;
	}

	/**
	*プログラムオブジェクトを作成する
	*@param vsCode 頂点シェーダコードへのポインタ
	*@param fsCode フラグメントシェーダコードへのポインタ
	*
	*@return 作成したプログラムオブジェクト
	*/
	GLuint CreateShaderProgram(const GLchar* vsCode, const GLchar* fsCode) {
		GLuint vs = CompileShader(GL_VERTEX_SHADER, vsCode);
		GLuint fs = CompileShader(GL_FRAGMENT_SHADER, fsCode);
		if (!vs || !fs) {
			return 0;
		}
		GLuint program = glCreateProgram();
		glAttachShader(program, fs);
		glDeleteShader(fs);
		glAttachShader(program, vs);
		glDeleteShader(vs);
		glLinkProgram(program);
		GLint linkStatus = GL_FALSE;
		glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
		if (linkStatus != GL_TRUE) {
			GLint infoLen = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLen);
			if (infoLen) {
				std::vector<char> buf;
				buf.resize(infoLen);
				if (static_cast<int>(buf.size()) >= infoLen) {
					glGetProgramInfoLog(program, infoLen, NULL, buf.data());
					std::cerr << "ERROR: シェーダーのコンパイルに失敗\n" << buf.data() << std::endl;
				}
			}
			glDeleteProgram(program);
			return 0;
		}
		return program;


	}

	/**
	 *ファイルを読み込む
	 *
	 *@param filename 読み込むファイル名
	 *@param buf 読み込み先バッファ
	 *
	 *@return true 読み込み成功
	 *@return false 読み込み失敗
	 */
	bool ReadFile(const char* filename, std::vector<char>& buf)
	{
		struct stat st;
		if (stat(filename, &st)) {
			return false;
		}
		FILE* fp = fopen(filename, "rb");
		if (!fp) {
			return false;
		}
		buf.resize(st.st_size + 1);
		const size_t readSize = fread(buf.data(), 1, st.st_size, fp);
		fclose(fp);
		if (readSize != st.st_size) {
			return false;
		}
		buf.back() = '\0';
		return true;
	}

	/**
	 *  ファイルからシェーダープログラムを作成する
	 * @param vsCode 頂点シェーダファイル名
	 * @param fsCode フラグメントシェーダーファイル名
	 *
	 * @return 作成したプログラムオブジェクト
	 *
	 */
	GLuint CreateProgramFromFile(const char* vsFilename, const char* fsFilename)
	{
		std::vector<char>vsBuf;
		if (!ReadFile(vsFilename, vsBuf)) {
			std::cerr << "ERRROR in Sharder::CreateProgramFromFile:\n" << vsFilename << "読み込めません." << std::endl;
			return 0;
		}
		std::vector<char>fsBuf;
		if (!ReadFile(fsFilename, fsBuf)) {
			std::cerr << "ERRROR in Sharder::CreateProgramFromFile:\n" << fsFilename << "読み込めません." << std::endl;
			return 0;
		}
		
		return CreateShaderProgram(vsBuf.data(), fsBuf.data());
	}


}




