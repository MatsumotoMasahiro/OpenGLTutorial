#include "GameEngine.h"
#include"GLFWEW.h"
#include<glm/gtc/matrix_transform.hpp>
#include<iostream>



//���_�f�[�^�^
struct Vertex {
	glm::vec3 position;//���W
	glm::vec4 color;//�F
	glm::vec2 texCoord;//�e�N�X�`�����W
};

//���_�f�[�^
const Vertex vertices[] = {


	{ { -0.5f,-0.3f,0.5f },{ 0.0f,0.0f,1.0f,1.0f } ,{ 0.0f,0.0f } },
	{ { 0.3f,-0.3f,0.5f },{ 0.0f,1.0f,0.0f,1.0f } ,{ 1.0f,0.0f } },
	{ { 0.3f, 0.5f,0.5f },{ 0.0f,0.0f,1.0f,1.0f } ,{ 1.0f,1.0f } },
	{ { -0.5f, 0.5f,0.5f },{ 0.0f,0.0f,1.0f,1.0f },{ 0.0f,1.0f } },


	{ { -0.3f, 0.3f,0.1f },{ 0.0f,0.0f,1.0f,1.0f } ,{ 0.0f,1.0f } },
	{ { -0.3f,-0.5f,0.1f },{ 0.0f,1.0f,1.0f,1.0f } ,{ 0.0f,0.0f } },
	{ { 0.5f,-0.5f,0.1f },{ 0.0f,0.0f,1.0f,1.0f } ,{ 1.0f,0.0f } },

	{ { 0.5f,-0.5f,0.1f },{ 1.0f,0.0f,0.0f,1.0f } ,{ 1.0f,0.0f } },
	{ { 0.5f, 0.3f,0.1f },{ 1.0f,1.0f,0.0f,1.0f } ,{ 1.0f,1.0f } },
	{ { -0.3f, 0.3f,0.1f },{ 1.0f,0.0f,0.0f,1.0f } ,{ 0.0f,1.0f } },


	{ { -1.0f,-1.0f,0.5f },{ 1.0f,1.0f,1.0f,1.0f } ,{ 1.0f,0.0f } },
	{ { 1.0f,-1.0f,0.5f },{ 1.0f,1.0f,1.0f,1.0f } ,{ 0.0f,0.0f } },
	{ { 1.0f, 1.0f,0.5f },{ 1.0f,1.0f,1.0f,1.0f } ,{ 0.0f,1.0f } },
	{ { -1.0f, 1.0f,0.5f },{ 1.0f,1.0f,1.0f,1.0f },{ 1.0f,1.0f } },

};

//�C���f�b�N�X�f�[�^
const GLuint indices[] = {
	0,1,2,2,3,0,
	4,5,6,7,8,9,
	10,11,12,12,13,10,
};


//�����`��f�[�^
struct RenderingPart {
	GLsizei size;//�`�悷��C���f�b�N�X
	GLvoid* offset;//�`��J�n�C���f�b�N�X�̔{�ƃI�t�Z�b�g
};

/**
* RenderingPart���쐬����
*
*�@@param size ���`�悷��C���f�b�N�X��
*  @param offset �`��J�n�C���f�b�N�X�̃I�t�Z�b�g(�C���f�b�N�X�P��)
*
*
* @return �쐬���������`��I�u�W�F�N�g
*/
constexpr RenderingPart MakeRenderingPart(GLsizei size, GLsizei offset) {
	return{ size,reinterpret_cast<GLvoid*>(offset * sizeof(GLuint)) };
}

/**
* �����`��f�[�^���X�g
*/
static const RenderingPart renderingParts[] = {
	MakeRenderingPart(12,0),
	MakeRenderingPart(6,12),
};


/**
*Vertex Buffer Object���쐬����
*
*@param size���_�f�[�^�̃T�C�Y
*
*@param data ���_�f�[�^�ւ̃|�C���^
*
*@return �쐬����VBO
*/
GLuint CreateVBO(GLsizeiptr size, const GLvoid* data) {
	GLuint vbo = 0;
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	return vbo;
}
/**
*Index Buffer Object���쐬����
*
*@param size�C���f�b�N�X�f�[�^�̃T�C�Y
*
*@param data �C���f�b�N�X�f�[�^�ւ̃|�C���^
*
*@return �쐬��IBO
*/
GLuint CreateIBO(GLsizeiptr size, const GLvoid* data) {
	GLuint ibo = 0;
	glGenBuffers(1, &ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return ibo;
}


/**
*���_�A�g���r���[�g��ݒ肷��
*
*@param index ���_�A�g���r���[�g�̃C���f�b�N�X
*@param cls	���_�f�[�^�^��
*@param mbr ���_�A�g���r���[�g�ɐݒ肷��cls�̃����o�ϐ���
*/
#define SetVertexAttribPointer(index,cls,mbr)SetVertexAttribPointerI(\
index,\
sizeof(cls::mbr) / sizeof(float),\
sizeof(cls),\
reinterpret_cast<GLvoid*>(offsetof(cls,mbr)))

