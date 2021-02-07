#include "GLFWConversions.h"

namespace Zuazo {

constexpr GLFW::KeyboardKey toGLFW(KeyboardKey key) {
	switch(key) {
	case KeyboardKey::SPACE:			return GLFW::KeyboardKey::SPACE;
	case KeyboardKey::APOSTROPHE:		return GLFW::KeyboardKey::APOSTROPHE;
	case KeyboardKey::COMMA:			return GLFW::KeyboardKey::COMMA;
	case KeyboardKey::MINUS:			return GLFW::KeyboardKey::MINUS;
	case KeyboardKey::PERIOD:			return GLFW::KeyboardKey::PERIOD;
	case KeyboardKey::SLASH:			return GLFW::KeyboardKey::SLASH;
	case KeyboardKey::NB0:				return GLFW::KeyboardKey::NB0;
	case KeyboardKey::NB1:				return GLFW::KeyboardKey::NB1;
	case KeyboardKey::NB2:				return GLFW::KeyboardKey::NB2;
	case KeyboardKey::NB3:				return GLFW::KeyboardKey::NB3;
	case KeyboardKey::NB4:				return GLFW::KeyboardKey::NB4;
	case KeyboardKey::NB5:				return GLFW::KeyboardKey::NB5;
	case KeyboardKey::NB6:				return GLFW::KeyboardKey::NB6;
	case KeyboardKey::NB7:				return GLFW::KeyboardKey::NB7;
	case KeyboardKey::NB8:				return GLFW::KeyboardKey::NB8;
	case KeyboardKey::NB9:				return GLFW::KeyboardKey::NB9;
	case KeyboardKey::SEMICOLON:		return GLFW::KeyboardKey::SEMICOLON;
	case KeyboardKey::EQUAL:			return GLFW::KeyboardKey::EQUAL;
	case KeyboardKey::A:				return GLFW::KeyboardKey::A;
	case KeyboardKey::B:				return GLFW::KeyboardKey::B;
	case KeyboardKey::C:				return GLFW::KeyboardKey::C;
	case KeyboardKey::D:				return GLFW::KeyboardKey::D;
	case KeyboardKey::E:				return GLFW::KeyboardKey::E;
	case KeyboardKey::F:				return GLFW::KeyboardKey::F;
	case KeyboardKey::G:				return GLFW::KeyboardKey::G;
	case KeyboardKey::H:				return GLFW::KeyboardKey::H;
	case KeyboardKey::I:				return GLFW::KeyboardKey::I;
	case KeyboardKey::J:				return GLFW::KeyboardKey::J;
	case KeyboardKey::K:				return GLFW::KeyboardKey::K;
	case KeyboardKey::L:				return GLFW::KeyboardKey::L;
	case KeyboardKey::M:				return GLFW::KeyboardKey::M;
	case KeyboardKey::N:				return GLFW::KeyboardKey::N;
	case KeyboardKey::O:				return GLFW::KeyboardKey::O;
	case KeyboardKey::P:				return GLFW::KeyboardKey::P;
	case KeyboardKey::Q:				return GLFW::KeyboardKey::Q;
	case KeyboardKey::R:				return GLFW::KeyboardKey::R;
	case KeyboardKey::S:				return GLFW::KeyboardKey::S;
	case KeyboardKey::T:				return GLFW::KeyboardKey::T;
	case KeyboardKey::U:				return GLFW::KeyboardKey::U;
	case KeyboardKey::V:				return GLFW::KeyboardKey::V;
	case KeyboardKey::W:				return GLFW::KeyboardKey::W;
	case KeyboardKey::X:				return GLFW::KeyboardKey::X;
	case KeyboardKey::Y:				return GLFW::KeyboardKey::Y;
	case KeyboardKey::Z:				return GLFW::KeyboardKey::Z;
	case KeyboardKey::LEFT_BRACKET:		return GLFW::KeyboardKey::LEFT_BRACKET;
	case KeyboardKey::BACKSLASH:		return GLFW::KeyboardKey::BACKSLASH;
	case KeyboardKey::RIGHT_BRACKET:	return GLFW::KeyboardKey::RIGHT_BRACKET;
	case KeyboardKey::GRAVE_ACCENT:		return GLFW::KeyboardKey::GRAVE_ACCENT;
	//::KeyboardKey::WORLD_1:			GLFW::	return KeyboardKey::WORLD_1;
	//::KeyboardKey::WORLD_2:			GLFW::	return KeyboardKey::WORLD_2;
	case KeyboardKey::ESCAPE:			return GLFW::KeyboardKey::ESCAPE;
	case KeyboardKey::ENTER:			return GLFW::KeyboardKey::ENTER;
	case KeyboardKey::TAB:				return GLFW::KeyboardKey::TAB;
	case KeyboardKey::BACKSPACE:		return GLFW::KeyboardKey::BACKSPACE;
	case KeyboardKey::INSERT:			return GLFW::KeyboardKey::INSERT;
	case KeyboardKey::DELETE:			return GLFW::KeyboardKey::DELETE;
	case KeyboardKey::RIGHT:			return GLFW::KeyboardKey::RIGHT;
	case KeyboardKey::LEFT:				return GLFW::KeyboardKey::LEFT;
	case KeyboardKey::DOWN:				return GLFW::KeyboardKey::DOWN;
	case KeyboardKey::UP:				return GLFW::KeyboardKey::UP;
	case KeyboardKey::PAGE_UP:			return GLFW::KeyboardKey::PAGE_UP;
	case KeyboardKey::PAGE_DOWN:		return GLFW::KeyboardKey::PAGE_DOWN;
	case KeyboardKey::HOME:				return GLFW::KeyboardKey::HOME;
	case KeyboardKey::END:				return GLFW::KeyboardKey::END;
	case KeyboardKey::CAPS_LOCK:		return GLFW::KeyboardKey::CAPS_LOCK;
	case KeyboardKey::SCROLL_LOCK:		return GLFW::KeyboardKey::SCROLL_LOCK;
	case KeyboardKey::NUM_LOCK:			return GLFW::KeyboardKey::NUM_LOCK;
	case KeyboardKey::PRINT_SCREEN:		return GLFW::KeyboardKey::PRINT_SCREEN;
	case KeyboardKey::PAUSE:			return GLFW::KeyboardKey::PAUSE;
	case KeyboardKey::F1:				return GLFW::KeyboardKey::F1;
	case KeyboardKey::F2:				return GLFW::KeyboardKey::F2;
	case KeyboardKey::F3:				return GLFW::KeyboardKey::F3;
	case KeyboardKey::F4:				return GLFW::KeyboardKey::F4;
	case KeyboardKey::F5:				return GLFW::KeyboardKey::F5;
	case KeyboardKey::F6:				return GLFW::KeyboardKey::F6;
	case KeyboardKey::F7:				return GLFW::KeyboardKey::F7;
	case KeyboardKey::F8:				return GLFW::KeyboardKey::F8;
	case KeyboardKey::F9:				return GLFW::KeyboardKey::F9;
	case KeyboardKey::F10:				return GLFW::KeyboardKey::F10;
	case KeyboardKey::F11:				return GLFW::KeyboardKey::F11;
	case KeyboardKey::F12:				return GLFW::KeyboardKey::F12;
	case KeyboardKey::F13:				return GLFW::KeyboardKey::F13;
	case KeyboardKey::F14:				return GLFW::KeyboardKey::F14;
	case KeyboardKey::F15:				return GLFW::KeyboardKey::F15;
	case KeyboardKey::F16:				return GLFW::KeyboardKey::F16;
	case KeyboardKey::F17:				return GLFW::KeyboardKey::F17;
	case KeyboardKey::F18:				return GLFW::KeyboardKey::F18;
	case KeyboardKey::F19:				return GLFW::KeyboardKey::F19;
	case KeyboardKey::F20:				return GLFW::KeyboardKey::F20;
	case KeyboardKey::F21:				return GLFW::KeyboardKey::F21;
	case KeyboardKey::F22:				return GLFW::KeyboardKey::F22;
	case KeyboardKey::F23:				return GLFW::KeyboardKey::F23;
	case KeyboardKey::F24:				return GLFW::KeyboardKey::F24;
	case KeyboardKey::F25:				return GLFW::KeyboardKey::F25;
	case KeyboardKey::KP_0:				return GLFW::KeyboardKey::KP_0;
	case KeyboardKey::KP_1:				return GLFW::KeyboardKey::KP_1;
	case KeyboardKey::KP_2:				return GLFW::KeyboardKey::KP_2;
	case KeyboardKey::KP_3:				return GLFW::KeyboardKey::KP_3;
	case KeyboardKey::KP_4:				return GLFW::KeyboardKey::KP_4;
	case KeyboardKey::KP_5:				return GLFW::KeyboardKey::KP_5;
	case KeyboardKey::KP_6:				return GLFW::KeyboardKey::KP_6;
	case KeyboardKey::KP_7:				return GLFW::KeyboardKey::KP_7;
	case KeyboardKey::KP_8:				return GLFW::KeyboardKey::KP_8;
	case KeyboardKey::KP_9:				return GLFW::KeyboardKey::KP_9;
	case KeyboardKey::KP_DECIMAL:		return GLFW::KeyboardKey::KP_DECIMAL;
	case KeyboardKey::KP_DIVIDE:		return GLFW::KeyboardKey::KP_DIVIDE;
	case KeyboardKey::KP_MULTIPLY:		return GLFW::KeyboardKey::KP_MULTIPLY;
	case KeyboardKey::KP_SUBTRACT:		return GLFW::KeyboardKey::KP_SUBTRACT;
	case KeyboardKey::KP_ADD:			return GLFW::KeyboardKey::KP_ADD;
	case KeyboardKey::KP_ENTER:			return GLFW::KeyboardKey::KP_ENTER;
	case KeyboardKey::KP_EQUAL:			return GLFW::KeyboardKey::KP_EQUAL;
	case KeyboardKey::LEFT_SHIFT:		return GLFW::KeyboardKey::LEFT_SHIFT;
	case KeyboardKey::LEFT_CONTROL:		return GLFW::KeyboardKey::LEFT_CONTROL;
	case KeyboardKey::LEFT_ALT:			return GLFW::KeyboardKey::LEFT_ALT;
	case KeyboardKey::LEFT_SUPER:		return GLFW::KeyboardKey::LEFT_SUPER;
	case KeyboardKey::RIGHT_SHIFT:		return GLFW::KeyboardKey::RIGHT_SHIFT;
	case KeyboardKey::RIGHT_CONTROL:	return GLFW::KeyboardKey::RIGHT_CONTROL;
	case KeyboardKey::RIGHT_ALT:		return GLFW::KeyboardKey::RIGHT_ALT;
	case KeyboardKey::RIGHT_SUPER:		return GLFW::KeyboardKey::RIGHT_SUPER;
	case KeyboardKey::MENU:				return GLFW::KeyboardKey::MENU;

	default:							return GLFW::KeyboardKey::NONE;
	}
}

constexpr KeyboardKey fromGLFW(GLFW::KeyboardKey key) {
	switch(key) {
	case GLFW::KeyboardKey::SPACE:			return KeyboardKey::SPACE;
	case GLFW::KeyboardKey::APOSTROPHE:		return KeyboardKey::APOSTROPHE;
	case GLFW::KeyboardKey::COMMA:			return KeyboardKey::COMMA;
	case GLFW::KeyboardKey::MINUS:			return KeyboardKey::MINUS;
	case GLFW::KeyboardKey::PERIOD:			return KeyboardKey::PERIOD;
	case GLFW::KeyboardKey::SLASH:			return KeyboardKey::SLASH;
	case GLFW::KeyboardKey::NB0:			return KeyboardKey::NB0;
	case GLFW::KeyboardKey::NB1:			return KeyboardKey::NB1;
	case GLFW::KeyboardKey::NB2:			return KeyboardKey::NB2;
	case GLFW::KeyboardKey::NB3:			return KeyboardKey::NB3;
	case GLFW::KeyboardKey::NB4:			return KeyboardKey::NB4;
	case GLFW::KeyboardKey::NB5:			return KeyboardKey::NB5;
	case GLFW::KeyboardKey::NB6:			return KeyboardKey::NB6;
	case GLFW::KeyboardKey::NB7:			return KeyboardKey::NB7;
	case GLFW::KeyboardKey::NB8:			return KeyboardKey::NB8;
	case GLFW::KeyboardKey::NB9:			return KeyboardKey::NB9;
	case GLFW::KeyboardKey::SEMICOLON:		return KeyboardKey::SEMICOLON;
	case GLFW::KeyboardKey::EQUAL:			return KeyboardKey::EQUAL;
	case GLFW::KeyboardKey::A:				return KeyboardKey::A;
	case GLFW::KeyboardKey::B:				return KeyboardKey::B;
	case GLFW::KeyboardKey::C:				return KeyboardKey::C;
	case GLFW::KeyboardKey::D:				return KeyboardKey::D;
	case GLFW::KeyboardKey::E:				return KeyboardKey::E;
	case GLFW::KeyboardKey::F:				return KeyboardKey::F;
	case GLFW::KeyboardKey::G:				return KeyboardKey::G;
	case GLFW::KeyboardKey::H:				return KeyboardKey::H;
	case GLFW::KeyboardKey::I:				return KeyboardKey::I;
	case GLFW::KeyboardKey::J:				return KeyboardKey::J;
	case GLFW::KeyboardKey::K:				return KeyboardKey::K;
	case GLFW::KeyboardKey::L:				return KeyboardKey::L;
	case GLFW::KeyboardKey::M:				return KeyboardKey::M;
	case GLFW::KeyboardKey::N:				return KeyboardKey::N;
	case GLFW::KeyboardKey::O:				return KeyboardKey::O;
	case GLFW::KeyboardKey::P:				return KeyboardKey::P;
	case GLFW::KeyboardKey::Q:				return KeyboardKey::Q;
	case GLFW::KeyboardKey::R:				return KeyboardKey::R;
	case GLFW::KeyboardKey::S:				return KeyboardKey::S;
	case GLFW::KeyboardKey::T:				return KeyboardKey::T;
	case GLFW::KeyboardKey::U:				return KeyboardKey::U;
	case GLFW::KeyboardKey::V:				return KeyboardKey::V;
	case GLFW::KeyboardKey::W:				return KeyboardKey::W;
	case GLFW::KeyboardKey::X:				return KeyboardKey::X;
	case GLFW::KeyboardKey::Y:				return KeyboardKey::Y;
	case GLFW::KeyboardKey::Z:				return KeyboardKey::Z;
	case GLFW::KeyboardKey::LEFT_BRACKET:	return KeyboardKey::LEFT_BRACKET;
	case GLFW::KeyboardKey::BACKSLASH:		return KeyboardKey::BACKSLASH;
	case GLFW::KeyboardKey::RIGHT_BRACKET:	return KeyboardKey::RIGHT_BRACKET;
	case GLFW::KeyboardKey::GRAVE_ACCENT:	return KeyboardKey::GRAVE_ACCENT;
	//case GLFW::KeyboardKey::WORLD_1:		return KeyboardKey::WORLD_1;
	//case GLFW::KeyboardKey::WORLD_2:		return KeyboardKey::WORLD_2;
	case GLFW::KeyboardKey::ESCAPE:			return KeyboardKey::ESCAPE;
	case GLFW::KeyboardKey::ENTER:			return KeyboardKey::ENTER;
	case GLFW::KeyboardKey::TAB:			return KeyboardKey::TAB;
	case GLFW::KeyboardKey::BACKSPACE:		return KeyboardKey::BACKSPACE;
	case GLFW::KeyboardKey::INSERT:			return KeyboardKey::INSERT;
	case GLFW::KeyboardKey::DELETE:			return KeyboardKey::DELETE;
	case GLFW::KeyboardKey::RIGHT:			return KeyboardKey::RIGHT;
	case GLFW::KeyboardKey::LEFT:			return KeyboardKey::LEFT;
	case GLFW::KeyboardKey::DOWN:			return KeyboardKey::DOWN;
	case GLFW::KeyboardKey::UP:				return KeyboardKey::UP;
	case GLFW::KeyboardKey::PAGE_UP:		return KeyboardKey::PAGE_UP;
	case GLFW::KeyboardKey::PAGE_DOWN:		return KeyboardKey::PAGE_DOWN;
	case GLFW::KeyboardKey::HOME:			return KeyboardKey::HOME;
	case GLFW::KeyboardKey::END:			return KeyboardKey::END;
	case GLFW::KeyboardKey::CAPS_LOCK:		return KeyboardKey::CAPS_LOCK;
	case GLFW::KeyboardKey::SCROLL_LOCK:	return KeyboardKey::SCROLL_LOCK;
	case GLFW::KeyboardKey::NUM_LOCK:		return KeyboardKey::NUM_LOCK;
	case GLFW::KeyboardKey::PRINT_SCREEN:	return KeyboardKey::PRINT_SCREEN;
	case GLFW::KeyboardKey::PAUSE:			return KeyboardKey::PAUSE;
	case GLFW::KeyboardKey::F1:				return KeyboardKey::F1;
	case GLFW::KeyboardKey::F2:				return KeyboardKey::F2;
	case GLFW::KeyboardKey::F3:				return KeyboardKey::F3;
	case GLFW::KeyboardKey::F4:				return KeyboardKey::F4;
	case GLFW::KeyboardKey::F5:				return KeyboardKey::F5;
	case GLFW::KeyboardKey::F6:				return KeyboardKey::F6;
	case GLFW::KeyboardKey::F7:				return KeyboardKey::F7;
	case GLFW::KeyboardKey::F8:				return KeyboardKey::F8;
	case GLFW::KeyboardKey::F9:				return KeyboardKey::F9;
	case GLFW::KeyboardKey::F10:			return KeyboardKey::F10;
	case GLFW::KeyboardKey::F11:			return KeyboardKey::F11;
	case GLFW::KeyboardKey::F12:			return KeyboardKey::F12;
	case GLFW::KeyboardKey::F13:			return KeyboardKey::F13;
	case GLFW::KeyboardKey::F14:			return KeyboardKey::F14;
	case GLFW::KeyboardKey::F15:			return KeyboardKey::F15;
	case GLFW::KeyboardKey::F16:			return KeyboardKey::F16;
	case GLFW::KeyboardKey::F17:			return KeyboardKey::F17;
	case GLFW::KeyboardKey::F18:			return KeyboardKey::F18;
	case GLFW::KeyboardKey::F19:			return KeyboardKey::F19;
	case GLFW::KeyboardKey::F20:			return KeyboardKey::F20;
	case GLFW::KeyboardKey::F21:			return KeyboardKey::F21;
	case GLFW::KeyboardKey::F22:			return KeyboardKey::F22;
	case GLFW::KeyboardKey::F23:			return KeyboardKey::F23;
	case GLFW::KeyboardKey::F24:			return KeyboardKey::F24;
	case GLFW::KeyboardKey::F25:			return KeyboardKey::F25;
	case GLFW::KeyboardKey::KP_0:			return KeyboardKey::KP_0;
	case GLFW::KeyboardKey::KP_1:			return KeyboardKey::KP_1;
	case GLFW::KeyboardKey::KP_2:			return KeyboardKey::KP_2;
	case GLFW::KeyboardKey::KP_3:			return KeyboardKey::KP_3;
	case GLFW::KeyboardKey::KP_4:			return KeyboardKey::KP_4;
	case GLFW::KeyboardKey::KP_5:			return KeyboardKey::KP_5;
	case GLFW::KeyboardKey::KP_6:			return KeyboardKey::KP_6;
	case GLFW::KeyboardKey::KP_7:			return KeyboardKey::KP_7;
	case GLFW::KeyboardKey::KP_8:			return KeyboardKey::KP_8;
	case GLFW::KeyboardKey::KP_9:			return KeyboardKey::KP_9;
	case GLFW::KeyboardKey::KP_DECIMAL:		return KeyboardKey::KP_DECIMAL;
	case GLFW::KeyboardKey::KP_DIVIDE:		return KeyboardKey::KP_DIVIDE;
	case GLFW::KeyboardKey::KP_MULTIPLY:	return KeyboardKey::KP_MULTIPLY;
	case GLFW::KeyboardKey::KP_SUBTRACT:	return KeyboardKey::KP_SUBTRACT;
	case GLFW::KeyboardKey::KP_ADD:			return KeyboardKey::KP_ADD;
	case GLFW::KeyboardKey::KP_ENTER:		return KeyboardKey::KP_ENTER;
	case GLFW::KeyboardKey::KP_EQUAL:		return KeyboardKey::KP_EQUAL;
	case GLFW::KeyboardKey::LEFT_SHIFT:		return KeyboardKey::LEFT_SHIFT;
	case GLFW::KeyboardKey::LEFT_CONTROL:	return KeyboardKey::LEFT_CONTROL;
	case GLFW::KeyboardKey::LEFT_ALT:		return KeyboardKey::LEFT_ALT;
	case GLFW::KeyboardKey::LEFT_SUPER:		return KeyboardKey::LEFT_SUPER;
	case GLFW::KeyboardKey::RIGHT_SHIFT:	return KeyboardKey::RIGHT_SHIFT;
	case GLFW::KeyboardKey::RIGHT_CONTROL:	return KeyboardKey::RIGHT_CONTROL;
	case GLFW::KeyboardKey::RIGHT_ALT:		return KeyboardKey::RIGHT_ALT;
	case GLFW::KeyboardKey::RIGHT_SUPER:	return KeyboardKey::RIGHT_SUPER;
	case GLFW::KeyboardKey::MENU:			return KeyboardKey::MENU;

	default:										return KeyboardKey::NONE;
	}
}



static_assert(static_cast<int>(GLFW::MouseButton::NB1) == static_cast<int>(MouseKey::NB1), "Mouse button NB1 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB2) == static_cast<int>(MouseKey::NB2), "Mouse button NB2 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB3) == static_cast<int>(MouseKey::NB3), "Mouse button NB3 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB4) == static_cast<int>(MouseKey::NB4), "Mouse button NB4 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB5) == static_cast<int>(MouseKey::NB5), "Mouse button NB5 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB6) == static_cast<int>(MouseKey::NB6), "Mouse button NB6 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB7) == static_cast<int>(MouseKey::NB7), "Mouse button NB7 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::NB8) == static_cast<int>(MouseKey::NB8), "Mouse button NB8 does not match");

