#include "GLFW.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Exception.h>

#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <type_traits>

#define GLFW_INCLUDE_NONE //Don't include GL
#include <GLFW/glfw3.h>


namespace Zuazo {

/*
 * Enumeration checks
 */

static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NONE) == GLFW_KEY_UNKNOWN, "NONE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SPACE) == GLFW_KEY_SPACE, "SPACE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::APOSTROPHE) == GLFW_KEY_APOSTROPHE, "APOSTROPHE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::COMMA) == GLFW_KEY_COMMA, "COMMA keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::MINUS) == GLFW_KEY_MINUS, "MINUS keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PERIOD) == GLFW_KEY_PERIOD, "PERIOD keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SLASH) == GLFW_KEY_SLASH, "SLASH keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB0) == GLFW_KEY_0, "NB0 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB1) == GLFW_KEY_1, "NB1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB2) == GLFW_KEY_2, "NB2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB3) == GLFW_KEY_3, "NB3 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB4) == GLFW_KEY_4, "NB4 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB5) == GLFW_KEY_5, "NB5 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB6) == GLFW_KEY_6, "NB6 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB7) == GLFW_KEY_7, "NB7 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB8) == GLFW_KEY_8, "NB8 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB9) == GLFW_KEY_9, "NB9 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SEMICOLON) == GLFW_KEY_SEMICOLON, "SEMICOLON keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::EQUAL) == GLFW_KEY_EQUAL, "EQUAL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::A) == GLFW_KEY_A, "A keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::B) == GLFW_KEY_B, "B keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::C) == GLFW_KEY_C, "C keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::D) == GLFW_KEY_D, "D keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::E) == GLFW_KEY_E, "E keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F) == GLFW_KEY_F, "F keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::G) == GLFW_KEY_G, "G keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::H) == GLFW_KEY_H, "H keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::I) == GLFW_KEY_I, "I keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::J) == GLFW_KEY_J, "J keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::K) == GLFW_KEY_K, "K keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::L) == GLFW_KEY_L, "L keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::M) == GLFW_KEY_M, "M keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::N) == GLFW_KEY_N, "N keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::O) == GLFW_KEY_O, "O keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::P) == GLFW_KEY_P, "P keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::Q) == GLFW_KEY_Q, "Q keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::R) == GLFW_KEY_R, "R keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::S) == GLFW_KEY_S, "S keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::T) == GLFW_KEY_T, "T keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::U) == GLFW_KEY_U, "U keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::V) == GLFW_KEY_V, "V keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::W) == GLFW_KEY_W, "W keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::X) == GLFW_KEY_X, "X keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::Y) == GLFW_KEY_Y, "Y keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::Z) == GLFW_KEY_Z, "Z keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_BRACKET) == GLFW_KEY_LEFT_BRACKET, "LEFT_BRACKET keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::BACKSLASH) == GLFW_KEY_BACKSLASH, "BACKSLASH keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_BRACKET) == GLFW_KEY_RIGHT_BRACKET, "RIGHT_BRACKET keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::GRAVE_ACCENT) == GLFW_KEY_GRAVE_ACCENT, "GRAVE_ACCENT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::WORLD_1) == GLFW_KEY_WORLD_1, "WORLD_1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::WORLD_2) == GLFW_KEY_WORLD_2, "WORLD_2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::ESCAPE) == GLFW_KEY_ESCAPE, "ESCAPE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::ENTER) == GLFW_KEY_ENTER, "ENTER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::TAB) == GLFW_KEY_TAB, "TAB keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::BACKSPACE) == GLFW_KEY_BACKSPACE, "BACKSPACE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::INSERT) == GLFW_KEY_INSERT, "INSERT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::DELETE) == GLFW_KEY_DELETE, "DELETE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT) == GLFW_KEY_RIGHT, "RIGHT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT) == GLFW_KEY_LEFT, "LEFT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::DOWN) == GLFW_KEY_DOWN, "DOWN keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::UP) == GLFW_KEY_UP, "UP keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PAGE_UP) == GLFW_KEY_PAGE_UP, "PAGE_UP keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PAGE_DOWN) == GLFW_KEY_PAGE_DOWN, "PAGE_DOWN keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::HOME) == GLFW_KEY_HOME, "HOME keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::END) == GLFW_KEY_END, "END keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::CAPS_LOCK) == GLFW_KEY_CAPS_LOCK, "CAPS_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SCROLL_LOCK) == GLFW_KEY_SCROLL_LOCK, "SCROLL_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NUM_LOCK) == GLFW_KEY_NUM_LOCK, "NUM_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PRINT_SCREEN) == GLFW_KEY_PRINT_SCREEN, "PRINT_SCREEN keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PAUSE) == GLFW_KEY_PAUSE, "PAUSE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F1) == GLFW_KEY_F1, "F1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F2) == GLFW_KEY_F2, "F2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F3) == GLFW_KEY_F3, "F3 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F4) == GLFW_KEY_F4, "F4 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F5) == GLFW_KEY_F5, "F5 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F6) == GLFW_KEY_F6, "F6 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F7) == GLFW_KEY_F7, "F7 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F8) == GLFW_KEY_F8, "F8 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F9) == GLFW_KEY_F9, "F9 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F10) == GLFW_KEY_F10, "F10 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F11) == GLFW_KEY_F11, "F11 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F12) == GLFW_KEY_F12, "F12 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F13) == GLFW_KEY_F13, "F13 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F14) == GLFW_KEY_F14, "F14 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F15) == GLFW_KEY_F15, "F15 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F16) == GLFW_KEY_F16, "F16 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F17) == GLFW_KEY_F17, "F17 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F18) == GLFW_KEY_F18, "F18 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F19) == GLFW_KEY_F19, "F19 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F20) == GLFW_KEY_F20, "F20 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F21) == GLFW_KEY_F21, "F21 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F22) == GLFW_KEY_F22, "F22 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F23) == GLFW_KEY_F23, "F23 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F24) == GLFW_KEY_F24, "F24 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F25) == GLFW_KEY_F25, "F25 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_0) == GLFW_KEY_KP_0, "KP_0 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_1) == GLFW_KEY_KP_1, "KP_1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_2) == GLFW_KEY_KP_2, "KP_2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_3) == GLFW_KEY_KP_3, "KP_3 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_4) == GLFW_KEY_KP_4, "KP_4 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_5) == GLFW_KEY_KP_5, "KP_5 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_6) == GLFW_KEY_KP_6, "KP_6 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_7) == GLFW_KEY_KP_7, "KP_7 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_8) == GLFW_KEY_KP_8, "KP_8 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_9) == GLFW_KEY_KP_9, "KP_9 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_DECIMAL) == GLFW_KEY_KP_DECIMAL, "KP_DECIMAL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_DIVIDE) == GLFW_KEY_KP_DIVIDE, "KP_DIVIDE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_MULTIPLY) == GLFW_KEY_KP_MULTIPLY, "KP_MULTIPLY keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_SUBTRACT) == GLFW_KEY_KP_SUBTRACT, "KP_SUBTRACT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_ADD) == GLFW_KEY_KP_ADD, "KP_ADD keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_ENTER) == GLFW_KEY_KP_ENTER, "KP_ENTER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_EQUAL) == GLFW_KEY_KP_EQUAL, "KP_EQUAL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_SHIFT) == GLFW_KEY_LEFT_SHIFT, "LEFT_SHIFT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_CONTROL) == GLFW_KEY_LEFT_CONTROL, "LEFT_CONTROL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_ALT) == GLFW_KEY_LEFT_ALT, "LEFT_ALT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_SUPER) == GLFW_KEY_LEFT_SUPER, "LEFT_SUPER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_SHIFT) == GLFW_KEY_RIGHT_SHIFT, "RIGHT_SHIFT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_CONTROL) == GLFW_KEY_RIGHT_CONTROL, "RIGHT_CONTROL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_ALT) == GLFW_KEY_RIGHT_ALT, "RIGHT_ALT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_SUPER) == GLFW_KEY_RIGHT_SUPER, "RIGHT_SUPER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::MENU) == GLFW_KEY_MENU, "MENU keycode does not match");


