#pragma once
/**
 *@file Entity.h
 */
#include<GL/glew.h>
#include"Mesh.h"
#include"Texture.h"
#include"Shader.h"
#include"UniformBuffer.h"
#include<glm/glm.hpp>
#include<glm/gtx/quaternion.hpp>
#include<memory>
#include<functional>
#include<vector>

namespace Entity {
	class Entity;
	class Buffer;
	typedef std::shared_ptr<Buffer>BufferPtr;

	//衝突
	typedef std::function<void(Entity&, Entity&)>CollisionHandlerType;

	static const int maxGroupId = 31;

	/**
	 *衝突判定形状
	 */
	struct CollisionData {
		glm::vec3 min;
		glm::vec3 max;
	};

	/**
	 *エンテイテイ
	 */
	class Entity {
		friend class Buffer;

	public:
		//状態更新関数型
		typedef std::function<void(Entity&, void*, double, const glm::mat4&, const glm::mat4&)>UpdateFuncType;

		void Position(const glm::vec3& v) { position = v; }
		const glm::vec3& Position() const { return position; }
		void Rotation(const glm::quat& q) { rotation = q; }
		const glm::quat& Rotation()const { return rotation; }
		void Scale(const glm::vec3& v) { scale = v; }
		const glm::vec3& Scale()const { return scale; }
		void Velocity(const glm::vec3& v) { velocity = v; }
		const glm::vec3& Velocity()const { return velocity; }
		void Color(const glm::vec4& c) { color = c; }
		const glm::vec4& Color()const { return color; }
		void UpdateFunc(const UpdateFuncType& func) { updateFunc = func; }
		const UpdateFuncType& UpdateFunc() const { return updateFunc; }
		void Collision(const CollisionData&  c) { colLocal = c; }
		const CollisionData&  Collision() const { return colLocal; }

		glm::mat4 CalcModelMaterial() const;
		int GroupId() const { return groupId; }
		void Destroy();


	private:
		Entity() = default;
		~Entity() = default;
		Entity(const Entity&) = default;
		Entity& operator=(const Entity&) = default;

	private:
		int groupId = -1;
		Buffer* pBuffer = nullptr;
		glm::vec3 position;
		glm::vec3 scale = glm::vec3(1, 1, 1);
		glm::quat rotation;
		glm::vec3 velocity;
		glm::vec4 color = glm::vec4(1, 1, 1, 1);
		UpdateFuncType updateFunc;
		Mesh::MeshPtr mesh;
		TexturePtr texture;
		Shader::ProgramPtr program;
		GLintptr uboOffset;
		CollisionData colLocal;
		CollisionData colWorld;
		bool isActive = false;

	};

	class Buffer {
	public:
		static BufferPtr Create(size_t maxEntityCount, GLsizeiptr ubSizePerEntity, int bindingPoint, const char* name);

		Entity* AddEntity(int groupId,const glm::vec3& pos, const Mesh::MeshPtr& m, const TexturePtr& t,
			const Shader::ProgramPtr& p, Entity::UpdateFuncType func);
		void RemoveEntity(Entity* entity);
		void Update(double delta, const glm::mat4& matView, const glm::mat4& matProj);
		void Draw( const Mesh::BufferPtr& meshBuffer )const;

		void CollisionHandler(int gid0, int gid1, CollisionHandlerType handler);
		const CollisionHandlerType& CollisionHandler(int gid0,int gid1)const;
		void ClearCollisionHandelerList();

		private:
			Buffer() = default;
			~Buffer() = default;
			Buffer(const Buffer&) = delete;
			Buffer& operator=(const Buffer&) = delete;

	private:
		//エンテイテイ用リンクリスト
		struct Link
		{
			void Insert(Link* e);
			void Remove();
			Link* prev = this;
			Link* next = this;
		};
		//リンク付きエンテイテイ
		struct LinkEntity : public Link,public Entity{};
		//リンク付きエンテイテイ配列の削除用関数オブジェクト
		struct EntityArrayDelete { void operator()(LinkEntity* p) { delete[] p; } };

		std::unique_ptr<LinkEntity[], EntityArrayDelete>buffer;//エンテイテイの配列
		size_t bufferSize;//エンテイテイの総数
		Link freeList;//未使用のエンテイテイのリンクリスト
		Link activeList[maxGroupId + 1];//使用中のエンテイテイのリンクリスト
		GLsizeiptr ubSizePerEntity;//各エンテイテイが使えるUniform Bufferのバイト数
		UniformBufferPtr ubo;//エンテイテイ用UBO
		Link* itrUpdate = nullptr;//UpdateとRemoveEntityの相互作用に対応するためのイテレーター
		Link* itrUpdateRhs = nullptr;

		struct CollisionHandlerInfo {
			int groupId[2];
			CollisionHandlerType handler;
		};
		std::vector<CollisionHandlerInfo> collisionHandlerList;
	};

}