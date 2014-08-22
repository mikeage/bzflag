/* bzflag
 * Copyright (c) 1993-2013 Tim Riker
 *
 * This package is free software;  you can redistribute it and/or
 * modify it under the terms of the license found in the file
 * named COPYING that should have accompanied this file.
 *
 * THIS PACKAGE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 */

// Own include
#include "SDL2Window.h"

// Common includes
#include "OpenGLGState.h"

SDLWindow::SDLWindow(const SDLDisplay* _display, SDLVisual*)
  : BzfWindow(_display), hasGamma(true), windowId(NULL), glContext(NULL),
  canGrabMouse(true), fullScreen(false), oldFullScreen(false), oldWidth(0), oldHeight(0),
  base_width(640), base_height(480)
{
}

void SDLWindow::setTitle(const char *_title) {
  title = _title;
  if (windowId)
    SDL_SetWindowTitle(windowId, title.c_str());
}

void SDLWindow::setFullscreen(bool on) {
  const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->setFullscreen(on);
  fullScreen = on;
}

void SDLWindow::iconify(void) {
  SDL_MinimizeWindow(windowId);
}

void SDLWindow::warpMouse(int _x, int _y) {
  SDL_WarpMouseInWindow(windowId, _x, _y);
}

void SDLWindow::getMouse(int& _x, int& _y) const {
  const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->getMouse(_x, _y);
}

void SDLWindow::setSize(int _width, int _height) {
  base_width  = _width;
  base_height = _height;
  if (!fullScreen && windowId) {
    SDL_SetWindowSize(windowId, base_width, base_height);
  }
}

void SDLWindow::getSize(int& width, int& height) const {
  if (fullScreen) {
    const_cast<SDLDisplay *>((const SDLDisplay *)getDisplay())->getWindowSize(width, height);
  } else {
    width  = base_width;
    height = base_height;
  }
}

void SDLWindow::setGamma(float gamma) {
  int result = SDL_SetWindowBrightness(windowId, gamma);
  if (result == -1) {
    printf("Could not set Gamma: %s.\n", SDL_GetError());
    hasGamma = false;
  }
}

float SDLWindow::getGamma() const {
  return SDL_GetWindowBrightness(windowId);
}

bool SDLWindow::hasGammaControl() const {
  return hasGamma;
}

void SDLWindow::swapBuffers() {
  SDL_GL_SwapWindow(windowId);
}

bool SDLWindow::create(void) {
  int    width;
  int    height;
  Uint32 flags = SDL_WINDOW_OPENGL;
  // getting width, height & flags for SetVideoMode
  getSize(width, height);
  if (fullScreen) {
    flags |= SDL_WINDOW_FULLSCREEN;
  } else {
    flags |= SDL_WINDOW_RESIZABLE;
  }

  // if they are the same, don't bother building a new window
  if ((width == oldWidth) && (height == oldHeight)
      && (fullScreen == oldFullScreen))
    return true;

  // save the values for the next
  oldWidth      = width;
  oldHeight     = height;
  oldFullScreen = fullScreen;
  // always disable vsync when building with SDL
  SDL_GL_SetSwapInterval(0);
  // Set the video mode and hope for no errors
  windowId = SDL_CreateWindow(title.c_str(),
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, flags);
  if (!windowId) {
    printf("Could not set Video Mode: %s.\n", SDL_GetError());
    return false;
  }
  makeContext();
  makeCurrent();
  // init opengl context
  OpenGLGState::initContext();
  return true;
}

void SDLWindow::enableGrabMouse(bool on) {
  canGrabMouse = on;
  if (canGrabMouse)
    SDL_SetWindowGrab(windowId, SDL_TRUE);
  else
    SDL_SetWindowGrab(windowId, SDL_FALSE);
}

void SDLWindow::makeContext() {
  glContext = SDL_GL_CreateContext(windowId);
  if (!glContext)
    printf("Could not Create GL Context: %s.\n", SDL_GetError());
}

void SDLWindow::makeCurrent() {
  if (!windowId)
    return;
  if (!glContext)
    return;
  int result = SDL_GL_MakeCurrent(windowId, glContext);
  if (result < 0) {
    printf("Could not Make GL Context Current: %s.\n", SDL_GetError());
    abort();
  }
}

void SDLWindow::freeContext() {
  SDL_GL_DeleteContext(glContext);
}

// Local Variables: ***
// mode:C++ ***
// tab-width: 8 ***
// c-basic-offset: 2 ***
// indent-tabs-mode: t ***
// End: ***
// ex: shiftwidth=2 tabstop=8
