#include "FRPch.h"
#include "WindowSystem.h"
#include "Core/Base/Macro.h"

namespace FakeReal
{

	WindowSystem::WindowSystem()
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
	}

	void WindowSystem::Shutdown()
	{

	}

}
