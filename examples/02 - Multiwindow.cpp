/*
 * This example shows how multiple windows can be instantiated
 * 
 * How to compile:
 * c++ 02\ -\ Multiwindow.cpp -std=c++17 -Wall -Wextra -lzuazo -lzuazo-window -lglfw -ldl -lpthread
 */

#include "TestSource.h"

#include <zuazo/Instance.h>
#include <zuazo/Modules/Window.h>
#include <zuazo/Consumers/Window.h>
#include <zuazo/Signal/Delay.h>

#include <mutex>

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

	//Construct the window objects
	Zuazo::Consumers::Window window1(
		instance, 						//Instance
		"Output Window 1",				//Layout name
		videoMode,						//Video mode limits
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates)
		Zuazo::Consumers::Window::NO_MONITOR //No monitor
	);

	Zuazo::Consumers::Window window2(
		instance, 						//Instance
		"Output Window 2",				//Layout name
		videoMode,						//Video mode limits
		Zuazo::Math::Vec2i(1280, 720),	//Window size (in screen coordinates)
		Zuazo::Consumers::Window::NO_MONITOR //No monitor
	);


	//Open the windows (now becomes visible)
	window1.open();
	window2.open();

	//Let's route a test source to the first window
	const Zuazo::VideoMode testVideoMode(
		Zuazo::Utils::MustBe<Zuazo::Rate>(Zuazo::Rate(25, 1)), //Just specify the desired rate
		Zuazo::Utils::MustBe<Zuazo::Resolution>(Zuazo::Resolution(1280, 720)),
		Zuazo::Utils::MustBe<Zuazo::AspectRatio>(Zuazo::AspectRatio(1, 1)),
		Zuazo::Utils::MustBe<Zuazo::ColorPrimaries>(Zuazo::ColorPrimaries::BT709),
		Zuazo::Utils::MustBe<Zuazo::ColorModel>(Zuazo::ColorModel::RGB),
		Zuazo::Utils::MustBe<Zuazo::ColorTransferFunction>(Zuazo::ColorTransferFunction::IEC61966_2_1),
		Zuazo::Utils::MustBe<Zuazo::ColorSubsampling>(Zuazo::ColorSubsampling::RB_444),
		Zuazo::Utils::MustBe<Zuazo::ColorRange>(Zuazo::ColorRange::FULL_RGB),
		Zuazo::Utils::MustBe<Zuazo::ColorFormat>(Zuazo::ColorFormat::B8G8R8A8)	
	);

	TestSource testSrc(instance, "", testVideoMode);
	testSrc.open();
	window1 << Zuazo::Signal::getOutput<Zuazo::Video>(testSrc);

	//And the same signal delayed to the second
	Zuazo::Signal::Delay<Zuazo::Video> delayLine("Delay", Zuazo::Duration(std::chrono::seconds(1)));
	window2 << delayLine << Zuazo::Signal::getOutput<Zuazo::Video>(testSrc);

	const auto updateCallback = std::make_shared<Zuazo::Instance::ScheduledCallback>(
		[&instance, &delayLine] () {
			delayLine.update(instance.getTime());
		}
	);
	instance.addRegularCallback(updateCallback, Zuazo::Instance::PROCESSOR_PRIORITY);

	//Done!
	lock.unlock();
	getchar();
	lock.lock();

	instance.removeRegularCallback(updateCallback);
}