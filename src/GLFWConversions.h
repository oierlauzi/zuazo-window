#pragma once

#include "GLFW/Instance.h"

#include <zuazo/Keyboard.h>

namespace Zuazo {

constexpr GLFW::KeyboardKey toGLFW(KeyboardKey key);
constexpr KeyboardKey fromGLFW(GLFW::KeyboardKey key);

constexpr GLFW::MouseButton toGLFW(MouseKey key);
constexpr MouseKey fromGLFW(GLFW::MouseButton key);

constexpr GLFW::KeyModifiers toGLFW(KeyModifiers modifiers);
constexpr KeyModifiers fromGLFW(GLFW::KeyModifiers modifiers);

constexpr GLFW::KeyEvent toGLFW(KeyEvent event);
constexpr KeyEvent fromGLFW(GLFW::KeyEvent event);

}

#include "GLFWConversions.inl"