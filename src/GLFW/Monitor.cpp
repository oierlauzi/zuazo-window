#include "Monitor.h"

namespace Zuazo::GLFW {

Monitor::Monitor(MonitorHandle monitor)
	: m_monitor(monitor)
{
}

void Monitor::setUserPointer(void* ptr) {
	Instance::get().setUserPointer(m_monitor, ptr);
}

void* Monitor::getUserPointer() const {
	return Instance::get().getUserPointer(m_monitor);
}


std::string_view Monitor::getName() const {
	return Instance::get().getName(m_monitor);
}

Math::Vec2i Monitor::getPhysicalSize() const {
	return Instance::get().getPhysicalSize(m_monitor);
}

Math::Vec2i Monitor::getPosition() const {
	return Instance::get().getPosition(m_monitor);
}

const VideoMode& Monitor::getVideoMode() const {
	return Instance::get().getVideoMode(m_monitor);
}

Utils::BufferView<const VideoMode> Monitor::getVideoModes() const {
	return Instance::get().getVideoModes(m_monitor);
}


Monitor Monitor::getPrimaryMonitor() {
	const auto monitor = Instance::get().getPrimaryMonitor();
	return reinterpret_cast<const Monitor&>(monitor);
}

Utils::BufferView<Monitor> Monitor::getMonitors() {
	const auto monitors = Instance::get().getMonitors();

	static_assert(sizeof(MonitorHandle) == sizeof(Monitor), "Sizes must match");
	return Utils::BufferView<Monitor>(
		reinterpret_cast<Monitor*>(monitors.data()),
		monitors.size()
	);
}

MonitorCallback Monitor::setMonitorCallback(MonitorCallback cbk) {
	return Instance::get().setMonitorCallback(cbk);
}
	
}