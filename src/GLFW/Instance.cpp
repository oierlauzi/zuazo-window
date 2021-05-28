#include "Instance.h"

#include <future>
#include <cassert>

extern "C" {
#define GLFW_INCLUDE_NONE //Don't include GL
#include <GLFW/glfw3.h>
}


namespace Zuazo::GLFW {

/*
 * Static assertions
 */

//Video mode
static_assert(sizeof(VideoMode) == sizeof(GLFWvidmode), "Video mode's size must match");
static_assert(alignof(VideoMode) == alignof(GLFWvidmode), "Video mode's alignment must match");
static_assert(offsetof(VideoMode, VideoMode::size.x) == offsetof(GLFWvidmode, GLFWvidmode::width), "Width offset does not match");
static_assert(offsetof(VideoMode, VideoMode::size.y) == offsetof(GLFWvidmode, GLFWvidmode::height), "Height offset does not match");
static_assert(offsetof(VideoMode, VideoMode::colorDepth.x) == offsetof(GLFWvidmode, GLFWvidmode::redBits), "Red bits offset does not match");
static_assert(offsetof(VideoMode, VideoMode::colorDepth.y) == offsetof(GLFWvidmode, GLFWvidmode::greenBits), "Green bits offset does not match");
static_assert(offsetof(VideoMode, VideoMode::colorDepth.z) == offsetof(GLFWvidmode, GLFWvidmode::blueBits), "Blue bits offset does not match");
static_assert(offsetof(VideoMode, VideoMode::frameRate) == offsetof(GLFWvidmode, GLFWvidmode::refreshRate), "Refresh rate offset does not match");

//MonitorEvent
static_assert(std::is_same<std::underlying_type<GLFW::MonitorEvent>::type, decltype(GLFW_CONNECTED)>::value, "Types do not match");
static_assert(static_cast<int>(GLFW::MonitorEvent::connected) == GLFW_CONNECTED, "CONNECTED event does not match");
static_assert(static_cast<int>(GLFW::MonitorEvent::disconnected) == GLFW_DISCONNECTED, "DISCONNECTED event does not match");

//Key event
static_assert(std::is_same<std::underlying_type<GLFW::KeyEvent>::type, decltype(GLFW_RELEASE)>::value, "Types do not match");
static_assert(static_cast<int>(GLFW::KeyEvent::release) == GLFW_RELEASE, "RELEASE event does not match");
static_assert(static_cast<int>(GLFW::KeyEvent::press) == GLFW_PRESS, "PRESS event does not match");
static_assert(static_cast<int>(GLFW::KeyEvent::repeat) == GLFW_REPEAT, "REPEAT event does not match");

//Key modifiers
static_assert(std::is_same<std::underlying_type<GLFW::KeyModifiers>::type, decltype(GLFW_MOD_SHIFT)>::value, "Types do not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::none) == 0, "NONE keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::shift) == GLFW_MOD_SHIFT, "SHIFT keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::control) == GLFW_MOD_CONTROL, "CONTROL keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::alt) == GLFW_MOD_ALT, "ALT keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::super) == GLFW_MOD_SUPER, "SUPER keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::capsLock) == GLFW_MOD_CAPS_LOCK, "CAPS_LOCK keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::numLock) == GLFW_MOD_NUM_LOCK, "NUM_LOCK keyboard modifier bit does not match");