static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::RELEASE) == GLFW_RELEASE, "RELEASE event does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::PRESS) == GLFW_PRESS, "PRESS event does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::REPEAT) == GLFW_REPEAT, "REPEAT event does not match");

static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::NONE) == 0, "NONE keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::SHIFT) == GLFW_MOD_SHIFT, "SHIFT keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CONTROL) == GLFW_MOD_CONTROL, "CONTROL keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::ALT) == GLFW_MOD_ALT, "ALT keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::SUPER) == GLFW_MOD_SUPER, "SUPER keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CAPS_LOCK) == GLFW_MOD_CAPS_LOCK, "CAPS_LOCK keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::NUM_LOCK) == GLFW_MOD_NUM_LOCK, "NUM_LOCK keyboard modifier bit does not match");


static_assert(static_cast<int>(GLFW::Window::MouseButton::NB1) == GLFW_MOUSE_BUTTON_1, "NB1 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB2) == GLFW_MOUSE_BUTTON_2, "NB2 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB3) == GLFW_MOUSE_BUTTON_3, "NB3 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB4) == GLFW_MOUSE_BUTTON_4, "NB4 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB5) == GLFW_MOUSE_BUTTON_5, "NB5 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB6) == GLFW_MOUSE_BUTTON_6, "NB6 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB7) == GLFW_MOUSE_BUTTON_7, "NB7 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB8) == GLFW_MOUSE_BUTTON_8, "NB8 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::LEFT) == GLFW_MOUSE_BUTTON_LEFT, "LEFT mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::RIGHT) == GLFW_MOUSE_BUTTON_RIGHT, "RIGHT mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::MIDDLE) == GLFW_MOUSE_BUTTON_MIDDLE, "MIDDLE mouse button's keycode does not match");


/*
 * GLFW::Callbacks
 */
struct GLFW::Callbacks {
	static inline std::mutex mutex;
	static inline MonitorHandle currentMonitor = nullptr;
	static inline bool enabled = false;



	static void setupUserPtr(WindowHandle win, void* userPtr) {
		glfwSetWindowUserPointer(win, userPtr);
	}

	static void setupCallbacks(WindowHandle win) {
		glfwSetWindowPosCallback(win, Callbacks::positionCbk);
		glfwSetWindowSizeCallback(win, Callbacks::sizeCbk);
		glfwSetWindowCloseCallback(win, Callbacks::closeCbk);
		glfwSetWindowRefreshCallback(win, Callbacks::refreshCbk);
		glfwSetWindowFocusCallback(win, Callbacks::focusCbk);
		glfwSetWindowIconifyCallback(win, Callbacks::iconifyCbk);
		glfwSetWindowMaximizeCallback(win, Callbacks::maximizeCbk);
		glfwSetFramebufferSizeCallback(win, Callbacks::framebufferCbk);
		glfwSetWindowContentScaleCallback(win, Callbacks::scaleCbk);
		glfwSetKeyCallback(win, Callbacks::keyboardCbk);
		glfwSetCharCallback(win, Callbacks::characterCbk);
		glfwSetMouseButtonCallback(win, Callbacks::mouseButtonCbk);
		glfwSetCursorPosCallback(win, Callbacks::mousePositionCbk);
		glfwSetScrollCallback(win, Callbacks::mouseScrollCbk);
		glfwSetCursorEnterCallback(win, Callbacks::cursorEnterCbk);
	}

