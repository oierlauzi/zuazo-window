#pragma once

#include "Instance.h"
#include "Monitor.h"

namespace Zuazo::GLFW {

class Window {
public:
	Window(WindowHandle handle = nullptr);
	Window(	Math::Vec2i size, 
			const char* name,
			Monitor mon,
			const WindowCallbacks& callbacks,
			void* usrPtr );
	Window(const Window& other) = delete;
	Window(Window&& other);
	~Window();

	Window&							operator=(const Window& other) = delete;
	Window&							operator=(Window&& other);

	operator WindowHandle() noexcept;

	WindowPositionCallback			setPositionCallback(WindowPositionCallback cbk);
	WindowSizeCallback				setSizeCallback(WindowSizeCallback cbk);
	WindowCloseCallback				setCloseCallback(WindowCloseCallback cbk);
	WindowRefreshCallback			setRefreshCallback(WindowRefreshCallback cbk);
	WindowFocusCallback				setFocusCallback(WindowFocusCallback cbk);
	WindowIconifyCallback			setIconifyCallback(WindowIconifyCallback cbk);
	WindowMaximizeCallback			setMaximizeCallback(WindowMaximizeCallback cbk);
	WindowResolutionCallback		setResolutionCallback(WindowResolutionCallback cbk);
	WindowScaleCallback				setScaleCallback(WindowScaleCallback cbk);
	WindowKeyCallback 				setKeyCallback(WindowKeyCallback cbk);
	WindowCharCallback 				setCharCallback(WindowCharCallback cbk);
	WindowMousePositionCallback 	setMousePositionCallback(WindowMousePositionCallback cbk);
	WindowMouseEnterCallback 		setMouseEnterCallback(WindowMouseEnterCallback cbk);
	WindowMouseButtonCallback 		setMouseButtonCallback(WindowMouseButtonCallback cbk);
	WindowMouseScrollCallback 		setMouseScrollCallback(WindowMouseScrollCallback cbk);

	void							setUserPointer(void* usrPtr);
	void*							getUserPointer() const;

	void 							setTitle(const char* title);

	void							iconify();
	bool							isIconified() const;

	void							maximize();
	bool							isMaximized() const;

	void 							focus();
	bool							isFocused() const;

	void							restore();

	bool							shouldClose() const;

	void 							setMonitor(	Monitor mon,
												const VideoMode* videoMode );
	Monitor							getMonitor() const;

	void 							setPosition(Math::Vec2i pos);
	Math::Vec2i 					getPosition() const;

	void 							setSize(Math::Vec2i size);
	Math::Vec2i 					getSize() const;

	void 							setOpacity(float opa);
	float 							getOpacity() const;

	Resolution 						getResolution() const;
	Math::Vec2f 					getScale() const;

	void 							setDecorated(bool deco);
	bool 							getDecorated() const;

	void 							setResizeable(bool resizeable);
	bool							getResizeable() const;

	void							setVisibility(bool visibility);
	bool							getVisibility() const;

	KeyEvent 						getKeyState(KeyboardKey key) const;
	KeyEvent 						getMouseButtonState(MouseButton but) const;
	Math::Vec2d 					getMousePosition() const;

	vk::SurfaceKHR					createSurface(vk::Instance instance) const;

	static std::string_view			getKeyName(KeyboardKey key, int scancode);

private:
	WindowHandle					m_window;
	WindowGeometry					m_windowedGeometry;

};

}