//Keyboard key
static_assert(std::is_same<std::underlying_type<GLFW::KeyboardKey>::type, decltype(GLFW_KEY_UNKNOWN)>::value, "Types do not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::none) == GLFW_KEY_UNKNOWN, "NONE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::space) == GLFW_KEY_SPACE, "SPACE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::apostrophe) == GLFW_KEY_APOSTROPHE, "APOSTROPHE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::comma) == GLFW_KEY_COMMA, "COMMA keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::minus) == GLFW_KEY_MINUS, "MINUS keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::period) == GLFW_KEY_PERIOD, "PERIOD keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::slash) == GLFW_KEY_SLASH, "SLASH keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb0) == GLFW_KEY_0, "NB0 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb1) == GLFW_KEY_1, "NB1 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb2) == GLFW_KEY_2, "NB2 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb3) == GLFW_KEY_3, "NB3 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb4) == GLFW_KEY_4, "NB4 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb5) == GLFW_KEY_5, "NB5 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb6) == GLFW_KEY_6, "NB6 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb7) == GLFW_KEY_7, "NB7 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb8) == GLFW_KEY_8, "NB8 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::nb9) == GLFW_KEY_9, "NB9 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::semicolon) == GLFW_KEY_SEMICOLON, "SEMICOLON keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::equal) == GLFW_KEY_EQUAL, "EQUAL keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::a) == GLFW_KEY_A, "A keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::b) == GLFW_KEY_B, "B keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::c) == GLFW_KEY_C, "C keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::d) == GLFW_KEY_D, "D keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::e) == GLFW_KEY_E, "E keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f) == GLFW_KEY_F, "F keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::g) == GLFW_KEY_G, "G keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::h) == GLFW_KEY_H, "H keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::i) == GLFW_KEY_I, "I keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::j) == GLFW_KEY_J, "J keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::k) == GLFW_KEY_K, "K keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::l) == GLFW_KEY_L, "L keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::m) == GLFW_KEY_M, "M keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::n) == GLFW_KEY_N, "N keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::o) == GLFW_KEY_O, "O keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::p) == GLFW_KEY_P, "P keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::q) == GLFW_KEY_Q, "Q keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::r) == GLFW_KEY_R, "R keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::s) == GLFW_KEY_S, "S keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::t) == GLFW_KEY_T, "T keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::u) == GLFW_KEY_U, "U keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::v) == GLFW_KEY_V, "V keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::w) == GLFW_KEY_W, "W keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::x) == GLFW_KEY_X, "X keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::y) == GLFW_KEY_Y, "Y keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::z) == GLFW_KEY_Z, "Z keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::leftBracket) == GLFW_KEY_LEFT_BRACKET, "LEFT_BRACKET keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::backSlash) == GLFW_KEY_BACKSLASH, "BACKSLASH keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::rightBracket) == GLFW_KEY_RIGHT_BRACKET, "RIGHT_BRACKET keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::graveAccent) == GLFW_KEY_GRAVE_ACCENT, "GRAVE_ACCENT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::world1) == GLFW_KEY_WORLD_1, "WORLD_1 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::world2) == GLFW_KEY_WORLD_2, "WORLD_2 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::escape) == GLFW_KEY_ESCAPE, "ESCAPE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::enter) == GLFW_KEY_ENTER, "ENTER keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::tab) == GLFW_KEY_TAB, "TAB keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::backspace) == GLFW_KEY_BACKSPACE, "BACKSPACE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::insert) == GLFW_KEY_INSERT, "INSERT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::del) == GLFW_KEY_DELETE, "DELETE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::right) == GLFW_KEY_RIGHT, "RIGHT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::left) == GLFW_KEY_LEFT, "LEFT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::down) == GLFW_KEY_DOWN, "DOWN keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::up) == GLFW_KEY_UP, "UP keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::pageUp) == GLFW_KEY_PAGE_UP, "PAGE_UP keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::pageDown) == GLFW_KEY_PAGE_DOWN, "PAGE_DOWN keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::home) == GLFW_KEY_HOME, "HOME keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::end) == GLFW_KEY_END, "END keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::capsLock) == GLFW_KEY_CAPS_LOCK, "CAPS_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::scrollLock) == GLFW_KEY_SCROLL_LOCK, "SCROLL_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::numLock) == GLFW_KEY_NUM_LOCK, "NUM_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::printScreen) == GLFW_KEY_PRINT_SCREEN, "PRINT_SCREEN keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::pause) == GLFW_KEY_PAUSE, "PAUSE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f1) == GLFW_KEY_F1, "F1 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f2) == GLFW_KEY_F2, "F2 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f3) == GLFW_KEY_F3, "F3 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f4) == GLFW_KEY_F4, "F4 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f5) == GLFW_KEY_F5, "F5 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f6) == GLFW_KEY_F6, "F6 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f7) == GLFW_KEY_F7, "F7 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f8) == GLFW_KEY_F8, "F8 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f9) == GLFW_KEY_F9, "F9 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f10) == GLFW_KEY_F10, "F10 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f11) == GLFW_KEY_F11, "F11 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f12) == GLFW_KEY_F12, "F12 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f13) == GLFW_KEY_F13, "F13 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f14) == GLFW_KEY_F14, "F14 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f15) == GLFW_KEY_F15, "F15 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f16) == GLFW_KEY_F16, "F16 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f17) == GLFW_KEY_F17, "F17 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f18) == GLFW_KEY_F18, "F18 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f19) == GLFW_KEY_F19, "F19 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f20) == GLFW_KEY_F20, "F20 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f21) == GLFW_KEY_F21, "F21 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f22) == GLFW_KEY_F22, "F22 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f23) == GLFW_KEY_F23, "F23 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f24) == GLFW_KEY_F24, "F24 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::f25) == GLFW_KEY_F25, "F25 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp0) == GLFW_KEY_KP_0, "KP_0 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp1) == GLFW_KEY_KP_1, "KP_1 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp2) == GLFW_KEY_KP_2, "KP_2 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp3) == GLFW_KEY_KP_3, "KP_3 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp4) == GLFW_KEY_KP_4, "KP_4 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp5) == GLFW_KEY_KP_5, "KP_5 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp6) == GLFW_KEY_KP_6, "KP_6 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp7) == GLFW_KEY_KP_7, "KP_7 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp8) == GLFW_KEY_KP_8, "KP_8 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kp9) == GLFW_KEY_KP_9, "KP_9 keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpDecimal) == GLFW_KEY_KP_DECIMAL, "KP_DECIMAL keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpDivide) == GLFW_KEY_KP_DIVIDE, "KP_DIVIDE keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpMultiply) == GLFW_KEY_KP_MULTIPLY, "KP_MULTIPLY keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpSubstract) == GLFW_KEY_KP_SUBTRACT, "KP_SUBTRACT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpAdd) == GLFW_KEY_KP_ADD, "KP_ADD keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpEnter) == GLFW_KEY_KP_ENTER, "KP_ENTER keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::kpEqual) == GLFW_KEY_KP_EQUAL, "KP_EQUAL keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::leftShift) == GLFW_KEY_LEFT_SHIFT, "LEFT_SHIFT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::leftControl) == GLFW_KEY_LEFT_CONTROL, "LEFT_CONTROL keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::leftAlt) == GLFW_KEY_LEFT_ALT, "LEFT_ALT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::leftSuper) == GLFW_KEY_LEFT_SUPER, "LEFT_SUPER keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::rightShift) == GLFW_KEY_RIGHT_SHIFT, "RIGHT_SHIFT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::rightControl) == GLFW_KEY_RIGHT_CONTROL, "RIGHT_CONTROL keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::rightAlt) == GLFW_KEY_RIGHT_ALT, "RIGHT_ALT keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::rightSuper) == GLFW_KEY_RIGHT_SUPER, "RIGHT_SUPER keycode does not match");
static_assert(static_cast<int>(GLFW::KeyboardKey::menu) == GLFW_KEY_MENU, "MENU keycode does not match");