	static void setupMonitorCallback() {
		glfwSetMonitorCallback(Callbacks::monitorCbk);
	}


	static void monitorCbk(MonitorHandle mon, int evnt){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			const auto& cbk = getGLFW().getMonitorCallback();
			if(cbk){
				assert(currentMonitor == nullptr);
				currentMonitor = mon;

				switch(evnt){
				case GLFW_CONNECTED: 
					cbk(GLFW::Monitor(mon), GLFW::MonitorEvent::CONNECTED); 
					break;
				case GLFW_DISCONNECTED: 
					cbk(GLFW::Monitor(mon), GLFW::MonitorEvent::DISCONNECTED); 
					break;
				default:
					break;
				}

				currentMonitor = nullptr;
			}
		}
	}

	static void positionCbk(WindowHandle win, int x, int y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getPositionCallback();

			if(cbk){
				cbk(Math::Vec2i(x, y));
			}
		}
	}

	static void sizeCbk(WindowHandle win, int x, int y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getSizeCallback();

			if(cbk){
				cbk(Math::Vec2i(x, y));
			}
		}
	}

	static void closeCbk(WindowHandle win){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getCloseCallback();

			if(cbk){
				cbk();
			}
		}
	}

	static void refreshCbk(WindowHandle win){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getRefreshCallback();

			if(cbk){
				cbk();
			}
		}
	}

	static void focusCbk(WindowHandle win, int x){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getFocusCallback();

			if(cbk){
				cbk(static_cast<bool>(x));
			}
		}
	}

	static void iconifyCbk(WindowHandle win, int x){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getStateCallback();

			if(cbk){
				if(x == GLFW_TRUE){
					cbk(Window::State::ICONIFIED);
				} else {
					cbk(Window::State::NORMAL);
				}
			}
		}
	}

	static void maximizeCbk(WindowHandle win, int x){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getStateCallback();

			if(cbk){
				if(x == GLFW_TRUE){
					cbk(Window::State::MAXIMIZED);
				} else {
					cbk(Window::State::NORMAL);
				}
			}
		}
	}

	static void framebufferCbk(WindowHandle win, int x, int y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getResolutionCallback();

			if(cbk){
				cbk(Resolution(x, y));
			}
		}
	}

	static void scaleCbk(WindowHandle win, float x, float y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getScaleCallback();

			if(cbk){
				cbk(Math::Vec2f(x, y));
			}
		}
	}

	static void keyboardCbk(WindowHandle win, int key, int scancode, int action, int mods) {
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getKeyboardCallback();

			if(cbk){
				cbk(
					static_cast<Window::KeyboardKey>(key), 
					static_cast<Window::KeyboardEvent>(action), 
					static_cast<Window::KeyboardModifiers>(mods)
				);
			}
		}

		ZUAZO_IGNORE_PARAM(scancode);
	} 

	static void characterCbk(WindowHandle win, uint character) {
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getCharacterCallback();

			if(cbk){
				cbk(character);
			}
		}
	} 

	static void mouseButtonCbk(WindowHandle win, int button, int action, int mods) {
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getMouseButtonCallback();

			if(cbk){
				cbk(
					static_cast<Window::MouseButton>(button), 
					static_cast<Window::KeyboardEvent>(action), 
					static_cast<Window::KeyboardModifiers>(mods)
				);
			}
		}
	} 

	static void mousePositionCbk(WindowHandle win, double xpos, double ypos) {
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getMousePositionCallback();

			if(cbk){
				cbk(Math::Vec2d(xpos, ypos));
			}
		}
	}

	static void mouseScrollCbk(WindowHandle win, double dx, double dy) {
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getMouseScrollCallback();

			if(cbk){
				cbk(Math::Vec2d(dx, dy));
			}
		}
	} 

	static void cursorEnterCbk(WindowHandle win, int entered) {
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getCursorEnterCallback();

			if(cbk){
				cbk(entered);
			}
		}
	} 

};


/*
 * GLFW::MainThread
 */

struct GLFW::MainThread {
	using Task = std::function<void(void)>;

	bool							exit;
	size_t							windowCount;

	std::mutex						mutex;
	std::condition_variable			continueCondition;
	std::condition_variable			completeCondition;
	std::queue<Task> 				tasks;
	std::thread						thread;

	MainThread()
		: exit(false)
		, windowCount(0)
		, thread(&MainThread::threadFunc, this)
	{
		//Wait for completion
		std::unique_lock<std::mutex> lock(mutex);
		threadContinue();
		completeCondition.wait(lock);
	}

	~MainThread() {
		std::unique_lock<std::mutex> lock(mutex);
		exit = true;
		threadContinue();
		lock.unlock();

		thread.join();
	}

	//Event stuff
	void pollEvents() {
		return execute(pollEventsImpl);
	}

	void waitEvents() {
		return execute(waitEventsImpl);
	}

	void waitEvents(Duration timeout) {
		return execute(waitEventsToImpl, timeout);
	}
	

	//Monitor stuff
	Monitor getPrimaryMonitor(){
		return execute(getPrimaryMonitorImpl);
	}

	std::vector<Monitor> getMonitors(){
		return execute(getMonitorsImpl);
	}

	bool isValid(MonitorHandle mon){
		return execute(isValidImpl, mon);
	}

	std::string_view getName(MonitorHandle mon){
		return execute(getNameImpl, mon);
	}

