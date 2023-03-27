#include "BBE/Window.h"
#include "BBE/PrimitiveBrush2D.h"
#include "BBE/PrimitiveBrush3D.h"
#include <iostream>
#include "BBE/MouseButtons.h"
#include "BBE/FatalErrors.h"
#ifndef BBE_RENDERER_NULL
#include "imgui_impl_glfw.h"
#endif
#ifdef BBE_RENDERER_VULKAN
#include "BBE/Vulkan/VulkanManager.h"
#endif
#ifdef BBE_RENDERER_NULL
#include "BBE/NullRenderer/NullRendererManager.h"
#endif
#ifdef BBE_RENDERER_OPENGL
#include "BBE/OpenGL/OpenGLManager.h"
#endif

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif

size_t bbe::Window::windowsAliveCounter = 0;
bbe::Window* bbe::Window::INTERNAL_firstInstance = nullptr;


bbe::Window::Window(int width, int height, const char * title, uint32_t major, uint32_t minor, uint32_t patch)
	: m_width(width), m_height(height)
{
#ifdef BBE_RENDERER_VULKAN
	m_renderManager.reset(new bbe::INTERNAL::vulkan::VulkanManager());
#endif
#ifdef BBE_RENDERER_NULL
	m_renderManager.reset(new bbe::INTERNAL::nullRenderer::NullRendererManager());
#endif
#ifdef BBE_RENDERER_OPENGL
	m_renderManager.reset(new bbe::INTERNAL::openGl::OpenGLManager());
#endif

	if(bbe::Window::INTERNAL_firstInstance == nullptr)
	{
		bbe::Window::INTERNAL_firstInstance = this;
	}
	if (windowsAliveCounter == 0)
	{
		if (glfwWrapper::glfwInit() == GLFW_FALSE)
		{
			bbe::INTERNAL::triggerFatalError("An error occurred while initializing GLFW.");
		}
#ifdef BBE_RENDERER_VULKAN
		if (glfwWrapper::glfwVulkanSupported() == GLFW_FALSE)
		{
			bbe::INTERNAL::triggerFatalError("Your GPU and/or driver does not support vulkan!");
		}
#endif
	}

#ifdef BBE_RENDERER_VULKAN
	glfwWrapper::glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
#endif
#ifdef BBE_RENDERER_OPENGL
	glfwWrapper::glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
	glfwWrapper::glfwWindowHint(GLFW_SAMPLES, 4);
#endif
	//glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
#ifndef __EMSCRIPTEN__
	glfwWrapper::glfwWindowHint(GLFW_SCALE_TO_MONITOR, GLFW_TRUE);
#endif

	m_pwindow = glfwWrapper::glfwCreateWindow(width, height, title, nullptr, nullptr);
	if (m_pwindow == nullptr)
	{
		bbe::INTERNAL::triggerFatalError("Could not create window!");
	}

	std::cout << "Init render manager" << std::endl;

	float windowXScale = 0;
	float windowYScale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, &windowXScale, &windowYScale);
	m_renderManager->init(title, major, minor, patch, m_pwindow, static_cast<uint32_t>(width * windowXScale), static_cast<uint32_t>(height * windowYScale));


	std::cout << "Setting glwf callbacks" << std::endl;
	glfwWrapper::glfwSetKeyCallback(m_pwindow, INTERNAL_keyCallback);
	glfwWrapper::glfwSetCharCallback(m_pwindow, INTERNAL_charCallback);
	glfwWrapper::glfwSetCursorPosCallback(m_pwindow, INTERNAL_cursorPosCallback);
	glfwWrapper::glfwSetMouseButtonCallback(m_pwindow, INTERNAL_mouseButtonCallback);
	glfwWrapper::glfwSetWindowSizeCallback(m_pwindow, INTERNAL_windowResizeCallback);
	glfwWrapper::glfwSetScrollCallback(m_pwindow, INTERNAL_mouseScrollCallback);
	double mX = 0;
	double mY = 0;
	glfwWrapper::glfwGetCursorPos(m_pwindow, &mX, &mY);
	
	std::cout << "Init mouse" << std::endl;
	INTERNAL_mouse.INTERNAL_moveMouse((float)mX, (float)mY);
	windowsAliveCounter++;
}

