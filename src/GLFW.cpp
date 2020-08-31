#include "GLFW.h"

#include <zuazo/Graphics/Vulkan.h>
#include <zuazo/Exception.h>

#include <future>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <type_traits>

#define GLFW_INCLUDE_NONE //Don't include GL
#include <GLFW/glfw3.h>

namespace Zuazo {

/*
 * GLFW::Callbacks
 */
struct GLFW::Callbacks {
	static inline std::mutex mutex;
	static inline MonitorHandle currentMonitor = nullptr;
	static inline bool enabled = false;



	static void setupUserPtr(WindowHandle win, void* userPtr) {
		glfwSetWindowUserPointer(win, userPtr);
	}

	static void setupCallbacks(WindowHandle win) {
		glfwSetWindowPosCallback(win, Callbacks::positionCbk);
		glfwSetWindowSizeCallback(win, Callbacks::sizeCbk);
		glfwSetWindowCloseCallback(win, Callbacks::closeCbk);
		glfwSetWindowRefreshCallback(win, Callbacks::refreshCbk);
		glfwSetWindowFocusCallback(win, Callbacks::focusCbk);
		glfwSetWindowIconifyCallback(win, Callbacks::iconifyCbk);
		glfwSetWindowMaximizeCallback(win, Callbacks::maximizeCbk);
		glfwSetFramebufferSizeCallback(win, Callbacks::framebufferCbk);
		glfwSetWindowContentScaleCallback(win, Callbacks::scaleCbk);
	}

	static void setupMonitorCallback() {
		glfwSetMonitorCallback(Callbacks::monitorCbk);
	}


	static void monitorCbk(MonitorHandle mon, int evnt){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			const auto& cbk = getGLFW().getMonitorCallback();
			if(cbk){
				assert(currentMonitor == nullptr);
				currentMonitor = mon;

				switch(evnt){
				case GLFW_CONNECTED: 
					cbk(GLFW::Monitor(mon), GLFW::MonitorEvent::CONNECTED); 
					break;
				case GLFW_DISCONNECTED: 
					cbk(GLFW::Monitor(mon), GLFW::MonitorEvent::DISCONNECTED); 
					break;
				default:
					break;
				}

				currentMonitor = nullptr;
			}
		}
	}

	static void positionCbk(WindowHandle win, int x, int y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getPositionCallback();

			if(cbk){
				cbk(Math::Vec2i(x, y));
			}
		}
	}

	static void sizeCbk(WindowHandle win, int x, int y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getSizeCallback();

			if(cbk){
				cbk(Math::Vec2i(x, y));
			}
		}
	}

	static void closeCbk(WindowHandle win){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getCloseCallback();

			if(cbk){
				cbk();
			}
		}
	}

	static void refreshCbk(WindowHandle win){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getRefreshCallback();

			if(cbk){
				cbk();
			}
		}
	}

	static void focusCbk(WindowHandle win, int x){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getFocusCallback();

			if(cbk){
				cbk(static_cast<bool>(x));
			}
		}
	}

	static void iconifyCbk(WindowHandle win, int x){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getStateCallback();

			if(cbk){
				if(x == GLFW_TRUE){
					cbk(Window::State::ICONIFIED);
				} else {
					cbk(Window::State::NORMAL);
				}
			}
		}
	}

	static void maximizeCbk(WindowHandle win, int x){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getStateCallback();

			if(cbk){
				if(x == GLFW_TRUE){
					cbk(Window::State::MAXIMIZED);
				} else {
					cbk(Window::State::NORMAL);
				}
			}
		}
	}

	static void framebufferCbk(WindowHandle win, int x, int y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getResolutionCallback();

			if(cbk){
				cbk(Resolution(x, y));
			}
		}
	}

	static void scaleCbk(WindowHandle win, float x, float y){
		if(enabled) {
			std::lock_guard<std::mutex> lock(mutex);

			auto* window = static_cast<Window*>(glfwGetWindowUserPointer(win));
			assert(window);
			const auto& cbk = window->getScaleCallback();

			if(cbk){
				cbk(Math::Vec2f(x, y));
			}
		}
	}

};


/*
 * GLFW::MainThread
 */

struct GLFW::MainThread {
	using Task = std::function<void(void)>;

	bool							exit;
	size_t							windowCount;

