#include <zuazo/Outputs/Window.h>

#include "../GLFW.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Graphics/VulkanConversions.h>
#include <zuazo/Graphics/ColorTransfer.h>
#include <zuazo/Graphics/StagedBuffer.h>
#include <zuazo/Utils/StaticId.h>
#include <zuazo/Utils/Functions.h>
#include <zuazo/Math/Functions.h>

#include <cmath>
#include <algorithm>
#include <limits>
#include <set>
#include <bitset>
#include <mutex>
#include <sstream>


namespace Zuazo::Outputs {

/*
 * Enumeration checks
 */

static_assert(static_cast<int>(GLFW::Window::State::NORMAL) == static_cast<int>(Window::State::NORMAL), "NORMAL state value does not match");
static_assert(static_cast<int>(GLFW::Window::State::HIDDEN) == static_cast<int>(Window::State::HIDDEN), "HIDDEN state value does not match");
static_assert(static_cast<int>(GLFW::Window::State::FULLSCREEN) == static_cast<int>(Window::State::FULLSCREEN), "FULLSCREEN state value does not match");
static_assert(static_cast<int>(GLFW::Window::State::ICONIFIED) == static_cast<int>(Window::State::ICONIFIED), "ICONIFIED state value does not match");
static_assert(static_cast<int>(GLFW::Window::State::MAXIMIZED) == static_cast<int>(Window::State::MAXIMIZED), "MAXIMIZED state value does not match");

static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NONE) == static_cast<int>(Window::KeyboardKey::NONE), "NONE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SPACE) == static_cast<int>(Window::KeyboardKey::SPACE), "SPACE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::APOSTROPHE) == static_cast<int>(Window::KeyboardKey::APOSTROPHE), "APOSTROPHE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::COMMA) == static_cast<int>(Window::KeyboardKey::COMMA), "COMMA keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::MINUS) == static_cast<int>(Window::KeyboardKey::MINUS), "MINUS keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PERIOD) == static_cast<int>(Window::KeyboardKey::PERIOD), "PERIOD keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SLASH) == static_cast<int>(Window::KeyboardKey::SLASH), "SLASH keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB0) == static_cast<int>(Window::KeyboardKey::NB0), "NB0 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB1) == static_cast<int>(Window::KeyboardKey::NB1), "NB1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB2) == static_cast<int>(Window::KeyboardKey::NB2), "NB2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB3) == static_cast<int>(Window::KeyboardKey::NB3), "NB3 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB4) == static_cast<int>(Window::KeyboardKey::NB4), "NB4 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB5) == static_cast<int>(Window::KeyboardKey::NB5), "NB5 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB6) == static_cast<int>(Window::KeyboardKey::NB6), "NB6 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB7) == static_cast<int>(Window::KeyboardKey::NB7), "NB7 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB8) == static_cast<int>(Window::KeyboardKey::NB8), "NB8 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NB9) == static_cast<int>(Window::KeyboardKey::NB9), "NB9 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SEMICOLON) == static_cast<int>(Window::KeyboardKey::SEMICOLON), "SEMICOLON keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::EQUAL) == static_cast<int>(Window::KeyboardKey::EQUAL), "EQUAL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::A) == static_cast<int>(Window::KeyboardKey::A), "A keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::B) == static_cast<int>(Window::KeyboardKey::B), "B keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::C) == static_cast<int>(Window::KeyboardKey::C), "C keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::D) == static_cast<int>(Window::KeyboardKey::D), "D keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::E) == static_cast<int>(Window::KeyboardKey::E), "E keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F) == static_cast<int>(Window::KeyboardKey::F), "F keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::G) == static_cast<int>(Window::KeyboardKey::G), "G keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::H) == static_cast<int>(Window::KeyboardKey::H), "H keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::I) == static_cast<int>(Window::KeyboardKey::I), "I keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::J) == static_cast<int>(Window::KeyboardKey::J), "J keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::K) == static_cast<int>(Window::KeyboardKey::K), "K keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::L) == static_cast<int>(Window::KeyboardKey::L), "L keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::M) == static_cast<int>(Window::KeyboardKey::M), "M keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::N) == static_cast<int>(Window::KeyboardKey::N), "N keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::O) == static_cast<int>(Window::KeyboardKey::O), "O keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::P) == static_cast<int>(Window::KeyboardKey::P), "P keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::Q) == static_cast<int>(Window::KeyboardKey::Q), "Q keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::R) == static_cast<int>(Window::KeyboardKey::R), "R keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::S) == static_cast<int>(Window::KeyboardKey::S), "S keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::T) == static_cast<int>(Window::KeyboardKey::T), "T keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::U) == static_cast<int>(Window::KeyboardKey::U), "U keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::V) == static_cast<int>(Window::KeyboardKey::V), "V keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::W) == static_cast<int>(Window::KeyboardKey::W), "W keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::X) == static_cast<int>(Window::KeyboardKey::X), "X keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::Y) == static_cast<int>(Window::KeyboardKey::Y), "Y keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::Z) == static_cast<int>(Window::KeyboardKey::Z), "Z keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_BRACKET) == static_cast<int>(Window::KeyboardKey::LEFT_BRACKET), "LEFT_BRACKET keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::BACKSLASH) == static_cast<int>(Window::KeyboardKey::BACKSLASH), "BACKSLASH keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_BRACKET) == static_cast<int>(Window::KeyboardKey::RIGHT_BRACKET), "RIGHT_BRACKET keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::GRAVE_ACCENT) == static_cast<int>(Window::KeyboardKey::GRAVE_ACCENT), "GRAVE_ACCENT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::WORLD_1) == static_cast<int>(Window::KeyboardKey::WORLD_1), "WORLD_1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::WORLD_2) == static_cast<int>(Window::KeyboardKey::WORLD_2), "WORLD_2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::ESCAPE) == static_cast<int>(Window::KeyboardKey::ESCAPE), "ESCAPE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::ENTER) == static_cast<int>(Window::KeyboardKey::ENTER), "ENTER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::TAB) == static_cast<int>(Window::KeyboardKey::TAB), "TAB keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::BACKSPACE) == static_cast<int>(Window::KeyboardKey::BACKSPACE), "BACKSPACE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::INSERT) == static_cast<int>(Window::KeyboardKey::INSERT), "INSERT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::DELETE) == static_cast<int>(Window::KeyboardKey::DELETE), "DELETE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT) == static_cast<int>(Window::KeyboardKey::RIGHT), "RIGHT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT) == static_cast<int>(Window::KeyboardKey::LEFT), "LEFT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::DOWN) == static_cast<int>(Window::KeyboardKey::DOWN), "DOWN keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::UP) == static_cast<int>(Window::KeyboardKey::UP), "UP keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PAGE_UP) == static_cast<int>(Window::KeyboardKey::PAGE_UP), "PAGE_UP keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PAGE_DOWN) == static_cast<int>(Window::KeyboardKey::PAGE_DOWN), "PAGE_DOWN keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::HOME) == static_cast<int>(Window::KeyboardKey::HOME), "HOME keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::END) == static_cast<int>(Window::KeyboardKey::END), "END keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::CAPS_LOCK) == static_cast<int>(Window::KeyboardKey::CAPS_LOCK), "CAPS_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::SCROLL_LOCK) == static_cast<int>(Window::KeyboardKey::SCROLL_LOCK), "SCROLL_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::NUM_LOCK) == static_cast<int>(Window::KeyboardKey::NUM_LOCK), "NUM_LOCK keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PRINT_SCREEN) == static_cast<int>(Window::KeyboardKey::PRINT_SCREEN), "PRINT_SCREEN keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::PAUSE) == static_cast<int>(Window::KeyboardKey::PAUSE), "PAUSE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F1) == static_cast<int>(Window::KeyboardKey::F1), "F1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F2) == static_cast<int>(Window::KeyboardKey::F2), "F2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F3) == static_cast<int>(Window::KeyboardKey::F3), "F3 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F4) == static_cast<int>(Window::KeyboardKey::F4), "F4 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F5) == static_cast<int>(Window::KeyboardKey::F5), "F5 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F6) == static_cast<int>(Window::KeyboardKey::F6), "F6 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F7) == static_cast<int>(Window::KeyboardKey::F7), "F7 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F8) == static_cast<int>(Window::KeyboardKey::F8), "F8 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F9) == static_cast<int>(Window::KeyboardKey::F9), "F9 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F10) == static_cast<int>(Window::KeyboardKey::F10), "F10 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F11) == static_cast<int>(Window::KeyboardKey::F11), "F11 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F12) == static_cast<int>(Window::KeyboardKey::F12), "F12 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F13) == static_cast<int>(Window::KeyboardKey::F13), "F13 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F14) == static_cast<int>(Window::KeyboardKey::F14), "F14 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F15) == static_cast<int>(Window::KeyboardKey::F15), "F15 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F16) == static_cast<int>(Window::KeyboardKey::F16), "F16 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F17) == static_cast<int>(Window::KeyboardKey::F17), "F17 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F18) == static_cast<int>(Window::KeyboardKey::F18), "F18 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F19) == static_cast<int>(Window::KeyboardKey::F19), "F19 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F20) == static_cast<int>(Window::KeyboardKey::F20), "F20 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F21) == static_cast<int>(Window::KeyboardKey::F21), "F21 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F22) == static_cast<int>(Window::KeyboardKey::F22), "F22 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F23) == static_cast<int>(Window::KeyboardKey::F23), "F23 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F24) == static_cast<int>(Window::KeyboardKey::F24), "F24 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::F25) == static_cast<int>(Window::KeyboardKey::F25), "F25 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_0) == static_cast<int>(Window::KeyboardKey::KP_0), "KP_0 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_1) == static_cast<int>(Window::KeyboardKey::KP_1), "KP_1 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_2) == static_cast<int>(Window::KeyboardKey::KP_2), "KP_2 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_3) == static_cast<int>(Window::KeyboardKey::KP_3), "KP_3 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_4) == static_cast<int>(Window::KeyboardKey::KP_4), "KP_4 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_5) == static_cast<int>(Window::KeyboardKey::KP_5), "KP_5 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_6) == static_cast<int>(Window::KeyboardKey::KP_6), "KP_6 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_7) == static_cast<int>(Window::KeyboardKey::KP_7), "KP_7 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_8) == static_cast<int>(Window::KeyboardKey::KP_8), "KP_8 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_9) == static_cast<int>(Window::KeyboardKey::KP_9), "KP_9 keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_DECIMAL) == static_cast<int>(Window::KeyboardKey::KP_DECIMAL), "KP_DECIMAL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_DIVIDE) == static_cast<int>(Window::KeyboardKey::KP_DIVIDE), "KP_DIVIDE keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_MULTIPLY) == static_cast<int>(Window::KeyboardKey::KP_MULTIPLY), "KP_MULTIPLY keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_SUBTRACT) == static_cast<int>(Window::KeyboardKey::KP_SUBTRACT), "KP_SUBTRACT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_ADD) == static_cast<int>(Window::KeyboardKey::KP_ADD), "KP_ADD keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_ENTER) == static_cast<int>(Window::KeyboardKey::KP_ENTER), "KP_ENTER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::KP_EQUAL) == static_cast<int>(Window::KeyboardKey::KP_EQUAL), "KP_EQUAL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_SHIFT) == static_cast<int>(Window::KeyboardKey::LEFT_SHIFT), "LEFT_SHIFT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_CONTROL) == static_cast<int>(Window::KeyboardKey::LEFT_CONTROL), "LEFT_CONTROL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_ALT) == static_cast<int>(Window::KeyboardKey::LEFT_ALT), "LEFT_ALT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::LEFT_SUPER) == static_cast<int>(Window::KeyboardKey::LEFT_SUPER), "LEFT_SUPER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_SHIFT) == static_cast<int>(Window::KeyboardKey::RIGHT_SHIFT), "RIGHT_SHIFT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_CONTROL) == static_cast<int>(Window::KeyboardKey::RIGHT_CONTROL), "RIGHT_CONTROL keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_ALT) == static_cast<int>(Window::KeyboardKey::RIGHT_ALT), "RIGHT_ALT keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::RIGHT_SUPER) == static_cast<int>(Window::KeyboardKey::RIGHT_SUPER), "RIGHT_SUPER keycode does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardKey::MENU) == static_cast<int>(Window::KeyboardKey::MENU), "MENU keycode does not match");


