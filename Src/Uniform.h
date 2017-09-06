#pragma once

/**
 *@file Uniform.h
 */

#include<glm/glm.hpp>


namespace Uniform {

	//���_�V�F�[�_�̃p�����[�^�^
	struct  VertexData
	{
		glm::mat4 matMVP;
		glm::mat4 matModel;
		glm::mat3x4 matNormal;
	};


	/**
	*���C�g�f�[�^
	*/
	struct  PointerLight
	{
		glm::vec4 position;
		glm::vec4 color;
	};


	const int maxLightCount = 4;

	/**
	* ���C�e�B���O�p�����[�^
	*/
	struct  LightData
	{
		glm::vec4 ambientColor;
		PointerLight light[maxLightCount];
	};
	/**
	* �|�X�g�G�t�F�N�g�f�[�^
	*/
	struct PostEffectData {
		glm::mat4x4 matColor;
	};
}