	Math::Vec2d getPhysicalSize(MonitorHandle mon){
		return execute(getPhysicalSizeImpl, mon);
	}

	Math::Vec2i getSize(MonitorHandle mon){
		return execute(getMonitorSizeImpl, mon);
	}

	Math::Vec2i getPosition(MonitorHandle mon){
		return execute(getMonitorPositionImpl, mon);
	}

	int getFrameRate(MonitorHandle mon){
		return execute(getFrameRateImpl, mon);
	}

	Monitor::Mode getMode(MonitorHandle mon){
		return execute(getModeImpl, mon);
	}

	std::vector<Monitor::Mode> getModes(MonitorHandle mon){
		return execute(getModesImpl, mon);
	}


	//Window stuff
	GLFW::WindowHandle createWindow(const Math::Vec2i& size, 
									const char* name,
									Monitor mon,
									Window* usrPtr )
	{
		return execute(
			std::mem_fn(&MainThread::createWindowImpl),
			this, size, name, mon, usrPtr
		);
	}

	void destroyWindow(WindowHandle window) {
		execute(
			std::mem_fn(&MainThread::destroyWindowImpl),
			this, window
		);
	}

	void setName(WindowHandle win, const char* name){
		execute(setNameImpl, win, name);
	}



	void setState(	WindowHandle win,  
					Window::Geometry* windowedGeom,
					Window::State st )
	{
		execute(setStateImpl, win, windowedGeom, st);
	}

	Window::State getState(WindowHandle win){
		return execute(getStateImpl, win);
	}



	void setMonitor(WindowHandle win, 
					Window::Geometry* windowedGeom,
					const Monitor& newMon )
	{
		execute(setMonitorImpl, win, windowedGeom, newMon);
	}

	Monitor getMonitor(WindowHandle win){
		return execute(getMonitorImpl, win);
	}



	void setPosition(WindowHandle win, Math::Vec2i pos){
		execute(setPositionImpl, win, pos);
	}

	Math::Vec2i getPosition(WindowHandle win){
		return execute(getWindowPositionImpl, win);
	}



	void setSize(WindowHandle win, Math::Vec2i size){
		execute(setSizeImpl, win, size);
	}

	Math::Vec2i getSize(WindowHandle win){
		return execute(getSizeImpl, win);
	}



	void setOpacity(WindowHandle win, float opa){
		execute(setOpacityImpl, win, opa);
	}

	float getOpacity(WindowHandle win){
		return execute(getOpacityImpl, win);
	}



	Resolution getResolution(WindowHandle win){
		return execute(getResolutionImpl, win);
	}



	Math::Vec2f getScale(WindowHandle win){
		return execute(getScaleImpl, win);
	}


	void focus(WindowHandle win){
		execute(focusImpl, win);
	}



	void setDecorated(WindowHandle win, bool deco){
		execute(setDecoratedImpl, win, deco);
	}

	bool getDecorated(WindowHandle win){
		return execute(getDecoratedImpl, win);
	}



	void setResizeable(WindowHandle win, bool resizeable){
		execute(setResizeableImpl, win, resizeable);
	}

	bool getResizeable(WindowHandle win){
		return execute(getResizeableImpl, win);
	}


	Window::KeyboardEvent getKeyState(WindowHandle win, Window::KeyboardKey key) {
		return execute(getKeyStateImpl, win, key);
	}

	Window::KeyboardEvent getMouseButtonState(WindowHandle win, Window::MouseButton but) {
		return execute(getMouseButtonStateImpl, win, but);
	}

	Math::Vec2d getMousePosition(WindowHandle win) {
		return execute(getMousePositionImpl, win);
	}


	std::string_view getKeyName(Window::KeyboardKey key, int scancode) {
		return execute(getKeyNameImpl, key, scancode);
	}

private:
	void threadFunc(){
		std::unique_lock<std::mutex> lock(mutex);

		glfwInit();

		Callbacks::setupMonitorCallback();

		while(exit == false){
			//Notify waiting threads
			completeCondition.notify_all();
			continueCondition.wait(lock);

			//Invoke all pending tasks
			while(tasks.size() > 0){
				tasks.front()();
				tasks.pop();
			}
		}

		glfwTerminate();
	}

	void threadContinue(){
		continueCondition.notify_all();
	}


	template<typename Func, typename... Args>
	typename std::invoke_result<Func, Args...>::type execute(Func&& func, Args&&... args) {
		using Ret = typename std::invoke_result<Func, Args...>::type;

		if(std::this_thread::get_id() == thread.get_id()){
			//We are on the main thread. Simply execute it
			return std::forward<Func>(func)(std::forward<Args>(args)...); 
		}else {
			std::unique_lock<std::mutex> lock(mutex);

			//Create a future object to pass it to the main thread
			std::packaged_task<Ret(Args...)> task(std::forward<Func>(func));
			tasks.push(std::bind(std::ref(task), std::forward<Args>(args)...));
			
			//Wait until execution is complete
			threadContinue();
			completeCondition.wait(lock);

			return task.get_future().get();
		}
	}



	//Event implementations
	static void pollEventsImpl() {
		assert(Callbacks::enabled == false);
		Callbacks::enabled = true;

		glfwPollEvents();

		Callbacks::enabled = false;
	}

	static void waitEventsImpl() {
		assert(Callbacks::enabled == false);
		Callbacks::enabled = true;

		glfwWaitEvents();

		Callbacks::enabled = false;
	}

	static void waitEventsToImpl(Duration timeout) {	
		assert(Callbacks::enabled == false);
		Callbacks::enabled = true;

		using seconds = std::chrono::duration<double, std::ratio<1, 1>>;
		glfwWaitEventsTimeout(seconds(timeout).count());

		Callbacks::enabled = false;
	}



