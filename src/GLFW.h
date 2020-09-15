#pragma once

#include <zuazo/Resolution.h>
#include <zuazo/Chrono.h>
#include <zuazo/Math/Vector.h>
#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Utils/Functions.h>

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

	static const Monitor NO_MONITOR;

	static void							initialize();
	static void							terminate();

	static std::vector<vk::ExtensionProperties> getRequiredVulkanInstanceExtensions();
	static std::vector<vk::ExtensionProperties> getRequiredVulkanDeviceExtensions();
	static bool 						getPresentationSupport(	vk::Instance instance, 
																vk::PhysicalDevice device, 
																uint32_t family );

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

	static GLFW*						s_singleton;
	
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

	enum class KeyboardKey {
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

	enum class KeyboardEvent {
		RELEASE			= 0,
		PRESS 			= 1,
		REPEAT			= 2
	};

	enum class KeyboardModifiers {
		NONE			= 0,
		SHIFT 			= Utils::bit(0),
		CONTROL			= Utils::bit(1),
		ALT				= Utils::bit(2),
		SUPER			= Utils::bit(3),
		CAPS_LOCK		= Utils::bit(4),
		NUM_LOCK		= Utils::bit(5)
	};


	enum class MouseButton {
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

	using StateCallback = std::function<void(State)>;
	using PositionCallback = std::function<void(Math::Vec2i)>;
	using SizeCallback = std::function<void(Math::Vec2i)>;
	using ResolutionCallback = std::function<void(Resolution)>;
	using ScaleCallback = std::function<void(Math::Vec2f)>;
	using CloseCallback = std::function<void()>;
	using RefreshCallback = std::function<void()>;
	using FocusCallback = std::function<void(bool)>;
	using KeyboardCallback = std::function<void(KeyboardKey, KeyboardEvent, KeyboardModifiers)>;
	using CharacterCallback = std::function<void(uint)>;
	using MouseButtonCallback = std::function<void(MouseButton, KeyboardEvent, KeyboardModifiers)>;
	using MousePositionCallback = std::function<void(Math::Vec2d)>;
	using MouseScrollCallback = std::function<void(Math::Vec2d)>;
	using CursorEnterCallback = std::function<void(bool)>;

	struct Callbacks {
		StateCallback				stateCbk;
		PositionCallback			positionCbk;
		SizeCallback				sizeCbk;
		ResolutionCallback			resolutionCbk;
		ScaleCallback				scaleCbk;
		CloseCallback				closeCbk;
		RefreshCallback				refreshCbk;
		FocusCallback				focusCbk;
		KeyboardCallback			keyboardCbk;
		CharacterCallback			characterCbk;
		MouseButtonCallback			mouseButtonCbk;
		MousePositionCallback		mousePositionCbk;
		MouseScrollCallback			mouseScrollCbk;
		CursorEnterCallback			cursorEnterCbk;
	};

	Window(WindowHandle handle);
	Window(	Math::Vec2i size, 
			const char* name,
			Monitor mon = NO_MONITOR,
			Callbacks cbk = {} );
	Window(const Window& other) = delete;
	Window(Window&& other);
	~Window();

	Window& operator=(const Window& other) = delete;
	Window& operator=(Window&& other);

	operator bool() const;

	vk::UniqueSurfaceKHR 		getSurface(const Graphics::Vulkan& vulkan) const;

	void						setName(const char* name);

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


	KeyboardEvent				getKeyState(KeyboardKey key) const;
	void						setKeyboardCallback(KeyboardCallback cbk);
	const KeyboardCallback&		getKeyboardCallback() const;

	void						setCharacterCallback(CharacterCallback cbk);
	const CharacterCallback&	getCharacterCallback() const;

	KeyboardEvent				getMouseButtonState(MouseButton but) const;
	void						setMouseButtonCallback(MouseButtonCallback cbk);
	const MouseButtonCallback&	getMouseButtonCallback() const;
	
	Math::Vec2d					getMousePosition() const;
	void						setMousePositionCallback(MousePositionCallback cbk);
	const MousePositionCallback& getMousePositionCallback() const;

	void						setMouseScrollCallback(MouseScrollCallback cbk);
	const MouseScrollCallback& 	getMouseScrollCallback() const;

	void						setCursorEnterCallback(CursorEnterCallback cbk);
	const CursorEnterCallback& 	getCursorEnterCallback() const;

	static std::string_view		getKeyName(KeyboardKey key, int scancode);

private:
	struct Geometry {
		Math::Vec2i pos, size;
	};

	WindowHandle 					m_window;

	Callbacks						m_callbacks;	
	Geometry 						m_windowedState;

};

}