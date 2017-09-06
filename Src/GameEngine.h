#pragma once

#include<GL/glew.h>
#include"UniformBuffer.h"
#include"OffscreenBuffer.h"
#include"Shader.h"
#include"Texture.h"
#include"Mesh.h"
#include"Entity.h"
#include"Uniform.h"
#include"GamePad.h"
#include"Font.h"
#include<time.h>
#include<glm/glm.hpp>
#include<functional>
#include<random>



/**
 * ゲームエンジンクラス
 */
class GameEngine {
public:
	typedef std::function<void(double)> UpdateFuncType;
	//カメラデータ
	struct CameraData
	{
		glm::vec3 postion;
		glm::vec3 target;
		glm::vec3 up;

	};

	static GameEngine& Instance();
	bool Init(int w,int h,const char* title);
	void Run();
	void UpdateFunc(const UpdateFuncType& func);
	const UpdateFuncType& UpdateFunc()  const;



	bool LoadTextureFromFile(const char* filename);
	bool LoadMeshFromFile(const char* filename);
	Entity::Entity* AddEntity(int groupId,const glm::vec3& pos, const char* meshName,
		const char* texName, Entity::Entity::UpdateFuncType func,bool hasLight = true);
	void RemoveEntity(Entity::Entity*);
	void Light(int index, const Uniform::PointerLight& light);
	const Uniform::PointerLight& Light(int index) const;
	void AmbientLight(const glm::vec4& color);
	const glm::vec4& AmbientLight()const;
	void Camera(const CameraData& cam);
	const CameraData& Camera() const;
	std::mt19937& Rand();
	const GamePad& GetGamePad() const;

	void CollisionHandler(int gid0, int gid1, Entity::CollisionHandlerType handler);
	const Entity::CollisionHandlerType& CollisionHandler(int gid0, int gid1)const;
	void ClearCollisionHandlerList();

	bool LoadFontFile(const char* filename) {
		return fontRenderer.LoadFromFile(filename);
	}
	bool AddString(const glm::vec2& pos, const char* str) {
		return fontRenderer.AddString(pos, str);
	}
	void FontScale(const glm::vec2& scale) { fontRenderer.Scale(scale); }
	void FontColor(const glm::vec4& color) { fontRenderer.Color(color); }

	void Score(int s) { score = s; }
	int Score() const { return score; }

	const TexturePtr& GetTexture(const char* filename) const{
		return TextureBuffer.find(filename)->second;
	}


private:
	GameEngine() = default;
	~GameEngine();
	GameEngine(const GameEngine&) = delete;
	GameEngine&  operator = (const GameEngine&) = default;
	void Update(double delte);
	void Render()const;

private:
	bool isInitalized = false;
	UpdateFuncType updateFunc;





private:
	GLuint vbo = 0;
	GLuint ibo = 0;
	GLuint vao = 0;
	UniformBufferPtr uboLight;
	UniformBufferPtr uboPosEffect;
	Shader::ProgramPtr progTurtorial;
	Shader::ProgramPtr progColorFilter;
	OffscreenBufferPtr offscreen;

	std::unordered_map<std::string, TexturePtr>TextureBuffer;
	Mesh::BufferPtr meshBuffer;
	Entity::BufferPtr entityBuffer;
	Font::Renderer fontRenderer;
	Uniform::LightData lightData;
	CameraData camera;
	std::mt19937 rand;

	int score = 0;

};