static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::RELEASE) == static_cast<int>(Window::KeyboardEvent::RELEASE), "RELEASE event does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::PRESS) == static_cast<int>(Window::KeyboardEvent::PRESS), "PRESS event does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardEvent::REPEAT) == static_cast<int>(Window::KeyboardEvent::REPEAT), "REPEAT event does not match");

static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::NONE) == static_cast<int>(Window::KeyboardModifiers::NONE), "NONE keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::SHIFT) == static_cast<int>(Window::KeyboardModifiers::SHIFT), "SHIFT keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CONTROL) == static_cast<int>(Window::KeyboardModifiers::CONTROL), "CONTROL keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::ALT) == static_cast<int>(Window::KeyboardModifiers::ALT), "ALT keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::SUPER) == static_cast<int>(Window::KeyboardModifiers::SUPER), "SUPER keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::CAPS_LOCK) == static_cast<int>(Window::KeyboardModifiers::CAPS_LOCK), "CAPS_LOCK keyboard modifier bit does not match");
static_assert(static_cast<int>(GLFW::Window::KeyboardModifiers::NUM_LOCK) == static_cast<int>(Window::KeyboardModifiers::NUM_LOCK), "NUM_LOCK keyboard modifier bit does not match");


static_assert(static_cast<int>(GLFW::Window::MouseButton::NB1) == static_cast<int>(Window::MouseButton::NB1), "NB1 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB2) == static_cast<int>(Window::MouseButton::NB2), "NB2 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB3) == static_cast<int>(Window::MouseButton::NB3), "NB3 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB4) == static_cast<int>(Window::MouseButton::NB4), "NB4 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB5) == static_cast<int>(Window::MouseButton::NB5), "NB5 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB6) == static_cast<int>(Window::MouseButton::NB6), "NB6 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB7) == static_cast<int>(Window::MouseButton::NB7), "NB7 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::NB8) == static_cast<int>(Window::MouseButton::NB8), "NB8 mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::LEFT) == static_cast<int>(Window::MouseButton::LEFT), "LEFT mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::RIGHT) == static_cast<int>(Window::MouseButton::RIGHT), "RIGHT mouse button's keycode does not match");
static_assert(static_cast<int>(GLFW::Window::MouseButton::MIDDLE) == static_cast<int>(Window::MouseButton::MIDDLE), "MIDDLE mouse button's keycode does not match");


/*
 * Window::Monitor::Impl
 */

struct Window::Monitor::Impl {
	GLFW::Monitor monitor;

	Impl()
		: monitor(nullptr)
	{
	}

	Impl(GLFW::Monitor mon)
		: monitor(std::move(mon))
	{
	}

	~Impl() = default;

	std::string_view getName() const {
		return monitor.getName();
	}

	Math::Vec2d getPhysicalSize() const {
		return monitor.getPhysicalSize();
	}

	Math::Vec2i getSize() const {
		return monitor.getSize();
	}

	Math::Vec2i getPosition() const {
		return monitor.getPosition();
	}

	Rate getFrameRate() const {
		return Rate(monitor.getFrameRate(), 1);
	}

};

/*
 * Window::Monitor
 */

Window::Monitor::Monitor()
	: m_impl({})
{
}

Window::Monitor::Monitor(Utils::Pimpl<Impl> pimpl)
	: m_impl(std::move(pimpl))
{
}

Window::Monitor::Monitor(Monitor&& other) = default;

Window::Monitor::~Monitor() = default;

Window::Monitor& Window::Monitor::operator=(Monitor&& other) = default;



std::string_view Window::Monitor::getName() const {
	return m_impl->getName();
}

Math::Vec2d Window::Monitor::getPhysicalSize() const {
	return m_impl->getPhysicalSize();
}

Math::Vec2i Window::Monitor::getSize() const {
	return m_impl->getSize();
}

Math::Vec2i Window::Monitor::getPosition() const {
	return m_impl->getPosition();
}

Rate Window::Monitor::getFrameRate() const {
	return m_impl->getFrameRate();
}


/*
 * Window::Impl
 */
struct Window::Impl {
	struct Open {
		struct Vertex {
			Math::Vec2f position;
			Math::Vec2f texCoord;
		};


		enum DescriptorLayouts {
			DESCRIPTOR_LAYOUT_WINDOW,
			DESCRIPTOR_LAYOUT_FRAME,

			DESCRIPTOR_LAYOUT_COUNT
		};

		enum WindowDescriptors {
			WINDOW_DESCRIPTOR_VIEWPORT,
			WINDOW_DESCRIPTOR_COLOR_TRANSFER,

			WINDOW_DESCRIPTOR_COUNT
		};


		static constexpr uint32_t VERTEX_BUFFER_BINDING = 0;
		static constexpr uint32_t VERTEX_POSITION = 0;
		static constexpr uint32_t VERTEX_TEXCOORD = 1;

		static constexpr size_t COLOR_TRANSFER_UNIFORM_OFFSET = 0;
		static inline const size_t COLOR_TRANSFER_UNIFORM_SIZE = Graphics::OutputColorTransfer::size();
		static inline const size_t VIEWPORT_UNIFORM_OFFSET = Utils::align(COLOR_TRANSFER_UNIFORM_OFFSET + COLOR_TRANSFER_UNIFORM_SIZE, 0x100); //256 is the maximum
		static constexpr size_t VIEWPORT_UNIFORM_SIZE = sizeof(glm::vec2);

		static inline const size_t UNIFORM_BUFFER_SIZE = VIEWPORT_UNIFORM_OFFSET + VIEWPORT_UNIFORM_SIZE;



		const Graphics::Vulkan&						vulkan;

		GLFW::Window								window;
		vk::UniqueSurfaceKHR						surface;
		vk::UniqueCommandPool						commandPool;
		vk::CommandBuffer							commandBuffer;
		Graphics::StagedBuffer						vertexBuffer;
		Graphics::StagedBuffer						uniformBuffer;
		vk::UniqueDescriptorPool					descriptorPool;
		vk::DescriptorSet							descriptorSet;
		vk::UniqueSemaphore 						imageAvailableSemaphore;
		vk::UniqueSemaphore							renderFinishedSemaphore;
		vk::UniqueFence								renderFinishedFence;

		vk::Extent2D								extent;
		vk::Format 									format;
		vk::ColorSpaceKHR 							colorSpace;
		Graphics::OutputColorTransfer				colorTransfer;
		vk::Filter									filter;
		Graphics::Frame::Geometry					geometry;

		vk::UniqueSwapchainKHR						swapchain;
		std::vector<vk::UniqueImageView>			swapchainImageViews;
		vk::UniqueRenderPass						renderPass;
		std::vector<vk::UniqueFramebuffer>			framebuffers;
		vk::PipelineLayout							pipelineLayout;
		vk::UniquePipeline							pipeline;	


		Open(	const Graphics::Vulkan& vulkan,
				Math::Vec2i size,
				const char* name,
				GLFW::Monitor mon,
				GLFW::Window::Callbacks cbks,
				ScalingMode scalingMode,
				ScalingFilter scalingFilter ) 
			: vulkan(vulkan)
			, window(size, name, mon, std::move(cbks))
			, surface(window.getSurface(vulkan))
			, commandPool(createCommandPool(vulkan))
			, commandBuffer(createCommandBuffer(vulkan, *commandPool))
			, vertexBuffer(createVertexBuffer(vulkan))
			, uniformBuffer(createUniformBuffer(vulkan))
			, descriptorPool(createDescriptorPool(vulkan))
			, descriptorSet(createDescriptorSet(vulkan, *descriptorPool))
			, imageAvailableSemaphore(vulkan.createSemaphore())
			, renderFinishedSemaphore(vulkan.createSemaphore())
			, renderFinishedFence(vulkan.createFence(true))

