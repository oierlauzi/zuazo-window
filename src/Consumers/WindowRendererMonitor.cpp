#include <zuazo/Consumers/WindowRenderer.h>

#include "../GLFW/Monitor.h"
#include "../GLFWConversions.h"

namespace Zuazo::Consumers {

//Check compatibility among types
static_assert(sizeof(void*) == sizeof(GLFW::Monitor), "Sizes must match in order to reinterpret cast");
static_assert(alignof(void*) == alignof(GLFW::Monitor), "Alignment must match in order to reinterpret cast");
static_assert(offsetof(WindowRenderer::Monitor::Mode, WindowRenderer::Monitor::Mode::size) == offsetof(GLFW::VideoMode, GLFW::VideoMode::size), "Size offset does not match");
static_assert(offsetof(WindowRenderer::Monitor::Mode, WindowRenderer::Monitor::Mode::colorDepth) == offsetof(GLFW::VideoMode, GLFW::VideoMode::colorDepth), "Color depth offset does not match");
static_assert(offsetof(WindowRenderer::Monitor::Mode, WindowRenderer::Monitor::Mode::frameRate) == offsetof(GLFW::VideoMode, GLFW::VideoMode::frameRate), "Refresh rate offset does not match");

static GLFW::Monitor reinterpretMonitor(void* handle) {
	return reinterpret_cast<const GLFW::Monitor&>(handle);
}



WindowRenderer::Monitor::Monitor()
	: m_handle(nullptr)
{
}

WindowRenderer::Monitor::Monitor(const Monitor& other) = default;

WindowRenderer::Monitor::~Monitor() = default;

WindowRenderer::Monitor& WindowRenderer::Monitor::operator=(const Monitor& other) = default;



bool WindowRenderer::Monitor::operator==(const Monitor& other) const noexcept {
	return m_handle == other.m_handle;
}

bool WindowRenderer::Monitor::operator!=(const Monitor& other) const noexcept {
	return m_handle != other.m_handle;
}



std::string_view WindowRenderer::Monitor::getName() const {
	return reinterpretMonitor(m_handle).getName();
}

Math::Vec2d WindowRenderer::Monitor::getPhysicalSize() const {
	return reinterpretMonitor(m_handle).getPhysicalSize();
}

Math::Vec2i WindowRenderer::Monitor::getPosition() const {
	return reinterpretMonitor(m_handle).getPosition();
}

const WindowRenderer::Monitor::Mode& WindowRenderer::Monitor::getMode() const {
	const auto& vm = reinterpretMonitor(m_handle).getVideoMode();
	return reinterpret_cast<const Mode&>(vm);
}

Utils::BufferView<const WindowRenderer::Monitor::Mode> WindowRenderer::Monitor::getModes() const {
	const auto vms = reinterpretMonitor(m_handle).getVideoModes();
	return Utils::BufferView<const WindowRenderer::Monitor::Mode>(
		reinterpret_cast<const Mode*>(vms.data()),
		vms.size()
	);
}

}