//Mouse button
static_assert(std::is_same<std::underlying_type<GLFW::MouseButton>::type, decltype(GLFW_MOUSE_BUTTON_1)>::value, "Types do not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb1) == GLFW_MOUSE_BUTTON_1, "NB1 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb2) == GLFW_MOUSE_BUTTON_2, "NB2 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb3) == GLFW_MOUSE_BUTTON_3, "NB3 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb4) == GLFW_MOUSE_BUTTON_4, "NB4 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb5) == GLFW_MOUSE_BUTTON_5, "NB5 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb6) == GLFW_MOUSE_BUTTON_6, "NB6 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb7) == GLFW_MOUSE_BUTTON_7, "NB7 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb8) == GLFW_MOUSE_BUTTON_8, "NB8 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::left) == GLFW_MOUSE_BUTTON_LEFT, "LEFT mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::right) == GLFW_MOUSE_BUTTON_RIGHT, "RIGHT mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::MouseButton::middle) == GLFW_MOUSE_BUTTON_MIDDLE, "MIDDLE mouse button's keycode does not match");


/*
 * GLFW function wrappers
 */

//A couple of forward declarations are needed:
static MonitorHandle getWindowMonitorImpl(WindowHandle win) noexcept;
static Math::Vec2i getWindowPositionImpl(WindowHandle win) noexcept;
static Math::Vec2i getWindowSizeImpl(WindowHandle win) noexcept;

//Event stuff
static MonitorCallback setMonitorCallbackImpl(MonitorCallback cbk) noexcept {
	return reinterpret_cast<MonitorCallback>(glfwSetMonitorCallback(reinterpret_cast<GLFWmonitorfun>(cbk)));
}

static WindowPositionCallback setWindowPositionCallbackImpl(WindowHandle win, WindowPositionCallback cbk) noexcept {
	return glfwSetWindowPosCallback(win, cbk);
}

static WindowSizeCallback setWindowSizeCallbackImpl(WindowHandle win, WindowSizeCallback cbk) noexcept {
	return glfwSetWindowSizeCallback(win, cbk);
}

static WindowCloseCallback setWindowCloseCallbackImpl(WindowHandle win, WindowCloseCallback cbk) noexcept {
	return glfwSetWindowCloseCallback(win, cbk);
}

static WindowRefreshCallback setWindowRefreshCallbackImpl(WindowHandle win, WindowRefreshCallback cbk) noexcept {
	return glfwSetWindowRefreshCallback(win, cbk);
}

static WindowFocusCallback setWindowFocusCallbackImpl(WindowHandle win, WindowFocusCallback cbk) noexcept {
	return glfwSetWindowFocusCallback(win, cbk);
}

static WindowIconifyCallback setWindowIconifyCallbackImpl(WindowHandle win, WindowIconifyCallback cbk) noexcept {
	return glfwSetWindowIconifyCallback(win, cbk);
}

static WindowMaximizeCallback setWindowMaximizeCallbackImpl(WindowHandle win, WindowMaximizeCallback cbk) noexcept {
	return glfwSetWindowMaximizeCallback(win, cbk);
}

static WindowResolutionCallback setWindowResolutionCallbackImpl(WindowHandle win, WindowResolutionCallback cbk) noexcept {
	return glfwSetFramebufferSizeCallback(win, cbk);
}

