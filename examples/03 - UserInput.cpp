/*
 * This example shows how to get notified when the user performs a change
 * 
 * How to compile:
 * c++ 03\ -\ UserInput.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lglfw -ldl -lpthread
 */

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Outputs/Window.h>

#include <mutex>
#include <iostream>

int main() {
	//Instantiate Zuazo as usual
	Zuazo::Instance::ApplicationInfo appInfo(
		"Window Example 00",						//Application's name
		Zuazo::Version(0, 1, 0),					//Application's version
		Zuazo::Verbosity::GEQ_INFO,					//Verbosity 
		{ Zuazo::Modules::Window::get() }			//Modules
	);
	Zuazo::Instance instance(std::move(appInfo));
	std::unique_lock<Zuazo::Instance> lock(instance);

	//Construct the desired video mode
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

	//Create a callback struct containing all callbacks. They may be modified on the go, so dont hurry
	Zuazo::Outputs::Window::Callbacks callbacks {
		//Size Callback
		[] (Zuazo::Outputs::Window& win, Zuazo::Math::Vec2i size) {
			std::cout << win.getName() << " resized: " << Zuazo::toString(size) << std::endl;
		},

		//Position Callback
		[] (Zuazo::Outputs::Window& win, Zuazo::Math::Vec2i pos) {
			std::cout << win.getName() << " moved: " << Zuazo::toString(pos) << std::endl;
		},
		
		//State Callback
		[] (Zuazo::Outputs::Window& win, Zuazo::Outputs::Window::State state) {
			std::cout << win.getName() << " switched to state: " << Zuazo::toString(state) << std::endl;
		},

		//Scale Callback
		[] (Zuazo::Outputs::Window& win, Zuazo::Math::Vec2f scale) {
			std::cout << win.getName() << " changed scale: " << Zuazo::toString(scale) << std::endl;
		},

		//Focus Callback
		[] (Zuazo::Outputs::Window& win, bool focus) {
			std::cout << win.getName() << ( focus ? " has focus" : " has *NOT* focus" ) << std::endl;
		},

		//ShouldClose Callback
		[] (Zuazo::Outputs::Window& win) {
			std::cout << win.getName() << " should close" << std::endl;
		},

		//Keyboard Callback
		Zuazo::Outputs::Window::KeyboardCallback(), //Leave it empty

		//Character Callback
		[] (Zuazo::Outputs::Window&, uint character) {
			std::cout << static_cast<char>(character);
		},

		//Mouse Button Callback
		Zuazo::Outputs::Window::MouseButtonCallback(), //Leave it empty

		//Mouse Position Callback
		[] (Zuazo::Outputs::Window& win, Zuazo::Math::Vec2d pos) {
			std::cout << win.getName() << " mouse moved to: " << Zuazo::toString(pos) << std::endl;
		},

		//Mouse Scroll Callback
		[] (Zuazo::Outputs::Window& win, Zuazo::Math::Vec2d delta) {
			std::cout << win.getName() << " mouse scrolled: " << Zuazo::toString(delta) << std::endl;
		},

		//Cursor Enter Callback
		[] (Zuazo::Outputs::Window& win, bool entered) {
			std::cout << win.getName() << " cursor " << ( entered ? "entered" : "exited" ) << std::endl;
		},
	};

	//Construct the window object
	Zuazo::Outputs::Window window(
		instance, 						//Instance
		"Output Window",				//Layout name
		videoMode,						//Video mode limits
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates)
		Zuazo::Outputs::Window::NO_MONITOR, //No monitor
		std::move(callbacks)
	);

	//Open the window (now becomes visible)
	window.open();

	//Just another way to set callbacks:
	window.setKeyboardCallback(
		[] (Zuazo::Outputs::Window& win, Zuazo::Outputs::Window::KeyboardKey key, Zuazo::Outputs::Window::KeyboardEvent event, Zuazo::Outputs::Window::KeyboardModifiers mod) {
			std::cout << win.getName() << " " << Zuazo::toString(key) << " key " << Zuazo::toString(event) << " modifier keys: " << Zuazo::toString(mod) << std::endl;
		}
	);

	window.setMouseButtonCallback(
		[] (Zuazo::Outputs::Window& win, Zuazo::Outputs::Window::MouseButton but, Zuazo::Outputs::Window::KeyboardEvent event, Zuazo::Outputs::Window::KeyboardModifiers mod) {
			std::cout << win.getName() << " mouse " << Zuazo::toString(but) << " button " << Zuazo::toString(event) << " modifier keys: " << Zuazo::toString(mod) << std::endl;
		}
	);

	//Done!
	lock.unlock();
	getchar();
	lock.lock();
}