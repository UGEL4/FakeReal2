#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

namespace FakeReal
{
	struct WindowCreateInfo
	{
		int			width;
		int			height;
		bool		full_screen;
		const char* title{ "FR Engine" };
	};

	class WindowSystem
	{
	public:
		WindowSystem();
		~WindowSystem();

		void Initialize(WindowCreateInfo info);
		void Shutdown();
		bool ShouldClose() const { return glfwWindowShouldClose(m_pWindow); }
		void PollEvents() { glfwPollEvents(); }
		void SetTitle(const char* title) { glfwSetWindowTitle(m_pWindow, title); }

	private:
		GLFWwindow* m_pWindow;
		int mWidth;
		int mHeight;

	};
}