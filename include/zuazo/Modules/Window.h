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

	static constexpr std::string_view name = "Window";
	static constexpr Version version = Version(0, 1, 0);

	static const Window& 				get();

private:
	Window();
	Window(const Window& other) = delete;

	Window& 							operator=(const Window& other) = delete;

	virtual VulkanExtensions			getRequiredVulkanInstanceExtensions() const override;
	virtual VulkanExtensions			getRequiredVulkanDeviceExtensions() const override;
	virtual bool						getPresentationSupport(	vk::Instance  instance, 
																vk::PhysicalDevice device, 
																uint32_t queueIndex ) const override;



	static std::unique_ptr<Window> 		s_singleton;
	
};

}