			, extent(Graphics::toVulkan(window.getResolution()))
			, format(vk::Format::eUndefined)
			, colorSpace(static_cast<vk::ColorSpaceKHR>(-1))
			, colorTransfer()
			, filter(Graphics::toVulkan(scalingFilter))
			, geometry(createGeometry(vertexBuffer.data(), scalingMode, Math::Vec2f(extent.width, extent.height)))

			, swapchain()
			, swapchainImageViews()
			, renderPass()
			, framebuffers()
			, pipelineLayout(createPipelineLayout(vulkan, filter))
			, pipeline()

		{
			writeDescriptorSets();
			updateUniforms();
		}

		~Open() {
			vertexBuffer.waitCompletion(vulkan);
			uniformBuffer.waitCompletion(vulkan);
			waitCompletion();
		}

		void reconfigure(vk::Format fmt, vk::ColorSpaceKHR cs, Graphics::OutputColorTransfer ct) {
			enum {
				RECREATE_SWAPCHAIN,
				UPDATE_COLOR_TRANSFER,

				MODIFICATION_COUNT
			};

			std::bitset<MODIFICATION_COUNT> modifications;

			if(format != fmt) {
				//Format has changed
				format = fmt;

				modifications.set(RECREATE_SWAPCHAIN);
			}

			if(colorSpace != cs) {
				//Color space has changed
				colorSpace = cs;

				modifications.set(RECREATE_SWAPCHAIN);
			}

			if(colorTransfer != ct) {
				//Color transfer characteristics have changed
				colorTransfer = std::move(ct);

				modifications.set(UPDATE_COLOR_TRANSFER);
			}

			//Recreate stuff accordingly
			if(modifications.any()) {
				//Wait until rendering finishes
				waitCompletion();

				if(modifications.test(RECREATE_SWAPCHAIN)) recreateSwapchain();
				if(modifications.test(UPDATE_COLOR_TRANSFER)) updateColorTransferUniform();
			}
		}

		void setScalingMode(ScalingMode mode) {
			waitCompletion();
			geometry.setScalingMode(mode);
		}

		void setScalingFilter(ScalingFilter filt) {
			waitCompletion();
			filter = Graphics::toVulkan(filt);
			recreatePipelineLayout();
		}

		void draw(const std::shared_ptr<const Graphics::Frame>& frame) {
			//Wait for the previous rendering to be completed
			waitCompletion();

			//Acquire an image from the swapchain
			const auto index = acquireImage();

			//Resize the geometry if needed
			if(frame) {
				vertexBuffer.waitCompletion(vulkan);
				if(geometry.useFrame(*frame)){
					//Vertex buffer has changed, update it
					vertexBuffer.flushData(
						vulkan, 
						vulkan.getGraphicsQueueIndex(),
						vk::AccessFlagBits::eVertexAttributeRead,
						vk::PipelineStageFlagBits::eVertexInput
					);
				}	
			}

			const auto& frameBuffer = *(framebuffers[index]);

			//Begin writing to the command buffer. //TODO maybe reset pool?
			constexpr vk::CommandBufferBeginInfo cmdBegin(
				vk::CommandBufferUsageFlagBits::eOneTimeSubmit, 
				nullptr
			);
			vulkan.begin(commandBuffer, cmdBegin);

			//Begin a render pass
			const std::array clearValue = {
				vk::ClearValue(std::array{ 0.0f, 0.0f, 0.0f, 0.0f })
			};
			const vk::RenderPassBeginInfo rendBegin(
				*renderPass,														//Renderpass
				frameBuffer,														//Target framebuffer
				vk::Rect2D({0, 0}, extent),											//Extent
				clearValue.size(), clearValue.data()								//Attachment clear values
			);
			vulkan.beginRenderPass(commandBuffer, rendBegin, vk::SubpassContents::eInline);


			//If it is a valid frame, draw it.
			if(frame) {
				vulkan.bindPipeline(commandBuffer, vk::PipelineBindPoint::eGraphics, *pipeline);

				vulkan.bindVertexBuffers(
					commandBuffer,													//Command buffer
					VERTEX_BUFFER_BINDING,											//Binding
					vertexBuffer.getBuffer(),										//Vertex buffers
					0UL																//Offsets
				);

				vulkan.bindDescriptorSets(
					commandBuffer,													//Command buffer
					vk::PipelineBindPoint::eGraphics,								//Pipeline bind point
					pipelineLayout,													//Pipeline layout
					DESCRIPTOR_LAYOUT_WINDOW,										//First index
					descriptorSet,													//Descriptor sets
					{}																//Dynamic offsets
				);

				frame->bind(commandBuffer, pipelineLayout, DESCRIPTOR_LAYOUT_FRAME, filter);

				vulkan.draw(commandBuffer, Graphics::Frame::Geometry::VERTEX_COUNT, 1, 0, 0);
			}

			//End everything
			vulkan.endRenderPass(commandBuffer);
			vulkan.end(commandBuffer);

			//Send it to the queue
			const std::array imageAvailableSemaphores = {
				*imageAvailableSemaphore
			};
			const std::array renderFinishedSemaphores = {
				*renderFinishedSemaphore
			};
			const std::array commandBuffers = {
				commandBuffer
			};
			const std::array pipelineStages = {
				vk::PipelineStageFlags(vk::PipelineStageFlagBits::eColorAttachmentOutput)
			};
			const vk::SubmitInfo subInfo(
				imageAvailableSemaphores.size(), imageAvailableSemaphores.data(),	//Wait semaphores
				pipelineStages.data(),												//Pipeline stages
				commandBuffers.size(), commandBuffers.data(),						//Command buffers
				renderFinishedSemaphores.size(), renderFinishedSemaphores.data()	//Signal semaphores
			);
			vulkan.resetFences(*renderFinishedFence);
			vulkan.submit(vulkan.getGraphicsQueue(), subInfo, *renderFinishedFence);

			//Present it
			vulkan.present(*swapchain, index, renderFinishedSemaphores.front());
		}

		void resizeFramebuffer(Resolution res) {
			waitCompletion();

			extent = Graphics::toVulkan(res);
			geometry.setTargetSize(res);
			updateViewportUniform();
			recreateSwapchain();
		}


	private:
		void recreateSwapchain() {
			swapchain = createSwapchain(vulkan, *surface, extent, format, colorSpace, *swapchain);
			swapchainImageViews = createImageViews(vulkan, *swapchain, format);
			renderPass = createRenderPass(vulkan, format);
			framebuffers = createFramebuffers(vulkan, *renderPass, swapchainImageViews, extent);
			pipeline = createPipeline(vulkan, *renderPass, pipelineLayout, extent);
		}

		void recreatePipelineLayout() {
			pipelineLayout = createPipelineLayout(vulkan, filter);
			pipeline = createPipeline(vulkan, *renderPass, pipelineLayout, extent);
		}

		void updateViewportUniform() {
			uniformBuffer.waitCompletion(vulkan);

			auto& size = *(reinterpret_cast<Math::Vec2f*>(uniformBuffer.data() + VIEWPORT_UNIFORM_OFFSET));
			size = geometry.getTargetSize();

			uniformBuffer.flushData(
				vulkan,
				VIEWPORT_UNIFORM_OFFSET,
				VIEWPORT_UNIFORM_SIZE,
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eVertexShader
			);
		}

		void updateColorTransferUniform() {
			uniformBuffer.waitCompletion(vulkan);
			
			std::memcpy(
				uniformBuffer.data() + COLOR_TRANSFER_UNIFORM_OFFSET,
				colorTransfer.data(),
				COLOR_TRANSFER_UNIFORM_SIZE
			);

			uniformBuffer.flushData(
				vulkan,
				COLOR_TRANSFER_UNIFORM_OFFSET,
				COLOR_TRANSFER_UNIFORM_SIZE,
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eFragmentShader
			);
		}

		void updateUniforms() {
			uniformBuffer.waitCompletion(vulkan);		
			
			auto& size = *(reinterpret_cast<Math::Vec2f*>(uniformBuffer.data() + VIEWPORT_UNIFORM_OFFSET));
			size = geometry.getTargetSize();

			std::memcpy(
				uniformBuffer.data() + COLOR_TRANSFER_UNIFORM_OFFSET,
				colorTransfer.data(),
				COLOR_TRANSFER_UNIFORM_SIZE
			);

			uniformBuffer.flushData(
				vulkan,
				vulkan.getGraphicsQueueIndex(),
				vk::AccessFlagBits::eUniformRead,
				vk::PipelineStageFlagBits::eVertexShader |
				vk::PipelineStageFlagBits::eFragmentShader
			);
		}

		void waitCompletion() {
			vulkan.waitForFences(*renderFinishedFence);
		}

		uint32_t acquireImage() {
			vk::Result result;
			uint32_t index;

			//Try to acquire an image as many times as needed.
			do {
				result = vulkan.getDevice().acquireNextImageKHR(
					*swapchain, 						
					Graphics::Vulkan::NO_TIMEOUT,
					*imageAvailableSemaphore,
					nullptr,
					&index,
					vulkan.getDispatcher()
				);

				//Evaluate wether it was a success
				switch(result) {
				case vk::Result::eErrorOutOfDateKHR:
				case vk::Result::eSuboptimalKHR:
					recreateSwapchain();
					break;

				default: 
					break;
				}
			} while(result != vk::Result::eSuccess);

			return index;
		}
	
