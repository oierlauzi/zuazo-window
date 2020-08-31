#pragma once

#include <zuazo/Resolution.h>
#include <zuazo/Chrono.h>
#include <zuazo/Math/Vector.h>
#include <zuazo/Graphics/Vulkan.h>

#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <vector>
#include <string_view>

struct GLFWmonitor;
struct GLFWwindow;

namespace Zuazo {

class GLFW {
public:
	class Monitor;
	class Window;

	enum class MonitorEvent {
		CONNECTED,
		DISCONNECTED
	};

	using MonitorCallback = std::function<void(Monitor, MonitorEvent)>;

	~GLFW();

	Monitor								getPrimaryMonitor() const;
	std::vector<Monitor>				getMonitors() const;
	void								setMonitorCallback(MonitorCallback cbk);
	const MonitorCallback&				getMonitorCallback() const;

	void								pollEvents() const;
	void								waitEvents() const;
	void								waitEvents(Duration timeout) const;
	void								postEmptyEvent() const;

	bool 								getPresentationSupport(	vk::Instance instance, 
																vk::PhysicalDevice device, 
																uint32_t family ) const;

	static const Monitor NO_MONITOR;

	static void							init();
	static GLFW&						getGLFW();

private:
	using MonitorHandle = GLFWmonitor*;
	using WindowHandle = GLFWwindow*;
	struct MainThread;
	struct Callbacks;

	MonitorCallback						m_monitorCbk;
	std::unique_ptr<MainThread>			m_mainThread;

	GLFW();
	GLFW(const GLFW& other) = delete;

	GLFW& 								operator=(const GLFW& other) = delete;

	static std::unique_ptr<GLFW>		s_instance;
	
};


class GLFW::Monitor {
	friend GLFW::MainThread;
public:
	using ColorDepth = Math::Vec3i;

	struct Mode {
		ColorDepth colorDepth;
		Math::Vec2i size;
		int frameRate;
	};

	Monitor(MonitorHandle mon);
	Monitor(const Monitor& other) = default; 
	~Monitor() = default;

	Monitor&							operator=(const Monitor& other) = default;

	operator bool() const;

	std::string_view                    getName() const;
	Math::Vec2d                         getPhysicalSize() const;
	Math::Vec2i                         getSize() const;
	Math::Vec2i                         getPosition() const;
	int									getFrameRate() const;
	Mode                                getMode() const;
	std::vector<Mode>                   getModes() const;

private:
	MonitorHandle						m_monitor = nullptr;

};


class GLFW::Window {
	friend GLFW::MainThread;
public:
	enum class State {
		NORMAL,
		HIDDEN,
		FULLSCREEN,
		ICONIFIED,
		MAXIMIZED
	};

	using StateCallback = std::function<void(State)>;
	using PositionCallback = std::function<void(Math::Vec2i)>;
	using SizeCallback = std::function<void(Math::Vec2i)>;
	using ResolutionCallback = std::function<void(Resolution)>;
	using ScaleCallback = std::function<void(Math::Vec2f)>;
	using CloseCallback = std::function<void()>;
	using RefreshCallback = std::function<void()>;
	using FocusCallback = std::function<void(bool)>;

	struct Callbacks {
		StateCallback				stateCbk;
		PositionCallback			positionCbk;
		SizeCallback				sizeCbk;
		ResolutionCallback			resolutionCbk;
		ScaleCallback				scaleCbk;
		CloseCallback				closeCbk;
		RefreshCallback				refreshCbk;
		FocusCallback				focusCbk;
	};

	Window(WindowHandle handle);
	Window(	Math::Vec2i size, 
			std::string_view name,
			Monitor mon = NO_MONITOR,
			Callbacks cbk = {} );
	Window(const Window& other) = delete;
	Window(Window&& other);
	~Window();

	Window& operator=(const Window& other) = delete;
	Window& operator=(Window&& other);

	operator bool() const;

	vk::UniqueSurfaceKHR 		getSurface(const Graphics::Vulkan& vulkan) const;

	void						setName(std::string_view name);

	void						setState(State st);
	State						getState() const;
	void						setStateCallback(StateCallback cbk);
	const StateCallback&		getStateCallback() const;

	void						setMonitor(const Monitor& mon);
	Monitor						getMonitor() const;

	void						setPosition(const Math::Vec2i& pos);
	Math::Vec2i					getPosition() const;
	void						setPositionCallback(PositionCallback cbk);
	const PositionCallback&		getPositionCallback() const;

	void						setSize(const Math::Vec2i& size);
	Math::Vec2i					getSize() const;
	void						setSizeCallback(SizeCallback cbk);
	const SizeCallback&			getSizeCallback() const;


	void						setOpacity(float opa);
	float						getOpacity() const;

	Resolution					getResolution() const;
	void						setResolutionCallback(ResolutionCallback cbk);
	const ResolutionCallback&	getResolutionCallback() const;

	Math::Vec2f					getScale() const;
	void						setScaleCallback(ScaleCallback cbk);
	const ScaleCallback&		getScaleCallback() const;

	void						close();
	bool						shouldClose() const;
	void						setCloseCallback(CloseCallback cbk);
	const CloseCallback&		getCloseCallback() const;

	void						focus();
	void						setFocusCallback(FocusCallback cbk);
	const FocusCallback&		getFocusCallback() const;

	void						setRefreshCallback(RefreshCallback cbk);
	const RefreshCallback&		getRefreshCallback() const;

	void						setDecorated(bool deco);
	bool						getDecorated() const;

	void						setResizeable(bool resizeable);
	bool						getResizeable() const;


private:
	struct Geometry {
		Math::Vec2i pos, size;
	};

	WindowHandle 					m_window;

	Callbacks						m_callbacks;	
	Geometry 						m_windowedState;

};

}