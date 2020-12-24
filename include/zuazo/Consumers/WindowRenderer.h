#pragma once

#include <zuazo/Macros.h>
#include <zuazo/ZuazoBase.h>
#include <zuazo/RendererBase.h>
#include <zuazo/Video.h>
#include <zuazo/ScalingMode.h>
#include <zuazo/ScalingFilter.h>
#include <zuazo/Keyboard.h>
#include <zuazo/Math/Vector.h>
#include <zuazo/Utils/Pimpl.h>
#include <zuazo/Signal/ConsumerLayout.h>

#include <tuple>
#include <vector>
#include <mutex>

namespace Zuazo::Consumers{

class WindowRenderer final
	: public Utils::Pimpl<struct WindowRendererImpl>
	, public ZuazoBase
	, public VideoBase
	, public RendererBase
{
	friend WindowRendererImpl;
public:
	class Monitor;

	enum class State {
		NORMAL,
		HIDDEN,
		FULLSCREEN,
		ICONIFIED,
		MAXIMIZED
	};

	using SizeCallback = std::function<void(WindowRenderer&, Math::Vec2i)>;
	using PositionCallback = std::function<void(WindowRenderer&, Math::Vec2i)>;
	using StateCallback = std::function<void(WindowRenderer&, State)>;
	using ScaleCallback = std::function<void(WindowRenderer&, Math::Vec2f)>;
	using FocusCallback = std::function<void(WindowRenderer&, bool)>;
	using ShouldCloseCallback = std::function<void(WindowRenderer&)>;
	using KeyboardCallback = std::function<void(WindowRenderer&, KeyboardKey, KeyEvent, KeyModifiers)>;
	using CharacterCallback = std::function<void(WindowRenderer&, uint)>;
	using MouseButtonCallback = std::function<void(WindowRenderer&, MouseKey, KeyEvent, KeyModifiers)>;
	using MousePositionCallback = std::function<void(WindowRenderer&, Math::Vec2d)>;
	using MouseScrollCallback = std::function<void(WindowRenderer&, Math::Vec2d)>;
	using CursorEnterCallback = std::function<void(WindowRenderer&, bool)>;

	struct Callbacks {
		SizeCallback				sizeCbk;
		PositionCallback			positionCbk;
		StateCallback				stateCbk;
		ScaleCallback				scaleCbk;
		FocusCallback				focusCbk;
		ShouldCloseCallback			shouldCloseCbk;
		KeyboardCallback			keyboardCbk;
		CharacterCallback			characterCbk;
		MouseButtonCallback			mouseButtonCbk;
		MousePositionCallback		mousePositionCbk;
		MouseScrollCallback			mouseScrollCbk;
		CursorEnterCallback			cursorEnterCbk;
	};

	WindowRenderer(	Instance& instance, 
					std::string name,
					VideoMode videoMode = VideoMode::ANY,
					Utils::Limit<DepthStencilFormat> depthStencil = Utils::Any<DepthStencilFormat>(),
					Math::Vec2i size = Math::Vec2i(640, 480),
					const Monitor& mon = NO_MONITOR,
					Callbacks cbks = {} );
	WindowRenderer(const WindowRenderer& other) = delete;
	WindowRenderer(WindowRenderer&& other);
	virtual ~WindowRenderer();

	WindowRenderer&				operator=(const WindowRenderer& other) = delete;
	WindowRenderer&				operator=(WindowRenderer&& other);

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

	void						setMonitor(const Monitor& mon);
	Monitor						getMonitor() const;


	KeyEvent					getKeyState(KeyboardKey key) const;
	void						setKeyboardCallback(KeyboardCallback cbk);
	const KeyboardCallback&		getKeyboardCallback() const;

	void						setCharacterCallback(CharacterCallback cbk);
	const CharacterCallback&	getCharacterCallback() const;

	KeyEvent					getMouseButtonState(MouseKey but) const;
	void						setMouseButtonCallback(MouseButtonCallback cbk);
	const MouseButtonCallback&	getMouseButtonCallback() const;
	
	Math::Vec2d					getMousePosition() const;
	void						setMousePositionCallback(MousePositionCallback cbk);
	const MousePositionCallback& getMousePositionCallback() const;

	void						setMouseScrollCallback(MouseScrollCallback cbk);
	const MouseScrollCallback& 	getMouseScrollCallback() const;

	void						setCursorEnterCallback(CursorEnterCallback cbk);
	const CursorEnterCallback& 	getCursorEnterCallback() const;

	static const Monitor		NO_MONITOR;

	static Monitor				getPrimaryMonitor();
	static std::vector<Monitor>	getMonitors();

};


class WindowRenderer::Monitor {
	friend WindowRendererImpl;
public:
	Monitor();
	Monitor(const Monitor& other) = delete;
	Monitor(Monitor&& other);
	~Monitor();

	Monitor&					operator=(const Monitor& other) = delete;
	Monitor&					operator=(Monitor&& other);

	std::string_view			getName() const;
	Math::Vec2d					getPhysicalSize() const;
	Math::Vec2i					getSize() const;
	Math::Vec2i					getPosition() const;
	Rate						getFrameRate() const;

private:
	struct Impl;
	Utils::Pimpl<Impl>			m_impl;

	Monitor(Utils::Pimpl<Impl> pimpl);

};

}