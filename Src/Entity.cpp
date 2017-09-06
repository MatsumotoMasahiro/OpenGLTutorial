/**
 *@file Entity.cpp
 */
#include "Entity.h"
#include <glm/gtc/matrix_transform.hpp>
#include<iostream>
#include<algorithm>

/**
 *エンテイテイに関するコードを格納する名前空間
 */
namespace Entity {

	/**
	 *移動・回転・拡縮行列を取得する
	 *
	 *@return TRS行列
	 */
	glm::mat4 Entity::CalcModelMaterial()const {
		const glm::mat4 t = glm::translate(glm::mat4(), position);
		const glm::mat4 r = glm::mat4_cast(rotation);
		const glm::mat4 s = glm::scale(glm::mat4(), scale);
		return t * r * s;
	}

	void Entity::Destroy() {

		this->pBuffer;
		if (pBuffer) {
			pBuffer->RemoveEntity(this);
		}
	}

	/**
	 * 自分自身をリンクりすとから霧hナス
	 *
	 *自分はどこにも接続されていない状態になる
	 */
	void Buffer::Link::Remove() {
		next->prev = prev;
		prev->next = next;
		prev = this;
		next = this;
	}

	/**
	* リンクオブジェクトを自分の手前に追加する
	*
	* @pram p 追加するリンクオブジェクトへのポインタ
	*
	*pを所属元のリンクリストから切り離し自分の手前に追加する
	*/
	void Buffer::Link::Insert(Link* p) {
		p->Remove();
		p->prev = prev;
		p->next = this;
		prev->next = p;
		prev = p;
	}

	/**
	* エンテイテイバッファを作成する
	*
	*@param maxEntityCount 扱えるエンテイテイの最大数
	*@param ubSizePerEntity エンテイテイのごとのUniformBufferの倍とス
	*@param bindingPoint エンテイテイ用UBOのバインディングポインタ
	*@param uName   エンテイテイ用UniforBufferの名前
	*
	*@return　作成したエンテイテイバッファへのポインタ
	*
	*/
	BufferPtr Buffer::Create(size_t maxEntityConut, GLsizeiptr ubSizePerEntity, int bindingPoint,
		const char* ubName) {
		struct Impl : Buffer{Impl(){} ~Impl(){} };
		
		BufferPtr p = std::make_shared<Impl>();
		if (!p) {
			std::cerr << "WARNIG in Entity::Buffer::Create:バッファの作成に失敗" << std::endl;
			return{};
		}
		p->ubo = UniformBuffer::Create(
			maxEntityConut * ubSizePerEntity, bindingPoint, ubName);
			p->buffer.reset(new LinkEntity[maxEntityConut]);
			if (!p->ubo || !p->buffer) {
				std::cerr << "WARNIG in Entity::Buffer::Create:バッファの作成に失敗" << std::endl;
				return{};
		}
			p->bufferSize = maxEntityConut;
			p->ubSizePerEntity = ubSizePerEntity;
			GLintptr offset = 0;
			const LinkEntity* const end = &p->buffer[maxEntityConut];
			for (LinkEntity* itr = &p->buffer[0];itr != end;++itr) {
				itr->uboOffset = offset;
				itr->pBuffer = p.get();
				p->freeList.Insert(itr);
				offset += ubSizePerEntity;
			}
			p->collisionHandlerList.reserve(maxGroupId);
			return p;

	}

	/**
	 *エンテイテイを追加
	 *
	 *@param position エンテイテイの座標
	 *@param mesh エンテイテイの表示に使用するメッシュ
	 *@param texture　エンテイテイの表示に使うテクスチャ
	 *@param program  エンテイテイの表示に使うシェーダープログラム
	 *@param func     エンテイテイの状態を更新する関数
	 *
	 *@reuturn 追加したエンテイテイへのポインタ
	 *			これ以上エンテイテイを追加できない場合はnillptrが返される
	 *			回転や拡大率を設定する場合はこのポインタ経由で行う
	 *			このポインタをアプリケーションがわで保持する必要はない
	 */
	Entity* Buffer::AddEntity(int  groupId, const glm::vec3& position, const Mesh::MeshPtr& mesh,
		const TexturePtr& texture, const Shader::ProgramPtr& program,Entity::UpdateFuncType func) 
	{
		if (freeList.prev == freeList.next) {
			std::cerr << "WARNIG in Entity::Buffer::AddEntity:空きエンテイテイがありません" << std::endl;
			return{};
		}
		if (groupId < 0 || groupId > maxGroupId) {
			std::cerr << "ERROR  in Entity::Buffer::Addentity: 範囲外のグループID(" << groupId <<
				")が渡されました.\nグループIDは0~" << maxGroupId <<
				"でなければなりません" << std::endl;
			return nullptr;
		}
		LinkEntity* entity = static_cast<LinkEntity*>(freeList.prev);
		activeList[groupId].Insert(entity);
		entity->groupId = groupId;
		entity->position = position;
		entity->rotation = glm::quat();
		entity->scale = glm::vec3(1, 1, 1);
		entity->velocity = glm::vec3();
		entity->mesh = mesh;
		entity->texture = texture;
		entity->program = program;
		entity->updateFunc = func;
		entity->isActive = true;
		return entity;
	}

