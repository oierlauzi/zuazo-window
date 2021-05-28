#pragma once

extern "C" {

struct GLFWwindow;
struct GLFWmonitor;

}

#include <zuazo/Resolution.h>
#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Utils/BufferView.h>
#include <zuazo/Utils/Bit.h>
#include <zuazo/Math/Vector.h>

#include <chrono>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <functional>
#include <string_view>

namespace Zuazo::GLFW {

using WindowHandle = GLFWwindow*;
using MonitorHandle = GLFWmonitor*;

enum class MonitorEvent : int {
	connected = 0x00040001,
	disconnected = 0x00040002
};

enum class KeyEvent : int {
	release			= 0,
	press 			= 1,
	repeat			= 2
};

enum class KeyModifiers : int {
	none			= 0,
	shift 			= Utils::bit(0),
	control			= Utils::bit(1),
	alt				= Utils::bit(2),
	super			= Utils::bit(3),
	capsLock		= Utils::bit(4),
	numLock			= Utils::bit(5)
};

enum class KeyboardKey : int {
	none 			= -1,

	space 			= 32,
	apostrophe 		= 39,
	comma 			= 44,
	minus 			= 45,
	period 			= 46,
	slash 			= 47,
	nb0 			= 48,
	nb1 			= 49,
	nb2 			= 50,
	nb3 			= 51,
	nb4 			= 52,
	nb5 			= 53,
	nb6 			= 54,
	nb7 			= 55,
	nb8 			= 56,
	nb9 			= 57,
	semicolon 		= 59,
	equal 			= 61,
	a 				= 65,
	b 				= 66,
	c 				= 67,
	d 				= 68,
	e 				= 69,
	f 				= 70,
	g 				= 71,
	h 				= 72,
	i 				= 73,
	j 				= 74,
	k 				= 75,
	l 				= 76,
	m 				= 77,
	n 				= 78,
	o 				= 79,
	p 				= 80,
	q 				= 81,
	r 				= 82,
	s 				= 83,
	t 				= 84,
	u 				= 85,
	v 				= 86,
	w 				= 87,
	x 				= 88,
	y 				= 89,
	z 				= 90,
	leftBracket 	= 91,
	backSlash 		= 92,
	rightBracket 	= 93,
	graveAccent 	= 96,
	world1 			= 161,
	world2 			= 162,

	escape 			= 256,
	enter 			= 257,
	tab 			= 258,
	backspace 		= 259,
	insert 			= 260,
	del 			= 261,
	right 			= 262,
	left 			= 263,
	down 			= 264,
	up 				= 265,
	pageUp 			= 266,
	pageDown 		= 267,
	home 			= 268,
	end 			= 269,
	capsLock 		= 280,
	scrollLock 		= 281,
	numLock 		= 282,
	printScreen 	= 283,
	pause 			= 284,
	f1 				= 290,
	f2 				= 291,
	f3 				= 292,
	f4 				= 293,
	f5 				= 294,
	f6 				= 295,
	f7 				= 296,
	f8 				= 297,
	f9 				= 298,
	f10 			= 299,
	f11 			= 300,
	f12 			= 301,
	f13 			= 302,
	f14 			= 303,
	f15 			= 304,
	f16 			= 305,
	f17 			= 306,
	f18 			= 307,
	f19 			= 308,
	f20 			= 309,
	f21 			= 310,
	f22 			= 311,
	f23 			= 312,
	f24 			= 313,
	f25 			= 314,
	kp0 			= 320,
	kp1 			= 321,
	kp2 			= 322,
	kp3 			= 323,
	kp4 			= 324,
	kp5 			= 325,
	kp6 			= 326,
	kp7 			= 327,
	kp8 			= 328,
	kp9 			= 329,
	kpDecimal		= 330,
	kpDivide 		= 331,
	kpMultiply 		= 332,
	kpSubstract 	= 333,
	kpAdd 			= 334,
	kpEnter 		= 335,
	kpEqual 		= 336,
	leftShift 		= 340,
	leftControl 	= 341,
	leftAlt 		= 342,
	leftSuper 		= 343,
	rightShift 		= 344,
	rightControl	= 345,
	rightAlt 		= 346,
	rightSuper		= 347,
	menu 			= 348,
};

enum class MouseButton : int {
	nb1				= 0,
	nb2				= 1,
	nb3				= 2,
	nb4				= 3,
	nb5				= 4,
	nb6				= 5,
	nb7				= 6,
	nb8				= 7,
	