	std::mutex						mutex;
	std::condition_variable			continueCondition;
	std::condition_variable			completeCondition;
	std::queue<Task> 				tasks;
	std::thread						thread;

	MainThread()
		: exit(false)
		, windowCount(0)
		, thread(&MainThread::threadFunc, this)
	{
		//Wait for completion
		std::unique_lock<std::mutex> lock(mutex);
		threadContinue();
		completeCondition.wait(lock);
	}

	~MainThread() {
		std::unique_lock<std::mutex> lock(mutex);
		exit = true;
		threadContinue();
		lock.unlock();

		thread.join();
	}

	//Event stuff
	void pollEvents() {
		return execute(pollEventsImpl);
	}

	void waitEvents() {
		return execute(waitEventsImpl);
	}

	void waitEvents(Duration timeout) {
		return execute(waitEventsToImpl, timeout);
	}
	

	//Monitor stuff
	std::vector<Monitor> getMonitors(){
		return execute(getMonitorsImpl);
	}

	bool isValid(MonitorHandle mon){
		return execute(isValidImpl, mon);
	}

	std::string_view getName(MonitorHandle mon){
		return execute(getNameImpl, mon);
	}

	Math::Vec2i getPosition(MonitorHandle mon){
		return execute(getMonitorPositionImpl, mon);
	}

	Math::Vec2d getPhysicalSize(MonitorHandle mon){
		return execute(getPhysicalSizeImpl, mon);
	}

	Monitor::Mode getMode(MonitorHandle mon){
		return execute(getModeImpl, mon);
	}

	std::vector<Monitor::Mode> getModes(MonitorHandle mon){
		return execute(getModesImpl, mon);
	}


	//Window stuff
	GLFW::WindowHandle createWindow(const Math::Vec2i& size, 
									std::string_view name,
									Monitor mon,
									Window* usrPtr )
	{
		return execute(
			std::mem_fn(&MainThread::createWindowImpl),
			this, size, name, mon, usrPtr
		);
	}

	void destroyWindow(WindowHandle window) {
		execute(
			std::mem_fn(&MainThread::destroyWindowImpl),
			this, window
		);
	}

	void setName(WindowHandle win, std::string_view name){
		execute(setNameImpl, win, name);
	}



	void setState(	WindowHandle win,  
					Window::Geometry* windowedGeom,
					Window::State st )
	{
		execute(setStateImpl, win, windowedGeom, st);
	}

	Window::State getState(WindowHandle win){
		return execute(getStateImpl, win);
	}



	void setMonitor(WindowHandle win, 
					Window::Geometry* windowedGeom,
					const Monitor& newMon, 
					const Monitor::Mode& mode )
	{
		execute(setMonitorImpl, win, windowedGeom, newMon, mode);
	}

	Monitor getMonitor(WindowHandle win){
		return execute(getMonitorImpl, win);
	}



	void setPosition(WindowHandle win, Math::Vec2i pos){
		execute(setPositionImpl, win, pos);
	}

	Math::Vec2i getPosition(WindowHandle win){
		return execute(getWindowPositionImpl, win);
	}



	void setSize(WindowHandle win, Math::Vec2i size){
		execute(setSizeImpl, win, size);
	}

	Math::Vec2i getSize(WindowHandle win){
		return execute(getSizeImpl, win);
	}



	void setOpacity(WindowHandle win, float opa){
		execute(setOpacityImpl, win, opa);
	}

	float getOpacity(WindowHandle win){
		return execute(getOpacityImpl, win);
	}



	Resolution getResolution(WindowHandle win){
		return execute(getResolutionImpl, win);
	}



	Math::Vec2f getScale(WindowHandle win){
		return execute(getScaleImpl, win);
	}


	void focus(WindowHandle win){
		execute(focusImpl, win);
	}



	void setDecorated(WindowHandle win, bool deco){
		execute(setDecoratedImpl, win, deco);
	}

	bool getDecorated(WindowHandle win){
		return execute(getDecoratedImpl, win);
	}



	void setResizeable(WindowHandle win, bool resizeable){
		execute(setResizeableImpl, win, resizeable);
	}