void bbe::Window::preDraw2D()
{
	m_renderManager->preDraw2D();
}

void bbe::Window::preDraw3D()
{
	m_renderManager->preDraw3D();
}

void bbe::Window::preDraw()
{
	m_renderManager->preDraw();
	m_renderManager->setColor2D(bbe::Color(1.0f, 1.0f, 1.0f, 1.0f));
	m_renderManager->setColor3D(bbe::Color(1.0f, 1.0f, 1.0f, 1.0f));
}

bool bbe::Window::keepAlive()
{
	if (glfwWrapper::glfwWindowShouldClose(m_pwindow))
	{
		return false;
	}

	glfwWrapper::glfwPollEvents();
	return true;
}

void bbe::Window::postDraw()
{
	m_renderManager->postDraw();
}

void bbe::Window::waitEndDraw()
{
	m_renderManager->waitEndDraw();
}

void bbe::Window::waitTillIdle()
{
	m_renderManager->waitTillIdle();
}

bool bbe::Window::isReadyToDraw()
{
	return m_renderManager->isReadyToDraw();
}

void bbe::Window::setCursorMode(bbe::CursorMode cursorMode)
{
	switch (cursorMode)
	{
	case bbe::CursorMode::DISABLED:
		glfwWrapper::glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		break;
	case bbe::CursorMode::NORMAL:
		glfwWrapper::glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		break;
	case bbe::CursorMode::HIDDEN:
		glfwWrapper::glfwSetInputMode(m_pwindow, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
		break;
	default:
		throw IllegalArgumentException();
	}
}

GLFWwindow * bbe::Window::getRaw()
{
	return m_pwindow;
}

bbe::Window::~Window()
{
	m_renderManager->destroy();
	glfwWrapper::glfwDestroyWindow(m_pwindow);
	if (windowsAliveCounter == 1)
	{
		glfwWrapper::glfwTerminate();
	}

	windowsAliveCounter--;

	if(this == bbe::Window::INTERNAL_firstInstance)
	{
		bbe::Window::INTERNAL_firstInstance = nullptr;
	}
}

int bbe::Window::getWidth() const
{
	return m_width;
}

int bbe::Window::getScaledWidth() const
{
	float scale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, &scale, nullptr);
	return static_cast<int>(getWidth() * scale);
}

int bbe::Window::getHeight() const
{
	return m_height;
}

int bbe::Window::getScaledHeight() const
{
	float scale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, nullptr, &scale);
	return static_cast<int>(getHeight() * scale);
}

bbe::Vector2 bbe::Window::getGlobalMousePos() const
{
	int windowPosX;
	int windowPosY;
	glfwWrapper::glfwGetWindowPos(m_pwindow, &windowPosX, &windowPosY);

	double mousePosX;
	double mousePosY;
	glfwWrapper::glfwGetCursorPos(m_pwindow, &mousePosX, &mousePosY);

	return Vector2(static_cast<float>(mousePosX + windowPosX), static_cast<float>(mousePosY + windowPosY));
}

bbe::PrimitiveBrush2D& bbe::Window::getBrush2D()
{
	return m_renderManager->getBrush2D();
}

bbe::PrimitiveBrush3D& bbe::Window::getBrush3D()
{
	return m_renderManager->getBrush3D();
}

void bbe::Window::INTERNAL_resize(int width, int height)
{
	float windowXScale = 0;
	float windowYScale = 0;
	glfwWrapper::glfwGetWindowContentScale(m_pwindow, &windowXScale, &windowYScale);

	m_width = int(width / windowXScale);
	m_height = int(height / windowYScale);

	m_renderManager->resize(width, height);
}

void bbe::Window::screenshot(const bbe::String& path)
{
	m_renderManager->screenshot(path);
}

void bbe::Window::setVideoRenderingMode(const char* path)
{
	m_renderManager->setVideoRenderingMode(path);
}

