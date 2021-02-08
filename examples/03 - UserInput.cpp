/*
 * This example shows how to get notified when the user performs a change
 * 
 * How to compile:
 * c++ 03\ -\ UserInput.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lglfw -ldl -lpthread
 */

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Consumers/WindowRenderer.h>

#include <mutex>
#include <iostream>

int main() {
	//Instantiate Zuazo as usual. Note that we're loading the Window module
	Zuazo::Instance::ApplicationInfo appInfo(
		"Window Example 03",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	//Construct the desired parameters
	const Zuazo::VideoMode videoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::Any<Zuazo::Resolution>(),
		Zuazo::Utils::Any<Zuazo::AspectRatio>(),
		Zuazo::Utils::Any<Zuazo::ColorPrimaries>(),
		Zuazo::Utils::Any<Zuazo::ColorModel>(),
		Zuazo::Utils::Any<Zuazo::ColorTransferFunction>(),
		Zuazo::Utils::Any<Zuazo::ColorSubsampling>(),
		Zuazo::Utils::Any<Zuazo::ColorRange>(),
		Zuazo::Utils::Any<Zuazo::ColorFormat>()	
	);

	const Zuazo::Utils::Limit<Zuazo::DepthStencilFormat> depthStencil(
		Zuazo::Utils::MustBe<Zuazo::DepthStencilFormat>(Zuazo::DepthStencilFormat::NONE) //Not interested in the depth buffer
	);

	const auto windowSize = Zuazo::Math::Vec2i(1280, 720);

	const auto& monitor = Zuazo::Consumers::WindowRenderer::NO_MONITOR; //Not interested in the full-screen mode

	//Create a callback struct containing all callbacks. They may be modified on the go, so dont hurry
	Zuazo::Consumers::WindowRenderer::Callbacks callbacks {
		//Size Callback
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::Math::Vec2i size) {
			std::cout << win.getName() << " resized: " << Zuazo::Resolution(size) << std::endl;
		},

		//Position Callback
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::Math::Vec2i pos) {
			std::cout << win.getName() << " moved: " << "[" << pos.x << ", " << pos.y << "]" << std::endl;
		},
		
		//Iconify Callback
		[] (Zuazo::Consumers::WindowRenderer& win, bool iconified) {
			std::cout << win.getName() << " " << (iconified ? "iconified" : "not iconified") << std::endl;
		},

		//Maximize Callback
		[] (Zuazo::Consumers::WindowRenderer& win, bool maximized) {
			std::cout << win.getName() << " " << (maximized ? "maximized" : "not maximized") << std::endl;
		},

		//Focus Callback
		[] (Zuazo::Consumers::WindowRenderer& win, bool focused) {
			std::cout << win.getName() << " " << (focused ? "focused" : "not focused") << std::endl;
		},

		//Scale Callback
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::Math::Vec2f scale) {
			std::cout << win.getName() << " changed scale: " << "[" << scale.x << ", " << scale.y << "]" << std::endl;
		},
		
		//ShouldClose Callback
		[] (Zuazo::Consumers::WindowRenderer& win) {
			std::cout << win.getName() << " should close" << std::endl;
		},

		//Keyboard Callback
		Zuazo::Consumers::WindowRenderer::KeyboardCallback(), //Leave it empty

		//Character Callback
		[] (Zuazo::Consumers::WindowRenderer&, uint character) {
			std::cout << static_cast<char>(character);
		},

		//Mouse Button Callback
		Zuazo::Consumers::WindowRenderer::MouseButtonCallback(), //Leave it empty

		//Mouse Position Callback
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::Math::Vec2d pos) {
			std::cout << win.getName() << " mouse moved to: " << "[" << pos.x << ", " << pos.y << "]" << std::endl;
		},

		//Mouse Scroll Callback
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::Math::Vec2d delta) {
			std::cout << win.getName() << " mouse scrolled: " << "[" << delta.x << ", " << delta.y << "]" << std::endl;
		},

		//Cursor Enter Callback
		[] (Zuazo::Consumers::WindowRenderer& win, bool entered) {
			std::cout << win.getName() << " cursor " << ( entered ? "entered" : "exited" ) << std::endl;
		},
	};

	//Construct the window object
	Zuazo::Consumers::WindowRenderer window(
		instance, 						//Instance
		"Output Window",				//Layout name
		videoMode,						//Video mode limits
		depthStencil,					//Depth buffer limits
		windowSize,						//Window size (in screen coordinates)
		monitor,						//Monitor for setting fullscreen
		std::move(callbacks)
	);

	//Open the window (now becomes visible)
	window.asyncOpen(lock);

	//Just another way to set callbacks:
	window.setKeyboardCallback(
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::KeyboardKey key, Zuazo::KeyEvent event, Zuazo::KeyModifiers mod) {
			std::cout << win.getName() << " " << key << " key " << event << " modifier keys: " << static_cast<int>(mod) << std::endl;
		}
	);

	window.setMouseButtonCallback(
		[] (Zuazo::Consumers::WindowRenderer& win, Zuazo::MouseKey key, Zuazo::KeyEvent event, Zuazo::KeyModifiers mod) {
			std::cout << win.getName() << " mouse " << key << " button " << event << " modifier keys: " << static_cast<int>(mod) << std::endl;
		}
	);

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}