	bool getResizeable(WindowHandle win){
		return execute(getResizeableImpl, win);
	}

private:
	void threadFunc(){
		std::unique_lock<std::mutex> lock(mutex);

		glfwInit();

		registerVulkanRequirements();
		Callbacks::setupMonitorCallback();

		while(exit == false){
			//Notify waiting threads
			completeCondition.notify_all();
			continueCondition.wait(lock);

			//Invoke all pending tasks
			while(tasks.size() > 0){
				tasks.front()();
				tasks.pop();
			}
		}

		glfwTerminate();
	}

	void threadContinue(){
		continueCondition.notify_all();
	}


	template<typename Func, typename... Args>
	typename std::invoke_result<Func, Args...>::type execute(Func&& func, Args&&... args) {
		using Ret = typename std::invoke_result<Func, Args...>::type;

		if(std::this_thread::get_id() == thread.get_id()){
			//We are on the main thread. Simply execute it
			return std::forward<Func>(func)(std::forward<Args>(args)...); 
		}else {
			std::unique_lock<std::mutex> lock(mutex);

			//Create a future object to pass it to the main thread
			std::packaged_task<Ret(Args...)> task(std::forward<Func>(func));
			tasks.push(std::bind(std::ref(task), std::forward<Args>(args)...));
			
			//Wait until execution is complete
			threadContinue();
			completeCondition.wait(lock);

			return task.get_future().get();
		}
	}


	//Vulkan related
	static std::vector<vk::ExtensionProperties> getRequiredInstanceExtensions() {
		uint32_t glfwExtensionCount;
		const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<vk::ExtensionProperties> extensions(glfwExtensionCount);
		for(size_t i = 0; i < extensions.size(); i++){
			strncpy(extensions[i].extensionName, glfwExtensions[i], VK_MAX_EXTENSION_NAME_SIZE);
		}

		return extensions;
	}

	static std::vector<vk::ExtensionProperties> getRequiredDeviceExtensions() {
		return {
			vk::ExtensionProperties(std::array<char, VK_MAX_EXTENSION_NAME_SIZE>{VK_KHR_SWAPCHAIN_EXTENSION_NAME})
		};
	}

	static void registerVulkanRequirements() {
		static bool registered = false;

		if(!registered) {
			Graphics::Vulkan::registerRequiredInstanceExtensions(
				getRequiredInstanceExtensions()
			);
			Graphics::Vulkan::registerRequiredDeviceExtensions(
				getRequiredDeviceExtensions()
			);
			Graphics::Vulkan::registerPresentationSupportCallback(
				[] (vk::Instance inst, vk::PhysicalDevice dev, uint32_t index) -> bool {
					return getGLFW().getPresentationSupport(inst, dev, index);	
				}
			);

			registered = true;
		}

		assert(registered);
	}


	//Event implementations
	static void pollEventsImpl() {
		assert(Callbacks::enabled == false);
		Callbacks::enabled = true;

		glfwPollEvents();

		Callbacks::enabled = false;
	}

	static void waitEventsImpl() {
		assert(Callbacks::enabled == false);
		Callbacks::enabled = true;

		glfwWaitEvents();

		Callbacks::enabled = false;
	}

	static void waitEventsToImpl(Duration timeout) {	
		assert(Callbacks::enabled == false);
		Callbacks::enabled = true;

		using seconds = std::chrono::duration<double, std::ratio<1, 1>>;
		glfwWaitEventsTimeout(seconds(timeout).count());

		Callbacks::enabled = false;
	}



	//Monitor implementations
	static std::vector<Monitor> getMonitorsImpl(){
		int cnt;
		MonitorHandle* monitors = glfwGetMonitors(&cnt);
		return std::vector<GLFW::Monitor>(monitors, monitors + cnt);
	}

	static bool isValidImpl(MonitorHandle mon){
		if(mon == nullptr) {
			return false;
		}

		if(Callbacks::currentMonitor == mon){
			return true;
		}

		int cnt;
		MonitorHandle* start = glfwGetMonitors(&cnt);
		MonitorHandle* end = start + cnt;

		return std::find(start, end, mon) != end;
	}

