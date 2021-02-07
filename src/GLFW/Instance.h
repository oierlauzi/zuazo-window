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
	CONNECTED = 0x00040001,
	DISCONNECTED = 0x00040002
};

enum class KeyEvent : int {
	RELEASE			= 0,
	PRESS 			= 1,
	REPEAT			= 2
};

enum class KeyModifiers : int {
	NONE			= 0,
	SHIFT 			= Utils::bit(0),
	CONTROL			= Utils::bit(1),
	ALT				= Utils::bit(2),
	SUPER			= Utils::bit(3),
	CAPS_LOCK		= Utils::bit(4),
	NUM_LOCK		= Utils::bit(5)
};

enum class KeyboardKey : int {
	NONE 			= -1,

	SPACE 			= 32,
	APOSTROPHE 		= 39,
	COMMA 			= 44,
	MINUS 			= 45,
	PERIOD 			= 46,
	SLASH 			= 47,
	NB0 			= 48,
	NB1 			= 49,
	NB2 			= 50,
	NB3 			= 51,
	NB4 			= 52,
	NB5 			= 53,
	NB6 			= 54,
	NB7 			= 55,
	NB8 			= 56,
	NB9 			= 57,
	SEMICOLON 		= 59,
	EQUAL 			= 61,
	A 				= 65,
	B 				= 66,
	C 				= 67,
	D 				= 68,
	E 				= 69,
	F 				= 70,
	G 				= 71,
	H 				= 72,
	I 				= 73,
	J 				= 74,
	K 				= 75,
	L 				= 76,
	M 				= 77,
	N 				= 78,
	O 				= 79,
	P 				= 80,
	Q 				= 81,
	R 				= 82,
	S 				= 83,
	T 				= 84,
	U 				= 85,
	V 				= 86,
	W 				= 87,
	X 				= 88,
	Y 				= 89,
	Z 				= 90,
	LEFT_BRACKET 	= 91,
	BACKSLASH 		= 92,
	RIGHT_BRACKET 	= 93,
	GRAVE_ACCENT 	= 96,
	WORLD_1 		= 161,
	WORLD_2 		= 162,

	ESCAPE 			= 256,
	ENTER 			= 257,
	TAB 			= 258,
	BACKSPACE 		= 259,
	INSERT 			= 260,
	DELETE 			= 261,
	RIGHT 			= 262,
	LEFT 			= 263,
	DOWN 			= 264,
	UP 				= 265,
	PAGE_UP 		= 266,
	PAGE_DOWN 		= 267,
	HOME 			= 268,
	END 			= 269,
	CAPS_LOCK 		= 280,
	SCROLL_LOCK 	= 281,
	NUM_LOCK 		= 282,
	PRINT_SCREEN 	= 283,
	PAUSE 			= 284,
	F1 				= 290,
	F2 				= 291,
	F3 				= 292,
	F4 				= 293,
	F5 				= 294,
	F6 				= 295,
	F7 				= 296,
	F8 				= 297,
	F9 				= 298,
	F10 			= 299,
	F11 			= 300,
	F12 			= 301,
	F13 			= 302,
	F14 			= 303,
	F15 			= 304,
	F16 			= 305,
	F17 			= 306,
	F18 			= 307,
	F19 			= 308,
	F20 			= 309,
	F21 			= 310,
	F22 			= 311,
	F23 			= 312,
	F24 			= 313,
	F25 			= 314,
	KP_0 			= 320,
	KP_1 			= 321,
	KP_2 			= 322,
	KP_3 			= 323,
	KP_4 			= 324,
	KP_5 			= 325,
	KP_6 			= 326,
	KP_7 			= 327,
	KP_8 			= 328,
	KP_9 			= 329,
	KP_DECIMAL 		= 330,
	KP_DIVIDE 		= 331,
	KP_MULTIPLY 	= 332,
	KP_SUBTRACT 	= 333,
	KP_ADD 			= 334,
	KP_ENTER 		= 335,
	KP_EQUAL 		= 336,
	LEFT_SHIFT 		= 340,
	LEFT_CONTROL 	= 341,
	LEFT_ALT 		= 342,
	LEFT_SUPER 		= 343,
	RIGHT_SHIFT 	= 344,
	RIGHT_CONTROL 	= 345,
	RIGHT_ALT 		= 346,
	RIGHT_SUPER 	= 347,
	MENU 			= 348,
};

enum class MouseButton : int {
	NB1				= 0,
	NB2				= 1,
	NB3				= 2,
	NB4				= 3,
	NB5				= 4,
	NB6				= 5,
	NB7				= 6,
	NB8				= 7,
	LEFT 			= NB1,
	RIGHT			= NB2,
	MIDDLE			= NB3
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