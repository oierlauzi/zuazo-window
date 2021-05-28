#include "GLFWConversions.h"

namespace Zuazo {

constexpr GLFW::KeyboardKey toGLFW(KeyboardKey key) {
	switch(key) {
	case KeyboardKey::space:			return GLFW::KeyboardKey::space;
	case KeyboardKey::apostrophe:		return GLFW::KeyboardKey::apostrophe;
	case KeyboardKey::comma:			return GLFW::KeyboardKey::comma;
	case KeyboardKey::minus:			return GLFW::KeyboardKey::minus;
	case KeyboardKey::period:			return GLFW::KeyboardKey::period;
	case KeyboardKey::slash:			return GLFW::KeyboardKey::slash;
	case KeyboardKey::nb0:				return GLFW::KeyboardKey::nb0;
	case KeyboardKey::nb1:				return GLFW::KeyboardKey::nb1;
	case KeyboardKey::nb2:				return GLFW::KeyboardKey::nb2;
	case KeyboardKey::nb3:				return GLFW::KeyboardKey::nb3;
	case KeyboardKey::nb4:				return GLFW::KeyboardKey::nb4;
	case KeyboardKey::nb5:				return GLFW::KeyboardKey::nb5;
	case KeyboardKey::nb6:				return GLFW::KeyboardKey::nb6;
	case KeyboardKey::nb7:				return GLFW::KeyboardKey::nb7;
	case KeyboardKey::nb8:				return GLFW::KeyboardKey::nb8;
	case KeyboardKey::nb9:				return GLFW::KeyboardKey::nb9;
	case KeyboardKey::semicolon:		return GLFW::KeyboardKey::semicolon;
	case KeyboardKey::equal:			return GLFW::KeyboardKey::equal;
	case KeyboardKey::a:				return GLFW::KeyboardKey::a;
	case KeyboardKey::b:				return GLFW::KeyboardKey::b;
	case KeyboardKey::c:				return GLFW::KeyboardKey::c;
	case KeyboardKey::d:				return GLFW::KeyboardKey::d;
	case KeyboardKey::e:				return GLFW::KeyboardKey::e;
	case KeyboardKey::f:				return GLFW::KeyboardKey::f;
	case KeyboardKey::g:				return GLFW::KeyboardKey::g;
	case KeyboardKey::h:				return GLFW::KeyboardKey::h;
	case KeyboardKey::i:				return GLFW::KeyboardKey::i;
	case KeyboardKey::j:				return GLFW::KeyboardKey::j;
	case KeyboardKey::k:				return GLFW::KeyboardKey::k;
	case KeyboardKey::l:				return GLFW::KeyboardKey::l;
	case KeyboardKey::m:				return GLFW::KeyboardKey::m;
	case KeyboardKey::n:				return GLFW::KeyboardKey::n;
	case KeyboardKey::o:				return GLFW::KeyboardKey::o;
	case KeyboardKey::p:				return GLFW::KeyboardKey::p;
	case KeyboardKey::q:				return GLFW::KeyboardKey::q;
	case KeyboardKey::r:				return GLFW::KeyboardKey::r;
	case KeyboardKey::s:				return GLFW::KeyboardKey::s;
	case KeyboardKey::t:				return GLFW::KeyboardKey::t;
	case KeyboardKey::u:				return GLFW::KeyboardKey::u;
	case KeyboardKey::v:				return GLFW::KeyboardKey::v;
	case KeyboardKey::w:				return GLFW::KeyboardKey::w;
	case KeyboardKey::x:				return GLFW::KeyboardKey::x;
	case KeyboardKey::y:				return GLFW::KeyboardKey::y;
	case KeyboardKey::z:				return GLFW::KeyboardKey::z;
	case KeyboardKey::leftBracket:		return GLFW::KeyboardKey::leftBracket;
	case KeyboardKey::backSlash:		return GLFW::KeyboardKey::backSlash;
	case KeyboardKey::rightBracket:		return GLFW::KeyboardKey::rightBracket;
	case KeyboardKey::graveAccent:		return GLFW::KeyboardKey::graveAccent;
	//::KeyboardKey::WORLD_1:			GLFW::	return KeyboardKey::WORLD_1;
	//::KeyboardKey::WORLD_2:			GLFW::	return KeyboardKey::WORLD_2;
	case KeyboardKey::escape:			return GLFW::KeyboardKey::escape;
	case KeyboardKey::enter:			return GLFW::KeyboardKey::enter;
	case KeyboardKey::tab:				return GLFW::KeyboardKey::tab;
	case KeyboardKey::backspace:		return GLFW::KeyboardKey::backspace;
	case KeyboardKey::insert:			return GLFW::KeyboardKey::insert;
	case KeyboardKey::del:				return GLFW::KeyboardKey::del;
	case KeyboardKey::right:			return GLFW::KeyboardKey::right;
	case KeyboardKey::left:				return GLFW::KeyboardKey::left;
	case KeyboardKey::down:				return GLFW::KeyboardKey::down;
	case KeyboardKey::up:				return GLFW::KeyboardKey::up;
	case KeyboardKey::pageUp:			return GLFW::KeyboardKey::pageUp;
	case KeyboardKey::pageDown:			return GLFW::KeyboardKey::pageDown;
	case KeyboardKey::home:				return GLFW::KeyboardKey::home;
	case KeyboardKey::end:				return GLFW::KeyboardKey::end;
	case KeyboardKey::capsLock:			return GLFW::KeyboardKey::capsLock;
	case KeyboardKey::scrollLock:		return GLFW::KeyboardKey::scrollLock;
	case KeyboardKey::numLock:			return GLFW::KeyboardKey::numLock;
	case KeyboardKey::printScreen:		return GLFW::KeyboardKey::printScreen;
	case KeyboardKey::pause:			return GLFW::KeyboardKey::pause;
	case KeyboardKey::f1:				return GLFW::KeyboardKey::f1;
	case KeyboardKey::f2:				return GLFW::KeyboardKey::f2;
	case KeyboardKey::f3:				return GLFW::KeyboardKey::f3;
	case KeyboardKey::f4:				return GLFW::KeyboardKey::f4;
	case KeyboardKey::f5:				return GLFW::KeyboardKey::f5;
	case KeyboardKey::f6:				return GLFW::KeyboardKey::f6;
	case KeyboardKey::f7:				return GLFW::KeyboardKey::f7;
	case KeyboardKey::f8:				return GLFW::KeyboardKey::f8;
	case KeyboardKey::f9:				return GLFW::KeyboardKey::f9;
	case KeyboardKey::f10:				return GLFW::KeyboardKey::f10;
	case KeyboardKey::f11:				return GLFW::KeyboardKey::f11;
	case KeyboardKey::f12:				return GLFW::KeyboardKey::f12;
	case KeyboardKey::f13:				return GLFW::KeyboardKey::f13;
	case KeyboardKey::f14:				return GLFW::KeyboardKey::f14;
	case KeyboardKey::f15:				return GLFW::KeyboardKey::f15;
	case KeyboardKey::f16:				return GLFW::KeyboardKey::f16;
	case KeyboardKey::f17:				return GLFW::KeyboardKey::f17;
	case KeyboardKey::f18:				return GLFW::KeyboardKey::f18;
	case KeyboardKey::f19:				return GLFW::KeyboardKey::f19;
	case KeyboardKey::f20:				return GLFW::KeyboardKey::f20;
	case KeyboardKey::f21:				return GLFW::KeyboardKey::f21;
	case KeyboardKey::f22:				return GLFW::KeyboardKey::f22;
	case KeyboardKey::f23:				return GLFW::KeyboardKey::f23;
	case KeyboardKey::f24:				return GLFW::KeyboardKey::f24;
	case KeyboardKey::f25:				return GLFW::KeyboardKey::f25;
	case KeyboardKey::kp0:				return GLFW::KeyboardKey::kp0;
	case KeyboardKey::kp1:				return GLFW::KeyboardKey::kp1;
	case KeyboardKey::kp2:				return GLFW::KeyboardKey::kp2;
	case KeyboardKey::kp3:				return GLFW::KeyboardKey::kp3;
	case KeyboardKey::kp4:				return GLFW::KeyboardKey::kp4;
	case KeyboardKey::kp5:				return GLFW::KeyboardKey::kp5;
	case KeyboardKey::kp6:				return GLFW::KeyboardKey::kp6;
	case KeyboardKey::kp7:				return GLFW::KeyboardKey::kp7;
	case KeyboardKey::kp8:				return GLFW::KeyboardKey::kp8;
	case KeyboardKey::kp9:				return GLFW::KeyboardKey::kp9;
	case KeyboardKey::kpDecimal:		return GLFW::KeyboardKey::kpDecimal;
	case KeyboardKey::kpDivide:			return GLFW::KeyboardKey::kpDivide;
	case KeyboardKey::kpMultiply:		return GLFW::KeyboardKey::kpMultiply;
	case KeyboardKey::kpSubstract:		return GLFW::KeyboardKey::kpSubstract;
	case KeyboardKey::kpAdd:			return GLFW::KeyboardKey::kpAdd;
	case KeyboardKey::kpEnter:			return GLFW::KeyboardKey::kpEnter;
	case KeyboardKey::kpEqual:			return GLFW::KeyboardKey::kpEqual;
	case KeyboardKey::leftShift:		return GLFW::KeyboardKey::leftShift;
	case KeyboardKey::leftControl:		return GLFW::KeyboardKey::leftControl;
	case KeyboardKey::leftAlt:			return GLFW::KeyboardKey::leftAlt;
	case KeyboardKey::leftSuper:		return GLFW::KeyboardKey::leftSuper;
	case KeyboardKey::rightShift:		return GLFW::KeyboardKey::rightShift;
	case KeyboardKey::rightControl:		return GLFW::KeyboardKey::rightControl;
	case KeyboardKey::rightAlt:			return GLFW::KeyboardKey::rightAlt;
	case KeyboardKey::rightSuper:		return GLFW::KeyboardKey::rightSuper;
	case KeyboardKey::menu:				return GLFW::KeyboardKey::menu;

	default:							return GLFW::KeyboardKey::none;
	}
}

constexpr KeyboardKey fromGLFW(GLFW::KeyboardKey key) {
	switch(key) {
	case GLFW::KeyboardKey::space:			return KeyboardKey::space;
	case GLFW::KeyboardKey::apostrophe:		return KeyboardKey::apostrophe;
	case GLFW::KeyboardKey::comma:			return KeyboardKey::comma;
	case GLFW::KeyboardKey::minus:			return KeyboardKey::minus;
	case GLFW::KeyboardKey::period:			return KeyboardKey::period;
	case GLFW::KeyboardKey::slash:			return KeyboardKey::slash;
	case GLFW::KeyboardKey::nb0:			return KeyboardKey::nb0;
	case GLFW::KeyboardKey::nb1:			return KeyboardKey::nb1;
	case GLFW::KeyboardKey::nb2:			return KeyboardKey::nb2;
	case GLFW::KeyboardKey::nb3:			return KeyboardKey::nb3;
	case GLFW::KeyboardKey::nb4:			return KeyboardKey::nb4;
	case GLFW::KeyboardKey::nb5:			return KeyboardKey::nb5;
	case GLFW::KeyboardKey::nb6:			return KeyboardKey::nb6;
	case GLFW::KeyboardKey::nb7:			return KeyboardKey::nb7;
	case GLFW::KeyboardKey::nb8:			return KeyboardKey::nb8;
	case GLFW::KeyboardKey::nb9:			return KeyboardKey::nb9;
	case GLFW::KeyboardKey::semicolon:		return KeyboardKey::semicolon;
	case GLFW::KeyboardKey::equal:			return KeyboardKey::equal;
	case GLFW::KeyboardKey::a:				return KeyboardKey::a;
	case GLFW::KeyboardKey::b:				return KeyboardKey::b;
	case GLFW::KeyboardKey::c:				return KeyboardKey::c;
	case GLFW::KeyboardKey::d:				return KeyboardKey::d;
	case GLFW::KeyboardKey::e:				return KeyboardKey::e;
	case GLFW::KeyboardKey::f:				return KeyboardKey::f;
	case GLFW::KeyboardKey::g:				return KeyboardKey::g;
	case GLFW::KeyboardKey::h:				return KeyboardKey::h;
	case GLFW::KeyboardKey::i:				return KeyboardKey::i;
	case GLFW::KeyboardKey::j:				return KeyboardKey::j;
	case GLFW::KeyboardKey::k:				return KeyboardKey::k;
	case GLFW::KeyboardKey::l:				return KeyboardKey::l;
	case GLFW::KeyboardKey::m:				return KeyboardKey::m;
	case GLFW::KeyboardKey::n:				return KeyboardKey::n;
	case GLFW::KeyboardKey::o:				return KeyboardKey::o;
	case GLFW::KeyboardKey::p:				return KeyboardKey::p;
	case GLFW::KeyboardKey::q:				return KeyboardKey::q;
	case GLFW::KeyboardKey::r:				return KeyboardKey::r;
	case GLFW::KeyboardKey::s:				return KeyboardKey::s;
	case GLFW::KeyboardKey::t:				return KeyboardKey::t;
	case GLFW::KeyboardKey::u:				return KeyboardKey::u;
	case GLFW::KeyboardKey::v:				return KeyboardKey::v;
	case GLFW::KeyboardKey::w:				return KeyboardKey::w;
	case GLFW::KeyboardKey::x:				return KeyboardKey::x;
	case GLFW::KeyboardKey::y:				return KeyboardKey::y;
	case GLFW::KeyboardKey::z:				return KeyboardKey::z;
	case GLFW::KeyboardKey::leftBracket:	return KeyboardKey::leftBracket;
	case GLFW::KeyboardKey::backSlash:		return KeyboardKey::backSlash;
	case GLFW::KeyboardKey::rightBracket:	return KeyboardKey::rightBracket;
	case GLFW::KeyboardKey::graveAccent:	return KeyboardKey::graveAccent;
	//case GLFW::KeyboardKey::WORLD_1:		return KeyboardKey::WORLD_1;
	//case GLFW::KeyboardKey::WORLD_2:		return KeyboardKey::WORLD_2;
	case GLFW::KeyboardKey::escape:			return KeyboardKey::escape;
	case GLFW::KeyboardKey::enter:			return KeyboardKey::enter;
	case GLFW::KeyboardKey::tab:			return KeyboardKey::tab;
	case GLFW::KeyboardKey::backspace:		return KeyboardKey::backspace;
	case GLFW::KeyboardKey::insert:			return KeyboardKey::insert;
	case GLFW::KeyboardKey::del:			return KeyboardKey::del;
	case GLFW::KeyboardKey::right:			return KeyboardKey::right;
	case GLFW::KeyboardKey::left:			return KeyboardKey::left;
	case GLFW::KeyboardKey::down:			return KeyboardKey::down;
	case GLFW::KeyboardKey::up:				return KeyboardKey::up;
	case GLFW::KeyboardKey::pageUp:			return KeyboardKey::pageUp;
	case GLFW::KeyboardKey::pageDown:		return KeyboardKey::pageDown;
	case GLFW::KeyboardKey::home:			return KeyboardKey::home;
	case GLFW::KeyboardKey::end:			return KeyboardKey::end;
	case GLFW::KeyboardKey::capsLock:		return KeyboardKey::capsLock;
	case GLFW::KeyboardKey::scrollLock:		return KeyboardKey::scrollLock;
	case GLFW::KeyboardKey::numLock:		return KeyboardKey::numLock;
	case GLFW::KeyboardKey::printScreen:	return KeyboardKey::printScreen;
	case GLFW::KeyboardKey::pause:			return KeyboardKey::pause;
	case GLFW::KeyboardKey::f1:				return KeyboardKey::f1;
	case GLFW::KeyboardKey::f2:				return KeyboardKey::f2;
	case GLFW::KeyboardKey::f3:				return KeyboardKey::f3;
	case GLFW::KeyboardKey::f4:				return KeyboardKey::f4;
	case GLFW::KeyboardKey::f5:				return KeyboardKey::f5;
	case GLFW::KeyboardKey::f6:				return KeyboardKey::f6;
	case GLFW::KeyboardKey::f7:				return KeyboardKey::f7;
	case GLFW::KeyboardKey::f8:				return KeyboardKey::f8;
	case GLFW::KeyboardKey::f9:				return KeyboardKey::f9;
	case GLFW::KeyboardKey::f10:			return KeyboardKey::f10;
	case GLFW::KeyboardKey::f11:			return KeyboardKey::f11;
	case GLFW::KeyboardKey::f12:			return KeyboardKey::f12;
	case GLFW::KeyboardKey::f13:			return KeyboardKey::f13;
	case GLFW::KeyboardKey::f14:			return KeyboardKey::f14;
	case GLFW::KeyboardKey::f15:			return KeyboardKey::f15;
	case GLFW::KeyboardKey::f16:			return KeyboardKey::f16;
	case GLFW::KeyboardKey::f17:			return KeyboardKey::f17;
	case GLFW::KeyboardKey::f18:			return KeyboardKey::f18;
	case GLFW::KeyboardKey::f19:			return KeyboardKey::f19;
	case GLFW::KeyboardKey::f20:			return KeyboardKey::f20;
	case GLFW::KeyboardKey::f21:			return KeyboardKey::f21;
	case GLFW::KeyboardKey::f22:			return KeyboardKey::f22;
	case GLFW::KeyboardKey::f23:			return KeyboardKey::f23;
	case GLFW::KeyboardKey::f24:			return KeyboardKey::f24;
	case GLFW::KeyboardKey::f25:			return KeyboardKey::f25;
	case GLFW::KeyboardKey::kp0:			return KeyboardKey::kp0;
	case GLFW::KeyboardKey::kp1:			return KeyboardKey::kp1;
	case GLFW::KeyboardKey::kp2:			return KeyboardKey::kp2;
	case GLFW::KeyboardKey::kp3:			return KeyboardKey::kp3;
	case GLFW::KeyboardKey::kp4:			return KeyboardKey::kp4;
	case GLFW::KeyboardKey::kp5:			return KeyboardKey::kp5;
	case GLFW::KeyboardKey::kp6:			return KeyboardKey::kp6;
	case GLFW::KeyboardKey::kp7:			return KeyboardKey::kp7;
	case GLFW::KeyboardKey::kp8:			return KeyboardKey::kp8;
	case GLFW::KeyboardKey::kp9:			return KeyboardKey::kp9;
	case GLFW::KeyboardKey::kpDecimal:		return KeyboardKey::kpDecimal;
	case GLFW::KeyboardKey::kpDivide:		return KeyboardKey::kpDivide;
	case GLFW::KeyboardKey::kpMultiply:		return KeyboardKey::kpMultiply;
	case GLFW::KeyboardKey::kpSubstract:	return KeyboardKey::kpSubstract;
	case GLFW::KeyboardKey::kpAdd:			return KeyboardKey::kpAdd;
	case GLFW::KeyboardKey::kpEnter:		return KeyboardKey::kpEnter;
	case GLFW::KeyboardKey::kpEqual:		return KeyboardKey::kpEqual;
	case GLFW::KeyboardKey::leftShift:		return KeyboardKey::leftShift;
	case GLFW::KeyboardKey::leftControl:	return KeyboardKey::leftControl;
	case GLFW::KeyboardKey::leftAlt:		return KeyboardKey::leftAlt;
	case GLFW::KeyboardKey::leftSuper:		return KeyboardKey::leftSuper;
	case GLFW::KeyboardKey::rightShift:		return KeyboardKey::rightShift;
	case GLFW::KeyboardKey::rightControl:	return KeyboardKey::rightControl;
	case GLFW::KeyboardKey::rightAlt:		return KeyboardKey::rightAlt;
	case GLFW::KeyboardKey::rightSuper:		return KeyboardKey::rightSuper;
	case GLFW::KeyboardKey::menu:			return KeyboardKey::menu;

	default:										return KeyboardKey::NONE;
	}
}



static_assert(static_cast<int>(GLFW::MouseButton::nb1) == static_cast<int>(MouseKey::nb1), "Mouse button NB1 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb2) == static_cast<int>(MouseKey::nb2), "Mouse button NB2 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb3) == static_cast<int>(MouseKey::nb3), "Mouse button NB3 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb4) == static_cast<int>(MouseKey::nb4), "Mouse button NB4 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb5) == static_cast<int>(MouseKey::nb5), "Mouse button NB5 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb6) == static_cast<int>(MouseKey::nb6), "Mouse button NB6 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb7) == static_cast<int>(MouseKey::nb7), "Mouse button NB7 does not match");
static_assert(static_cast<int>(GLFW::MouseButton::nb8) == static_cast<int>(MouseKey::nb8), "Mouse button NB8 does not match");