	static std::string_view getNameImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		return std::string_view(glfwGetMonitorName(mon));
	}

	static Math::Vec2i getMonitorPositionImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		Math::Vec2i result;
		glfwGetMonitorPos(mon, &result.x, &result.y);
		return result;
	}

	static Math::Vec2d getPhysicalSizeImpl(MonitorHandle mon){
		assert(isValidImpl(mon));
		Math::Vec2i result;
		glfwGetMonitorPhysicalSize(mon, &result.x, &result.y);
		return Math::Vec2d(result) * 1e-3; //In metres
	}

	static Monitor::Mode getModeImpl(MonitorHandle mon){
		assert(isValidImpl(mon));

		const GLFWvidmode* mod = glfwGetVideoMode(mon);
		assert(mod);

		return Monitor::Mode{
			Monitor::ColorDepth(mod->redBits, mod->greenBits, mod->blueBits),
			Math::Vec2i(mod->width, mod->height),
			mod->refreshRate
		};
	}

	static std::vector<Monitor::Mode> getModesImpl(MonitorHandle mon){
		assert(isValidImpl(mon));

		int cnt;
		const GLFWvidmode* mod = glfwGetVideoModes(mon, &cnt);
		assert(mod);

		std::vector<Monitor::Mode> modes; modes.reserve(cnt);
		for(int i = 0; i < cnt; i++){
			modes.emplace_back(Monitor::Mode{
				Monitor::ColorDepth(mod[i].redBits, mod[i].greenBits, mod[i].blueBits),
				Math::Vec2i(mod[i].width, mod[i].height),
				mod[i].refreshRate
			});
		}

		return modes;
	}



	//Window implementations
	GLFW::WindowHandle createWindowImpl(Math::Vec2i size, 
										std::string_view name,
										Monitor mon,
										Window* usrPtr )
	{
		//Set Vulkan compatibility
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

		//Create the window
		WindowHandle win = glfwCreateWindow(
			size.x, size.y,
			name.data(),
			mon.m_monitor,
			static_cast<WindowHandle>(nullptr)
		);

		if(win == nullptr){
			return nullptr;
		}

		//Set all callbacks
		Callbacks::setupUserPtr(win, static_cast<void*>(usrPtr));
		Callbacks::setupCallbacks(win);

		windowCount++;
		return win;
	}

	void destroyWindowImpl(WindowHandle window) {
		glfwDestroyWindow(window);
		windowCount--;
	}

	static void setNameImpl(WindowHandle win, std::string_view name){
		glfwSetWindowTitle(win, name.data());
	}



	static void setStateImpl(	WindowHandle win,  
								Window::Geometry* windowedGeom,
								Window::State st )
	{
		const auto lastState = getStateImpl(win);

		if(lastState != st){
			//Leave it on normal state
			switch (lastState) {
			case Window::State::HIDDEN:
				glfwShowWindow(win);
				break;

			case Window::State::FULLSCREEN:
				setMonitorImpl(	
					win, 
					windowedGeom, 
					GLFW::NO_MONITOR, 
					Monitor::Mode()
				);
				break;

			case Window::State::ICONIFIED:
				glfwRestoreWindow(win);
				break;

			case Window::State::MAXIMIZED:
				glfwRestoreWindow(win);
				break;
			
			default: //State::NORMAL
				break;
			}

			//Switch to the desired state
			switch (st) {
			case Window::State::HIDDEN:
				glfwHideWindow(win);
				break;

			case Window::State::FULLSCREEN:
				setMonitorImpl(
					win, 
					windowedGeom, 
					Monitor(glfwGetPrimaryMonitor()),
					getModeImpl(glfwGetPrimaryMonitor())
				);
				break;

			case Window::State::ICONIFIED:
				glfwIconifyWindow(win);
				break;

			case Window::State::MAXIMIZED:
				glfwMaximizeWindow(win);
				break;
			
			default: //State::NORMAL
				break;
			}
		}
	}

	static Window::State getStateImpl(WindowHandle win){
		Window::State result;

		if(!glfwGetWindowAttrib(win, GLFW_VISIBLE)){
			result = Window::State::HIDDEN;
		} else if(glfwGetWindowMonitor(win)){
			result = Window::State::FULLSCREEN;
		}else if(glfwGetWindowAttrib(win, GLFW_ICONIFIED)){
			result = Window::State::ICONIFIED;
		} else if(glfwGetWindowAttrib(win, GLFW_MAXIMIZED)){
			result = Window::State::MAXIMIZED;
		} else {
			result = Window::State::NORMAL;
		}

		return result;
	}



	static void setMonitorImpl(	WindowHandle win, 
								Window::Geometry* windowedGeom,
								const Monitor& newMon, 
								const Monitor::Mode& mode )
	{
		const auto oldMonHandle = glfwGetWindowMonitor(win);
		const auto newMonHandle = newMon.m_monitor;

		if(newMonHandle != oldMonHandle){
			//State has changed. Evaluate what to do
			if(isValidImpl(newMonHandle)) {
				//It is going to be full-screen
				if(!isValidImpl(oldMonHandle)){
					//It was windowed. Save its state
					*windowedGeom = {
						getWindowPositionImpl(win),
						getSizeImpl(win)
					};
				}

				const Math::Vec2i pos = getMonitorPositionImpl(newMonHandle);

				glfwSetWindowMonitor(
					win, 
					newMonHandle, 
					pos.x,
					pos.y,
					mode.size.x,
					mode.size.y,
					mode.frameRate
				);
			} else {
				//It is going to be windowed
				glfwSetWindowMonitor(
					win, 
					static_cast<MonitorHandle>(nullptr), 
					windowedGeom->pos.x,
					windowedGeom->pos.y,
					windowedGeom->size.x,
					windowedGeom->size.y,
					0
				);
			}
		}
	}

	static Monitor getMonitorImpl(WindowHandle win){
		return Monitor(glfwGetWindowMonitor(win));
	}



	static void setPositionImpl(WindowHandle win, const Math::Vec2i& pos){
		glfwSetWindowPos(win, pos.x, pos.y);
	}

	static Math::Vec2i getWindowPositionImpl(WindowHandle win){
		Math::Vec2i result;
		glfwGetWindowPos(win, &result.x, &result.y);
		return result;
	}



	static void setSizeImpl(WindowHandle win, const Math::Vec2i& size){
		glfwSetWindowSize(win, size.x, size.y);
	}

	static Math::Vec2i getSizeImpl(WindowHandle win){
		Math::Vec2i result;
		glfwGetWindowSize(win, &result.x, &result.y);
		return result;
	}



	static void setOpacityImpl(WindowHandle win, float opa){
		glfwSetWindowOpacity(win, opa);
	}

	static float getOpacityImpl(WindowHandle win){
		return glfwGetWindowOpacity(win);
	}



	static Resolution getResolutionImpl(WindowHandle win){
		int x, y;
		glfwGetFramebufferSize(win, &x, &y);
		return Resolution(x, y);
	}



	static Math::Vec2f getScaleImpl(WindowHandle win){
		Math::Vec2f result;
		glfwGetWindowContentScale(win, &result.x, &result.y);
		return result;
	}


	static void focusImpl(WindowHandle win){
		glfwFocusWindow(win);
	}



	static void setDecoratedImpl(WindowHandle win, bool deco){
		glfwSetWindowAttrib(win, GLFW_DECORATED, deco);
	}

	static bool getDecoratedImpl(WindowHandle win){
		return glfwGetWindowAttrib(win, GLFW_DECORATED);
	}



	static void setResizeableImpl(WindowHandle win, bool resizeable){
		glfwSetWindowAttrib(win, GLFW_RESIZABLE, resizeable);
	}

	static bool getResizeableImpl(WindowHandle win){
		return glfwGetWindowAttrib(win, GLFW_RESIZABLE);
	}

};