	left 			= nb1,
	right			= nb2,
	middle			= nb3
};

struct VideoMode {
	Math::Vec2i	size;
	Math::Vec3i colorDepth;
	int			frameRate;
};

struct WindowGeometry {
	Math::Vec2i	position;
	Math::Vec2i	size;
};


typedef void(*MonitorCallback) (MonitorHandle, MonitorEvent);
typedef void(*WindowPositionCallback) (WindowHandle, int, int);
typedef void(*WindowSizeCallback) (WindowHandle, int, int);
typedef void(*WindowCloseCallback) (WindowHandle);
typedef void(*WindowRefreshCallback) (WindowHandle);
typedef void(*WindowFocusCallback) (WindowHandle, int);
typedef void(*WindowIconifyCallback) (WindowHandle, int);
typedef void(*WindowMaximizeCallback) (WindowHandle, int);
typedef void(*WindowResolutionCallback) (WindowHandle, int, int);
typedef void(*WindowScaleCallback) (WindowHandle, float, float);
typedef void(*WindowKeyCallback) (WindowHandle, KeyboardKey, int, KeyEvent, KeyModifiers);
typedef void(*WindowCharCallback) (WindowHandle, unsigned int);
typedef void(*WindowMousePositionCallback) (WindowHandle, double, double);
typedef void(*WindowMouseEnterCallback) (WindowHandle, int);
typedef void(*WindowMouseButtonCallback) (WindowHandle, MouseButton, KeyEvent, KeyModifiers);
typedef void(*WindowMouseScrollCallback) (WindowHandle, double, double);


struct WindowCallbacks {
	WindowPositionCallback		positionCallback;
	WindowSizeCallback			sizeCallback;
	WindowCloseCallback			closeCallback;
	WindowRefreshCallback		refreshCallback;
	WindowFocusCallback			focusCallback;
	WindowIconifyCallback		iconifyCallback;
	WindowMaximizeCallback		maximizeCallback;
	WindowResolutionCallback	resolutionCallback;
	WindowScaleCallback			scaleCallback;
	WindowKeyCallback			keyCallback;
	WindowCharCallback			charCallback;
	WindowMousePositionCallback	mousePositionCallback;
	WindowMouseEnterCallback	mouseEnterCallback;
	WindowMouseButtonCallback	mouseButtonCallback;
	WindowMouseScrollCallback	mouseScrollCallback;
};


class Instance {
public:
	~Instance();

	//Event stuff
	MonitorCallback										setMonitorCallback(MonitorCallback cbk) const;
	WindowPositionCallback								setPositionCallback(WindowHandle win, WindowPositionCallback cbk) const;
	WindowSizeCallback									setSizeCallback(WindowHandle win, WindowSizeCallback cbk) const;
	WindowCloseCallback									setCloseCallback(WindowHandle win, WindowCloseCallback cbk) const;
	WindowRefreshCallback								setRefreshCallback(WindowHandle win, WindowRefreshCallback cbk) const;
	WindowFocusCallback									setFocusCallback(WindowHandle win, WindowFocusCallback cbk) const;
	WindowIconifyCallback								setIconifyCallback(WindowHandle win, WindowIconifyCallback cbk) const;
	WindowMaximizeCallback								setMaximizeCallback(WindowHandle win, WindowMaximizeCallback cbk) const;
	WindowResolutionCallback							setResolutionCallback(WindowHandle win, WindowResolutionCallback cbk) const;
	WindowScaleCallback									setScaleCallback(WindowHandle win, WindowScaleCallback cbk) const;
	WindowKeyCallback 									setKeyCallback(WindowHandle win, WindowKeyCallback cbk) const;
	WindowCharCallback 									setCharCallback(WindowHandle win, WindowCharCallback cbk) const;
	WindowMousePositionCallback 						setMousePositionCallback(WindowHandle win, WindowMousePositionCallback cbk) const;
	WindowMouseEnterCallback 							setMouseEnterCallback(WindowHandle win, WindowMouseEnterCallback cbk) const;
	WindowMouseButtonCallback 							setMouseButtonCallback(WindowHandle win, WindowMouseButtonCallback cbk) const;
	WindowMouseScrollCallback 							setMouseScrollCallback(WindowHandle win, WindowMouseScrollCallback cbk) const;
	void												setUserPointer(WindowHandle win, void* usrPtr) const;
	void*												getUserPointer(WindowHandle win) const;
	void												setUserPointer(MonitorHandle mon, void* usrPtr) const;
	void*												getUserPointer(MonitorHandle mon) const;