void SetVertexAttribPointerI(GLuint index, GLint size, GLsizei stride, const GLvoid* pointer) {
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, GL_FLOAT, GL_FALSE, stride, pointer);
}


/**
*Vertex Array Object���쐬����
*
*@param vbo VAO�Ɋ֘A�t������VBO
*@param ibo VAO�Ɋ֘A�t������IBO
*
*@return �쐬����VAO
*/
GLuint CreateVAO(GLuint vbo, GLuint ibo) {
	GLuint vao = 0;
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	SetVertexAttribPointer(0, Vertex, position);
	SetVertexAttribPointer(1, Vertex, color);
	SetVertexAttribPointer(2, Vertex, texCoord);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
	glBindVertexArray(0);
	return vao;
}






GameEngine& GameEngine::Instance() {
	static  GameEngine instance;
	return instance;
}

bool GameEngine::Init(int w, int h, const  char* title) {



	if (isInitalized) {
		return true;
	}
	if (!GLFWEW::Window::Instance().Init(w, h, title)) {
		return false;
	}




	 vbo = CreateVBO(sizeof(vertices), vertices);
	 ibo = CreateIBO(sizeof(indices), indices);
	 vao = CreateVAO(vbo, ibo);
	 uboLight = UniformBuffer::Create(sizeof(Uniform::LightData), 1, "LightData");
	 uboPosEffect = UniformBuffer::Create(sizeof(Uniform::PostEffectData), 2, "PostEffectData,");
	 progTurtorial = Shader::Program::Create("Res/Tutorial.vert", "Res/Tutorial.frag");
	 progColorFilter = Shader::Program::Create("Res/ColorFilter.vert", "Res/ColorFiter.frag");
	 offscreen = OffscreenBuffer::Create(800, 600/*, GL_RGBA16F*/);
	 if (!vbo || !ibo || !vao || !uboLight || !uboPosEffect || !progTurtorial || !progColorFilter || ! offscreen) {
		 std::cerr << "ERROR:GameEngine�̏������Ɏ��s" << std::endl;
		 return false;
	 }


	 progTurtorial->UniformBlockBinding("VertexData", 0);
	 progTurtorial->UniformBlockBinding("LightData", 1);
	 progColorFilter->UniformBlockBinding("PostEffectData", 2);

	meshBuffer = Mesh::Buffer::Create(10 * 1024, 30 * 1024);
	if (!meshBuffer) {
		std::cerr << "ERROR:GameEngine�̏������Ɏ��s" << std::endl;
		return false;
		} 
	entityBuffer = Entity::Buffer::Create(1024,sizeof(Uniform::VertexData),0,"VertexData");
	if (!entityBuffer) {
		std::cerr << "ERROR:GameEngine�̏������Ɏ��s" << std::endl;
		return false;
	}
	rand.seed(std::random_device()());
	fontRenderer.Init(1024, glm::vec2(800, 600));
	isInitalized = true;
	return true;
}


/**
 *�Q�[�������s
 */
void GameEngine::Run() {
	//const double delta = 1.0 / 60.0;
	GLFWEW::Window& window = GLFWEW::Window::Instance();

	double prevTime = glfwGetTime();
	while (!window.ShouldClose()) {

		const double curTime = glfwGetTime();
		const double delta = curTime - prevTime;
		prevTime = curTime;
		window.UpdateGamePad();
		Update(glm::min(0.25, delta));
		Render();
		window.SwapBuffers();
		
	}
}


/**
 *��ԍX�V�֐���ݒ肷��
 *
 *@param func�@�ݒ肷��X�V�֐�
 *
 */
void GameEngine::UpdateFunc(const UpdateFuncType& func){
	updateFunc = func;
}

/**
 *�e�N�X�`����ǂݍ���
 *
 *@param filename	�e�N�X�`���t�@�C����
 *@retval true		�ǂݍ��ݐ���
 *@retval false		�ǂݍ��ݎ��s
 */
bool GameEngine::LoadTextureFromFile(const char* filename) {
	const auto itr = TextureBuffer.find(filename);
	if (itr != TextureBuffer.end()) {
		return true;
	}
	TexturePtr texture = Texture::LoadFromFile(filename);
	if (!texture) {
		return false;
	}
	TextureBuffer.insert(std::make_pair(std::string(filename), texture));
	return true;
}

/**
 * ���b�V����ǂݍ���
 *@param filename�@���b�V���t�@�C����
 *@retval true  �ǂݍ��ݐ���
 *@retval false �ǂݍ��ݎ��s
 */
bool GameEngine::LoadMeshFromFile(const char* filename) {
	return meshBuffer->LoadMeshFromFile(filename);
}

