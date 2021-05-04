#include <zuazo/Renderers/Window.h>

#include "../GLFW/Monitor.h"
#include "../GLFWConversions.h"

namespace Zuazo::Renderers {

//Check compatibility among types
static_assert(sizeof(void*) == sizeof(GLFW::Monitor), "Sizes must match in order to reinterpret cast");
static_assert(alignof(void*) == alignof(GLFW::Monitor), "Alignment must match in order to reinterpret cast");
static_assert(offsetof(Window::Monitor::Mode, Window::Monitor::Mode::size) == offsetof(GLFW::VideoMode, GLFW::VideoMode::size), "Size offset does not match");
static_assert(offsetof(Window::Monitor::Mode, Window::Monitor::Mode::colorDepth) == offsetof(GLFW::VideoMode, GLFW::VideoMode::colorDepth), "Color depth offset does not match");
static_assert(offsetof(Window::Monitor::Mode, Window::Monitor::Mode::frameRate) == offsetof(GLFW::VideoMode, GLFW::VideoMode::frameRate), "Refresh rate offset does not match");

static GLFW::Monitor reinterpretMonitor(void* handle) {
	return reinterpret_cast<const GLFW::Monitor&>(handle);
}



Window::Monitor::Monitor()
	: m_handle(nullptr)
{
}

Window::Monitor::Monitor(const Monitor& other) = default;

Window::Monitor::~Monitor() = default;

Window::Monitor& Window::Monitor::operator=(const Monitor& other) = default;



bool Window::Monitor::operator==(const Monitor& other) const noexcept {
	return m_handle == other.m_handle;
}

bool Window::Monitor::operator!=(const Monitor& other) const noexcept {
	return m_handle != other.m_handle;
}



std::string_view Window::Monitor::getName() const {
	return reinterpretMonitor(m_handle).getName();
}

Math::Vec2d Window::Monitor::getPhysicalSize() const {
	return reinterpretMonitor(m_handle).getPhysicalSize();
}

Math::Vec2i Window::Monitor::getPosition() const {
	return reinterpretMonitor(m_handle).getPosition();
}

const Window::Monitor::Mode& Window::Monitor::getMode() const {
	const auto& vm = reinterpretMonitor(m_handle).getVideoMode();
	return reinterpret_cast<const Mode&>(vm);
}

Utils::BufferView<const Window::Monitor::Mode> Window::Monitor::getModes() const {
	const auto vms = reinterpretMonitor(m_handle).getVideoModes();
	return Utils::BufferView<const Window::Monitor::Mode>(
		reinterpret_cast<const Mode*>(vms.data()),
		vms.size()
	);
}

}