static WindowScaleCallback setWindowScaleCallbackImpl(WindowHandle win, WindowScaleCallback cbk) noexcept {
	return glfwSetWindowContentScaleCallback(win, cbk);
}

static WindowKeyCallback setWindowKeyCallbackImpl(WindowHandle win, WindowKeyCallback cbk) noexcept {
	return reinterpret_cast<WindowKeyCallback>(glfwSetKeyCallback(win, reinterpret_cast<GLFWkeyfun>(cbk)));
}

static WindowCharCallback setWindowCharCallbackImpl(WindowHandle win, WindowCharCallback cbk) noexcept {
	return glfwSetCharCallback(win, cbk);
}

static WindowMousePositionCallback setWindowMousePositionCallbackImpl(WindowHandle win, WindowMousePositionCallback cbk) noexcept {
	return glfwSetCursorPosCallback(win, cbk);
}

static WindowMouseEnterCallback setWindowMouseEnterCallbackImpl(WindowHandle win, WindowMouseEnterCallback cbk) noexcept {
	return glfwSetCursorEnterCallback(win, cbk);
}

static WindowMouseButtonCallback setWindowMouseButtonCallbackImpl(WindowHandle win, WindowMouseButtonCallback cbk) noexcept {
	return reinterpret_cast<WindowMouseButtonCallback>(glfwSetMouseButtonCallback(win, reinterpret_cast<GLFWmousebuttonfun>(cbk)));
}

static WindowMouseScrollCallback setWindowMouseScrollCallbackImpl(WindowHandle win, WindowMouseScrollCallback cbk) noexcept {
	return glfwSetScrollCallback(win, cbk);
}

static void setWindowUserPointerImpl(WindowHandle win, void* usrPtr) noexcept {
	glfwSetWindowUserPointer(win, usrPtr);
}

static void* getWindowUserPointerImpl(WindowHandle win) noexcept {
	return glfwGetWindowUserPointer(win);
}

static void setMonitorUserPointerImpl(MonitorHandle win, void* usrPtr) noexcept {
	glfwSetMonitorUserPointer(win, usrPtr);
}

static void* getMonitorUserPointerImpl(MonitorHandle win) noexcept {
	return glfwGetMonitorUserPointer(win);
}


//Monitor stuff
static MonitorHandle getPrimaryMonitorImpl() noexcept {
	return glfwGetPrimaryMonitor();
}

static Utils::BufferView<MonitorHandle> getMonitorsImpl() noexcept {
	int cnt;
	MonitorHandle* monitors = glfwGetMonitors(&cnt);
	return Utils::BufferView<MonitorHandle>(monitors, static_cast<size_t>(cnt));
}

static std::string_view getMonitorNameImpl(MonitorHandle mon) noexcept {
	const char* name = glfwGetMonitorName(mon);
	if(name) {
		return std::string_view(name);
	} else {
		return std::string_view();
	}
}

static Math::Vec2i getMonitorPhysicalSizeImpl(MonitorHandle mon) noexcept {
	Math::Vec2i result;
	glfwGetMonitorPhysicalSize(mon, &result.x, &result.y);
	return result;
}

static Math::Vec2i getMonitorPositionImpl(MonitorHandle mon) noexcept {
	Math::Vec2i result;
	glfwGetMonitorPos(mon, &result.x, &result.y);
	return result;
}

static const VideoMode& getVideoModeImpl(MonitorHandle mon) noexcept {
	const auto ptr = glfwGetVideoMode(mon);
	assert(ptr);
	return *reinterpret_cast<const VideoMode*>(ptr);
}

static Utils::BufferView<const VideoMode> getVideoModesImpl(MonitorHandle mon) noexcept {
	int cnt;
	const auto ptr = glfwGetVideoModes(mon, &cnt);
	assert(ptr);
	return Utils::BufferView<const VideoMode>(
		reinterpret_cast<const VideoMode*>(ptr), 
		static_cast<size_t>(cnt)
	);
}