	//Monitor stuff
	MonitorHandle 										getPrimaryMonitor() const;
	Utils::BufferView<MonitorHandle> 					getMonitors() const;
	std::string_view 									getName(MonitorHandle mon) const;
	Math::Vec2i 										getPhysicalSize(MonitorHandle mon) const;
	Math::Vec2i 										getPosition(MonitorHandle mon) const;
	const VideoMode&									getVideoMode(MonitorHandle mon) const;
	Utils::BufferView<const VideoMode>					getVideoModes(MonitorHandle mon) const;

	//Window stuff
	WindowHandle 										createWindow(	Math::Vec2i size, 
																		const char* name,
																		MonitorHandle mon,
																		const WindowCallbacks& callbacks,
																		void* usrPtr ) const;

	void 												destroyWindow(WindowHandle win) const;
	void 												setTitle(WindowHandle win, const char* title) const;

	void												iconify(WindowHandle win) const;
	bool												isIconified(WindowHandle win) const;

	void												maximize(WindowHandle win) const;
	bool												isMaximized(WindowHandle win) const;

	void 												focus(WindowHandle win) const;
	bool												isFocused(WindowHandle win) const;

	void												restore(WindowHandle win) const;
	
	bool												shouldClose(WindowHandle win) const;

	void 												setMonitor(	WindowHandle win, 
																	MonitorHandle monitor,
																	const VideoMode* videoMode,
																	WindowGeometry* geometry ) const;
	MonitorHandle										getMonitor(WindowHandle win) const;

	void 												setPosition(WindowHandle win, 
																	Math::Vec2i pos) const;
	Math::Vec2i 										getPosition(WindowHandle win) const;

	void 												setSize(WindowHandle win, 
																Math::Vec2i size ) const;
	Math::Vec2i 										getSize(WindowHandle win) const;

	void 												setOpacity(	WindowHandle win,
																	float opa ) const;
	float 												getOpacity(WindowHandle win) const;

	Resolution 											getResolution(WindowHandle win) const;
	Math::Vec2f 										getScale(WindowHandle win) const;

	void 												setDecorated(	WindowHandle win,
																		bool deco ) const;
	bool 												getDecorated(WindowHandle win) const;

	void 												setResizeable(	WindowHandle win, 
																		bool resizeable ) const;
	bool												getResizeable(WindowHandle win) const;

	void												setVisibility(	WindowHandle win, 
																		bool visibility) const;
	bool												getVisibility(WindowHandle win) const;

	KeyEvent 											getKeyState(WindowHandle win, KeyboardKey key) const;
	KeyEvent 											getMouseButtonState(WindowHandle win, MouseButton but) const;
	Math::Vec2d 										getMousePosition(WindowHandle win) const;
	std::string_view									getKeyName(KeyboardKey key, int scancode) const;

	//Vulkan stuff
	std::vector<vk::ExtensionProperties> 				getRequiredVulkanInstanceExtensions() const;
	std::vector<vk::ExtensionProperties> 				getRequiredVulkanDeviceExtensions() const;
	bool 												getPresentationSupport(	vk::Instance instance, 
																				vk::PhysicalDevice device, 
																				uint32_t family ) const;
	vk::SurfaceKHR 										createSurface(	WindowHandle win, 
																		vk::Instance instance ) const;


	static void											initialize();
	static void											terminate();
	static Instance& 									get() noexcept;
private:
	Instance();
	Instance(const Instance& other) = delete;

	mutable std::mutex									m_mutex;
	mutable std::vector<std::function<void(void)>>		m_tasks;
	bool												m_exit;
	std::thread											m_thread;

	template<typename Func, typename... Args>
	typename std::invoke_result<Func, Args...>::type	execute(Func&& func, Args&&... args) const;
	void												threadFunc();
	void												threadContinue() const;
	void												threadWaitEvents(std::unique_lock<std::mutex>& lock) const;

	static Instance*									s_singleton;

};

}