/*
 * GLFW
 */

const GLFW::Monitor GLFW::NO_MONITOR(nullptr);
std::unique_ptr<GLFW> GLFW::s_instance;


GLFW::GLFW()
	: m_mainThread(std::make_unique<MainThread>())
{
}

GLFW::~GLFW() = default;

std::vector<GLFW::Monitor> GLFW::getMonitors(){
	return m_mainThread->getMonitors();
}

void GLFW::setMonitorCallback(MonitorCallback cbk){
	std::lock_guard<std::mutex> lock(Callbacks::mutex);
	m_monitorCbk = std::move(cbk);
}

const GLFW::MonitorCallback& GLFW::getMonitorCallback() const {
	return m_monitorCbk;
}

void GLFW::pollEvents() const {
	m_mainThread->pollEvents();
}

void GLFW::waitEvents() const {
	m_mainThread->waitEvents();
}

void GLFW::waitEvents(Duration timeout) const {
	m_mainThread->waitEvents(timeout);
}

void GLFW::postEmptyEvent() const {
	glfwPostEmptyEvent(); //Thread safe
}




bool GLFW::getPresentationSupport(	vk::Instance instance, 
									vk::PhysicalDevice device, 
									uint32_t family ) const
{
	//Thread safe
	return glfwGetPhysicalDevicePresentationSupport(
		static_cast<VkInstance>(instance), 
		static_cast<VkPhysicalDevice>(device), 
		family
	);
}



