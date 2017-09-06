#pragma once

/**
 *@file Uniform.h
 */

#include<glm/glm.hpp>


namespace Uniform {

	//頂点シェーダのパラメータ型
	struct  VertexData
	{
		glm::mat4 matMVP;
		glm::mat4 matModel;
		glm::mat3x4 matNormal;
	};


	/**
	*ライトデータ
	*/
	struct  PointerLight
	{
		glm::vec4 position;
		glm::vec4 color;
	};


	const int maxLightCount = 4;

	/**
	* ライティングパラメータ
	*/
	struct  LightData
	{
		glm::vec4 ambientColor;
		PointerLight light[maxLightCount];
	};
	/**
	* ポストエフェクトデータ
	*/
	struct PostEffectData {
		glm::mat4x4 matColor;
	};
}