void bbe::Window::registerCloseListener(const std::function<void()>& listener)
{
	m_closeListeners.add(listener);
}

void bbe::Window::executeCloseListeners()
{
	for (const std::function<void()> listener : m_closeListeners)
	{
		listener();
	}
}

void bbe::Window::registerFrameStartListener(const std::function<void()>& listener)
{
	m_frameStartListeners.add(listener);
}

void bbe::Window::executeFrameStartListeneres()
{
	for (const std::function<void()> listener : m_frameStartListeners)
	{
		listener();
	}
}

#ifdef BBE_RENDERER_OPENGL
bbe::Image bbe::Window::bakeLights(const bbe::Matrix4& transform, const Model& model, const Image* normals, const FragmentShader* shader, const bbe::Vector2i& resolution, const bbe::List<bbe::PointLight>& lights)
{
	return ((bbe::INTERNAL::openGl::OpenGLManager*)m_renderManager.get())->bakeLights(transform, model, normals, shader, resolution, lights);
}
uint32_t bbe::Window::getAmountOfDrawcalls() const
{
	return ((bbe::INTERNAL::openGl::OpenGLManager*)m_renderManager.get())->getAmountOfDrawcalls();
}
#endif

void bbe::INTERNAL_keyCallback(GLFWwindow * window, int keyCode, int scanCode, int action, int mods)
{
#ifndef BBE_RENDERER_NULL
	if (keyCode == GLFW_KEY_UNKNOWN)
	{
		// This can happen for example when pressing the FN key on some platforms.
		// As we don't care about that key, we just drop the event.
		return;
	}
	ImGui_ImplGlfw_KeyCallback(window, keyCode, scanCode, action, mods);
	if (ImGui::GetIO().WantCaptureKeyboard) return;
	if (action == GLFW_PRESS)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_keyboard.INTERNAL_press((bbe::Key)keyCode);
	}
	else if (action == GLFW_RELEASE)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_keyboard.INTERNAL_release((bbe::Key)keyCode);
	}
#endif
}

void bbe::INTERNAL_charCallback(GLFWwindow* window, unsigned int c)
{
#ifndef BBE_RENDERER_NULL
	ImGui_ImplGlfw_CharCallback(window, c);
#endif
}

void bbe::INTERNAL_cursorPosCallback(GLFWwindow * window, double xpos, double ypos)
{
#ifdef BBE_RENDERER_VULKAN
	if (ImGui::GetIO().WantCaptureMouse) return;
#endif
	float windowXScale = 0;
	float windowYScale = 0;
	glfwWrapper::glfwGetWindowContentScale(window, &windowXScale, &windowYScale);
	bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_moveMouse((float)(xpos / windowXScale), (float)(ypos / windowYScale));
}

void bbe::INTERNAL_windowResizeCallback(GLFWwindow * window, int width, int height)
{
	bbe::Window::INTERNAL_firstInstance->INTERNAL_resize(width, height);
}

void bbe::INTERNAL_mouseButtonCallback(GLFWwindow * window, int button, int action, int mods)
{
#ifndef BBE_RENDERER_NULL
	ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
	if (ImGui::GetIO().WantCaptureMouse) return;
#endif

	if (action == GLFW_PRESS)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_press((bbe::MouseButton)button);
	}
	else if (action == GLFW_RELEASE)
	{
		bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_release((bbe::MouseButton)button);
	}
}

void bbe::INTERNAL_mouseScrollCallback(GLFWwindow * window, double xoffset, double yoffset)
{
#ifndef BBE_RENDERER_NULL
	ImGui_ImplGlfw_ScrollCallback(window, xoffset, yoffset);
	if (ImGui::GetIO().WantCaptureMouse) return;
#endif
	bbe::Window::INTERNAL_firstInstance->INTERNAL_mouse.INTERNAL_scroll(static_cast<float>(xoffset), static_cast<float>(yoffset));
}

template<>
uint32_t bbe::hash(const bbe::Window & t)
{
	//UNTESTED
	return t.getWidth() * 7 + t.getHeight() * 13;
}