#include "BBE/FragmentShader.h"
#include "BBE/SimpleFile.h"
#include "BBE/Window.h"
#include "BBE/PrimitiveBrush2D.h"
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif
#ifdef BBE_RENDERER_OPENGL
#include "BBE/OpenGL/OpenGLFragmentShader.h"
#endif

bbe::FragmentShader::FragmentShader()
{
}

bbe::FragmentShader::FragmentShader(const char* path)
{
	load(path);
}

bbe::FragmentShader::FragmentShader(const bbe::List<unsigned char>& rawData)
{
	load(rawData);
}

void bbe::FragmentShader::load(const char* path)
{
	load(bbe::simpleFile::readBinaryFile(path));
}

void bbe::FragmentShader::load(const bbe::List<unsigned char>& rawData)
{
	if (bbe::Window::INTERNAL_firstInstance == nullptr)
	{
		throw NullPointerException();
	}
#ifdef BBE_RENDERER_VULKAN
	if (bbe::INTERNAL::vulkan::VulkanManager::s_pinstance == nullptr)
	{
		throw NullPointerException();
	}
#endif
	if (isLoaded)
	{
		throw AlreadyCreatedException();
	}

	m_rawData = rawData;
#ifdef BBE_RENDERER_VULKAN
	memset(pushConstants.getRaw(), 0, pushConstants.getLength());
#endif

	isLoaded = true;
}

#ifdef BBE_RENDERER_VULKAN
void bbe::FragmentShader::setPushConstant(uint32_t offset, uint32_t length, const void* data)
{
	if (offset < PUSHCONST_START_ADDR || offset + length > PUSHCONST_START_ADDR + pushConstants.getLength())
	{
		//Only in the range of [80..128) the push constants are guaranteed to be present.
		throw IllegalArgumentException();
	}
	memcpy(pushConstants.getRaw() + offset - PUSHCONST_START_ADDR, data, length);
}
#endif

#ifdef BBE_RENDERER_OPENGL
void bbe::FragmentShader::setUniform2fv(const char* name, GLsizei size, const bbe::Vector2* values)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)m_prendererData.get();
	glUseProgram(s->twoD.program);
	glUniform2fv(glGetUniformLocation(s->twoD.program, name), size * 2, (float*)values);
	glUseProgram(s->threeD.program);
	glUniform2fv(glGetUniformLocation(s->threeD.program, name), size * 2, (float*)values);
	glUseProgram(s->threeDBake.program);
	glUniform2fv(glGetUniformLocation(s->threeDBake.program, name), size * 2, (float*)values);
}
void bbe::FragmentShader::setUniform1d(const char* name, double value)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader *)m_prendererData.get();
	glUseProgram(s->twoD.program);
	glUniform1d(glGetUniformLocation(s->twoD.program, name), value);
	glUseProgram(s->threeD.program);
	glUniform1d(glGetUniformLocation(s->threeD.program, name), value);
	glUseProgram(s->threeDBake.program);
	glUniform1d(glGetUniformLocation(s->threeDBake.program, name), value);
}

void bbe::FragmentShader::setUniform1i(const char* name, GLint value)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)m_prendererData.get();
	glUseProgram(s->twoD.program);
	glUniform1i(glGetUniformLocation(s->twoD.program, name), value);
	glUseProgram(s->threeD.program);
	glUniform1i(glGetUniformLocation(s->threeD.program, name), value);
	glUseProgram(s->threeDBake.program);
	glUniform1i(glGetUniformLocation(s->threeDBake.program, name), value);
}

void bbe::FragmentShader::setUniform1f(const char* name, float value)
{
	if (m_prendererData == nullptr) return; // TODO: This leads to the uniforms not having the correct value on the first frame!
	bbe::INTERNAL::openGl::OpenGLFragmentShader* s = (bbe::INTERNAL::openGl::OpenGLFragmentShader*)m_prendererData.get();
	glUseProgram(s->twoD.program);
	glUniform1f(glGetUniformLocation(s->twoD.program, name), value);
	glUseProgram(s->threeD.program);
	glUniform1f(glGetUniformLocation(s->threeD.program, name), value);
	glUseProgram(s->threeDBake.program);
	glUniform1f(glGetUniformLocation(s->threeDBake.program, name), value);
}
#endif