//Window stuff
static WindowHandle createWindowImpl(	Math::Vec2i size, 
										const char* name,
										MonitorHandle mon,
										const WindowCallbacks& callbacks,
										void* usrPtr ) noexcept
{
	//Set Vulkan compatibility
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	//Create the window
	const auto result = glfwCreateWindow(
		size.x, size.y,
		name, mon,
		static_cast<WindowHandle>(nullptr)
	);

	if(!result) {
		return nullptr;
	}

	//Set up the callbacks and the user pointer
	setWindowPositionCallbackImpl(result, callbacks.positionCallback);
	setWindowSizeCallbackImpl(result, callbacks.sizeCallback);
	setWindowCloseCallbackImpl(result, callbacks.closeCallback);
	setWindowRefreshCallbackImpl(result, callbacks.refreshCallback);
	setWindowFocusCallbackImpl(result, callbacks.focusCallback);
	setWindowIconifyCallbackImpl(result, callbacks.iconifyCallback);
	setWindowMaximizeCallbackImpl(result, callbacks.maximizeCallback);
	setWindowResolutionCallbackImpl(result, callbacks.resolutionCallback);
	setWindowScaleCallbackImpl(result, callbacks.scaleCallback);
	setWindowKeyCallbackImpl(result, callbacks.keyCallback);
	setWindowCharCallbackImpl(result, callbacks.charCallback);
	setWindowMousePositionCallbackImpl(result, callbacks.mousePositionCallback);
	setWindowMouseEnterCallbackImpl(result, callbacks.mouseEnterCallback);
	setWindowMouseButtonCallbackImpl(result, callbacks.mouseButtonCallback);
	setWindowMouseScrollCallbackImpl(result, callbacks.mouseScrollCallback);
	setWindowUserPointerImpl(result, usrPtr);
	
	return result;
}

static void destroyWindowImpl(WindowHandle window) noexcept {
	glfwDestroyWindow(window);
}

static void setWindowTitleImpl(WindowHandle win, const char* title) noexcept {
	glfwSetWindowTitle(win, title);
}


static void setWindowVisibilityImpl(WindowHandle win, bool visibility) noexcept {
	if(visibility) {
		glfwShowWindow(win);
	} else {
		glfwHideWindow(win);
	}
}

static bool getWindowVisibilityImpl(WindowHandle win) noexcept {
	return glfwGetWindowAttrib(win, GLFW_VISIBLE);
}

static void restoreWindowImpl(WindowHandle win) noexcept {
	glfwRestoreWindow(win);
}

static void iconifyWindowImpl(WindowHandle win) noexcept {
	glfwIconifyWindow(win);
}

static bool isIconifiedWindowImpl(WindowHandle win) noexcept {
	return glfwGetWindowAttrib(win, GLFW_ICONIFIED);
}

static void maximizeWindowImpl(WindowHandle win) noexcept {
	glfwMaximizeWindow(win);
}

static bool isMaximizedWindowImpl(WindowHandle win) noexcept {
	return glfwGetWindowAttrib(win, GLFW_MAXIMIZED);
}

static void focusWindowImpl(WindowHandle win) noexcept {
	glfwFocusWindow(win);
}

static bool isFocusedWindowImpl(WindowHandle win) noexcept {
	return glfwGetWindowAttrib(win, GLFW_FOCUSED);
}


static void setWindowMonitorImpl(	WindowHandle win, 
									MonitorHandle monitor,
									const VideoMode* videoMode,
									WindowGeometry* windowedGeometry ) noexcept
{
	const auto oldMonHandle = getWindowMonitorImpl(win);
	const auto newMonHandle = monitor;

	if(newMonHandle){
		//A monitor has been specified
		if(newMonHandle != oldMonHandle || videoMode != &(getVideoModeImpl(newMonHandle))) {
			//Something has changed
			if(!oldMonHandle){
				//It was windowed. Save its state
				assert(windowedGeometry);
				*windowedGeometry = {
					getWindowPositionImpl(win),
					getWindowSizeImpl(win)
				};
			}

			//Set it fullscreen on the desired monitor
			const Math::Vec2i pos = getMonitorPositionImpl(newMonHandle);
			assert(videoMode);
			glfwSetWindowMonitor(
				win, 
				newMonHandle, 
				pos.x,
				pos.y,
				videoMode->size.x,
				videoMode->size.y,
				videoMode->frameRate
			);
		}
	} else if(oldMonHandle) {
		//It has become windowed
		assert(!newMonHandle);
		assert(windowedGeometry);
		glfwSetWindowMonitor(
			win, 
			static_cast<MonitorHandle>(nullptr), 
			windowedGeometry->position.x,
			windowedGeometry->position.y,
			windowedGeometry->size.x,
			windowedGeometry->size.y,
			GLFW_DONT_CARE
		);
	}
}

static MonitorHandle getWindowMonitorImpl(WindowHandle win) noexcept {
	return glfwGetWindowMonitor(win);
}

static void setWindowPositionImpl(	WindowHandle win, 
									Math::Vec2i pos) noexcept
{
	glfwSetWindowPos(win, pos.x, pos.y);
}

static Math::Vec2i getWindowPositionImpl(WindowHandle win) noexcept {
	Math::Vec2i result;
	glfwGetWindowPos(win, &result.x, &result.y);
	return result;
}

static void setWindowSizeImpl(	WindowHandle win, 
								Math::Vec2i size ) noexcept
{
	glfwSetWindowSize(win, size.x, size.y);
}

static Math::Vec2i getWindowSizeImpl(WindowHandle win) noexcept {
	Math::Vec2i result;
	glfwGetWindowSize(win, &result.x, &result.y);
	return result;
}

static void setWindowOpacityImpl(	WindowHandle win,
									float opa ) noexcept
{
	glfwSetWindowOpacity(win, opa);
}

