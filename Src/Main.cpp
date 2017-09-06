/**
 * @file Main.cpp
 */

#include"GameEngine.h"
#include <random>
#include<glm/gtc/matrix_transform.hpp>

enum EntityGroupId {
	EntityGroupId_Player,
	EntityGroupId_PlayerShot,
	EntityGroupId_Enemy,
	EntityGroupId_EnemyShot,
	EntityGroupId_Others,
};

static const Entity::CollisionData collisionDataList[] = {
	{glm::vec3(-1.0f,-1.0f,-1.0f),glm::vec3(1.0f,1.0f,1.0f)},
	{ glm::vec3(-0.5f,-0.5f,-1.0f),glm::vec3(0.5f,0.5f,1.0f) },
	{ glm::vec3(-1.0f,-1.0f,-1.0f),glm::vec3(1.0f,1.0f,1.0f) },
	{ glm::vec3(0.25f,0.25f,0.25f),glm::vec3(0.25f,0.25f,0.25f) },
};

 //3Dベクター型
struct Vector3 {
	float x, y, z;
};
//RGBAカラー型
struct Color {
	float r, g, b, a;
};


/**
*敵の円盤の状態を更新する
*/
struct UpdateToroid
{
	//explicit UpdateToroid(Entity::BufferPtr& buffer) : entityBuffer(buffer) {}

	void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView,
		const glm::mat4& matProj) {

		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || std::abs(pos.z) > 40.0f) {
			GameEngine::Instance().RemoveEntity(&entity);
			return;
		}

		//円盤を回転させる
		float rot = glm::angle(entity.Rotation());
		rot += glm::radians(180.0f) * static_cast<float>(delta);
		if (rot > glm::pi<float>() * 2.0f) {
			rot -= glm::pi<float>() * 2.0f;
		}

		entity.Rotation(glm::angleAxis(rot,glm::vec3(0,1,0)));

		//頂点シェーダーのパラメータをUBOにコピーする
		Uniform::VertexData data;
		data.matModel = entity.CalcModelMaterial();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		memcpy(ubo, &data, sizeof(Uniform::VertexData));

	}

	

	//Entity::BufferPtr& entityBuffer;
};


struct UpdatePlayerShot
{
	void operator()(Entity::Entity& entity, void* ubo, double delta, const glm::mat4& matView,
		const glm::mat4& matProj) {

		const glm::vec3 pos = entity.Position();
		if (std::abs(pos.x) > 40.0f || pos.z < -4 || pos.z > 40) {
			entity.Destroy();
			return;
		}


		//頂点シェーダーのパラメータをUBOにコピーする
		Uniform::VertexData data;
		data.matModel = entity.CalcModelMaterial();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		memcpy(ubo, &data, sizeof(Uniform::VertexData));

	}

};


/**
 *自機の更新
 */
struct UpdatePlayer {

	void operator()(Entity::Entity& entity, void* ubo, double delta,
	const glm::mat4&  matView, const  glm::mat4 matProj) {
		GameEngine& game = GameEngine::Instance();
		const GamePad gamepad = game.GetGamePad();
		glm::vec3 vec;
		float rotZ = 0;
		if(gamepad.buttons & GamePad::DPAD_LEFT){
			vec.x = -1;
			rotZ = -glm::radians(30.0f);
		}
		else if (gamepad.buttons & GamePad::DPAD_RIGHT) {
			vec.x = 1;
			rotZ = glm::radians(30.0f);
		}
		if (gamepad.buttons & GamePad::DPAD_UP) {
			vec.z = 1;
		}else if (gamepad.buttons & GamePad::DPAD_DOWN) {
			vec.z = -1;
		}
		if (vec.x || vec.z) {
			vec = glm::normalize(vec) * 15.0f;
		}
		entity.Velocity(vec);
		entity.Rotation(glm::quat(glm::vec3(0, 0, rotZ)));
		glm::vec3 pos = entity.Position();
		pos = glm::min(glm::vec3(11, 100, 20), glm::max(pos, glm::vec3(-11, -100, 1)));
		entity.Position(pos);

		if (gamepad.buttons & GamePad::A) {
			shotInterval -= delta;
			if (shotInterval <= 0) {
				glm::vec3 pos = entity.Position();
				pos.x -= 0.3f;
				for (int i = 0;i < 2; ++i) {
					if (Entity::Entity* p = game.AddEntity(EntityGroupId_PlayerShot, pos,
						"NormalShot", "Res/Model/Player.bmp", UpdatePlayerShot())) {
						p->Velocity(glm::vec3(0, 0, 80));
						p->Collision(collisionDataList[EntityGroupId_PlayerShot]);
					}
					pos.x += 0.6f;
				}
				shotInterval = 0.25;
			}
			
		}else {
				shotInterval = 0;
			}



		//頂点シェーダーのパラメータをUBOにコピーする
		Uniform::VertexData data;
		data.matModel = entity.CalcModelMaterial();
		data.matNormal = glm::mat4_cast(entity.Rotation());
		data.matMVP = matProj * matView * data.matModel;
		memcpy(ubo, &data, sizeof(Uniform::VertexData));

	}
private:
	double shotInterval = 0;
};


	struct UpdateBlast {
		void operator()(Entity::Entity& entity, void* ubo, double delta,
			const glm::mat4& matView, const glm::mat4& matProj) {
			timer += delta;
			
			if (timer >= 0.5) {
				entity.Destroy();
				return;
			}
			const float variation = static_cast<float>(timer * 4);
			entity.Scale(glm::vec3(static_cast<float>(1 + variation)));

			static const glm::vec4 color[] = {
				glm::vec4(1.0f,1.0f,0.75f,1),
				glm::vec4(1.0f,0.5f,0.1f,1),
				glm::vec4(0.25f,0.1f,0.1f,0),
			};
			const glm::vec4 co10 = color[static_cast<int>(variation)];
			const glm::vec4 co11 = color[static_cast<int>(variation) + 1];
			const glm::vec4 newColor = glm::mix(co10, co11, std::fmod(variation, 1));
			entity.Color(newColor);

			glm::vec3 euler = glm::eulerAngles(entity.Rotation());
			euler.y += glm::radians(60.0f) * static_cast<float>(delta);
			entity.Rotation(glm::quat(euler));

			//頂点シェーダーのパラメータをUBOにコピーする
			Uniform::VertexData data;
			data.matModel = entity.CalcModelMaterial();
			data.matNormal = glm::mat4_cast(entity.Rotation());
			data.matMVP = matProj * matView * data.matModel;
			memcpy(ubo, &data, sizeof(Uniform::VertexData));

		}
	
	double timer = 0;
};


