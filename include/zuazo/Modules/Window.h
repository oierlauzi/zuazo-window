#pragma once

#include <zuazo/Instance.h>

#include <memory>
#include <unordered_map>

namespace Zuazo::Modules {

class Window final
	: public Instance::Module
{
public:
	~Window();

	virtual void 						initialize(Instance& instance) override;
	virtual void 						terminate(Instance& instance) override;

	virtual VulkanExtensions			getRequiredVulkanInstanceExtensions() const override;
	virtual VulkanExtensions			getRequiredVulkanDeviceExtensions() const override;
	virtual bool						getPresentationSupport(	vk::Instance  instance, 
																vk::PhysicalDevice device, 
																uint32_t queueIndex ) const override;


	static Window& 						get();

private:
	using PollCallbacks = std::unordered_map<Instance*, std::shared_ptr<Instance::ScheduledCallback>>;

	Window();
	Window(const Window& other) = delete;

	Window& 							operator=(const Window& other) = delete;

 	PollCallbacks						m_pollCallbacks;

	static std::unique_ptr<Window> 		s_singleton;

	static void 						pollCallback(Instance& instance);
};

}