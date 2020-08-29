#pragma once

#include <zuazo/ZuazoBase.h>
#include <zuazo/Video.h>
#include <zuazo/ScalingMode.h>
#include <zuazo/ScalingFilter.h>
#include <zuazo/Math/Vector.h>
#include <zuazo/Utils/Pimpl.h>
#include <zuazo/Signal/Input.h>

#include <tuple>
#include <vector>

namespace Zuazo::Outputs{

class Window final
	: public ZuazoBase
	, public VideoBase
	, public VideoScalerBase
{
public:
	class EventSystem;
	class Monitor;

	enum class State {
		NORMAL,
		HIDDEN,
		FULLSCREEN,
		ICONIFIED,
		MAXIMIZED
	};

	using SizeCallback = std::function<void(Math::Vec2i)>;
	using PositionCallback = std::function<void(Math::Vec2i)>;
	using StateCallback = std::function<void(State)>;
	using ScaleCallback = std::function<void(Math::Vec2f)>;
	using FocusCallback = std::function<void(bool)>;
	using ShouldCloseCallback = std::function<void()>;

	struct Callbacks {
		SizeCallback				sizeCbk;
		PositionCallback			positionCbk;
		StateCallback				stateCbk;
		ScaleCallback				scaleCbk;
		FocusCallback				focusCbk;
		ShouldCloseCallback			shouldCloseCbk;
	};

	Window(	Instance& instance, 
			std::string name, 
			Math::Vec2i size = Math::Vec2i(640, 480),
			VideoMode videoMode = VideoMode::ANY,
			Callbacks cbks = {} );
	Window(const Window& other) = delete;
	Window(Window&& other);
	virtual ~Window();

	Window&						operator=(const Window& other) = delete;
	Window&						operator=(Window&& other);

	void						setWindowName(std::string name);
	const std::string&			getWindowName() const;

	void						setSize(Math::Vec2i size);
	Math::Vec2i					getSize() const;
	void						setSizeCallback(SizeCallback cbk);
	const SizeCallback&			getSizeCallback() const;

	void						setPosition(Math::Vec2i pos);
	Math::Vec2i					getPosition() const;
	void						setPositionCallback(PositionCallback cbk);
	const PositionCallback&		getPositionCallback() const;

	void						setState(State state);
	State						getState() const;
	void						setStateCallback(StateCallback cbk);
	const StateCallback&		getStateCallback() const;

	Math::Vec2f					getScale() const;
	void						setScaleCallback(ScaleCallback cbk);
	const ScaleCallback&		getScaleCallback() const;

	void						focus();
	void						setFocusCallback(FocusCallback cbk);
	const FocusCallback&		getFocusCallback() const;

	bool						shouldClose() const;
	void						setShouldCloseCallback(ShouldCloseCallback cbk);
	const ShouldCloseCallback&	getShouldCloseCallback() const;

	void						setOpacity(float opa);
	float						getOpacity() const;

	void						setResizeable(bool resizeable);
	bool						getResizeable() const;

	void						setDecorated(bool deco);
	bool						getDecorated() const;

	/*void						setMonitor(const Monitor& mon);
	void						setMonitor(const Monitor& mon, const Monitor::Mode& mode);
	Monitor						getMonitor() const;*/

	static const Monitor		NO_MONITOR;

	static void					init();
	static EventSystem&			getEventSystem();
	static std::vector<Monitor>	getMonitors();

private:
	struct Impl;
	Utils::Pimpl<Impl>			m_impl;

};



class Window::EventSystem {
public:


private:
	struct Impl;
	Utils::Pimpl<Impl>			m_impl;

};



class Window::Monitor {
public:


private:
	struct Impl;
	Utils::Pimpl<Impl>			m_impl;

};

}