	/**
	 * エンテイテイを削除する
	 *
	 *@param 削除するエンテイテイのポインタ
	 */
	void Buffer::RemoveEntity(Entity* entity) {
		if (!entity || !entity->isActive) {
			std::cerr << "WARNIG in Entity::Buuffer::RemoveEntity:非アクティブなエンテイテイを削除削除しようとしました" << std::endl;
			return;
		}
		LinkEntity* p = static_cast<LinkEntity*>(entity);
		if (p < &buffer[0] || p >= &buffer[bufferSize]) {
			std::cerr << "WARNIG in Entity::Buuffer::RemoveEntity:異なるバッファから取得したエンテイテイを削除しようとしました" << std::endl;
			return;
		}
		if (p == itrUpdate) {
			itrUpdate = p->prev;
		}
		if (p == itrUpdateRhs) {
			itrUpdateRhs = p->prev;
		}
		freeList.Insert(p);
		p->mesh.reset();
		p->texture.reset();
		p->program.reset();
		p->updateFunc = nullptr;
		p->isActive = false;
	}

	/**
	 *矩形同士の衝撃判定
	 */
	bool HasCollision(const CollisionData& lhs, const CollisionData& rhs) {
		if (lhs.max.x < rhs.min.x || lhs.min.x > rhs.max.x) return false;
		if (lhs.max.y < rhs.min.y || lhs.min.y > rhs.max.y) return false;
		if (lhs.max.z < rhs.min.z || lhs.min.z > rhs.max.z) return false;
		return true;
	}


	/**
	 *アクティブなエンテイテイの状態を更新する
	 *
	 *@param delta 前回の更新からの経過時間
	 *@param matView Vies行列
	 *@param matProj Project行列
	 */
	void Buffer::Update(double delta, const glm::mat4& matView, const glm::mat4& matProj)
	{
		for (int groupId = 0;groupId <= maxGroupId;++groupId) {
			for (Link* itr = activeList[groupId].next;itr != &activeList[groupId];itr = itr->next) {
				LinkEntity& e = *static_cast<LinkEntity*>(itr);
				e.position += e.velocity * static_cast<float>(delta);
				e.colWorld.min = e.colLocal.min + e.position;
				e.colWorld.max = e.colLocal.max + e.position;
			}
		}

		//衝突判定
		for (const auto& e : collisionHandlerList) {
			if (!e.handler) {
				continue;
			}
			Link* listL = &activeList[e.groupId[0]];
			Link* listR = &activeList[e.groupId[1]];
			for (itrUpdate = listL->next;itrUpdate != listL;itrUpdate = itrUpdate->next) {
				LinkEntity* entityL = static_cast<LinkEntity*>(itrUpdate);
				for (itrUpdateRhs = listR->next; itrUpdateRhs != listR;
					itrUpdateRhs = itrUpdateRhs->next) {
					LinkEntity* entityR = static_cast<LinkEntity*>(itrUpdateRhs);
					if (!HasCollision(entityL->colWorld, entityR->colWorld)) {
						continue;
					}
					e.handler(*entityL, *entityR);
					if (entityL != itrUpdate) {
						break;
					}
				}
			}
		}
		//各園庭テイ
		uint8_t* p = static_cast<uint8_t*>(ubo->MapBuffer());
		for (int groupId = 0;groupId <= maxGroupId;++groupId) {
			for (itrUpdate = activeList[groupId].next;itrUpdate != &activeList[groupId];
				itrUpdate = itrUpdate->next) {
				LinkEntity& e = *static_cast<LinkEntity*>(itrUpdate);
				if (e.updateFunc) {
					e.updateFunc(e, p + e.uboOffset, delta, matView, matProj);
				}
			}
		}
		itrUpdate = nullptr;
		itrUpdateRhs = nullptr;
		ubo->UnmapBuffer();
	}
	/** 
	 *アクティブなエンテイテイを描画する
	 *
	 *
	 *@param meshBuffer描画に使用するメッシュバッファへのポインタ
	 */
	void Buffer::Draw(const Mesh::BufferPtr& meshBuffer)const {
		meshBuffer->BindVAO();
		for (int groupId = 0;groupId <= maxGroupId;++groupId) {
			for (const Link* itr = activeList[groupId].next; itr != &activeList[groupId];itr = itr->next) {
				const LinkEntity& e = *static_cast<const LinkEntity*>(itr);
				if (e.mesh && e.texture  && e.program) {
					e.program->UseProgram();
					e.program->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, e.texture->Id());
					ubo->BindBufferRange(e.uboOffset, ubSizePerEntity);
					e.mesh->Draw(meshBuffer);
				}
			}
		}
	}



	void Buffer::CollisionHandler(int gid0, int gid1, CollisionHandlerType handler) {
		if (gid0 > gid1) {
			std::swap(gid0, gid1);
		}
		auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(),
			[&](const CollisionHandlerInfo& e)
		{ return e.groupId[0] == gid0 && e.groupId[1] == gid1;});
		if (itr == collisionHandlerList.end()) {
			collisionHandlerList.push_back({ {gid0,gid1},handler });
		}else{
			itr->handler = handler;
		}
	}


	const CollisionHandlerType& Buffer::CollisionHandler(int gid0, int gid1)const {
		if (gid0 > gid1) {
			std::swap(gid0, gid1);
		}
		auto itr = std::find_if(collisionHandlerList.begin(), collisionHandlerList.end(),
			[&](const CollisionHandlerInfo& e)
		{ return e.groupId[0] == gid0 && e.groupId[1] == gid1; });
		if (itr == collisionHandlerList.end()) {
			static const CollisionHandlerType dummy;
			return dummy;
		}
		return itr->handler;
	}

	void Buffer::ClearCollisionHandelerList() {
		collisionHandlerList.clear();
	}

}