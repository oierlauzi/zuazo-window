#include <zuazo/Modules/Window.h>

#include "../GLFW.h"

#include <zuazo/Utils/Functions.h>

#include <cassert>

namespace Zuazo::Modules {

std::unique_ptr<Window> Window::s_singleton;

Window::Window() {
	GLFW::initialize();
}

Window::~Window() {
	assert(m_pollCallbacks.size() == 0);
	GLFW::terminate();
}



void Window::initialize(Instance& instance) {
	const auto[ite, result] = m_pollCallbacks.emplace(
		&instance, 
		Utils::makeShared<Instance::ScheduledCallback>(std::bind(&Window::pollCallback, std::ref(instance)))
	);
	assert(result);

	//This callback must be the last one, as it unlocks the instance, which might be dangerous
	instance.addRegularCallback(ite->second, Instance::LOWEST_PRIORITY);
}

void Window::terminate(Instance& instance) {
	const auto ite = m_pollCallbacks.find(&instance);
	assert(ite != m_pollCallbacks.end());

	instance.removeRegularCallback(ite->second);
	m_pollCallbacks.erase(ite);
}


Window::VulkanExtensions Window::getRequiredVulkanInstanceExtensions() const {
	return GLFW::getRequiredVulkanInstanceExtensions();
}

Window::VulkanExtensions Window::getRequiredVulkanDeviceExtensions() const {
	return GLFW::getRequiredVulkanDeviceExtensions();
}

bool Window::getPresentationSupport(vk::Instance  instance, 
									vk::PhysicalDevice device, 
									uint32_t queueIndex ) const 
{
	return GLFW::getPresentationSupport(instance, device, queueIndex);
}



Window& Window::get() {
	if(!s_singleton) {
		s_singleton = std::unique_ptr<Window>(new Window);
	}

	assert(s_singleton);
	return *s_singleton;
}



void Window::pollCallback(Instance& instance) {
	//As it is being called from the loop, instance should be locked by this thread
	instance.unlock();
	GLFW::getGLFW().pollEvents();
	instance.lock();
}

}