/**
 *ゲームの状態を更新する
 *
 *@param entityBuffer	敵エンテイテイ追加先のエンテイテイバッファ 
 *@param meshBuffer		敵エンテイテイのメッシュを管理しているメシュバッファ
 *@param tex			敵エンテイテイ用のテクスチャ
 *@param prog			敵エンテイテイ用シェーダープログラム
 */
struct Update
{
	void operator()(double delta)
	{

		GameEngine& game = GameEngine::Instance();

		if (!pPlayer) {
			pPlayer = game.AddEntity(EntityGroupId_Player, glm::vec3(0, 0, 2),
				"Aircraft", "Res/Model/Player.bmp", UpdatePlayer());
			pPlayer->Collision(collisionDataList[EntityGroupId_Player]);
		}

		game.Camera({ glm::vec4(0,20,-8,1),glm::vec3(0,0,12),glm::vec3(0,0,1) });
		game.AmbientLight(glm::vec4(0.05f, 0.1f, 0.2f, 1));
		game.Light(0, { glm::vec4(40,100,10,1),glm::vec4(12000,12000,12000,1) });
		std::uniform_int_distribution<>distributerX(-12, 12);
		std::uniform_int_distribution<>distributerZ(40, 44);
		interval -= delta;

		if (interval <= 0) {
			const std::uniform_real_distribution<>randInterval(2.0, 5.0);
			const std::uniform_int_distribution<>randAddingCount(1, 5);
			for (int i = randAddingCount(game.Rand()); i > 0; --i) {
				const glm::vec3 pos(distributerX(game.Rand()), 0, distributerZ(game.Rand()));
				if (Entity::Entity* p = game.AddEntity(
					EntityGroupId_Enemy, pos, "Toroid", "Res/Model/Toroid.bmp", UpdateToroid())) {
					p->Velocity(glm::vec3(pos.x < 0 ? 4.0f : -4.0f, 0, -16.0f));
					//p->Velocity(glm::vec3(pos.x < 0 ? 1.0f : -1.0f, 0, -4.0f));
					//p->Velocity(glm::vec3(pos.x < 0 ? 0.1f : -0.1f, 0, -0.4f));
					p->Collision(collisionDataList[EntityGroupId_Enemy]);
				}
			}
			interval = randInterval(game.Rand());
			
		}

		char str[16];
		snprintf(str, 16, "%08d", game.Score());
		game.FontScale(glm::vec2(2));
		game.FontColor(glm::vec4(1));
		game.AddString(glm::vec2(-0.2f, 0.9f), str);

	}

	double interval = 0;
	Entity::Entity* pPlayer = nullptr;
	
};


void PlayerShotAndEnemyCollisionHandler(Entity::Entity& lhs, Entity::Entity& rhs) {
	GameEngine& game = GameEngine::Instance();
	if (Entity::Entity* p = game.AddEntity(EntityGroupId_Others, rhs.Position(),
		"Blast", "Res/Model/Toroid.bmp", UpdateBlast())) {
		const std::uniform_real_distribution<float>rotRange(0.0f, glm::pi<float>() * 2);
		p->Rotation(glm::quat(glm::vec3(0, rotRange(game.Rand()), 0)));
		game.Score(game.Score() + 100);
	}
	lhs.Destroy();
	rhs.Destroy();
}

//エントリーポイント
int main() {

	GameEngine& game = GameEngine::Instance();
	if (!game.Init(800, 600, "OpenGL Tutorial")) {
		return 1;
	}
	game.LoadTextureFromFile("Res/Model/Toroid.bmp");
	game.LoadTextureFromFile("Res/Model/Player.bmp");
	game.LoadMeshFromFile("Res/Model/Toroid.fbx");
	game.LoadMeshFromFile("Res/Model/Player.fbx");
	game.LoadMeshFromFile("Res/Model/Blast.fbx");
	game.LoadFontFile("Res/UniNeue.fnt");

	game.CollisionHandler(EntityGroupId_PlayerShot, EntityGroupId_Enemy, &PlayerShotAndEnemyCollisionHandler);

	game.UpdateFunc(Update());
	game.Run();

	return 0;
}