		void writeDescriptorSets() {
			const std::array viewportBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),								//Buffer
					VIEWPORT_UNIFORM_OFFSET,								//Offset
					VIEWPORT_UNIFORM_SIZE									//Size
				)
			};
			const std::array colorTransferBuffers = {
				vk::DescriptorBufferInfo(
					uniformBuffer.getBuffer(),								//Buffer
					COLOR_TRANSFER_UNIFORM_OFFSET,							//Offset
					COLOR_TRANSFER_UNIFORM_SIZE								//Size
				)
			};

			const std::array writeDescriptorSets = {
				vk::WriteDescriptorSet( //Viewport UBO
					descriptorSet,											//Descriptor set
					WINDOW_DESCRIPTOR_VIEWPORT,								//Binding
					0, 														//Index
					viewportBuffers.size(),									//Descriptor count		
					vk::DescriptorType::eUniformBuffer,						//Descriptor type
					nullptr, 												//Images 
					viewportBuffers.data(), 								//Buffers
					nullptr													//Texel buffers
				),
				vk::WriteDescriptorSet( //ColorTransfer UBO
					descriptorSet,											//Descriptor set
					WINDOW_DESCRIPTOR_COLOR_TRANSFER,						//Binding
					0, 														//Index
					colorTransferBuffers.size(),							//Descriptor count		
					vk::DescriptorType::eUniformBuffer,						//Descriptor type
					nullptr, 												//Images 
					colorTransferBuffers.data(), 							//Buffers
					nullptr													//Texel buffers
				)
			};

			vulkan.updateDescriptorSets(writeDescriptorSets, {});
		}



		static vk::UniqueCommandPool createCommandPool(const Graphics::Vulkan& vulkan)
		{
			constexpr auto createFlags = 
				vk::CommandPoolCreateFlagBits::eTransient | 		//Re-recorded often
				vk::CommandPoolCreateFlagBits::eResetCommandBuffer;	//Re-recorded individually

			const vk::CommandPoolCreateInfo createInfo(
				createFlags,										//Flags
				vulkan.getGraphicsQueueIndex()						//Queue index
			);

			return vulkan.createCommandPool(createInfo);
		}

		static vk::CommandBuffer createCommandBuffer(	const Graphics::Vulkan& vulkan,
														vk::CommandPool pool )
		{
			const vk::CommandBufferAllocateInfo allocInfo(
				pool,
				vk::CommandBufferLevel::ePrimary,
				1
			);

			//Perform a manual allocation, as there is only need for one. 
			//Dont use a smart handle, as it gets freed automatically with the command pool
			vk::CommandBuffer result;
			vulkan.getDevice().allocateCommandBuffers(&allocInfo, &result, vulkan.getDispatcher());
			return result;
		}


		static Graphics::StagedBuffer createVertexBuffer(const Graphics::Vulkan& vulkan) {
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eVertexBuffer,
				sizeof(Vertex) * Graphics::Frame::Geometry::VERTEX_COUNT
			);
		}

		static Graphics::StagedBuffer createUniformBuffer(const Graphics::Vulkan& vulkan) {
			return Graphics::StagedBuffer(
				vulkan,
				vk::BufferUsageFlagBits::eUniformBuffer,
				UNIFORM_BUFFER_SIZE
			);
		}

		static vk::DescriptorSetLayout createDescriptorSetLayout(const Graphics::Vulkan& vulkan) {
			static const Utils::StaticId id;

			auto result = vulkan.createDescriptorSetLayout(id);

			if(!result) {
				//Create the bindings
				const std::array bindings = {
					vk::DescriptorSetLayoutBinding(	//UBO binding
						WINDOW_DESCRIPTOR_VIEWPORT,						//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eVertex,				//Shader stage
						nullptr											//Inmutable samplers
					), 
					vk::DescriptorSetLayoutBinding(	//UBO binding
						WINDOW_DESCRIPTOR_COLOR_TRANSFER,				//Binding
						vk::DescriptorType::eUniformBuffer,				//Type
						1,												//Count
						vk::ShaderStageFlagBits::eFragment,				//Shader stage
						nullptr											//Inmutable samplers
					), 
				};

				const vk::DescriptorSetLayoutCreateInfo createInfo(
					{},
					bindings.size(), bindings.data()
				);

				result = vulkan.createDescriptorSetLayout(id, createInfo);
			}

			return result;
		}

		static vk::UniqueDescriptorPool createDescriptorPool(const Graphics::Vulkan& vulkan){
			const std::array poolSizes = {
				vk::DescriptorPoolSize(
					vk::DescriptorType::eUniformBuffer,					//Descriptor type
					WINDOW_DESCRIPTOR_COUNT								//Descriptor count
				)
			};

			const vk::DescriptorPoolCreateInfo createInfo(
				{},														//Flags
				1,														//Descriptor set count
				poolSizes.size(), poolSizes.data()						//Pool sizes
			);

			return vulkan.createDescriptorPool(createInfo);
		}

		static vk::DescriptorSet createDescriptorSet(	const Graphics::Vulkan& vulkan,
														vk::DescriptorPool pool )
		{
			const std::array layouts {
				createDescriptorSetLayout(vulkan)
			};

			const vk::DescriptorSetAllocateInfo allocInfo(
				pool,													//Pool
				layouts.size(), layouts.data()							//Layouts
			);

			//Allocate it
			vk::DescriptorSet descriptorSet;
			static_assert(layouts.size() == 1);
			const auto result = vulkan.getDevice().allocateDescriptorSets(&allocInfo, &descriptorSet, vulkan.getDispatcher());

			if(result != vk::Result::eSuccess){
				throw Exception("Error allocating descriptor sets");
			}

			return descriptorSet;
		}

		static Graphics::Frame::Geometry createGeometry(std::byte* data,
														ScalingMode scalingMode,
														Math::Vec2f size )
		{
			return Graphics::Frame::Geometry(
				data, sizeof(Vertex), offsetof(Vertex, position), offsetof(Vertex, texCoord),
				scalingMode, size
			);
		}


		static vk::UniqueSwapchainKHR createSwapchain(	const Graphics::Vulkan& vulkan, 
														vk::SurfaceKHR surface, 
														vk::Extent2D& extent, 
														vk::Format format,
														vk::ColorSpaceKHR colorSpace,
														vk::SwapchainKHR old )
		{
			const auto& physicalDevice = vulkan.getPhysicalDevice();

			if(!physicalDevice.getSurfaceSupportKHR(0, surface, vulkan.getDispatcher())){
				throw Exception("Window surface not suppoted by the physical device");
			}

			const auto capabilities = physicalDevice.getSurfaceCapabilitiesKHR(surface, vulkan.getDispatcher());
			const auto imageCount = getImageCount(capabilities);
			extent = getExtent(capabilities, extent);

			const auto surfaceFormats = physicalDevice.getSurfaceFormatsKHR(surface, vulkan.getDispatcher());
			const auto surfaceFormat = getSurfaceFormat(surfaceFormats, vk::SurfaceFormatKHR(format, colorSpace));

			const auto queueFamilies = getQueueFamilies(vulkan);
			const auto sharingMode = (queueFamilies.size() > 1)
										? vk::SharingMode::eConcurrent
										: vk::SharingMode::eExclusive;
			
			const auto presentModes = physicalDevice.getSurfacePresentModesKHR(surface, vulkan.getDispatcher());
			const auto presentMode = getPresentMode(presentModes);

			const vk::SwapchainCreateInfoKHR createInfo(
				{},													//Flags
				surface,											//Sufrace
				imageCount,											//Image count
				surfaceFormat.format,								//Image format
				surfaceFormat.colorSpace,							//Image color space
				extent,												//Image extent
				1,													//Image layer count
				vk::ImageUsageFlagBits::eColorAttachment,			//Image usage
				sharingMode,										//Sharing
				queueFamilies.size(), queueFamilies.data(),			//Used queue families
				capabilities.currentTransform,						//Transformations
				vk::CompositeAlphaFlagBitsKHR::eOpaque,				//Compositing
				presentMode,										//Present mode
				true,												//Clipping
				old													//Old swapchain
			);

			return vulkan.createSwapchain(createInfo);
		}

		static std::vector<vk::UniqueImageView> createImageViews(	const Graphics::Vulkan& vulkan,
																	vk::SwapchainKHR swapchain,
																	vk::Format format ) 
		{
			const auto images = vulkan.getDevice().getSwapchainImagesKHR(swapchain, vulkan.getDispatcher());
			std::vector<vk::UniqueImageView> result(images.size());

			for(size_t i = 0; i < images.size(); i++){
				const vk::ImageViewCreateInfo createInfo(
					{},												//Flags
					images[i],										//Image
					vk::ImageViewType::e2D,							//ImageView type
					format,											//Image format
					{},												//Swizzle
					vk::ImageSubresourceRange(						//Image subresources
						vk::ImageAspectFlagBits::eColor,				//Aspect mask
						0, 1, 0, 1										//Base mipmap level, mipmap levels, base array layer, layers
					)
				);

				result[i] =  vulkan.createImageView(createInfo);
			}

			return result;
		}

		static vk::UniqueRenderPass createRenderPass(	const Graphics::Vulkan& vulkan, 
														vk::Format format )
		{
			const std::array attachments = {
				vk::AttachmentDescription(
					{},												//Flags
					format,											//Attachemnt format
					vk::SampleCountFlagBits::e1,					//Sample count
					vk::AttachmentLoadOp::eClear,					//Color attachment load operation
					vk::AttachmentStoreOp::eStore,					//Color attachemnt store operation
					vk::AttachmentLoadOp::eDontCare,				//Stencil attachment load operation
					vk::AttachmentStoreOp::eDontCare,				//Stencil attachment store operation
					vk::ImageLayout::eUndefined,					//Initial layout
					vk::ImageLayout::ePresentSrcKHR					//Final layout
				)
			};

			const std::array attachmentReferences = {
				vk::AttachmentReference(
					0, 												//Attachments index
					vk::ImageLayout::eColorAttachmentOptimal 		//Attachemnt layout
				)
			};

			const std::array subpasses = {
				vk::SubpassDescription(
					{},												//Flags
					vk::PipelineBindPoint::eGraphics,				//Pipeline bind point
					0, nullptr,										//Input attachments
					attachmentReferences.size(), attachmentReferences.data(), //Color attachments
					nullptr,										//Resolve attachemnts
					nullptr,										//Depth / Stencil attachemnts
					0, nullptr										//Preserve attachments
				)
			};

			const std::array subpassDependencies = {
				vk::SubpassDependency(
					VK_SUBPASS_EXTERNAL,							//Source subpass
					0,												//Destination subpass
					vk::PipelineStageFlagBits::eColorAttachmentOutput,//Source stage
					vk::PipelineStageFlagBits::eColorAttachmentOutput,//Destination stage
					{},												//Source access mask
					vk::AccessFlagBits::eColorAttachmentRead | 		//Destintation access mask
						vk::AccessFlagBits::eColorAttachmentWrite
				)
			};

			const vk::RenderPassCreateInfo createInfo(
				{},													//Flags
				attachments.size(), attachments.data(),				//Attachemnts
				subpasses.size(), subpasses.data(),					//Subpasses
				subpassDependencies.size(), subpassDependencies.data()//Subpass dependencies
			);

			return vulkan.createRenderPass(createInfo);
		}

		static std::vector<vk::UniqueFramebuffer> createFramebuffers(	const Graphics::Vulkan& vulkan,
																		vk::RenderPass renderPass,
																		const std::vector<vk::UniqueImageView>& imageViews,
																		vk::Extent2D extent )
		{
			std::vector<vk::UniqueFramebuffer> result(imageViews.size());

			for(size_t i = 0; i < result.size(); i++){
				const std::array attachments = {
					*imageViews[i]
				};

				const vk::FramebufferCreateInfo createInfo(
					{},
					renderPass,
					attachments.size(),
					attachments.data(),
					extent.width, extent.height,
					1
				);

				result[i] = vulkan.createFramebuffer(createInfo);
			}

			return result;
		}

		static vk::PipelineLayout createPipelineLayout(	const Graphics::Vulkan& vulkan, 
														vk::Filter filter ) 
		{
			static std::array<Utils::StaticId, Graphics::Frame::FILTER_COUNT> ids;
			const auto layoutId = ids[static_cast<size_t>(filter)];

			auto result = vulkan.createPipelineLayout(layoutId);

			if(!result) {
				const std::array layouts = {
					createDescriptorSetLayout(vulkan),
					Graphics::Frame::getDescriptorSetLayout(vulkan, filter)
				};

				const vk::PipelineLayoutCreateInfo createInfo(
					{},													//Flags
					layouts.size(), layouts.data(),						//Descriptor set layouts
					0, nullptr											//Push constants
				);

				result = vulkan.createPipelineLayout(layoutId, createInfo);
			}

			return result;
		}

		static vk::UniquePipeline createPipeline(	const Graphics::Vulkan& vulkan,
													vk::RenderPass renderPass,
													vk::PipelineLayout layout,
													vk::Extent2D extent )
		{
			static //So that its ptr can be used as an indentifier
			#include <window_vert.h>
			const size_t vertId = reinterpret_cast<uintptr_t>(window_vert);
			static
			#include <window_frag.h>
			const size_t fragId = reinterpret_cast<uintptr_t>(window_frag);

			//Try to retrive modules from cache
			auto vertexShader = vulkan.createShaderModule(vertId);
			auto fragmentShader = vulkan.createShaderModule(fragId);

			if(!vertexShader || !fragmentShader) {
				//Modules aren't in cache. Create them
				vertexShader = vulkan.createShaderModule(vertId, window_vert);
				fragmentShader = vulkan.createShaderModule(fragId, window_frag);
			}

			constexpr auto SHADER_ENTRY_POINT = "main";
			const std::array shaderStages = {
				vk::PipelineShaderStageCreateInfo(		
					{},												//Flags
					vk::ShaderStageFlagBits::eVertex,				//Shader type
					vertexShader,									//Shader handle
					SHADER_ENTRY_POINT ),							//Shader entry point
				vk::PipelineShaderStageCreateInfo(		
					{},												//Flags
					vk::ShaderStageFlagBits::eFragment,				//Shader type
					fragmentShader,									//Shader handle
					SHADER_ENTRY_POINT ),							//Shader entry point
			};

			constexpr std::array vertexBindings = {
				vk::VertexInputBindingDescription(
					VERTEX_BUFFER_BINDING,
					sizeof(Vertex),
					vk::VertexInputRate::eVertex
				)
			};

			constexpr std::array vertexAttributes = {
				vk::VertexInputAttributeDescription(
					VERTEX_POSITION,
					VERTEX_BUFFER_BINDING,
					vk::Format::eR32G32Sfloat,
					offsetof(Vertex, position)
				),
				vk::VertexInputAttributeDescription(
					VERTEX_TEXCOORD,
					VERTEX_BUFFER_BINDING,
					vk::Format::eR32G32Sfloat,
					offsetof(Vertex, texCoord)
				)
			};

			const vk::PipelineVertexInputStateCreateInfo vertexInput(
				{},
				vertexBindings.size(), vertexBindings.data(),		//Vertex bindings
				vertexAttributes.size(), vertexAttributes.data()	//Vertex attributes
			);

			constexpr vk::PipelineInputAssemblyStateCreateInfo inputAssembly(
				{},													//Flags
				vk::PrimitiveTopology::eTriangleStrip,				//Topology
				false												//Restart enable
			);

			const std::array viewports = {
				vk::Viewport(
					0.0f, 0.0f,										//Origin
					static_cast<float>(extent.width), 				//Width
					static_cast<float>(extent.height),				//Height
					0.0f, 1.0f										//min, max depth
				),
			};

			const std::array scissors = {
				vk::Rect2D(
					{ 0, 0 },										//Origin
					extent											//Size
				),
			};

			const vk::PipelineViewportStateCreateInfo viewport(
				{},													//Flags
				viewports.size(), viewports.data(),					//Viewports
				scissors.size(), scissors.data()					//Scissors
			);

			constexpr vk::PipelineRasterizationStateCreateInfo rasterizer(
				{},													//Flags
				false, 												//Depth clamp enabled
				false,												//Rasterizer discard enable
				vk::PolygonMode::eFill,								//Plygon mode
				vk:: CullModeFlagBits::eNone, 						//Cull faces
				vk::FrontFace::eClockwise,							//Front face direction
				false, 0.0f, 0.0f, 0.0f,							//Depth bias
				1.0f												//Line width
			);

			constexpr vk::PipelineMultisampleStateCreateInfo multisample(
				{},													//Flags
				vk::SampleCountFlagBits::e1,						//Sample count
				false, 1.0f,										//Sample shading enable, min sample shading
				nullptr,											//Sample mask
				false, false										//Alpha to coverage, alpha to 1 enable
			);

			constexpr vk::PipelineDepthStencilStateCreateInfo depthStencil(
				{},													//Flags
				false, false, 										//Depth test enable, write
				vk::CompareOp::eNever, 								//Depth compare op
				false,												//Depth bounds test
				false, 												//Stencil enabled
				{}, {},												//Stencil operation stete front, back
				0.0f, 0.0f											//min, max depth bounds
			);

			constexpr auto colorWriteMask = 
					vk::ColorComponentFlagBits::eR |				
					vk::ColorComponentFlagBits::eG |
					vk::ColorComponentFlagBits::eB |
					vk::ColorComponentFlagBits::eA ;

			constexpr std::array colorBlendAttachments = {
				vk::PipelineColorBlendAttachmentState(
					true,											//Enabled
					//Cf' = Ai*Ci + (1.0-Ai)*Cf; Typical color mixing equation
					vk::BlendFactor::eSrcAlpha,						//Source color weight
					vk::BlendFactor::eOneMinusSrcAlpha,				//Destination color weight
					vk::BlendOp::eAdd,								//Color operation
					//Af' = Ai + (1.0-Ai)*Af = Ai + Af - Ai*Af; So that Af' is always greater than Af and Ai
					//https://www.wolframalpha.com/input/?i=plot+%7C+x+%2B+%281-x%29+*+y+%7C+x+%3D+0+to+1+y+%3D+0+to+1
					vk::BlendFactor::eOne,							//Source alpha weight
					vk::BlendFactor::eOneMinusSrcAlpha,				//Destination alpha weight
					vk::BlendOp::eAdd,								//Alpha operation
					colorWriteMask									//Color write mask
				)
			};

			const vk::PipelineColorBlendStateCreateInfo colorBlend(
				{},													//Flags
				false,												//Enable logic operation
				vk::LogicOp::eCopy,									//Logic operation
				colorBlendAttachments.size(), colorBlendAttachments.data() //Blend attachments
			);

			constexpr vk::PipelineDynamicStateCreateInfo dynamicState(
				{},													//Flags
				0, nullptr											//Dynamis states
			);

			static const Utils::StaticId pipelineId;
			const vk::GraphicsPipelineCreateInfo createInfo(
				{},													//Flags
				shaderStages.size(), shaderStages.data(),			//Shader stages
				&vertexInput,										//Vertex input
				&inputAssembly,										//Vertex assembly
				nullptr,											//Tesselation
				&viewport,											//Viewports
				&rasterizer,										//Rasterizer
				&multisample,										//Multisampling
				&depthStencil,										//Depth / Stencil tests
				&colorBlend,										//Color blending
				&dynamicState,										//Dynamic states
				layout,												//Pipeline layout
				renderPass, 0,										//Renderpasses
				nullptr, static_cast<uint32_t>(pipelineId)			//Inherit
			);

			return vulkan.createGraphicsPipeline(createInfo);
		}



		static vk::Extent2D getExtent(	const vk::SurfaceCapabilitiesKHR& cap, 
										vk::Extent2D windowExtent )
		{
			constexpr auto INVALID_EXTENT = vk::Extent2D(
				std::numeric_limits<uint32_t>::max(), 
				std::numeric_limits<uint32_t>::max()
			);

			if(cap.currentExtent != INVALID_EXTENT){
				return cap.currentExtent;
			} else {
				return vk::Extent2D(
					Math::clamp(cap.minImageExtent.width, windowExtent.width, cap.maxImageExtent.width),
					Math::clamp(cap.minImageExtent.height, windowExtent.height, cap.maxImageExtent.height)
				);
			}
		}

		static vk::SurfaceFormatKHR getSurfaceFormat(	const std::vector<vk::SurfaceFormatKHR>& formats,
														vk::SurfaceFormatKHR desired )
		{
			if(std::find(formats.cbegin(), formats.cend(), desired) != formats.cend()){
				return desired;
			}

			throw Exception("Unsupported format!");
		}

		static uint32_t getImageCount(const vk::SurfaceCapabilitiesKHR& cap){
			const uint32_t desired = cap.minImageCount + 1;

			if(cap.maxImageCount){
				return std::min(desired, cap.maxImageCount);
			} else {
				return desired;
			}
		}

		static vk::PresentModeKHR getPresentMode(const std::vector<vk::PresentModeKHR>& presentModes){
			const std::array prefered = {
				vk::PresentModeKHR::eMailbox,
				vk::PresentModeKHR::eFifo //Required to be supported.
			};

			for(auto mode : prefered){
				if(std::find(presentModes.cbegin(), presentModes.cend(), mode) != presentModes.cend()){
					return mode; //Found a apropiate one
				}
			}

			throw Exception("No compatible presentation mode was found");
		}

		static std::vector<uint32_t> getQueueFamilies(const Graphics::Vulkan& vulkan){
			const std::set<uint32_t> families = {
				vulkan.getGraphicsQueueIndex(),
				vulkan.getPresentationQueueIndex()
			};

			return std::vector<uint32_t>(families.cbegin(), families.cend());
		}
	};

	std::reference_wrapper<Window>				owner;

	std::string									windowName;
	Math::Vec2i 								size;
	Math::Vec2i 								position;
	State 										state;
	float										opacity;
	bool										resizeable;
	bool										decorated;
	GLFW::Monitor								monitor;

	Callbacks									callbacks;
	
	Signal::Input<Video>						videoIn;
	std::unique_ptr<Open>						opened;
	bool										hasChanged;

	static constexpr auto PRIORITY = Instance::OUTPUT_PRIORITY;
	static constexpr auto NO_POSTION = Math::Vec2i(std::numeric_limits<int32_t>::min());

	Impl(	Window& owner,
			Math::Vec2i size,
			const Monitor& mon,
			Callbacks callbacks)
		: owner(owner)
		, windowName(owner.getInstance().getApplicationInfo().name)
		, size(size)
		, position(NO_POSTION)
		, state(State::NORMAL)
		, opacity(1.0f)
		, resizeable(true)
		, decorated(true)
		, monitor(getGLFWMonitor(mon))
		, callbacks(std::move(callbacks))
		, videoIn(std::string(Signal::makeInputName<Video>()))
	{
	}
	~Impl() = default;


	void moved(ZuazoBase& base) {
		owner = static_cast<Window&>(base);
	}

	void open(ZuazoBase& base) {
		assert(!opened);
		assert(&owner.get() == &static_cast<Window&>(base));
		ZUAZO_IGNORE_PARAM(base);

		Window& win = owner.get();
		const auto& vulkan = win.getInstance().getVulkan();

		//Try to open it
		opened = std::make_unique<Impl::Open>(
			vulkan,
			size,
			windowName.c_str(),
			monitor,
			createCallbacks(),
			win.getScalingMode(),
			win.getScalingFilter()
		);
		
		//Set everything as desired
		//opened->window.setName(windowName); //Already set when constructing
		//opened->window.setSize(size); //Already set when constructing
		if(position != NO_POSTION) opened->window.setPosition(position);
		opened->window.setState(static_cast<GLFW::Window::State>(state));
		opened->window.setOpacity(opacity);
		opened->window.setResizeable(resizeable);
		opened->window.setDecorated(decorated);

		win.setVideoModeCompatibility(getVideoModeCompatibility());

		//Timing will be enabled on the setVideoMode() callback
		//win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue())); 

		hasChanged = true;
	}

	void close(ZuazoBase& base) {
		assert(opened);
		assert(&owner.get() == &static_cast<Window&>(base));
		ZUAZO_IGNORE_PARAM(base);

		Window& win = owner.get();

		win.disablePeriodicUpdate();
		opened.reset();
		win.setVideoModeCompatibility(getVideoModeCompatibility());
		videoIn.reset();
	}


	void setVideoMode(VideoBase& base, const VideoMode& videoMode) {
		if(opened) {
			Window& win = static_cast<Window&>(base);

			auto [format, colorSpace, colorTransfer] = convertParameters(win.getInstance().getVulkan(), videoMode);
			opened->reconfigure(format, colorSpace, std::move(colorTransfer));

			win.disablePeriodicUpdate();
			win.enablePeriodicUpdate(PRIORITY, getPeriod(videoMode.getFrameRateValue()));

			hasChanged = true;
		}
	}

	void setScalingMode(VideoScalerBase&, ScalingMode mode) {
		if(opened) {
			opened->setScalingMode(mode);
			hasChanged = true;
		}
	}

	void setScalingFilter(VideoScalerBase&, ScalingFilter filter) {
		if(opened) {
			opened->setScalingFilter(filter);
			hasChanged = true;
		}
	}

	void update() {
		assert(opened);

		if(hasChanged || videoIn.hasChanged()) {
			//Input has changed, pull a frame from it
			const auto& frame = videoIn.pull();
			opened->draw(frame);
		}

		hasChanged = false;
	}

	std::vector<VideoMode> getVideoModeCompatibility() const {
		std::vector<VideoMode> result;

		if(opened) {
			//Select a monitor to depend on
			const auto& mon = monitor ? monitor : GLFW::getGLFW().getPrimaryMonitor();

			//Construct a base capability struct which will be common to all compatibilities
			const VideoMode baseCompatibility(
				Utils::Range<Rate>(Rate(0, 1), Rate(mon.getMode().frameRate, 1)),
				Utils::MustBe<Resolution>(Graphics::fromVulkan(opened->extent)),
				Utils::MustBe<AspectRatio>(AspectRatio(1, 1)),
				Utils::Any<ColorPrimaries>(),
				Utils::MustBe<ColorModel>(ColorModel::RGB),
				Utils::Any<ColorTransferFunction>(),
				Utils::MustBe<ColorSubsampling>(ColorSubsampling::RB_444),
				Utils::MustBe<ColorRange>(ColorRange::FULL),
				Utils::Any<ColorFormat>()
			);

			//Query for full compatibility
			Window& win = owner.get();
			const auto& vulkan = win.getInstance().getVulkan();
			const auto surfaceFormats = vulkan.getPhysicalDevice().getSurfaceFormatsKHR(*(opened->surface), vulkan.getDispatcher());

			for(const auto& surfaceFormat : surfaceFormats) {
				const auto [colorPrimary, colorTransferFunction] = Graphics::fromVulkan(surfaceFormat.colorSpace);
				const auto [format, colorTransferFunction2] = Graphics::fromVulkan(surfaceFormat.format);

				//Evaluate if it is a valid option
				if(	(colorPrimary != ColorPrimaries::NONE) &&
					(colorTransferFunction != ColorTransferFunction::NONE) &&
					(format != ColorFormat::NONE) )
				{
					//Copy the base compatibility in order to modify it
					VideoMode compatibility = baseCompatibility; 

					compatibility.setColorPrimaries(Utils::MustBe<ColorPrimaries>(colorPrimary));
					compatibility.setColorTransferFunction(Utils::MustBe<ColorTransferFunction>(colorTransferFunction));
					compatibility.setColorFormat(Utils::MustBe<ColorFormat>(format));

					result.emplace_back(std::move(compatibility));
				}
			}

			if(result.size() == 0) {
				//There must be at least one compatibility, even if it is invalid
				result.emplace_back();
			}
		}
		
		return result;
	}



	void setWindowName(std::string name) {
		if(windowName != name) {
			windowName = std::move(name);
			if(opened) opened->window.setName(windowName.c_str());
		}
	}

	const std::string& getWindowName() const {
		return windowName;
	}


	void setSize(Math::Vec2i s) {
		if(size != s) {
			size = s;
			if(opened) {
				opened->window.setSize(size);
				opened->resizeFramebuffer(opened->window.getResolution());
			}
		}
	}

	Math::Vec2i getSize() const {
		return size;
	}

	void setSizeCallback(SizeCallback cbk) {
		callbacks.sizeCbk = std::move(cbk);
	}

	const SizeCallback& getSizeCallback() const {
		return callbacks.sizeCbk;
	}


	void setPosition(Math::Vec2i pos) {
		if(position != pos) {
			position = pos;
			if(opened) opened->window.setPosition(position);
		}
	}

	Math::Vec2i getPosition() const {
		return position;
	}

	void setPositionCallback(PositionCallback cbk) {
		callbacks.positionCbk = std::move(cbk);
	}

	const PositionCallback& getPositionCallback() const {
		return callbacks.positionCbk;
	}


	void setState(State st) {
		if(state != st) {
			state = st;
			if(opened) opened->window.setState(static_cast<GLFW::Window::State>(state));
		}
	}

	State getState() const {
		return state;
	}

	void setStateCallback(StateCallback cbk) {
		callbacks.stateCbk = std::move(cbk);
	}

	const StateCallback& getStateCallback() const {
		return callbacks.stateCbk;
	}


	Math::Vec2f getScale() const {
		return opened ? opened->window.getScale() : Math::Vec2f(0.0f);
	}

	void setScaleCallback(ScaleCallback cbk) {
		callbacks.scaleCbk = std::move(cbk);
	}

	const ScaleCallback& getScaleCallback() const {
		return callbacks.scaleCbk;
	}


	void focus() {
		if(opened) opened->window.focus();
	}

	void setFocusCallback(FocusCallback cbk) {
		callbacks.focusCbk = std::move(cbk);
	}

	const FocusCallback& getFocusCallback() const {
		return callbacks.focusCbk;
	}


	bool shouldClose() const {
		return opened ? opened->window.shouldClose() : false;
	}

	void setShouldCloseCallback(ShouldCloseCallback cbk) {
		callbacks.shouldCloseCbk = std::move(cbk);
	}

	const ShouldCloseCallback& getShouldCloseCallback() const {
		return callbacks.shouldCloseCbk;
	}


	void setOpacity(float opa) {
		opacity = opa;
		if(opened) opened->window.setOpacity(opacity);
	}

	float getOpacity() const {
		return opacity;
	}


	void setResizeable(bool res) {
		resizeable = res;
		if(opened) opened->window.setResizeable(resizeable);
	}

	bool getResizeable() const {
		return resizeable;
	}


	void setDecorated(bool deco) {
		decorated = deco;
		if(opened) opened->window.setDecorated(decorated);
	}

	bool getDecorated() const {
		return decorated;
	}

	void setMonitor(const Monitor& mon) {
		monitor = getGLFWMonitor(mon);

		if(opened) {
			opened->window.setMonitor(monitor);
			size = opened->window.getSize();
			opened->resizeFramebuffer(opened->window.getResolution());
		}
	}
	
	Monitor getMonitor() const {
		return constructMonitor(monitor);
	}



	KeyboardEvent getKeyState(KeyboardKey key) const {
		return opened 
		? static_cast<KeyboardEvent>(opened->window.getKeyState(static_cast<GLFW::Window::KeyboardKey>(key)))
		: KeyboardEvent::RELEASE;
	}

	void setKeyboardCallback(KeyboardCallback cbk) {
		callbacks.keyboardCbk = std::move(cbk);
	}

	const KeyboardCallback& getKeyboardCallback() const {
		return callbacks.keyboardCbk;
	}


	void setCharacterCallback(CharacterCallback cbk) {
		callbacks.characterCbk = std::move(cbk);
	}

	const CharacterCallback& getCharacterCallback() const {
		return callbacks.characterCbk;
	}


	KeyboardEvent getMouseButtonState(MouseButton but) const {
		return opened 
		? static_cast<KeyboardEvent>(opened->window.getMouseButtonState(static_cast<GLFW::Window::MouseButton>(but)))
		: KeyboardEvent::RELEASE;
	}

	void setMouseButtonCallback(MouseButtonCallback cbk) {
		callbacks.mouseButtonCbk = std::move(cbk);
	}

	const MouseButtonCallback& getMouseButtonCallback() const {
		return callbacks.mouseButtonCbk;
	}

	
	Math::Vec2d getMousePosition() const {
		return opened 
		? opened->window.getMousePosition()
		: Math::Vec2d();
	}

	void setMousePositionCallback(MousePositionCallback cbk) {
		callbacks.mousePositionCbk = std::move(cbk);
	}

	const MousePositionCallback& getMousePositionCallback() const {
		return callbacks.mousePositionCbk;
	}


	void setMouseScrollCallback(MouseScrollCallback cbk) {
		callbacks.mouseScrollCbk = std::move(cbk);
	}

	const MouseScrollCallback& getMouseScrollCallback() const {
		return callbacks.mouseScrollCbk;
	}


	void setCursorEnterCallback(CursorEnterCallback cbk) {
		callbacks.cursorEnterCbk = std::move(cbk);
	}

	const CursorEnterCallback& getCursorEnterCallback() const {
		return callbacks.cursorEnterCbk;
	}



	static void init() {
		GLFW::init();
	}



	static Monitor getPrimaryMonitor() {
		return Impl::constructMonitor(GLFW::getGLFW().getPrimaryMonitor());
	}

	static std::vector<Monitor> getMonitors() {
		const auto monitors = GLFW::getGLFW().getMonitors();
		std::vector<Window::Monitor> result;
		result.reserve(monitors.size());

		for(const auto& mon : monitors) {
			result.push_back(constructMonitor(mon));
		}

		return result;
	}



	static void pollEvents(std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		lock.unlock();

		GLFW::getGLFW().pollEvents();

		lock.lock();
	}

	static void waitEvents(std::unique_lock<Instance>& lock) {
		assert(lock.owns_lock());
		lock.unlock();

		GLFW::getGLFW().waitEvents();
		
		lock.lock();
	}

	static void waitEvents(std::unique_lock<Instance>& lock, Duration timeout) {
		assert(lock.owns_lock());
		lock.unlock();

		GLFW::getGLFW().waitEvents(timeout);
		
		lock.lock();
	}

	static std::shared_ptr<Instance::ScheduledCallback> enableRegularEventPolling(Instance& instance) {
		auto callback = std::make_shared<Instance::ScheduledCallback>(
			[&instance] {
				//As it is being called from the loop, instance should be locked by this thread
				std::unique_lock<Instance> lock(instance, std::adopt_lock);
				Window::pollEvents(lock);
				lock.release(); //Leave it locked
			}
		);

		//This callback must be the last one, as it unlocks the instance, which might be dangerous
		instance.addRegularCallback(callback, Instance::LOWEST_PRIORITY);
		return callback;
	}