	//Monitor implementations
	static Monitor getPrimaryMonitorImpl(){
		return Monitor(glfwGetPrimaryMonitor());
	}

	static std::vector<Monitor> getMonitorsImpl(){
		int cnt;
		MonitorHandle* monitors = glfwGetMonitors(&cnt);
		return std::vector<GLFW::Monitor>(monitors, monitors + cnt);
	}

	static bool isValidImpl(MonitorHandle mon){
		if(mon == nullptr) {
			return false;
		}

		if(Callbacks::currentMonitor == mon){
			return true;
		}

		int cnt;
		MonitorHandle* start = glfwGetMonitors(&cnt);
		MonitorHandle* end = start + cnt;

		return std::find(start, end, mon) != end;
	}

	static std::string_view getNameImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		return std::string_view(glfwGetMonitorName(mon));
	}

	static Math::Vec2d getPhysicalSizeImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		Math::Vec2i result;
		glfwGetMonitorPhysicalSize(mon, &result.x, &result.y);
		return Math::Vec2d(result) * 1e-3; //In metres
	}

	static Math::Vec2i getMonitorSizeImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		const auto* mode = glfwGetVideoMode(mon);
		return Math::Vec2i(mode->width, mode->height);
	}

	static Math::Vec2i getMonitorPositionImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		Math::Vec2i result;
		glfwGetMonitorPos(mon, &result.x, &result.y);
		return result;
	}

	static int getFrameRateImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		const auto* mode = glfwGetVideoMode(mon);
		return mode->refreshRate;
	}

	static Monitor::Mode getModeImpl(MonitorHandle mon){
		assert(isValidImpl(mon));

		const GLFWvidmode* mod = glfwGetVideoMode(mon);
		assert(mod);

		return Monitor::Mode{
			Monitor::ColorDepth(mod->redBits, mod->greenBits, mod->blueBits),
			Math::Vec2i(mod->width, mod->height),
			mod->refreshRate
		};
	}

	static std::vector<Monitor::Mode> getModesImpl(MonitorHandle mon){
		assert(isValidImpl(mon));

		int cnt;
		const GLFWvidmode* mod = glfwGetVideoModes(mon, &cnt);
		assert(mod);

		std::vector<Monitor::Mode> modes; modes.reserve(cnt);
		for(int i = 0; i < cnt; i++){
			modes.emplace_back(Monitor::Mode{
				Monitor::ColorDepth(mod[i].redBits, mod[i].greenBits, mod[i].blueBits),
				Math::Vec2i(mod[i].width, mod[i].height),
				mod[i].refreshRate
			});
		}

		return modes;
	}



	//Window implementations
	GLFW::WindowHandle createWindowImpl(Math::Vec2i size, 
										const char* name,
										Monitor mon,
										Window* usrPtr )
	{
		//Set Vulkan compatibility
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//Create the window
		WindowHandle win = glfwCreateWindow(
			size.x, size.y,
			name,
			mon.m_monitor,
			static_cast<WindowHandle>(nullptr)
		);

		if(win == nullptr){
			return nullptr;
		}

		//Set all callbacks
		Callbacks::setupUserPtr(win, static_cast<void*>(usrPtr));
		Callbacks::setupCallbacks(win);

		windowCount++;
		return win;
	}

	void destroyWindowImpl(WindowHandle window) {
		glfwDestroyWindow(window);
		windowCount--;
	}

	static void setNameImpl(WindowHandle win, const char* name){
		glfwSetWindowTitle(win, name);
	}



	static void setStateImpl(	WindowHandle win,  
								Window::Geometry* windowedGeom,
								Window::State st )
	{
		const auto lastState = getStateImpl(win);

		if(lastState != st){
			//Leave it on normal state
			switch (lastState) {
			case Window::State::HIDDEN:
				glfwShowWindow(win);
				break;

			case Window::State::FULLSCREEN:
				setMonitorImpl(	
					win, 
					windowedGeom, 
					GLFW::NO_MONITOR
				);
				break;

			case Window::State::ICONIFIED:
				glfwRestoreWindow(win);
				break;

			case Window::State::MAXIMIZED:
				glfwRestoreWindow(win);
				break;
			
			default: //State::NORMAL
				break;
			}

			//Switch to the desired state
			switch (st) {
			case Window::State::HIDDEN:
				glfwHideWindow(win);
				break;

			case Window::State::FULLSCREEN:
				setMonitorImpl(
					win, 
					windowedGeom, 
					getPrimaryMonitorImpl()
				);
				break;

			case Window::State::ICONIFIED:
				glfwIconifyWindow(win);
				break;

			case Window::State::MAXIMIZED:
				glfwMaximizeWindow(win);
				break;
			
			default: //State::NORMAL
				break;
			}
		}
	}

	static Window::State getStateImpl(WindowHandle win){
		Window::State result;

		if(!glfwGetWindowAttrib(win, GLFW_VISIBLE)){
			result = Window::State::HIDDEN;
		} else if(glfwGetWindowMonitor(win)){
			result = Window::State::FULLSCREEN;
		}else if(glfwGetWindowAttrib(win, GLFW_ICONIFIED)){
			result = Window::State::ICONIFIED;
		} else if(glfwGetWindowAttrib(win, GLFW_MAXIMIZED)){
			result = Window::State::MAXIMIZED;
		} else {
			result = Window::State::NORMAL;
		}

		return result;
	}



	static void setMonitorImpl(	WindowHandle win, 
								Window::Geometry* windowedGeom,
								const Monitor& newMon )
	{
		const auto oldMonHandle = glfwGetWindowMonitor(win);
		const auto newMonHandle = newMon.m_monitor;

		if(newMonHandle != oldMonHandle){
			//State has changed. Evaluate what to do
			if(isValidImpl(newMonHandle)) {
				//It is going to be full-screen
				if(!isValidImpl(oldMonHandle)){
					//It was windowed. Save its state
					*windowedGeom = {
						getWindowPositionImpl(win),
						getSizeImpl(win)
					};
				}

				const Math::Vec2i pos = getMonitorPositionImpl(newMonHandle);
				const Math::Vec2i size = getMonitorSizeImpl(newMonHandle);

				glfwSetWindowMonitor(
					win, 
					newMonHandle, 
					pos.x,
					pos.y,
					size.x,
					size.y,
					GLFW_DONT_CARE
				);
			} else {
				//It is going to be windowed
				glfwSetWindowMonitor(
					win, 
					static_cast<MonitorHandle>(nullptr), 
					windowedGeom->pos.x,
					windowedGeom->pos.y,
					windowedGeom->size.x,
					windowedGeom->size.y,
					GLFW_DONT_CARE
				);
			}
		}
	}

	static Monitor getMonitorImpl(WindowHandle win){
		return Monitor(glfwGetWindowMonitor(win));
	}



	static void setPositionImpl(WindowHandle win, const Math::Vec2i& pos){
		glfwSetWindowPos(win, pos.x, pos.y);
	}

	static Math::Vec2i getWindowPositionImpl(WindowHandle win){
		Math::Vec2i result;
		glfwGetWindowPos(win, &result.x, &result.y);
		return result;
	}



	static void setSizeImpl(WindowHandle win, const Math::Vec2i& size){
		glfwSetWindowSize(win, size.x, size.y);
	}

	static Math::Vec2i getSizeImpl(WindowHandle win){
		Math::Vec2i result;
		glfwGetWindowSize(win, &result.x, &result.y);
		return result;
	}



	static void setOpacityImpl(WindowHandle win, float opa){
		glfwSetWindowOpacity(win, opa);
	}

	static float getOpacityImpl(WindowHandle win){
		return glfwGetWindowOpacity(win);
	}



	static Resolution getResolutionImpl(WindowHandle win){
		int x, y;
		glfwGetFramebufferSize(win, &x, &y);
		return Resolution(x, y);
	}



	static Math::Vec2f getScaleImpl(WindowHandle win){
		Math::Vec2f result;
		glfwGetWindowContentScale(win, &result.x, &result.y);
		return result;
	}


	static void focusImpl(WindowHandle win){
		glfwFocusWindow(win);
	}



	static void setDecoratedImpl(WindowHandle win, bool deco){
		glfwSetWindowAttrib(win, GLFW_DECORATED, deco);
	}

	static bool getDecoratedImpl(WindowHandle win){
		return glfwGetWindowAttrib(win, GLFW_DECORATED);
	}



	static void setResizeableImpl(WindowHandle win, bool resizeable){
		glfwSetWindowAttrib(win, GLFW_RESIZABLE, resizeable);
	}

	static bool getResizeableImpl(WindowHandle win){
		return glfwGetWindowAttrib(win, GLFW_RESIZABLE);
	}



	static Window::KeyboardEvent getKeyStateImpl(WindowHandle win, Window::KeyboardKey key) {
		return static_cast<Window::KeyboardEvent>(glfwGetKey(win, static_cast<int>(key)));
	}
	
	static Window::KeyboardEvent getMouseButtonStateImpl(WindowHandle win, Window::MouseButton but) {
		return static_cast<Window::KeyboardEvent>(glfwGetMouseButton(win, static_cast<int>(but)));
	}
	
	static Math::Vec2d getMousePositionImpl(WindowHandle win) {
		Math::Vec2d result;
		glfwGetCursorPos(win, &result.x, &result.y);
		return result;
	}
	

	static std::string_view getKeyNameImpl(Window::KeyboardKey key, int scancode) {
		const char* keyName = glfwGetKeyName(static_cast<int>(key), scancode);
		return keyName ? std::string_view(keyName) : std::string_view("");
	}
};


