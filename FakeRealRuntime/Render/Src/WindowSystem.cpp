#include "WindowSystem.h"
#include "Utils/Macro.h"

namespace FakeReal
{

	WindowSystem::WindowSystem()
		: m_pWindow(nullptr), mWidth(1280), mHeight(720), mFocusMode(false)
	{

	}

	WindowSystem::~WindowSystem()
	{
		glfwDestroyWindow(m_pWindow);
		glfwTerminate();
	}

	void WindowSystem::Initialize(WindowCreateInfo info)
	{
		if (!glfwInit())
		{
			LOG_FATAL(__FUNCTION__, "failed to initialize GLFW");
			return;
		}

		mWidth = info.width;
		mHeight = info.height;

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		m_pWindow = glfwCreateWindow(info.width, info.height, info.title, nullptr, nullptr);
		if (!m_pWindow)
		{
			LOG_FATAL(__FUNCTION__, "failed to create window");
			glfwTerminate();
			return;
		}

		// Setup input callbacks
		glfwSetWindowUserPointer(m_pWindow, this);
		glfwSetKeyCallback(m_pWindow, KeyCallback);
		glfwSetCharCallback(m_pWindow, CharCallback);
		glfwSetCharModsCallback(m_pWindow, CharModsCallback);
		glfwSetMouseButtonCallback(m_pWindow, MouseButtonCallback);
		glfwSetCursorPosCallback(m_pWindow, CursorPosCallback);
		glfwSetCursorEnterCallback(m_pWindow, CursorEnterCallback);
		glfwSetScrollCallback(m_pWindow, ScrollCallback);
		glfwSetDropCallback(m_pWindow, DropCallback);
		glfwSetWindowSizeCallback(m_pWindow, WindowSizeCallback);
		glfwSetWindowCloseCallback(m_pWindow, WindowCloseCallback);

		glfwSetInputMode(m_pWindow, GLFW_RAW_MOUSE_MOTION, GLFW_FALSE);
	}

	void WindowSystem::Shutdown()
	{

	}

	void WindowSystem::SetFocusMode(bool mode)
	{
		mFocusMode = mode;
		glfwSetInputMode(m_pWindow, GLFW_CURSOR, mFocusMode ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL);
	}

	std::array<int, 2> WindowSystem::GetWindowSize() const
	{
		return std::array<int, 2>({ mWidth, mHeight });
	}

}