private:
	GLFW::Window::Callbacks createCallbacks() {
		return {
			std::bind(&Impl::stateCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::positionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::sizeCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::resolutionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::scaleCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::shouldCloseCallback, std::ref(*this)),
			GLFW::Window::RefreshCallback(),
			std::bind(&Impl::focusCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::keyboardCallback, std::ref(*this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&Impl::characterCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::mouseButtonCallback, std::ref(*this), std::placeholders::_1, std::placeholders::_2, std::placeholders::_3),
			std::bind(&Impl::mousePositionCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::mouseScrollCallback, std::ref(*this), std::placeholders::_1),
			std::bind(&Impl::cursorEnterCallback, std::ref(*this), std::placeholders::_1)
		};
	}

	void resolutionCallback(Resolution res) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());
		opened->resizeFramebuffer(res);
		//TODO update compatibility and remove the above line

		hasChanged = true;
	}

	void sizeCallback(Math::Vec2i s) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		size = s;
		Utils::invokeIf(callbacks.sizeCbk, win, size);
	}

	void positionCallback(Math::Vec2i pos) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		position = pos;
		Utils::invokeIf(callbacks.positionCbk, win, position);
	}

	void stateCallback(GLFW::Window::State st) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		state = static_cast<State>(st);
		Utils::invokeIf(callbacks.stateCbk, win, state);
	}

	void scaleCallback(Math::Vec2f sc) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.scaleCbk, win, sc);
	}

	void focusCallback(bool foc) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.focusCbk, win, foc);
	}

	void shouldCloseCallback() {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.shouldCloseCbk, win);
	}

	void keyboardCallback(GLFW::Window::KeyboardKey key, GLFW::Window::KeyboardEvent event, GLFW::Window::KeyboardModifiers mod) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(
			callbacks.keyboardCbk, 
			win, 
			static_cast<KeyboardKey>(key),
			static_cast<KeyboardEvent>(event),
			static_cast<KeyboardModifiers>(mod)
		);
	}

	void characterCallback(uint character) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.characterCbk, win, character);
	}

	void mouseButtonCallback(GLFW::Window::MouseButton button, GLFW::Window::KeyboardEvent event, GLFW::Window::KeyboardModifiers mod) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(
			callbacks.mouseButtonCbk, 
			win, 
			static_cast<MouseButton>(button),
			static_cast<KeyboardEvent>(event),
			static_cast<KeyboardModifiers>(mod)
		);
	}

	void mousePositionCallback(Math::Vec2d pos) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.mousePositionCbk, win, pos);
	}

	void mouseScrollCallback(Math::Vec2d deltaScroll) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.mouseScrollCbk, win, deltaScroll);
	}

	void cursorEnterCallback(bool entered) {
		assert(opened);
		Window& win = owner.get();
		std::lock_guard<Instance> lock(win.getInstance());

		Utils::invokeIf(callbacks.cursorEnterCbk, win, entered);
	}



	static std::tuple<vk::Format, vk::ColorSpaceKHR, Graphics::OutputColorTransfer>
	convertParameters(	const Graphics::Vulkan& vulkan,
						const VideoMode& videoMode )
	{
		const auto frameDescriptor = videoMode.getFrameDescriptor();

		//Obatin the pixel format
		auto formats = Graphics::Frame::getPlaneDescriptors(frameDescriptor);
		assert(formats.size() == 1);

		auto& f = formats[0];
		std::tie(f.format, f.swizzle) = Graphics::optimizeFormat(std::make_tuple(f.format, f.swizzle));
		assert(formats[0].swizzle == vk::ComponentMapping());

		//Obtain the color space
		const auto colorSpace = Graphics::toVulkan(
			videoMode.getColorPrimariesValue(), 
			videoMode.getColorTransferFunctionValue()
		);

		//Create the color transfer characteristics
		Graphics::OutputColorTransfer colorTransfer(frameDescriptor);

		const auto& supportedFormats = vulkan.getFormatSupport().framebuffer;
		colorTransfer.optimize(formats, supportedFormats);

		return std::make_tuple(f.format, colorSpace, std::move(colorTransfer));
	}

	static Monitor constructMonitor(GLFW::Monitor mon) {
		return Monitor(Utils::Pimpl<Monitor::Impl>({}, std::move(mon)));
	}

	static GLFW::Monitor getGLFWMonitor(const Monitor& mon) {
		return mon.m_impl->monitor;
	}
};