void GLFW::init() {
	assert(!s_instance);
	s_instance = std::unique_ptr<GLFW>(new GLFW());
}

GLFW& GLFW::getGLFW() {
	assert(s_instance);
	return *s_instance;
}


/*
 * GLFW::Monitor
 */

GLFW::Monitor::Monitor(MonitorHandle mon) 
	: m_monitor(mon)
{
}

GLFW::Monitor::operator bool() const {
	return getGLFW().m_mainThread->isValid(m_monitor);
}

std::string_view GLFW::Monitor::getName() const {
	return getGLFW().m_mainThread->getName(m_monitor);
}

Math::Vec2i GLFW::Monitor::getPosition() const {
	return getGLFW().m_mainThread->getPosition(m_monitor);
}

Math::Vec2d GLFW::Monitor::getPhysicalSize() const {
	return getGLFW().m_mainThread->getPhysicalSize(m_monitor);
}

GLFW::Monitor::Mode GLFW::Monitor::getMode() const {
	return getGLFW().m_mainThread->getMode(m_monitor);
}

std::vector<GLFW::Monitor::Mode> GLFW::Monitor::getModes() const {
	return getGLFW().m_mainThread->getModes(m_monitor);
}



/*
 * GLFW::Window
 */

GLFW::Window::Window(WindowHandle handle)
	: m_window(handle)
{
}

GLFW::Window::Window(	Math::Vec2i size, 
						std::string_view name,
						Monitor mon,
						Callbacks callbacks )
	: m_window(getGLFW().m_mainThread->createWindow(size, name, mon, this))
	, m_callbacks(std::move(callbacks))
{
	if(m_window == nullptr){
		throw Exception("Error creating the GLFW window");
	}
}

GLFW::Window::Window(Window&& other) {
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);

	m_window = other.m_window;
	other.m_window = nullptr;

	GLFW::Callbacks::setupUserPtr(m_window, this); //Thread-safe GLFW function
	m_callbacks = std::move(other.m_callbacks);
}

GLFW::Window::~Window(){
	if(m_window){
		getGLFW().m_mainThread->destroyWindow(m_window);
	}
}

GLFW::Window& GLFW::Window::operator=(Window&& other){
	if(m_window){
		getGLFW().m_mainThread->destroyWindow(m_window);
	}

	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);

	m_window = other.m_window;
	other.m_window = nullptr;

	GLFW::Callbacks::setupUserPtr(m_window, this);
	m_callbacks = std::move(other.m_callbacks);

	return *this;
}

GLFW::Window::operator bool() const {
	return m_window;
}



//Getters / Setters
vk::UniqueSurfaceKHR GLFW::Window::getSurface(const Graphics::Vulkan& vulkan) const {
	using Deleter = vk::UniqueHandleTraits<vk::SurfaceKHR, vk::DispatchLoaderDynamic>::deleter;

	//Try to create the surface
	VkSurfaceKHR surface;
	VkResult err = glfwCreateWindowSurface( //Thread-safe GLFW function
		vulkan.getInstance(), 
		m_window, 
		nullptr, 
		&surface
	);

	if(err != VK_SUCCESS){
		throw Exception("Error creating Vulkan surface");
	}

	return vk::UniqueSurfaceKHR(
		surface,
		Deleter(vulkan.getInstance(), nullptr, vulkan.getDispatcher())
	);
}	


void GLFW::Window::setName(std::string_view name){
	getGLFW().m_mainThread->setName(m_window, name);
}



void GLFW::Window::setState(State st){
	getGLFW().m_mainThread->setState(m_window, &m_windowedState, st);
}

GLFW::Window::State GLFW::Window::getState() const{
	return getGLFW().m_mainThread->getState(m_window);
}

void GLFW::Window::setStateCallback(StateCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.stateCbk = std::move(cbk);
}

const GLFW::Window::StateCallback& GLFW::Window::getStateCallback() const{
	return m_callbacks.stateCbk;
}



