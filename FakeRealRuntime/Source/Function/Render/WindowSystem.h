#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>

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
		void SetFocusMode(bool mode);
		GLFWwindow* GetWindow() const { return m_pWindow; }

	public:
		using OnKeyCallback = std::function<void(GLFWwindow*, int, int, int, int)>;
		using OnCharCallback = std::function<void(GLFWwindow*, unsigned int)>;
		using OnCharModsCallback = std::function<void(GLFWwindow*, unsigned int, int)>;
		using OnMouseButtonCallback = std::function<void(GLFWwindow*, int, int, int)>;
		using OnCursorPosCallback = std::function<void(GLFWwindow*, double, double)>;
		using OnCursorEnterCallback = std::function<void(GLFWwindow*, int)>;
		using OnScrollCallback = std::function<void(GLFWwindow*, double, double)>;
		using OnDropCallback = std::function<void(GLFWwindow*, int, const char**)>;
		using OnWindowSizeCallback = std::function<void(GLFWwindow*, int, int)>;
		using OnWindowCloseCallback = std::function<void(GLFWwindow*)>;

	private:
		static void KeyCallback(GLFWwindow* pWindow, int key, int scancode, int action, int mods)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnKey(key, scancode, action, mods);
			}
		}

		static void CharCallback(GLFWwindow* pWindow, unsigned int codepoint)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnChar(codepoint);
			}
		}

		static void CharModsCallback(GLFWwindow* pWindow, unsigned int codepoint, int mods)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnCharMods(codepoint, mods);
			}
		}

		static void MouseButtonCallback(GLFWwindow* pWindow, int button, int action, int mods)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnMouseButton(button, action, mods);
			}
		}

		static void CursorPosCallback(GLFWwindow* pWindow, double xpos, double ypos)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnCursePos(xpos, ypos);
			}
		}

		static void CursorEnterCallback(GLFWwindow* pWindow, int entered)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnCurseEnter(entered);
			}
		}

		static void ScrollCallback(GLFWwindow* pWindow, double xoffset, double yoffset)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnScroll(xoffset, yoffset);
			}
		}

		static void DropCallback(GLFWwindow* pWindow, int path_count, const char* paths[])
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnDrop(path_count, paths);
			}
		}

		static void WindowSizeCallback(GLFWwindow* pWindow, int width, int height)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnWindowResize(width, height);
			}
		}

		static void WindowCloseCallback(GLFWwindow* pWindow)
		{
			WindowSystem* pApp = (WindowSystem*)glfwGetWindowUserPointer(pWindow);
			if (pApp)
			{
				pApp->OnWindowClose();
			}
		}

		void OnKey(int key, int scancode, int action, int mods)
		{

		}

		void OnChar(unsigned int codepoint)
		{

		}

		void OnCharMods(unsigned int codepoint, int mods)
		{

		}

		void OnMouseButton(int button, int action, int mods)
		{

		}

		void OnCursePos(double xpos, double ypos)
		{

		}

		void OnCurseEnter(int entered)
		{

		}

		void OnScroll(double xoffset, double yoffset)
		{

		}

		void OnDrop(int path_count, const char** paths)
		{

		}

		void OnWindowResize(int width, int height)
		{

		}

		void OnWindowClose()
		{

		}
	private:
		GLFWwindow* m_pWindow;
		int			mWidth;
		int			mHeight;
		bool		mFocusMode;

		std::vector<OnKeyCallback>			mOnKeyCallbacks;
		std::vector<OnCharCallback>			mOnCharCallbacks;
		std::vector<OnCharModsCallback>		mOnCharModsCallbacks;
		std::vector<OnMouseButtonCallback>	mOnMouseButtonCallbacks;
		std::vector<OnCursorPosCallback>	mOnCursorPosCallbacks;
		std::vector<OnCursorEnterCallback>	mOnCursorEnterCallbacks;
		std::vector<OnScrollCallback>		mOnScrollCallbacks;
		std::vector<OnDropCallback>			mOnDropCallbacks;
		std::vector<OnWindowSizeCallback>	mOnWindowSizeCallbacks;
		std::vector<OnWindowCloseCallback>	mOnWindowCloseCallbacks;
	};
}