/*
 * Window
 */

const Window::Monitor Window::NO_MONITOR = Window::Monitor();

Window::Window(	Instance& instance, 
				std::string name, 
				VideoMode videoMode,
				Math::Vec2i size,
				const Monitor& mon,
				Callbacks cbks )
	: ZuazoBase(instance, std::move(name))
	, VideoBase(std::move(videoMode))
	, VideoScalerBase()
	, m_impl({}, *this, size, mon, std::move(cbks))
{
	Layout::registerPad(m_impl->videoIn);
	setMoveCallback(std::bind(&Impl::moved, std::ref(*m_impl), std::placeholders::_1));
	setOpenCallback(std::bind(&Impl::open, std::ref(*m_impl), std::placeholders::_1));
	setCloseCallback(std::bind(&Impl::close, std::ref(*m_impl), std::placeholders::_1));
	setUpdateCallback(std::bind(&Impl::update, std::ref(*m_impl)));
	setVideoModeCallback(std::bind(&Impl::setVideoMode, std::ref(*m_impl), std::placeholders::_1, std::placeholders::_2));
	setScalingModeCallback(std::bind(&Impl::setScalingMode, std::ref(*m_impl), std::placeholders::_1, std::placeholders::_2));
	setScalingFilterCallback(std::bind(&Impl::setScalingFilter, std::ref(*m_impl), std::placeholders::_1, std::placeholders::_2));

	setVideoModeCompatibility(m_impl->getVideoModeCompatibility());
}