constexpr GLFW::MouseButton toGLFW(MouseKey key) {
	return static_cast<GLFW::MouseButton>(key);
}

constexpr MouseKey fromGLFW(GLFW::MouseButton key) {
	return static_cast<MouseKey>(key);
}



static_assert(static_cast<int>(GLFW::KeyModifiers::NONE) == static_cast<int>(KeyModifiers::NONE), "NONE modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::SHIFT) == static_cast<int>(KeyModifiers::SHIFT), "SHIFT modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::CONTROL) == static_cast<int>(KeyModifiers::CONTROL), "ALT modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::ALT) == static_cast<int>(KeyModifiers::ALT), "ALT modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::SUPER) == static_cast<int>(KeyModifiers::SUPER), "SUPER modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::CONTROL) == static_cast<int>(KeyModifiers::CONTROL), "SHIFT modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::CAPS_LOCK) == static_cast<int>(KeyModifiers::CAPS_LOCK), "CAPS_LOCK modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::NUM_LOCK) == static_cast<int>(KeyModifiers::NUM_LOCK), "NUM_LOCK modifier flag does not match");

constexpr GLFW::KeyModifiers toGLFW(KeyModifiers modifiers) {
	return static_cast<GLFW::KeyModifiers>(modifiers);
}

constexpr KeyModifiers fromGLFW(GLFW::KeyModifiers modifiers) {
	return static_cast<KeyModifiers>(modifiers);
}



static_assert(static_cast<int>(GLFW::KeyEvent::RELEASE) == static_cast<int>(KeyEvent::RELEASE), "RELEASE event does not match");
static_assert(static_cast<int>(GLFW::KeyEvent::PRESS) == static_cast<int>(KeyEvent::PRESS), "PRESS event does not match");
static_assert(static_cast<int>(GLFW::KeyEvent::REPEAT) == static_cast<int>(KeyEvent::REPEAT), "RELEASE event does not match");

constexpr GLFW::KeyEvent toGLFW(KeyEvent event) {
	return static_cast<GLFW::KeyEvent>(event);
}

constexpr KeyEvent fromGLFW(GLFW::KeyEvent event) {
	return static_cast<KeyEvent>(event);
}

}