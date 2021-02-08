#pragma once

#include "Instance.h"

namespace Zuazo::GLFW {

class Monitor {
public:
	explicit Monitor(MonitorHandle monitor = nullptr);
	Monitor(const Monitor& other) = default;
	~Monitor() = default;

	Monitor&							operator=(const Monitor& other) = default;

	operator MonitorHandle() noexcept;

	void								setUserPointer(void* ptr);
	void*								getUserPointer() const;

	std::string_view 					getName() const;
	Math::Vec2i 						getPhysicalSize() const;
	Math::Vec2i 						getPosition() const;
	const VideoMode&					getVideoMode() const;
	Utils::BufferView<const VideoMode>	getVideoModes() const;

	static Monitor						getPrimaryMonitor();
	static Utils::BufferView<Monitor>	getMonitors();
	static MonitorCallback				setMonitorCallback(MonitorCallback cbk);
	
private:
	MonitorHandle						m_monitor;

};

}