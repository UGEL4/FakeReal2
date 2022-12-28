#pragma once

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <functional>
#include <vector>
#include <array>

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
		std::array<int, 2> GetWindowSize() const;

	public:
		using OnKeyCallback			= std::function<void(int, int, int, int)>;
		using OnCharCallback		= std::function<void(unsigned int)>;
		using OnCharModsCallback	= std::function<void(unsigned int, int)>;
		using OnMouseButtonCallback = std::function<void(int, int, int)>;
		using OnCursorPosCallback	= std::function<void(double, double)>;
		using OnCursorEnterCallback = std::function<void(int)>;
		using OnScrollCallback		= std::function<void(double, double)>;
		using OnDropCallback		= std::function<void(int, const char**)>;
		using OnWindowSizeCallback	= std::function<void(int, int)>;
		using OnWindowCloseCallback = std::function<void()>;

		void RegisterKeyCallback(const OnKeyCallback& func)					{ mOnKeyCallbacks.emplace_back(func); }
		void RegisterCharCallback(const OnCharCallback& func)					{ mOnCharCallbacks.emplace_back(func); }
		void RegisterCharModsCallback(const OnCharModsCallback& func)			{ mOnCharModsCallbacks.emplace_back(func); }
		void RegisterMouseButtonCallback(const OnMouseButtonCallback& func)	{ mOnMouseButtonCallbacks.emplace_back(func); }
		void RegisterCursorPosCallback(const OnCursorPosCallback& func)		{ mOnCursorPosCallbacks.emplace_back(func); }
		void RegisterCursorEnterCallback(const OnCursorEnterCallback& func)	{ mOnCursorEnterCallbacks.emplace_back(func); }
		void RegisterScrollCallback(const OnScrollCallback& func)				{ mOnScrollCallbacks.emplace_back(func); }
		void RegisterDropCallback(const OnDropCallback& func)					{ mOnDropCallbacks.emplace_back(func); }
		void RegisterWindowSizeCallback(const OnWindowSizeCallback& func)		{ mOnWindowSizeCallbacks.emplace_back(func); }
		void RegisterWindowCloseCallback(const OnWindowCloseCallback& func)	{ mOnWindowCloseCallbacks.emplace_back(func); }
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
				pApp->mWidth	= width;
				pApp->mHeight	= height;
				pApp->OnWindowSize(width, height);
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
			for (auto& f : mOnKeyCallbacks)
			{
				f(key, scancode, action, mods);
			}
		}

		void OnChar(unsigned int codepoint)
		{
			for (auto& f : mOnCharCallbacks)
			{
				f(codepoint);
			}
		}

		void OnCharMods(unsigned int codepoint, int mods)
		{
			for (auto& f : mOnCharModsCallbacks)
			{
				f(codepoint, mods);
			}
		}

		void OnMouseButton(int button, int action, int mods)
		{
			for (auto& f : mOnMouseButtonCallbacks)
			{
				f(button, action, mods);
			}
		}

		void OnCursePos(double xpos, double ypos)
		{
			for (auto& f : mOnCursorPosCallbacks)
			{
				f(xpos, ypos);
			}
		}

		void OnCurseEnter(int entered)
		{
			for (auto& f : mOnCursorEnterCallbacks)
			{
				f(entered);
			}
		}

		void OnScroll(double xoffset, double yoffset)
		{
			for (auto& f : mOnScrollCallbacks)
			{
				f(xoffset, yoffset);
			}
		}

		void OnDrop(int path_count, const char** paths)
		{
			for (auto& f : mOnDropCallbacks)
			{
				f(path_count, paths);
			}
		}

		void OnWindowSize(int width, int height)
		{
			for (auto& f : mOnWindowSizeCallbacks)
			{
				f(width, height);
			}
		}

		void OnWindowClose()
		{
			for (auto& f : mOnWindowCloseCallbacks)
			{
				f();
			}
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