/*
 * GLFW
 */

const GLFW::Monitor GLFW::NO_MONITOR(nullptr);
GLFW* GLFW::s_singleton;


GLFW::GLFW()
	: m_mainThread(std::make_unique<MainThread>())
{
}

GLFW::~GLFW() = default;

GLFW::Monitor GLFW::getPrimaryMonitor() const {
	return m_mainThread->getPrimaryMonitor(); 
}

std::vector<GLFW::Monitor> GLFW::getMonitors() const {
	return m_mainThread->getMonitors();
}

void GLFW::setMonitorCallback(MonitorCallback cbk){
	std::lock_guard<std::mutex> lock(Callbacks::mutex);
	m_monitorCbk = std::move(cbk);
}

const GLFW::MonitorCallback& GLFW::getMonitorCallback() const {
	return m_monitorCbk;
}

void GLFW::pollEvents() const {
	m_mainThread->pollEvents();
}

void GLFW::waitEvents() const {
	m_mainThread->waitEvents();
}

void GLFW::waitEvents(Duration timeout) const {
	m_mainThread->waitEvents(timeout);
}

void GLFW::postEmptyEvent() const {
	glfwPostEmptyEvent(); //Thread safe
}



void GLFW::initialize() {
	assert(!s_singleton);
	s_singleton = new GLFW;
}

void GLFW::terminate() {
	assert(s_singleton);
	delete s_singleton;
	s_singleton = nullptr;
}