Window::Window(Window&& other) = default;

Window::~Window() = default;

Window& Window::operator=(Window&& other) = default;


void Window::setWindowName(std::string name) {
	m_impl->setWindowName(std::move(name));
}
const std::string& Window::getWindowName() const {
	return m_impl->getWindowName();
}


void Window::setSize(Math::Vec2i size) {
	m_impl->setSize(size);
}

Math::Vec2i Window::getSize() const {
	return m_impl->getSize();
}

void Window::setSizeCallback(SizeCallback cbk) {
	m_impl->setSizeCallback(std::move(cbk));
}

const Window::SizeCallback& Window::getSizeCallback() const {
	return m_impl->getSizeCallback();
}


void Window::setPosition(Math::Vec2i pos) {
	m_impl->setPosition(pos);
}

Math::Vec2i Window::getPosition() const {
	return m_impl->getPosition();
}

void Window::setPositionCallback(PositionCallback cbk) {
	m_impl->setPositionCallback(std::move(cbk));
}

const Window::PositionCallback&	Window::getPositionCallback() const {
	return m_impl->getPositionCallback();
}


void Window::setState(State state) {
	m_impl->setState(state);
}

Window::State Window::getState() const {
	return m_impl->getState();
}

void Window::setStateCallback(StateCallback cbk) {
	m_impl->setStateCallback(std::move(cbk));
}

