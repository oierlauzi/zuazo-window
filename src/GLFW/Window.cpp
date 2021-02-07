#include "Window.h"

namespace Zuazo::GLFW {

Window::Window(	Math::Vec2i size, 
				const char* name,
				Monitor mon,
				const WindowCallbacks& callbacks,
				void* usrPtr )
	: m_window(Instance::get().createWindow(size, name, reinterpret_cast<const MonitorHandle&>(mon), callbacks, usrPtr))
	, m_windowedGeometry()
{
}

Window::Window(Window&& other)
	: m_window(nullptr)
	, m_windowedGeometry(other.m_windowedGeometry)
{
	*this = std::move(other);
}

Window::~Window() {
	if(m_window) {
		Instance::get().destroyWindow(m_window);
	}
}


Window& Window::operator=(Window&& other) {
	if(m_window) {
		Instance::get().destroyWindow(m_window);
	}

	m_window = other.m_window;
	m_windowedGeometry = other.m_windowedGeometry;

	other.m_window = nullptr;

	return *this;
}


WindowPositionCallback Window::setPositionCallback(WindowPositionCallback cbk) {
	return Instance::get().setPositionCallback(m_window, cbk);
}

WindowSizeCallback Window::setSizeCallback(WindowSizeCallback cbk) {
	return Instance::get().setSizeCallback(m_window, cbk);
}

WindowCloseCallback Window::setCloseCallback(WindowCloseCallback cbk) {
	return Instance::get().setCloseCallback(m_window, cbk);
}

WindowRefreshCallback Window::setRefreshCallback(WindowRefreshCallback cbk) {
	return Instance::get().setRefreshCallback(m_window, cbk);
}

WindowFocusCallback Window::setFocusCallback(WindowFocusCallback cbk) {
	return Instance::get().setFocusCallback(m_window, cbk);
}

WindowIconifyCallback Window::setIconifyCallback(WindowIconifyCallback cbk) {
	return Instance::get().setIconifyCallback(m_window, cbk);
}

WindowMaximizeCallback Window::setMaximizeCallback(WindowMaximizeCallback cbk) {
	return Instance::get().setMaximizeCallback(m_window, cbk);
}

WindowResolutionCallback Window::setResolutionCallback(WindowResolutionCallback cbk) {
	return Instance::get().setResolutionCallback(m_window, cbk);
}

WindowScaleCallback Window::setScaleCallback(WindowScaleCallback cbk) {
	return Instance::get().setScaleCallback(m_window, cbk);
}

WindowKeyCallback Window::setKeyCallback(WindowKeyCallback cbk) {
	return Instance::get().setKeyCallback(m_window, cbk);
}

WindowCharCallback Window::setCharCallback(WindowCharCallback cbk) {
	return Instance::get().setCharCallback(m_window, cbk);
}

WindowMousePositionCallback Window::setMousePositionCallback(WindowMousePositionCallback cbk) {
	return Instance::get().setMousePositionCallback(m_window, cbk);
}

WindowMouseEnterCallback Window::setMouseEnterCallback(WindowMouseEnterCallback cbk) {
	return Instance::get().setMouseEnterCallback(m_window, cbk);
}

WindowMouseButtonCallback Window::setMouseButtonCallback(WindowMouseButtonCallback cbk) {
	return Instance::get().setMouseButtonCallback(m_window, cbk);
}

WindowMouseScrollCallback Window::setMouseScrollCallback(WindowMouseScrollCallback cbk) {
	return Instance::get().setMouseScrollCallback(m_window, cbk);
}



void Window::setUserPointer(void* usrPtr) {
	Instance::get().setUserPointer(m_window, usrPtr);
}

void* Window::getUserPointer() const {
	return Instance::get().getUserPointer(m_window);
}


void Window::setTitle(const char* title) {
	Instance::get().setTitle(m_window, title);
}



void Window::iconify() {
	Instance::get().iconify(m_window);
}

bool Window::isIconified() const {
	return Instance::get().isIconified(m_window);
}

void Window::maximize() {
	Instance::get().maximize(m_window);
}

bool Window::isMaximized() const {
	return Instance::get().isMaximized(m_window);
}

void Window::focus() {
	Instance::get().focus(m_window);
}

bool Window::isFocused() const {
	return Instance::get().isFocused(m_window);
}

void Window::restore() {
	Instance::get().restore(m_window);
}

bool Window::shouldClose() const {
	return Instance::get().shouldClose(m_window);
}



void Window::setMonitor(Monitor monitor,
						const VideoMode* videoMode )
{
	Instance::get().setMonitor(
		m_window, 
		reinterpret_cast<const MonitorHandle&>(monitor), 
		videoMode, 
		&m_windowedGeometry
	);
}

Monitor Window::getMonitor() const {
	const auto monitor = Instance::get().getMonitor(m_window);
	return reinterpret_cast<const Monitor&>(monitor);
}


void Window::setPosition(Math::Vec2i pos) {
	Instance::get().setPosition(m_window, pos);
}

Math::Vec2i Window::getPosition() const {
	return Instance::get().getPosition(m_window);
}


void Window::setSize(Math::Vec2i size) {
	Instance::get().setSize(m_window, size);
}

Math::Vec2i Window::getSize() const {
	return Instance::get().getSize(m_window);
}


void Window::setOpacity(float opa) {
	Instance::get().setOpacity(m_window, opa);
}

float Window::getOpacity() const {
	return Instance::get().getOpacity(m_window);
}


Resolution Window::getResolution() const {
	return Instance::get().getResolution(m_window);
}

Math::Vec2f Window::getScale() const {
	return Instance::get().getScale(m_window);
}


void Window::setDecorated(bool deco) {
	Instance::get().setDecorated(m_window, deco);
}

bool Window::getDecorated() const {
	return Instance::get().getDecorated(m_window);
}


void Window::setResizeable(bool resizeable) {
	Instance::get().setResizeable(m_window, resizeable);
}

bool Window::getResizeable() const {
	return Instance::get().getResizeable(m_window);
}

void Window::setVisibility(bool visibility) {
	Instance::get().setVisibility(m_window, visibility);
}

bool Window::getVisibility() const {
	return Instance::get().getVisibility(m_window);
}


KeyEvent Window::getKeyState(KeyboardKey key) const {
	return Instance::get().getKeyState(m_window, key);
}

KeyEvent Window::getMouseButtonState(MouseButton but) const {
	return Instance::get().getMouseButtonState(m_window, but);
}

Math::Vec2d Window::getMousePosition() const {
	return Instance::get().getMousePosition(m_window);
}

vk::SurfaceKHR Window::createSurface(vk::Instance instance) const {
	return Instance::get().createSurface(m_window, instance);
}



std::string_view Window::getKeyName(KeyboardKey key, int scancode) {
	return Instance::get().getKeyName(key, scancode);
}
	
}