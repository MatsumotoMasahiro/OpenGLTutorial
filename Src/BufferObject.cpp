
#include"BufferObject.h"

/**
 *
 *
 *
 *
 */
void BufferObject::Init(
	GLenum target,GLsizeiptr size,const GLvoid* data,GLenum usage){
	Destroy();
	glGenBuffers(1, &id);
	glBindBuffer(target, id);
	glBufferData(target,size,data,usage);
	glBindBuffer(target, 0);
}

/**
* 
*/
void BufferObject::Destroy() {
	if (id) {
		glDeleteBuffers(1, &id);
		id = 0;
	}
}

/**
 *
 *
 *
 */
void VertexArrayObject::Init(GLuint vbo, GLuint ibo) {
	Destroy();
	glGenVertexArrays(1, &id);
	glBindVertexArray(id);
	glBindBuffer(GL_ARRAY_BUFFER,vbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,ibo);
	glBindVertexArray(0);
}

/**
 *VAO��j������
 */
void VertexArrayObject::Destroy() {
	if (id) {
		glDeleteVertexArrays(1, &id);
		id = 0;
	}
}


/**
 *���_�A�g���r���[�g��ݒ�
 *
 *@param index
 *@param size
 *@param type
 *@param normalize
 *@param stride
 *@param offset
 */
void VertexArrayObject::VertexAttribPointer(GLuint index, GLint size, GLenum type,
	GLboolean normalized, GLsizei stride, size_t offset) {
	glEnableVertexAttribArray(index);
	glVertexAttribPointer(index, size, type, normalized, stride,
		reinterpret_cast<GLvoid*>(offset));
}

/**
 * VAO����������
 */
void VertexArrayObject::Bind()const {
	glBindVertexArray(id);
}

/** 
 * VAO�������Ώۂ���O��
 */
void VertexArrayObject::Unbind()const {
	glBindVertexArray(0);
}
