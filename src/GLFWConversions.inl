#include "GLFWConversions.h"

namespace Zuazo {

constexpr GLFW::Window::KeyboardKey toGLFW(KeyboardKey key) {
	switch(key) {
	case KeyboardKey::SPACE:			return GLFW::Window::KeyboardKey::SPACE;
	case KeyboardKey::APOSTROPHE:		return GLFW::Window::KeyboardKey::APOSTROPHE;
	case KeyboardKey::COMMA:			return GLFW::Window::KeyboardKey::COMMA;
	case KeyboardKey::MINUS:			return GLFW::Window::KeyboardKey::MINUS;
	case KeyboardKey::PERIOD:			return GLFW::Window::KeyboardKey::PERIOD;
	case KeyboardKey::SLASH:			return GLFW::Window::KeyboardKey::SLASH;
	case KeyboardKey::NB0:				return GLFW::Window::KeyboardKey::NB0;
	case KeyboardKey::NB1:				return GLFW::Window::KeyboardKey::NB1;
	case KeyboardKey::NB2:				return GLFW::Window::KeyboardKey::NB2;
	case KeyboardKey::NB3:				return GLFW::Window::KeyboardKey::NB3;
	case KeyboardKey::NB4:				return GLFW::Window::KeyboardKey::NB4;
	case KeyboardKey::NB5:				return GLFW::Window::KeyboardKey::NB5;
	case KeyboardKey::NB6:				return GLFW::Window::KeyboardKey::NB6;
	case KeyboardKey::NB7:				return GLFW::Window::KeyboardKey::NB7;
	case KeyboardKey::NB8:				return GLFW::Window::KeyboardKey::NB8;
	case KeyboardKey::NB9:				return GLFW::Window::KeyboardKey::NB9;
	case KeyboardKey::SEMICOLON:		return GLFW::Window::KeyboardKey::SEMICOLON;
	case KeyboardKey::EQUAL:			return GLFW::Window::KeyboardKey::EQUAL;
	case KeyboardKey::A:				return GLFW::Window::KeyboardKey::A;
	case KeyboardKey::B:				return GLFW::Window::KeyboardKey::B;
	case KeyboardKey::C:				return GLFW::Window::KeyboardKey::C;
	case KeyboardKey::D:				return GLFW::Window::KeyboardKey::D;
	case KeyboardKey::E:				return GLFW::Window::KeyboardKey::E;
	case KeyboardKey::F:				return GLFW::Window::KeyboardKey::F;
	case KeyboardKey::G:				return GLFW::Window::KeyboardKey::G;
	case KeyboardKey::H:				return GLFW::Window::KeyboardKey::H;
	case KeyboardKey::I:				return GLFW::Window::KeyboardKey::I;
	case KeyboardKey::J:				return GLFW::Window::KeyboardKey::J;
	case KeyboardKey::K:				return GLFW::Window::KeyboardKey::K;
	case KeyboardKey::L:				return GLFW::Window::KeyboardKey::L;
	case KeyboardKey::M:				return GLFW::Window::KeyboardKey::M;
	case KeyboardKey::N:				return GLFW::Window::KeyboardKey::N;
	case KeyboardKey::O:				return GLFW::Window::KeyboardKey::O;
	case KeyboardKey::P:				return GLFW::Window::KeyboardKey::P;
	case KeyboardKey::Q:				return GLFW::Window::KeyboardKey::Q;
	case KeyboardKey::R:				return GLFW::Window::KeyboardKey::R;
	case KeyboardKey::S:				return GLFW::Window::KeyboardKey::S;
	case KeyboardKey::T:				return GLFW::Window::KeyboardKey::T;
	case KeyboardKey::U:				return GLFW::Window::KeyboardKey::U;
	case KeyboardKey::V:				return GLFW::Window::KeyboardKey::V;
	case KeyboardKey::W:				return GLFW::Window::KeyboardKey::W;
	case KeyboardKey::X:				return GLFW::Window::KeyboardKey::X;
	case KeyboardKey::Y:				return GLFW::Window::KeyboardKey::Y;
	case KeyboardKey::Z:				return GLFW::Window::KeyboardKey::Z;
	case KeyboardKey::LEFT_BRACKET:		return GLFW::Window::KeyboardKey::LEFT_BRACKET;
	case KeyboardKey::BACKSLASH:		return GLFW::Window::KeyboardKey::BACKSLASH;
	case KeyboardKey::RIGHT_BRACKET:	return GLFW::Window::KeyboardKey::RIGHT_BRACKET;
	case KeyboardKey::GRAVE_ACCENT:		return GLFW::Window::KeyboardKey::GRAVE_ACCENT;
	//::KeyboardKey::WORLD_1:			GLFW::Window::	return KeyboardKey::WORLD_1;
	//::KeyboardKey::WORLD_2:			GLFW::Window::	return KeyboardKey::WORLD_2;
	case KeyboardKey::ESCAPE:			return GLFW::Window::KeyboardKey::ESCAPE;
	case KeyboardKey::ENTER:			return GLFW::Window::KeyboardKey::ENTER;
	case KeyboardKey::TAB:				return GLFW::Window::KeyboardKey::TAB;
	case KeyboardKey::BACKSPACE:		return GLFW::Window::KeyboardKey::BACKSPACE;
	case KeyboardKey::INSERT:			return GLFW::Window::KeyboardKey::INSERT;
	case KeyboardKey::DELETE:			return GLFW::Window::KeyboardKey::DELETE;
	case KeyboardKey::RIGHT:			return GLFW::Window::KeyboardKey::RIGHT;
	case KeyboardKey::LEFT:				return GLFW::Window::KeyboardKey::LEFT;
	case KeyboardKey::DOWN:				return GLFW::Window::KeyboardKey::DOWN;
	case KeyboardKey::UP:				return GLFW::Window::KeyboardKey::UP;
	case KeyboardKey::PAGE_UP:			return GLFW::Window::KeyboardKey::PAGE_UP;
	case KeyboardKey::PAGE_DOWN:		return GLFW::Window::KeyboardKey::PAGE_DOWN;
	case KeyboardKey::HOME:				return GLFW::Window::KeyboardKey::HOME;
	case KeyboardKey::END:				return GLFW::Window::KeyboardKey::END;
	case KeyboardKey::CAPS_LOCK:		return GLFW::Window::KeyboardKey::CAPS_LOCK;
	case KeyboardKey::SCROLL_LOCK:		return GLFW::Window::KeyboardKey::SCROLL_LOCK;
	case KeyboardKey::NUM_LOCK:			return GLFW::Window::KeyboardKey::NUM_LOCK;
	case KeyboardKey::PRINT_SCREEN:		return GLFW::Window::KeyboardKey::PRINT_SCREEN;
	case KeyboardKey::PAUSE:			return GLFW::Window::KeyboardKey::PAUSE;
	case KeyboardKey::F1:				return GLFW::Window::KeyboardKey::F1;
	case KeyboardKey::F2:				return GLFW::Window::KeyboardKey::F2;
	case KeyboardKey::F3:				return GLFW::Window::KeyboardKey::F3;
	case KeyboardKey::F4:				return GLFW::Window::KeyboardKey::F4;
	case KeyboardKey::F5:				return GLFW::Window::KeyboardKey::F5;
	case KeyboardKey::F6:				return GLFW::Window::KeyboardKey::F6;
	case KeyboardKey::F7:				return GLFW::Window::KeyboardKey::F7;
	case KeyboardKey::F8:				return GLFW::Window::KeyboardKey::F8;
	case KeyboardKey::F9:				return GLFW::Window::KeyboardKey::F9;
	case KeyboardKey::F10:				return GLFW::Window::KeyboardKey::F10;
	case KeyboardKey::F11:				return GLFW::Window::KeyboardKey::F11;
	case KeyboardKey::F12:				return GLFW::Window::KeyboardKey::F12;
	case KeyboardKey::F13:				return GLFW::Window::KeyboardKey::F13;
	case KeyboardKey::F14:				return GLFW::Window::KeyboardKey::F14;
	case KeyboardKey::F15:				return GLFW::Window::KeyboardKey::F15;
	case KeyboardKey::F16:				return GLFW::Window::KeyboardKey::F16;
	case KeyboardKey::F17:				return GLFW::Window::KeyboardKey::F17;
	case KeyboardKey::F18:				return GLFW::Window::KeyboardKey::F18;
	case KeyboardKey::F19:				return GLFW::Window::KeyboardKey::F19;
	case KeyboardKey::F20:				return GLFW::Window::KeyboardKey::F20;
	case KeyboardKey::F21:				return GLFW::Window::KeyboardKey::F21;
	case KeyboardKey::F22:				return GLFW::Window::KeyboardKey::F22;
	case KeyboardKey::F23:				return GLFW::Window::KeyboardKey::F23;
	case KeyboardKey::F24:				return GLFW::Window::KeyboardKey::F24;
	case KeyboardKey::F25:				return GLFW::Window::KeyboardKey::F25;
	case KeyboardKey::KP_0:				return GLFW::Window::KeyboardKey::KP_0;
	case KeyboardKey::KP_1:				return GLFW::Window::KeyboardKey::KP_1;
	case KeyboardKey::KP_2:				return GLFW::Window::KeyboardKey::KP_2;
	case KeyboardKey::KP_3:				return GLFW::Window::KeyboardKey::KP_3;
	case KeyboardKey::KP_4:				return GLFW::Window::KeyboardKey::KP_4;
	case KeyboardKey::KP_5:				return GLFW::Window::KeyboardKey::KP_5;
	case KeyboardKey::KP_6:				return GLFW::Window::KeyboardKey::KP_6;
	case KeyboardKey::KP_7:				return GLFW::Window::KeyboardKey::KP_7;
	case KeyboardKey::KP_8:				return GLFW::Window::KeyboardKey::KP_8;
	case KeyboardKey::KP_9:				return GLFW::Window::KeyboardKey::KP_9;
	case KeyboardKey::KP_DECIMAL:		return GLFW::Window::KeyboardKey::KP_DECIMAL;
	case KeyboardKey::KP_DIVIDE:		return GLFW::Window::KeyboardKey::KP_DIVIDE;
	case KeyboardKey::KP_MULTIPLY:		return GLFW::Window::KeyboardKey::KP_MULTIPLY;
	case KeyboardKey::KP_SUBTRACT:		return GLFW::Window::KeyboardKey::KP_SUBTRACT;
	case KeyboardKey::KP_ADD:			return GLFW::Window::KeyboardKey::KP_ADD;
	case KeyboardKey::KP_ENTER:			return GLFW::Window::KeyboardKey::KP_ENTER;
	case KeyboardKey::KP_EQUAL:			return GLFW::Window::KeyboardKey::KP_EQUAL;
	case KeyboardKey::LEFT_SHIFT:		return GLFW::Window::KeyboardKey::LEFT_SHIFT;
	case KeyboardKey::LEFT_CONTROL:		return GLFW::Window::KeyboardKey::LEFT_CONTROL;
	case KeyboardKey::LEFT_ALT:			return GLFW::Window::KeyboardKey::LEFT_ALT;
	case KeyboardKey::LEFT_SUPER:		return GLFW::Window::KeyboardKey::LEFT_SUPER;
	case KeyboardKey::RIGHT_SHIFT:		return GLFW::Window::KeyboardKey::RIGHT_SHIFT;
	case KeyboardKey::RIGHT_CONTROL:	return GLFW::Window::KeyboardKey::RIGHT_CONTROL;
	case KeyboardKey::RIGHT_ALT:		return GLFW::Window::KeyboardKey::RIGHT_ALT;
	case KeyboardKey::RIGHT_SUPER:		return GLFW::Window::KeyboardKey::RIGHT_SUPER;
	case KeyboardKey::MENU:				return GLFW::Window::KeyboardKey::MENU;

	default:							return GLFW::Window::KeyboardKey::NONE;
	}
}

constexpr KeyboardKey fromGLFW(GLFW::Window::KeyboardKey key) {
	switch(key) {
	case GLFW::Window::KeyboardKey::SPACE:			return KeyboardKey::SPACE;
	case GLFW::Window::KeyboardKey::APOSTROPHE:		return KeyboardKey::APOSTROPHE;
	case GLFW::Window::KeyboardKey::COMMA:			return KeyboardKey::COMMA;
	case GLFW::Window::KeyboardKey::MINUS:			return KeyboardKey::MINUS;
	case GLFW::Window::KeyboardKey::PERIOD:			return KeyboardKey::PERIOD;
	case GLFW::Window::KeyboardKey::SLASH:			return KeyboardKey::SLASH;
	case GLFW::Window::KeyboardKey::NB0:			return KeyboardKey::NB0;
	case GLFW::Window::KeyboardKey::NB1:			return KeyboardKey::NB1;
	case GLFW::Window::KeyboardKey::NB2:			return KeyboardKey::NB2;
	case GLFW::Window::KeyboardKey::NB3:			return KeyboardKey::NB3;
	case GLFW::Window::KeyboardKey::NB4:			return KeyboardKey::NB4;
	case GLFW::Window::KeyboardKey::NB5:			return KeyboardKey::NB5;
	case GLFW::Window::KeyboardKey::NB6:			return KeyboardKey::NB6;
	case GLFW::Window::KeyboardKey::NB7:			return KeyboardKey::NB7;
	case GLFW::Window::KeyboardKey::NB8:			return KeyboardKey::NB8;
	case GLFW::Window::KeyboardKey::NB9:			return KeyboardKey::NB9;
	case GLFW::Window::KeyboardKey::SEMICOLON:		return KeyboardKey::SEMICOLON;
	case GLFW::Window::KeyboardKey::EQUAL:			return KeyboardKey::EQUAL;
	case GLFW::Window::KeyboardKey::A:				return KeyboardKey::A;
	case GLFW::Window::KeyboardKey::B:				return KeyboardKey::B;
	case GLFW::Window::KeyboardKey::C:				return KeyboardKey::C;
	case GLFW::Window::KeyboardKey::D:				return KeyboardKey::D;
	case GLFW::Window::KeyboardKey::E:				return KeyboardKey::E;
	case GLFW::Window::KeyboardKey::F:				return KeyboardKey::F;
	case GLFW::Window::KeyboardKey::G:				return KeyboardKey::G;
	case GLFW::Window::KeyboardKey::H:				return KeyboardKey::H;
	case GLFW::Window::KeyboardKey::I:				return KeyboardKey::I;
	case GLFW::Window::KeyboardKey::J:				return KeyboardKey::J;
	case GLFW::Window::KeyboardKey::K:				return KeyboardKey::K;
	case GLFW::Window::KeyboardKey::L:				return KeyboardKey::L;
	case GLFW::Window::KeyboardKey::M:				return KeyboardKey::M;
	case GLFW::Window::KeyboardKey::N:				return KeyboardKey::N;
	case GLFW::Window::KeyboardKey::O:				return KeyboardKey::O;
	case GLFW::Window::KeyboardKey::P:				return KeyboardKey::P;
	case GLFW::Window::KeyboardKey::Q:				return KeyboardKey::Q;
	case GLFW::Window::KeyboardKey::R:				return KeyboardKey::R;
	case GLFW::Window::KeyboardKey::S:				return KeyboardKey::S;
	case GLFW::Window::KeyboardKey::T:				return KeyboardKey::T;
	case GLFW::Window::KeyboardKey::U:				return KeyboardKey::U;
	case GLFW::Window::KeyboardKey::V:				return KeyboardKey::V;
	case GLFW::Window::KeyboardKey::W:				return KeyboardKey::W;
	case GLFW::Window::KeyboardKey::X:				return KeyboardKey::X;
	case GLFW::Window::KeyboardKey::Y:				return KeyboardKey::Y;
	case GLFW::Window::KeyboardKey::Z:				return KeyboardKey::Z;
	case GLFW::Window::KeyboardKey::LEFT_BRACKET:	return KeyboardKey::LEFT_BRACKET;
	case GLFW::Window::KeyboardKey::BACKSLASH:		return KeyboardKey::BACKSLASH;
	case GLFW::Window::KeyboardKey::RIGHT_BRACKET:	return KeyboardKey::RIGHT_BRACKET;
	case GLFW::Window::KeyboardKey::GRAVE_ACCENT:	return KeyboardKey::GRAVE_ACCENT;
	//case GLFW::Window::KeyboardKey::WORLD_1:		return KeyboardKey::WORLD_1;
	//case GLFW::Window::KeyboardKey::WORLD_2:		return KeyboardKey::WORLD_2;
	case GLFW::Window::KeyboardKey::ESCAPE:			return KeyboardKey::ESCAPE;
	case GLFW::Window::KeyboardKey::ENTER:			return KeyboardKey::ENTER;
	case GLFW::Window::KeyboardKey::TAB:			return KeyboardKey::TAB;
	case GLFW::Window::KeyboardKey::BACKSPACE:		return KeyboardKey::BACKSPACE;
	case GLFW::Window::KeyboardKey::INSERT:			return KeyboardKey::INSERT;
	case GLFW::Window::KeyboardKey::DELETE:			return KeyboardKey::DELETE;
	case GLFW::Window::KeyboardKey::RIGHT:			return KeyboardKey::RIGHT;
	case GLFW::Window::KeyboardKey::LEFT:			return KeyboardKey::LEFT;
	case GLFW::Window::KeyboardKey::DOWN:			return KeyboardKey::DOWN;
	case GLFW::Window::KeyboardKey::UP:				return KeyboardKey::UP;
	case GLFW::Window::KeyboardKey::PAGE_UP:		return KeyboardKey::PAGE_UP;
	case GLFW::Window::KeyboardKey::PAGE_DOWN:		return KeyboardKey::PAGE_DOWN;
	case GLFW::Window::KeyboardKey::HOME:			return KeyboardKey::HOME;
	case GLFW::Window::KeyboardKey::END:			return KeyboardKey::END;
	case GLFW::Window::KeyboardKey::CAPS_LOCK:		return KeyboardKey::CAPS_LOCK;
	case GLFW::Window::KeyboardKey::SCROLL_LOCK:	return KeyboardKey::SCROLL_LOCK;
	case GLFW::Window::KeyboardKey::NUM_LOCK:		return KeyboardKey::NUM_LOCK;
	case GLFW::Window::KeyboardKey::PRINT_SCREEN:	return KeyboardKey::PRINT_SCREEN;
	case GLFW::Window::KeyboardKey::PAUSE:			return KeyboardKey::PAUSE;
	case GLFW::Window::KeyboardKey::F1:				return KeyboardKey::F1;
	case GLFW::Window::KeyboardKey::F2:				return KeyboardKey::F2;
	case GLFW::Window::KeyboardKey::F3:				return KeyboardKey::F3;
	case GLFW::Window::KeyboardKey::F4:				return KeyboardKey::F4;
	case GLFW::Window::KeyboardKey::F5:				return KeyboardKey::F5;
	case GLFW::Window::KeyboardKey::F6:				return KeyboardKey::F6;
	case GLFW::Window::KeyboardKey::F7:				return KeyboardKey::F7;
	case GLFW::Window::KeyboardKey::F8:				return KeyboardKey::F8;
	case GLFW::Window::KeyboardKey::F9:				return KeyboardKey::F9;
	case GLFW::Window::KeyboardKey::F10:			return KeyboardKey::F10;
	case GLFW::Window::KeyboardKey::F11:			return KeyboardKey::F11;
	case GLFW::Window::KeyboardKey::F12:			return KeyboardKey::F12;
	case GLFW::Window::KeyboardKey::F13:			return KeyboardKey::F13;
	case GLFW::Window::KeyboardKey::F14:			return KeyboardKey::F14;
	case GLFW::Window::KeyboardKey::F15:			return KeyboardKey::F15;
	case GLFW::Window::KeyboardKey::F16:			return KeyboardKey::F16;
	case GLFW::Window::KeyboardKey::F17:			return KeyboardKey::F17;
	case GLFW::Window::KeyboardKey::F18:			return KeyboardKey::F18;
	case GLFW::Window::KeyboardKey::F19:			return KeyboardKey::F19;
	case GLFW::Window::KeyboardKey::F20:			return KeyboardKey::F20;
	case GLFW::Window::KeyboardKey::F21:			return KeyboardKey::F21;
	case GLFW::Window::KeyboardKey::F22:			return KeyboardKey::F22;
	case GLFW::Window::KeyboardKey::F23:			return KeyboardKey::F23;
	case GLFW::Window::KeyboardKey::F24:			return KeyboardKey::F24;
	case GLFW::Window::KeyboardKey::F25:			return KeyboardKey::F25;
	case GLFW::Window::KeyboardKey::KP_0:			return KeyboardKey::KP_0;
	case GLFW::Window::KeyboardKey::KP_1:			return KeyboardKey::KP_1;
	case GLFW::Window::KeyboardKey::KP_2:			return KeyboardKey::KP_2;
	case GLFW::Window::KeyboardKey::KP_3:			return KeyboardKey::KP_3;
	case GLFW::Window::KeyboardKey::KP_4:			return KeyboardKey::KP_4;
	case GLFW::Window::KeyboardKey::KP_5:			return KeyboardKey::KP_5;
	case GLFW::Window::KeyboardKey::KP_6:			return KeyboardKey::KP_6;
	case GLFW::Window::KeyboardKey::KP_7:			return KeyboardKey::KP_7;
	case GLFW::Window::KeyboardKey::KP_8:			return KeyboardKey::KP_8;
	case GLFW::Window::KeyboardKey::KP_9:			return KeyboardKey::KP_9;
	case GLFW::Window::KeyboardKey::KP_DECIMAL:		return KeyboardKey::KP_DECIMAL;
	case GLFW::Window::KeyboardKey::KP_DIVIDE:		return KeyboardKey::KP_DIVIDE;
	case GLFW::Window::KeyboardKey::KP_MULTIPLY:	return KeyboardKey::KP_MULTIPLY;
	case GLFW::Window::KeyboardKey::KP_SUBTRACT:	return KeyboardKey::KP_SUBTRACT;
	case GLFW::Window::KeyboardKey::KP_ADD:			return KeyboardKey::KP_ADD;
	case GLFW::Window::KeyboardKey::KP_ENTER:		return KeyboardKey::KP_ENTER;
	case GLFW::Window::KeyboardKey::KP_EQUAL:		return KeyboardKey::KP_EQUAL;
	case GLFW::Window::KeyboardKey::LEFT_SHIFT:		return KeyboardKey::LEFT_SHIFT;
	case GLFW::Window::KeyboardKey::LEFT_CONTROL:	return KeyboardKey::LEFT_CONTROL;
	case GLFW::Window::KeyboardKey::LEFT_ALT:		return KeyboardKey::LEFT_ALT;
	case GLFW::Window::KeyboardKey::LEFT_SUPER:		return KeyboardKey::LEFT_SUPER;
	case GLFW::Window::KeyboardKey::RIGHT_SHIFT:	return KeyboardKey::RIGHT_SHIFT;
	case GLFW::Window::KeyboardKey::RIGHT_CONTROL:	return KeyboardKey::RIGHT_CONTROL;
	case GLFW::Window::KeyboardKey::RIGHT_ALT:		return KeyboardKey::RIGHT_ALT;
	case GLFW::Window::KeyboardKey::RIGHT_SUPER:	return KeyboardKey::RIGHT_SUPER;
	case GLFW::Window::KeyboardKey::MENU:			return KeyboardKey::MENU;

	default:										return KeyboardKey::NONE;
	}
}



static_assert(static_cast<int>(GLFW::Window::MouseButton::NB1) == static_cast<int>(MouseKey::NB1), "Mouse button NB1 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB2) == static_cast<int>(MouseKey::NB2), "Mouse button NB2 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB3) == static_cast<int>(MouseKey::NB3), "Mouse button NB3 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB4) == static_cast<int>(MouseKey::NB4), "Mouse button NB4 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB5) == static_cast<int>(MouseKey::NB5), "Mouse button NB5 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB6) == static_cast<int>(MouseKey::NB6), "Mouse button NB6 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB7) == static_cast<int>(MouseKey::NB7), "Mouse button NB7 does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB8) == static_cast<int>(MouseKey::NB8), "Mouse button NB8 does not match");

