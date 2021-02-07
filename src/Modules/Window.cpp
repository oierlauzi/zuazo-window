#include <zuazo/Modules/Window.h>

#include "../GLFW/Instance.h"

#include <zuazo/Utils/Functions.h>

#include <cassert>

namespace Zuazo::Modules {

std::unique_ptr<Window> Window::s_singleton;

Window::Window() 
	: Instance::Module(std::string(name), version)
{
	GLFW::Instance::initialize();
}

Window::~Window() {
	GLFW::Instance::terminate();
}



Window::VulkanExtensions Window::getRequiredVulkanInstanceExtensions() const {
	return GLFW::Instance::get().getRequiredVulkanInstanceExtensions();
}

Window::VulkanExtensions Window::getRequiredVulkanDeviceExtensions() const {
	return GLFW::Instance::get().getRequiredVulkanDeviceExtensions();
}

bool Window::getPresentationSupport(vk::Instance  instance, 
									vk::PhysicalDevice device, 
									uint32_t queueIndex ) const 
{
	return GLFW::Instance::get().getPresentationSupport(instance, device, queueIndex);
}



const Window& Window::get() {
	if(!s_singleton) {
		s_singleton = std::unique_ptr<Window>(new Window);
	}

	assert(s_singleton);
	return *s_singleton;
}

}