std::vector<vk::ExtensionProperties> GLFW::getRequiredVulkanInstanceExtensions() {
	//Thread safe
	uint32_t glfwExtensionCount;
	const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<vk::ExtensionProperties> extensions(glfwExtensionCount);
	for(size_t i = 0; i < extensions.size(); i++){
		std::strncpy(extensions[i].extensionName, glfwExtensions[i], VK_MAX_EXTENSION_NAME_SIZE);
	}

	return extensions;
}

std::vector<vk::ExtensionProperties> GLFW::getRequiredVulkanDeviceExtensions() {
	return {
		vk::ExtensionProperties(std::array<char, VK_MAX_EXTENSION_NAME_SIZE>{VK_KHR_SWAPCHAIN_EXTENSION_NAME})
	};
}

bool GLFW::getPresentationSupport(	vk::Instance instance, 
									vk::PhysicalDevice device, 
									uint32_t family )
{
	//Thread safe
	return glfwGetPhysicalDevicePresentationSupport(
		static_cast<VkInstance>(instance), 
		static_cast<VkPhysicalDevice>(device), 
		family
	);
}


GLFW& GLFW::getGLFW() {
	assert(s_singleton);
	return *s_singleton;
}


/*
 * GLFW::Monitor
 */

GLFW::Monitor::Monitor(MonitorHandle mon) 
	: m_monitor(mon)
{
}

GLFW::Monitor::operator bool() const {
	return getGLFW().m_mainThread->isValid(m_monitor);
}

std::string_view GLFW::Monitor::getName() const {
	return getGLFW().m_mainThread->getName(m_monitor);
}

Math::Vec2d GLFW::Monitor::getPhysicalSize() const {
	return getGLFW().m_mainThread->getPhysicalSize(m_monitor);
}

Math::Vec2i GLFW::Monitor::getSize() const {
	return getGLFW().m_mainThread->getSize(m_monitor);
}

Math::Vec2i GLFW::Monitor::getPosition() const {
	return getGLFW().m_mainThread->getPosition(m_monitor);
}

int GLFW::Monitor::getFrameRate() const {
	return getGLFW().m_mainThread->getFrameRate(m_monitor);
}

GLFW::Monitor::Mode GLFW::Monitor::getMode() const {
	return getGLFW().m_mainThread->getMode(m_monitor);
}

std::vector<GLFW::Monitor::Mode> GLFW::Monitor::getModes() const {
	return getGLFW().m_mainThread->getModes(m_monitor);
}



/*
 * GLFW::Window
 */

GLFW::Window::Window(WindowHandle handle)
	: m_window(handle)
{
}

GLFW::Window::Window(	Math::Vec2i size, 
						const char* name,
						Monitor mon,
						Callbacks callbacks )
	: m_window(getGLFW().m_mainThread->createWindow(size, name, mon, this))
	, m_callbacks(std::move(callbacks))
{
	if(m_window == nullptr){
		throw Exception("Error creating the GLFW window");
	}
}

GLFW::Window::Window(Window&& other) {
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);

	m_window = other.m_window;
	other.m_window = nullptr;

	GLFW::Callbacks::setupUserPtr(m_window, this); //Thread-safe GLFW function
	m_callbacks = std::move(other.m_callbacks);
}

GLFW::Window::~Window(){
	if(m_window){
		getGLFW().m_mainThread->destroyWindow(m_window);
	}
}

GLFW::Window& GLFW::Window::operator=(Window&& other){
	if(m_window){
		getGLFW().m_mainThread->destroyWindow(m_window);
	}

	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);

	m_window = other.m_window;
	other.m_window = nullptr;

	GLFW::Callbacks::setupUserPtr(m_window, this);
	m_callbacks = std::move(other.m_callbacks);

	return *this;
}

GLFW::Window::operator bool() const {
	return m_window;
}



//Getters / Setters
vk::UniqueSurfaceKHR GLFW::Window::getSurface(const Graphics::Vulkan& vulkan) const {
	using Deleter = vk::UniqueHandleTraits<vk::SurfaceKHR, vk::DispatchLoaderDynamic>::deleter;

	//Try to create the surface
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface( //Thread-safe GLFW function
		vulkan.getInstance(), 
		m_window, 
		nullptr, 
		&surface
	);

	if(err != VK_SUCCESS){
		throw Exception("Error creating Vulkan surface");
	}

	return vk::UniqueSurfaceKHR(
		surface,
		Deleter(vulkan.getInstance(), nullptr, vulkan.getDispatcher())
	);
}	


void GLFW::Window::setName(const char* name){
	getGLFW().m_mainThread->setName(m_window, name);
}



void GLFW::Window::setState(State st){
	getGLFW().m_mainThread->setState(m_window, &m_windowedState, st);
}

GLFW::Window::State GLFW::Window::getState() const{
	return getGLFW().m_mainThread->getState(m_window);
}

void GLFW::Window::setStateCallback(StateCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.stateCbk = std::move(cbk);
}

const GLFW::Window::StateCallback& GLFW::Window::getStateCallback() const{
	return m_callbacks.stateCbk;
}



void GLFW::Window::setMonitor(const Monitor& mon){
	getGLFW().m_mainThread->setMonitor(m_window, &m_windowedState, mon);
}

GLFW::Monitor GLFW::Window::getMonitor() const{
	return getGLFW().m_mainThread->getMonitor(m_window);
}



void GLFW::Window::setPosition(const Math::Vec2i& pos){
	getGLFW().m_mainThread->setPosition(m_window, pos);
}

Math::Vec2i GLFW::Window::getPosition() const{
	return getGLFW().m_mainThread->getPosition(m_window);
}

void GLFW::Window::setPositionCallback(PositionCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.positionCbk = std::move(cbk);
}