constexpr GLFW::Window::MouseButton toGLFW(MouseKey key) {
	return static_cast<GLFW::Window::MouseButton>(key);
}

constexpr MouseKey fromGLFW(GLFW::Window::MouseButton key) {
	return static_cast<MouseKey>(key);
}



static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::NONE) == static_cast<int>(KeyModifiers::NONE), "NONE modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::SHIFT) == static_cast<int>(KeyModifiers::SHIFT), "SHIFT modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CONTROL) == static_cast<int>(KeyModifiers::CONTROL), "ALT modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::ALT) == static_cast<int>(KeyModifiers::ALT), "ALT modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::SUPER) == static_cast<int>(KeyModifiers::SUPER), "SUPER modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CONTROL) == static_cast<int>(KeyModifiers::CONTROL), "SHIFT modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CAPS_LOCK) == static_cast<int>(KeyModifiers::CAPS_LOCK), "CAPS_LOCK modifier flag does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::NUM_LOCK) == static_cast<int>(KeyModifiers::NUM_LOCK), "NUM_LOCK modifier flag does not match");

constexpr GLFW::Window::KeyboardModifiers toGLFW(KeyModifiers modifiers) {
	return static_cast<GLFW::Window::KeyboardModifiers>(modifiers);
}

constexpr KeyModifiers fromGLFW(GLFW::Window::KeyboardModifiers modifiers) {
	return static_cast<KeyModifiers>(modifiers);
}



static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::RELEASE) == static_cast<int>(KeyEvent::RELEASE), "RELEASE event does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::PRESS) == static_cast<int>(KeyEvent::PRESS), "PRESS event does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::REPEAT) == static_cast<int>(KeyEvent::REPEAT), "RELEASE event does not match");

constexpr GLFW::Window::KeyboardEvent toGLFW(KeyEvent event) {
	return static_cast<GLFW::Window::KeyboardEvent>(event);
}

constexpr KeyEvent fromGLFW(GLFW::Window::KeyboardEvent event) {
	return static_cast<KeyEvent>(event);
}

}