static float getWindowOpacityImpl(WindowHandle win) noexcept {
	return glfwGetWindowOpacity(win);
}

static Resolution getWindowResolutionImpl(WindowHandle win) noexcept {
	int width, height;
	glfwGetFramebufferSize(win, &width, &height);
	return Resolution(width, height);
}

static Math::Vec2f getWindowScaleImpl(WindowHandle win) noexcept {
	Math::Vec2f result;
	glfwGetWindowContentScale(win, &result.x, &result.y);
	return result;
}

static void setWindowDecoratedImpl(	WindowHandle win,
									bool deco ) noexcept
{
	glfwSetWindowAttrib(win, GLFW_DECORATED, deco);
}

static bool getWindowDecoratedImpl(WindowHandle win) noexcept {
	return glfwGetWindowAttrib(win, GLFW_DECORATED);
}

static void setWindowResizeableImpl(	WindowHandle win, 
										bool resizeable ) noexcept
{
	glfwSetWindowAttrib(win, GLFW_RESIZABLE, resizeable);
}

static bool getWindowResizeableImpl(WindowHandle win) noexcept {
	return glfwGetWindowAttrib(win, GLFW_RESIZABLE);
}


static KeyEvent getKeyStateImpl(WindowHandle win, KeyboardKey key) noexcept {
	return static_cast<KeyEvent>(glfwGetKey(win, static_cast<int>(key)));
}

static KeyEvent getMouseButtonStateImpl(WindowHandle win, MouseButton but) noexcept {
	return static_cast<KeyEvent>(glfwGetMouseButton(win, static_cast<int>(but)));
}

static Math::Vec2d getMousePositionImpl(WindowHandle win) noexcept {
	Math::Vec2d result;
	glfwGetCursorPos(win, &result.x, &result.y);
	return result;
}



/*
 * Instance
 */

Instance* Instance::s_singleton = nullptr;


//Ctor/dtor
Instance::Instance()
	: m_mutex()
	, m_tasks()
	, m_exit(false)
	, m_thread(&Instance::threadFunc, this)
{
	//Wait initialization executing a no-op
	execute([]{});
}

Instance::~Instance() {
	//Raise the exit flag and signal it to the thread
	std::unique_lock<std::mutex> lock(m_mutex);
	m_exit = true;
	threadContinue();
	lock.unlock();

	//Wait thread finalization
	assert(m_thread.joinable());
	m_thread.join();
}


//Event stuff
MonitorCallback Instance::setMonitorCallback(MonitorCallback cbk) const {
	return execute(setMonitorCallbackImpl, cbk);
}

WindowPositionCallback Instance::setPositionCallback(WindowHandle win, WindowPositionCallback cbk) const {
	return execute(setWindowPositionCallbackImpl, win, cbk);
}

WindowSizeCallback Instance::setSizeCallback(WindowHandle win, WindowSizeCallback cbk) const {
	return execute(setWindowSizeCallbackImpl, win, cbk);
}

WindowCloseCallback Instance::setCloseCallback(WindowHandle win, WindowCloseCallback cbk) const {
	return execute(setWindowCloseCallbackImpl, win, cbk);
}

WindowRefreshCallback Instance::setRefreshCallback(WindowHandle win, WindowRefreshCallback cbk) const {
	return execute(setWindowRefreshCallbackImpl, win, cbk);
}

WindowFocusCallback Instance::setFocusCallback(WindowHandle win, WindowFocusCallback cbk) const {
	return execute(setWindowFocusCallbackImpl, win, cbk);
}

WindowIconifyCallback Instance::setIconifyCallback(WindowHandle win, WindowIconifyCallback cbk) const {
	return execute(setWindowIconifyCallbackImpl, win, cbk);
}

WindowMaximizeCallback Instance::setMaximizeCallback(WindowHandle win, WindowMaximizeCallback cbk) const {
	return execute(setWindowMaximizeCallbackImpl, win, cbk);
}

WindowResolutionCallback Instance::setResolutionCallback(WindowHandle win, WindowResolutionCallback cbk) const {
	return execute(setWindowResolutionCallbackImpl, win, cbk);
}

WindowScaleCallback Instance::setScaleCallback(WindowHandle win, WindowScaleCallback cbk) const {
	return execute(setWindowScaleCallbackImpl, win, cbk);
}

WindowKeyCallback Instance::setKeyCallback(WindowHandle win, WindowKeyCallback cbk) const {
	return execute(setWindowKeyCallbackImpl, win, cbk);
}

WindowCharCallback Instance::setCharCallback(WindowHandle win, WindowCharCallback cbk) const {
	return execute(setWindowCharCallbackImpl, win, cbk);
}

WindowMousePositionCallback Instance::setMousePositionCallback(WindowHandle win, WindowMousePositionCallback cbk) const {
	return execute(setWindowMousePositionCallbackImpl, win, cbk);
}