constexpr GLFW::MouseButton toGLFW(MouseKey key) {
	return static_cast<GLFW::MouseButton>(key);
}

constexpr MouseKey fromGLFW(GLFW::MouseButton key) {
	return static_cast<MouseKey>(key);
}



static_assert(static_cast<int>(GLFW::KeyModifiers::none) == static_cast<int>(KeyModifiers::none), "NONE modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::shift) == static_cast<int>(KeyModifiers::shift), "SHIFT modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::control) == static_cast<int>(KeyModifiers::control), "CONTROL modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::alt) == static_cast<int>(KeyModifiers::alt), "ALT modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::super) == static_cast<int>(KeyModifiers::super), "SUPER modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::capsLock) == static_cast<int>(KeyModifiers::capsLock), "CAPS_LOCK modifier flag does not match");
static_assert(static_cast<int>(GLFW::KeyModifiers::numLock) == static_cast<int>(KeyModifiers::numLock), "NUM_LOCK modifier flag does not match");

constexpr GLFW::KeyModifiers toGLFW(KeyModifiers modifiers) {
	return static_cast<GLFW::KeyModifiers>(modifiers);
}

constexpr KeyModifiers fromGLFW(GLFW::KeyModifiers modifiers) {
	return static_cast<KeyModifiers>(modifiers);
}



static_assert(static_cast<int>(GLFW::KeyEvent::release) == static_cast<int>(KeyEvent::release), "RELEASE event does not match");
static_assert(static_cast<int>(GLFW::KeyEvent::press) == static_cast<int>(KeyEvent::press), "PRESS event does not match");
static_assert(static_cast<int>(GLFW::KeyEvent::repeat) == static_cast<int>(KeyEvent::repeat), "RELEASE event does not match");

constexpr GLFW::KeyEvent toGLFW(KeyEvent event) {
	return static_cast<GLFW::KeyEvent>(event);
}

constexpr KeyEvent fromGLFW(GLFW::KeyEvent event) {
	return static_cast<KeyEvent>(event);
}

}