void GLFW::Window::setMonitor(const Monitor& mon){
	if(mon){
		setMonitor(mon, mon.getMode());
	}else {
		setMonitor(NO_MONITOR, Monitor::Mode());
	}
}

void GLFW::Window::setMonitor(const Monitor& mon, const Monitor::Mode& mode){
	getGLFW().m_mainThread->setMonitor(m_window, &m_windowedState, mon, mode);
}

GLFW::Monitor GLFW::Window::getMonitor() const{
	return getGLFW().m_mainThread->getMonitor(m_window);
}



void GLFW::Window::setPosition(const Math::Vec2i& pos){
	getGLFW().m_mainThread->setPosition(m_window, pos);
}

Math::Vec2i GLFW::Window::getPosition() const{
	return getGLFW().m_mainThread->getPosition(m_window);
}

void GLFW::Window::setPositionCallback(PositionCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.positionCbk = std::move(cbk);
}

const GLFW::Window::PositionCallback& GLFW::Window::getPositionCallback() const{
	return m_callbacks.positionCbk;
}



void GLFW::Window::setSize(const Math::Vec2i& size){
	getGLFW().m_mainThread->setSize(m_window, size);
}

Math::Vec2i GLFW::Window::getSize() const{
	return getGLFW().m_mainThread->getSize(m_window);
}

void GLFW::Window::setSizeCallback(SizeCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.sizeCbk = std::move(cbk);
}

const GLFW::Window::SizeCallback& GLFW::Window::getSizeCallback() const{
	return m_callbacks.sizeCbk;
}



void GLFW::Window::setOpacity(float opa){
	getGLFW().m_mainThread->setOpacity(m_window, opa);
}

float GLFW::Window::getOpacity() const{
	return getGLFW().m_mainThread->getOpacity(m_window);
}



Resolution GLFW::Window::getResolution() const{
	return getGLFW().m_mainThread->getResolution(m_window);
}

void GLFW::Window::setResolutionCallback(ResolutionCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.resolutionCbk = std::move(cbk);
}

const GLFW::Window::ResolutionCallback& GLFW::Window::getResolutionCallback() const{
	return m_callbacks.resolutionCbk;
}



Math::Vec2f GLFW::Window::getScale() const{
	return getGLFW().m_mainThread->getScale(m_window);
}

void GLFW::Window::setScaleCallback(ScaleCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.scaleCbk = std::move(cbk);
}

const GLFW::Window::ScaleCallback& GLFW::Window::getScaleCallback() const{
	return m_callbacks.scaleCbk;
}



void GLFW::Window::close(){
	getGLFW().m_mainThread->destroyWindow(m_window);
	m_window = nullptr;
}

bool GLFW::Window::shouldClose() const{
	return glfwWindowShouldClose(m_window); //Thread-safe GLFW function
}

void GLFW::Window::setCloseCallback(CloseCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.closeCbk = std::move(cbk);
}

const GLFW::Window::CloseCallback& GLFW::Window::getCloseCallback() const{
	return m_callbacks.closeCbk;
}



void GLFW::Window::focus(){
	getGLFW().m_mainThread->focus(m_window);
}

void GLFW::Window::setFocusCallback(FocusCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.focusCbk = std::move(cbk);
}

const GLFW::Window::FocusCallback& GLFW::Window::getFocusCallback() const{
	return m_callbacks.focusCbk;
}



void GLFW::Window::setRefreshCallback(RefreshCallback cbk){
	std::lock_guard<std::mutex> lock(GLFW::Callbacks::mutex);
	m_callbacks.refreshCbk = std::move(cbk);
}

const GLFW::Window::RefreshCallback& GLFW::Window::getRefreshCallback() const{
	return m_callbacks.refreshCbk;
}



void GLFW::Window::setDecorated(bool deco){
	getGLFW().m_mainThread->setDecorated(m_window, deco);
}

bool GLFW::Window::getDecorated() const{
	return getGLFW().m_mainThread->getDecorated(m_window);
}



void GLFW::Window::setResizeable(bool resizeable){
	getGLFW().m_mainThread->setResizeable(m_window, resizeable);
}

bool GLFW::Window::getResizeable() const{
	return getGLFW().m_mainThread->getResizeable(m_window);
}

}