WindowMouseEnterCallback Instance::setMouseEnterCallback(WindowHandle win, WindowMouseEnterCallback cbk) const {
	return execute(setWindowMouseEnterCallbackImpl, win, cbk);
}

WindowMouseButtonCallback Instance::setMouseButtonCallback(WindowHandle win, WindowMouseButtonCallback cbk) const {
	return execute(setWindowMouseButtonCallbackImpl, win, cbk);
}

WindowMouseScrollCallback Instance::setMouseScrollCallback(WindowHandle win, WindowMouseScrollCallback cbk) const {
	return execute(setWindowMouseScrollCallbackImpl, win, cbk);
}

void Instance::setUserPointer(WindowHandle win, void* usrPtr) const {
	return execute(setWindowUserPointerImpl, win, usrPtr);
}

void* Instance::getUserPointer(WindowHandle win) const {
	//Thread safe
	return getWindowUserPointerImpl(win);
}

void Instance::setUserPointer(MonitorHandle mon, void* usrPtr) const {
	return execute(setMonitorUserPointerImpl, mon, usrPtr);
}

void* Instance::getUserPointer(MonitorHandle mon) const {
	//Thread safe
	return getMonitorUserPointerImpl(mon);
}


//Monitor stuff
MonitorHandle Instance::getPrimaryMonitor() const {
	return execute(getPrimaryMonitorImpl);
}

Utils::BufferView<MonitorHandle> Instance::getMonitors() const {
	return execute(getMonitorsImpl);
}

std::string_view Instance::getName(MonitorHandle mon) const {
	return execute(getMonitorNameImpl, mon);
}

Math::Vec2i Instance::getPhysicalSize(MonitorHandle mon) const {
	return execute(getMonitorPhysicalSizeImpl, mon);
}

Math::Vec2i Instance::getPosition(MonitorHandle mon) const {
	return execute(getMonitorPositionImpl, mon);
}

const VideoMode& Instance::getVideoMode(MonitorHandle mon) const {
	return execute(getVideoModeImpl, mon);
}

Utils::BufferView<const VideoMode> Instance::getVideoModes(MonitorHandle mon) const {
	return execute(getVideoModesImpl, mon);
}


//Window stuff
WindowHandle Instance::createWindow(Math::Vec2i size, 
									const char* name,
									MonitorHandle mon,
									const WindowCallbacks& callbacks,
									void* usrPtr ) const
{
	return execute(createWindowImpl, size, name, mon, callbacks, usrPtr);
}

void Instance::destroyWindow(WindowHandle win) const {
	execute(destroyWindowImpl, win);
}

void Instance::setTitle(WindowHandle win, const char* title) const {
	execute(setWindowTitleImpl, win, title);
}



void Instance::restore(WindowHandle win) const {
	execute(restoreWindowImpl, win);
}

bool Instance::shouldClose(WindowHandle win) const {
	//Thread safe
	return glfwWindowShouldClose(win);
}


void Instance::iconify(WindowHandle win) const {
	execute(iconifyWindowImpl, win);
}

bool Instance::isIconified(WindowHandle win) const {
	return execute(isIconifiedWindowImpl, win);
}

void Instance::maximize(WindowHandle win) const {
	execute(maximizeWindowImpl, win);
}

bool Instance::isMaximized(WindowHandle win) const {
	return execute(isMaximizedWindowImpl, win);
}

void Instance::focus(WindowHandle win) const {
	execute(focusWindowImpl, win);
}

bool Instance::isFocused(WindowHandle win) const {
	return execute(isFocusedWindowImpl, win);
}


void Instance::setMonitor(	WindowHandle win, 
							MonitorHandle monitor,
							const VideoMode* videoMode,
							WindowGeometry* geometry ) const
{
	execute(setWindowMonitorImpl, win, monitor, videoMode, geometry);
}

MonitorHandle Instance::getMonitor(WindowHandle win) const {
	return execute(getWindowMonitorImpl, win);
}

void Instance::setPosition(	WindowHandle win, 
							Math::Vec2i pos) const
{
	execute(setWindowPositionImpl, win, pos);
}

Math::Vec2i Instance::getPosition(WindowHandle win) const {
	return execute(getWindowPositionImpl, win);
}

void Instance::setSize(	WindowHandle win, 
						Math::Vec2i size ) const
{
	execute(setWindowSizeImpl, win, size);
}

Math::Vec2i Instance::getSize(WindowHandle win) const {
	return execute(getWindowSizeImpl, win);
}

void Instance::setOpacity(	WindowHandle win,
							float opa ) const
{
	execute(setWindowOpacityImpl, win, opa);
}

float Instance::getOpacity(WindowHandle win) const {
	return execute(getWindowOpacityImpl, win);
}

Resolution Instance::getResolution(WindowHandle win) const {
	return execute(getWindowResolutionImpl, win);
}

Math::Vec2f Instance::getScale(WindowHandle win) const {
	return execute(getWindowScaleImpl, win);
}