const Window::StateCallback& Window::getStateCallback() const {
	return m_impl->getStateCallback();
}


Math::Vec2f Window::getScale() const {
	return m_impl->getScale();
}

void Window::setScaleCallback(ScaleCallback cbk) {
	m_impl->setScaleCallback(std::move(cbk));
}

const Window::ScaleCallback& Window::getScaleCallback() const {
	return m_impl->getScaleCallback();
}


void Window::focus() {
	m_impl->focus();
}

void Window::setFocusCallback(FocusCallback cbk) {
	m_impl->setFocusCallback(std::move(cbk));
}

const Window::FocusCallback& Window::getFocusCallback() const {
	return m_impl->getFocusCallback();
}


bool Window::shouldClose() const {
	return m_impl->shouldClose();
}

void Window::setShouldCloseCallback(ShouldCloseCallback cbk) {
	m_impl->setShouldCloseCallback(std::move(cbk));
}

const Window::ShouldCloseCallback& Window::getShouldCloseCallback() const {
	return m_impl->getShouldCloseCallback();
}


void Window::setOpacity(float opa) {
	m_impl->setOpacity(opa);
}

float Window::getOpacity() const {
	return m_impl->getOpacity();
}


void Window::setResizeable(bool resizeable) {
	m_impl->setResizeable(resizeable);
}

bool Window::getResizeable() const {
	return m_impl->getResizeable();
}


void Window::setDecorated(bool deco) {
	m_impl->setDecorated(deco);
}

bool Window::getDecorated() const {
	return m_impl->getDecorated();
}


void Window::setMonitor(const Monitor& mon) {
	m_impl->setMonitor(mon);
}

Window::Monitor Window::getMonitor() const {
	return m_impl->getMonitor();
}



Window::KeyboardEvent Window::getKeyState(KeyboardKey key) const {
	return m_impl->getKeyState(key);
}

void Window::setKeyboardCallback(KeyboardCallback cbk) {
	m_impl->setKeyboardCallback(std::move(cbk));
}

const Window::KeyboardCallback& Window::getKeyboardCallback() const {
	return m_impl->getKeyboardCallback();
}


void Window::setCharacterCallback(CharacterCallback cbk) {
	m_impl->setCharacterCallback(std::move(cbk));
}

const Window::CharacterCallback& Window::getCharacterCallback() const {
	return m_impl->getCharacterCallback();
}


Window::KeyboardEvent Window::getMouseButtonState(MouseButton but) const {
	return m_impl->getMouseButtonState(but);
}

void Window::setMouseButtonCallback(MouseButtonCallback cbk) {
	m_impl->setMouseButtonCallback(std::move(cbk));
}

const Window::MouseButtonCallback& Window::getMouseButtonCallback() const {
	return m_impl->getMouseButtonCallback();
}


Math::Vec2d Window::getMousePosition() const {
	return m_impl->getMousePosition();
}

void Window::setMousePositionCallback(MousePositionCallback cbk) {
	m_impl->setMousePositionCallback(std::move(cbk));
}

const Window::MousePositionCallback& Window::getMousePositionCallback() const {
	return m_impl->getMousePositionCallback();
}


void Window::setMouseScrollCallback(MouseScrollCallback cbk) {
	m_impl->setMouseScrollCallback(std::move(cbk));
}

const Window::MouseScrollCallback& Window::getMouseScrollCallback() const {
	return m_impl->getMouseScrollCallback();
}


void Window::setCursorEnterCallback(CursorEnterCallback cbk) {
	m_impl->setCursorEnterCallback(std::move(cbk));
}

const Window::CursorEnterCallback& Window::getCursorEnterCallback() const {
	return m_impl->getCursorEnterCallback();
}



void Window::init() {
	Impl::init();
}



Window::Monitor Window::getPrimaryMonitor() {
	return Impl::getPrimaryMonitor();
}

std::vector<Window::Monitor> Window::getMonitors() {
	return Impl::getMonitors();
}



void Window::pollEvents(std::unique_lock<Instance>& lock) {
	Impl::pollEvents(lock);
}

void Window::waitEvents(std::unique_lock<Instance>& lock) {
	Impl::waitEvents(lock);
}

void Window::waitEvents(std::unique_lock<Instance>& lock, Duration timeout) {
	Impl::waitEvents(lock, timeout);
}

std::shared_ptr<Instance::ScheduledCallback> Window::enableRegularEventPolling(Instance& instance) {
	return Impl::enableRegularEventPolling(instance);
}

}

namespace Zuazo {

std::string_view toString(Outputs::Window::State state) {
	switch(state){

	ZUAZO_ENUM2STR_CASE( Outputs::Window::State, NORMAL)
	ZUAZO_ENUM2STR_CASE( Outputs::Window::State, HIDDEN)
	ZUAZO_ENUM2STR_CASE( Outputs::Window::State, FULLSCREEN)
	ZUAZO_ENUM2STR_CASE( Outputs::Window::State, ICONIFIED)
	ZUAZO_ENUM2STR_CASE( Outputs::Window::State, MAXIMIZED)

	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, Outputs::Window::State state) {
	return os << toString(state);
}


std::string_view toString(Outputs::Window::KeyboardKey key) {
	return GLFW::Window::getKeyName(static_cast<GLFW::Window::KeyboardKey>(key), 0);
}

std::ostream& operator<<(std::ostream& os, Outputs::Window::KeyboardKey key) {
	return os << toString(key);
}


std::string_view toString(Outputs::Window::KeyboardEvent event) {
	switch(event){

	ZUAZO_ENUM2STR_CASE( Outputs::Window::KeyboardEvent, RELEASE)
	ZUAZO_ENUM2STR_CASE( Outputs::Window::KeyboardEvent, PRESS)
	ZUAZO_ENUM2STR_CASE( Outputs::Window::KeyboardEvent, REPEAT)

	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, Outputs::Window::KeyboardEvent event) {
	return os << toString(event);
}


std::string toString(Outputs::Window::KeyboardModifiers mod) {
	std::stringstream ss;
	ss << mod;
	return ss.str();
}

std::ostream& operator<<(std::ostream& os, Outputs::Window::KeyboardModifiers mod) {
	uint32_t count = 0;
	if((mod & Outputs::Window::KeyboardModifiers::SHIFT) != Outputs::Window::KeyboardModifiers::NONE)		os << (count++ ? " | " : "") << "SHIFT";
	if((mod & Outputs::Window::KeyboardModifiers::CONTROL) != Outputs::Window::KeyboardModifiers::NONE)		os << (count++ ? " | " : "") << "CONTROL";
	if((mod & Outputs::Window::KeyboardModifiers::ALT) != Outputs::Window::KeyboardModifiers::NONE)			os << (count++ ? " | " : "") << "ALT";
	if((mod & Outputs::Window::KeyboardModifiers::SUPER) != Outputs::Window::KeyboardModifiers::NONE)		os << (count++ ? " | " : "") << "SUPER";
	if((mod & Outputs::Window::KeyboardModifiers::CAPS_LOCK) != Outputs::Window::KeyboardModifiers::NONE)	os << (count++ ? " | " : "") << "CAPS_LOCK";
	if((mod & Outputs::Window::KeyboardModifiers::NUM_LOCK) != Outputs::Window::KeyboardModifiers::NONE)	os << (count++ ? " | " : "") << "NUM_LOCK";
	return os;
}


std::string_view toString(Outputs::Window::MouseButton but) {
	switch(but) {
	case Outputs::Window::MouseButton::LEFT: return "LEFT";
	case Outputs::Window::MouseButton::RIGHT: return "RIGHT";
	case Outputs::Window::MouseButton::MIDDLE: return "MIDDLE";
	default: return "";
	}
}

std::ostream& operator<<(std::ostream& os, Outputs::Window::MouseButton but) {
	return os << toString(but);
}


}