const GLFW::Window::PositionCallback& GLFW::Window::getPositionCallback() const{
	return m_callbacks.positionCbk;
}



void GLFW::Window::setSize(const Math::Vec2i& size){
	getGLFW().m_mainThread->setSize(m_window, size);
}

Math::Vec2i GLFW::Window::getSize() const{
	return getGLFW().m_mainThread->getSize(m_window);
}

void GLFW::Window::setSizeCallback(SizeCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.sizeCbk = std::move(cbk);
}

const GLFW::Window::SizeCallback& GLFW::Window::getSizeCallback() const{
	return m_callbacks.sizeCbk;
}



void GLFW::Window::setOpacity(float opa){
	getGLFW().m_mainThread->setOpacity(m_window, opa);
}

float GLFW::Window::getOpacity() const{
	return getGLFW().m_mainThread->getOpacity(m_window);
}



Resolution GLFW::Window::getResolution() const{
	return getGLFW().m_mainThread->getResolution(m_window);
}

void GLFW::Window::setResolutionCallback(ResolutionCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.resolutionCbk = std::move(cbk);
}

const GLFW::Window::ResolutionCallback& GLFW::Window::getResolutionCallback() const{
	return m_callbacks.resolutionCbk;
}



Math::Vec2f GLFW::Window::getScale() const{
	return getGLFW().m_mainThread->getScale(m_window);
}

void GLFW::Window::setScaleCallback(ScaleCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.scaleCbk = std::move(cbk);
}

const GLFW::Window::ScaleCallback& GLFW::Window::getScaleCallback() const{
	return m_callbacks.scaleCbk;
}



void GLFW::Window::close(){
	getGLFW().m_mainThread->destroyWindow(m_window);
	m_window = nullptr;
}

bool GLFW::Window::shouldClose() const{
	return glfwWindowShouldClose(m_window); //Thread-safe GLFW function
}

void GLFW::Window::setCloseCallback(CloseCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.closeCbk = std::move(cbk);
}

const GLFW::Window::CloseCallback& GLFW::Window::getCloseCallback() const{
	return m_callbacks.closeCbk;
}



void GLFW::Window::focus(){
	getGLFW().m_mainThread->focus(m_window);
}

void GLFW::Window::setFocusCallback(FocusCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.focusCbk = std::move(cbk);
}

const GLFW::Window::FocusCallback& GLFW::Window::getFocusCallback() const{
	return m_callbacks.focusCbk;
}



void GLFW::Window::setRefreshCallback(RefreshCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.refreshCbk = std::move(cbk);
}

const GLFW::Window::RefreshCallback& GLFW::Window::getRefreshCallback() const{
	return m_callbacks.refreshCbk;
}



void GLFW::Window::setDecorated(bool deco){
	getGLFW().m_mainThread->setDecorated(m_window, deco);
}

bool GLFW::Window::getDecorated() const{
	return getGLFW().m_mainThread->getDecorated(m_window);
}



void GLFW::Window::setResizeable(bool resizeable){
	getGLFW().m_mainThread->setResizeable(m_window, resizeable);
}

bool GLFW::Window::getResizeable() const{
	return getGLFW().m_mainThread->getResizeable(m_window);
}



GLFW::Window::KeyboardEvent GLFW::Window::getKeyState(KeyboardKey key) const {
	return getGLFW().m_mainThread->getKeyState(m_window, key);
}

void GLFW::Window::setKeyboardCallback(KeyboardCallback cbk) {
	m_callbacks.keyboardCbk = std::move(cbk);
}

const GLFW::Window::KeyboardCallback& GLFW::Window::getKeyboardCallback() const {
	return m_callbacks.keyboardCbk;
}


void GLFW::Window::setCharacterCallback(CharacterCallback cbk) {
	m_callbacks.characterCbk = std::move(cbk);
}

const GLFW::Window::CharacterCallback& GLFW::Window::getCharacterCallback() const {
	return m_callbacks.characterCbk;
}


GLFW::Window::KeyboardEvent GLFW::Window::getMouseButtonState(MouseButton but) const {
	return getGLFW().m_mainThread->getMouseButtonState(m_window, but);
}

void GLFW::Window::setMouseButtonCallback(MouseButtonCallback cbk) {
	m_callbacks.mouseButtonCbk = std::move(cbk);
}

const GLFW::Window::MouseButtonCallback& GLFW::Window::getMouseButtonCallback() const {
	return m_callbacks.mouseButtonCbk;
}


Math::Vec2d GLFW::Window::getMousePosition() const {
	return getGLFW().m_mainThread->getMousePosition(m_window);
}

void GLFW::Window::setMousePositionCallback(MousePositionCallback cbk) {
	m_callbacks.mousePositionCbk = std::move(cbk);
}

const GLFW::Window::MousePositionCallback& GLFW::Window::getMousePositionCallback() const {
	return m_callbacks.mousePositionCbk;
}


void GLFW::Window::setMouseScrollCallback(MouseScrollCallback cbk) {
	m_callbacks.mouseScrollCbk = std::move(cbk);
}

const GLFW::Window::MouseScrollCallback& GLFW::Window::getMouseScrollCallback() const {
	return m_callbacks.mouseScrollCbk;
}


void GLFW::Window::setCursorEnterCallback(CursorEnterCallback cbk) {
	m_callbacks.cursorEnterCbk = std::move(cbk);
}

const GLFW::Window::CursorEnterCallback& GLFW::Window::getCursorEnterCallback() const {
	return m_callbacks.cursorEnterCbk;
}



std::string_view GLFW::Window::getKeyName(KeyboardKey key, int scancode) {
	return getGLFW().m_mainThread->getKeyName(key, scancode); 
}

}