void Instance::setDecorated(WindowHandle win,
							bool deco ) const
{
	execute(setWindowDecoratedImpl, win, deco);
}

bool Instance::getDecorated(WindowHandle win) const {
	return execute(getWindowDecoratedImpl, win);
}

void Instance::setResizeable(	WindowHandle win, 
								bool resizeable ) const
{
	execute(setWindowResizeableImpl, win, resizeable);
}

bool Instance::getResizeable(WindowHandle win) const {
	return execute(getWindowResizeableImpl, win);
}

void Instance::setVisibility(	WindowHandle win, 
								bool visibility) const 
{
	execute(setWindowVisibilityImpl, win, visibility);
}

bool Instance::getVisibility(WindowHandle win) const {
	return execute(getWindowVisibilityImpl, win);
}


KeyEvent Instance::getKeyState(WindowHandle win, KeyboardKey key) const {
	return execute(getKeyStateImpl, win, key);
}

KeyEvent Instance::getMouseButtonState(WindowHandle win, MouseButton but) const {
	return execute(getMouseButtonStateImpl, win, but);
}

Math::Vec2d Instance::getMousePosition(WindowHandle win) const {
	return execute(getMousePositionImpl, win);
}

std::string_view Instance::getKeyName(KeyboardKey key, int scancode) const {
	//Thread safe
	const char* str = glfwGetKeyName(static_cast<int>(key), scancode);
	if(str) {
		return std::string_view(str);
	} else {
		return std::string_view();
	}	
}

std::vector<vk::ExtensionProperties> Instance::getRequiredVulkanInstanceExtensions() const {
	//Thread safe
	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<vk::ExtensionProperties> extensions(glfwExtensionCount);
	for(size_t i = 0; i < extensions.size(); i++){
		std::strncpy(extensions[i].extensionName, glfwExtensions[i], VK_MAX_EXTENSION_NAME_SIZE - 1);
	}

	return extensions;
}

std::vector<vk::ExtensionProperties> Instance::getRequiredVulkanDeviceExtensions() const {
	constexpr vk::ExtensionProperties SWAPCHAIN_EXTENSION(
		std::array<char, VK_MAX_EXTENSION_NAME_SIZE>{VK_KHR_SWAPCHAIN_EXTENSION_NAME}
	);

	return {
		SWAPCHAIN_EXTENSION
	};
}

bool Instance::getPresentationSupport(	vk::Instance instance, 
										vk::PhysicalDevice device, 
										uint32_t family ) const
{
	//Thread safe
	return glfwGetPhysicalDevicePresentationSupport(
		static_cast<VkInstance>(instance), 
		static_cast<VkPhysicalDevice>(device), 
		family
	);
}

vk::SurfaceKHR Instance::createSurface(	WindowHandle win, 
										vk::Instance instance ) const
{
	//Thread safe
	VkSurfaceKHR result = {};
	glfwCreateWindowSurface(				//Thread-safe GLFW function
		static_cast<VkInstance>(instance), 	//Vulkan instance
		win, 								//Window handle
		nullptr, 							//Allocator callbacks
		&result								//Result
	);

	return result;
}




void Instance::initialize() {
	assert(!s_singleton);
	s_singleton = new Instance();
}

void Instance::terminate() {
	assert(s_singleton);
	delete s_singleton;
	s_singleton = nullptr;
}

Instance& Instance::get() noexcept {
	assert(s_singleton);
	return *s_singleton;
}



template<typename Func, typename... Args>
typename std::invoke_result<Func, Args...>::type Instance::execute(Func&& func, Args&&... args) const {
	using Ret = typename std::invoke_result<Func, Args...>::type;

	if(std::this_thread::get_id() == m_thread.get_id()){
		//We are on the main thread. Simply execute it
		return std::forward<Func>(func)(std::forward<Args>(args)...); 
	}else {
		//Create a packaged task to pass it to the main thread
		std::packaged_task<Ret()> task(std::bind(std::forward<Func>(func), std::forward<Args>(args)...));

		//Pass the packaged task to the main thread and signal it
		std::unique_lock<std::mutex> lock(m_mutex);
		m_tasks.push_back(std::ref(task));
		threadContinue();
		
		//Wait until execution is complete
		lock.unlock();
		return task.get_future().get();
	}
}

void Instance::threadFunc() {
	std::unique_lock<std::mutex> lock(m_mutex);

	glfwInit();

	while(m_exit == false){
		//Wait until notified
		threadWaitEvents(lock);

		//Invoke all pending tasks
		for(const auto& task : m_tasks) {
			task();
		}
		m_tasks.clear();
	}

	glfwTerminate();
}

void Instance::threadContinue() const {
	glfwPostEmptyEvent();
}

void Instance::threadWaitEvents(std::unique_lock<std::mutex>& lock) const {
	assert(lock.owns_lock());
	lock.unlock();
	glfwWaitEvents();
	lock.lock();
}


}