Entity::Entity* GameEngine::AddEntity(int groupId,const glm::vec3& pos, const char* meshName,
	const char* texName, Entity::Entity::UpdateFuncType func,bool hasLight) {
	const Mesh::MeshPtr& mesh = meshBuffer->GetMesh(meshName);
	const TexturePtr& tex = TextureBuffer.find(texName)->second;
	return entityBuffer->AddEntity(groupId,pos, mesh, tex, progTurtorial, func);
}

void GameEngine::RemoveEntity(Entity::Entity* e) {
	entityBuffer->RemoveEntity(e);
}


/**
*���C�g 
*
*
*/
void GameEngine::Light(int index, const Uniform::PointerLight& light) {
	if (index < 0 || index >= Uniform::maxLightCount) {
		std::cerr << "WARNIG:  '" << index << "' �͕s���ȃ��C�g�C���f�b�N�X" << std::endl;
		return;
	}
	lightData.light[index] = light;
}


/**
 *���C�g�擾
 *
 *
 */
const Uniform::PointerLight& GameEngine::Light(int index)const {
	if (index < 0 || index >= Uniform::maxLightCount) {
		std::cerr << "WARNING:	'" << index << "'�͕s���ȃ��C�g�C���f�b�N�X" << std::endl;
		static const Uniform::PointerLight dummy;
		return dummy;
	}
	return lightData.light[index];
}


/**
 *��
 *
 */
void GameEngine::AmbientLight(const glm::vec4& color) {
	lightData.ambientColor = color;
}

/**
 *���@�擾
 *
 */
const glm::vec4& GameEngine::AmbientLight()const {
	return lightData.ambientColor;
}

/**
 *���_�̈ʒu
 *
 */
void GameEngine::Camera(const CameraData& cam) {
	camera = cam;
}

/** 
 * ���_�@�ʒu�@�擾
 */
const GameEngine::CameraData& GameEngine::Camera()const {
	return camera;
}

/**
 * �����@�擾
 */
std::mt19937& GameEngine::Rand() {
	return rand;
}


/**
* �Q�[���p�b�h�̏�Ԃ��擾����
*/
const GamePad& GameEngine::GetGamePad()const {
	return GLFWEW::Window::Instance().GetGamePad();
}

void GameEngine::CollisionHandler(int gid0, int gid1,
	Entity::CollisionHandlerType hasdler) {
	entityBuffer->CollisionHandler(gid0, gid1, hasdler);
}

const Entity::CollisionHandlerType& GameEngine::CollisionHandler(
int gid0,int gid1) const{
	return entityBuffer->CollisionHandler(gid0, gid1);
}

void GameEngine::ClearCollisionHandlerList() {
	entityBuffer->ClearCollisionHandelerList();
}


/**
 *��Ԍĉ��^�֐����擾����
 *@return �ݒ肳��Ă���X�V����
 */
const GameEngine::UpdateFuncType& GameEngine::UpdateFunc()const {
	return updateFunc;
}

/**
 * �f�X
 */
GameEngine::~GameEngine() {


	if (vao) {
		glDeleteVertexArrays(1, &vao);
	}
	if (ibo) {
		glDeleteBuffers(1, &ibo);
	}if (vbo) {
		glDeleteBuffers(1, &vbo);
	}


}

/** 
*�Q�[��ԍX�V
*@param delta�@�O����̍X�V�@�o��
*/
void GameEngine::Update(double delta) {

	fontRenderer.MapBuffer();
	
	if (updateFunc) {
		updateFunc(delta);
	}

	const glm::mat4x4 matProj = glm::perspective(
		glm::radians(45.0f), 800.0f / 600.0f, 1.0f, 200.0f);
	const glm::mat4x4 matView = glm::lookAt(camera.postion, camera.target,
		camera.up);
	entityBuffer->Update(delta, matView, matProj);
	fontRenderer.UnmapBuffer();

}

/**
* �`��ꏊ
*/
void GameEngine::Render()const{

	glBindFramebuffer(GL_FRAMEBUFFER, offscreen->GetFramebuffer());
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glViewport(0, 0, 800, 600);
	glScissor(0, 0, 800, 600);
	glClearColor(0.1f, 0.3f, 0.5f, 1.0f);
	glClearDepth(1);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	uboLight->BufferSubData(&lightData);
	entityBuffer->Draw(meshBuffer);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	glBindVertexArray(vao);
	progColorFilter->UseProgram();
	Uniform::PostEffectData posEffecr;
	uboPosEffect->BufferSubData(&posEffecr);
	progColorFilter->BindTexture(GL_TEXTURE0, GL_TEXTURE_2D, offscreen->GetTexutre());
	glDrawElements(GL_TRIANGLES, renderingParts[1].size, GL_UNSIGNED_INT, renderingParts[1].offset);
	fontRenderer.Draw();
}
