#pragma once
/**
 * @file Mesh.h
 */
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <vector>
#include <string>
#include <unordered_map>
#include <memory>

namespace Mesh {
	class Mesh;
	class Buffer;
	typedef std::shared_ptr<Mesh> MeshPtr;//メッシュデータポインタ型
	typedef std::shared_ptr<Buffer> BufferPtr;//メッシュバッファポインタ型
	/**
	 * マテリアル構造体
	 */
	struct  Material
	{
		GLenum type;//インデックスのデータ型
		GLsizei size;//描画するインデックス数
		GLvoid* offset;//描画開始インデックスのバイトのオフセット
		GLint baseVertex;//インデックス0とみなされる頂点配列内の位置
		glm::vec4 color;//マテリアルの色
	};


	
	/**
	 * メッシュ
	 */
	class Mesh {
		friend class Buffer;
	public:
		const std::string& Name() const { return name; }
		void Draw(const BufferPtr& buffer) const;


	private:
		Mesh() = default;
		Mesh(const std::string& n, size_t begin, size_t end);
		Mesh(const Mesh&) = default;
		~Mesh() = default;
		Mesh& operator = (const Mesh&) = default;

	private:
		std::string name;
		std::vector<std::string> textureList;
		size_t beginMaterial = 0;
		size_t endMaterial = 0;
	};

	/**
	 * メッシュバッファ
	 */
	class  Buffer {
	public:
		static BufferPtr Create(int vboSize, int iboSize);

		bool LoadMeshFromFile(const char* filename);
		const MeshPtr& GetMesh(const char* name)const;
		const Material& GetMaterial(size_t index)const;
		void BindVAO()const;

	private:
		Buffer() = default;
		~Buffer();
		Buffer(const Buffer&) = delete;
		Buffer& operator = (const Buffer&) = delete;

	private:
		GLuint vbo = 0;
		GLuint ibo = 0;
		GLuint vao = 0;
		GLintptr vboEnd = 0;
		GLintptr iboEnd = 0;
		std::vector<Material> materialList;
		std::unordered_map<std::string, MeshPtr>MeshList;


	};
}

