/**
 * This file is part of SDLGamepad.
 *
 * SDLGamepad is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SDLGamepad is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Manuel Blanquett
 * mail.nalla@gmail.com
 */

/**********************************************************************/
#include "sdlgamepad.h"

/**********************************************************************/
SDLGamepad::SDLGamepad()
{
  buttons = -1;
  axes = -1;
  index = -1;
  loop = false;
  tick = MIN_RATE;
  gamepad = 0;
}

/**********************************************************************/
SDLGamepad::~SDLGamepad()
{
  loop = false;

  if(gamepad)
    SDL_JoystickClose(gamepad);

  SDL_Quit();
}

/**********************************************************************/
bool SDLGamepad::init()
{
  if(SDL_Init(SDL_INIT_JOYSTICK) < 0)
    return false;

  if(SDL_NumJoysticks() > 0)
  {
    emit gamepads(SDL_NumJoysticks());

    if(!setGamepad(0))
      return false;

    for(qint8 i = 0; i < buttons; i++)
      buttonStates.append(0);
  }
  else
  {
    return false;
  }

  loop = true;
  return true;
}

/**********************************************************************/
void SDLGamepad::run()
{
  while(loop)
  {
    updateAxes();
    updateButtons();
    msleep(tick);
  }
}

/**********************************************************************/
bool SDLGamepad::setGamepad(qint16 index)
{
  if (index != this->index)
  {
    if(SDL_JoystickOpened(this->index))
      SDL_JoystickClose(gamepad);

    gamepad = SDL_JoystickOpen(index);

    if(gamepad)
    {
      buttons = SDL_JoystickNumButtons(gamepad);
      axes = SDL_JoystickNumAxes(gamepad);

      if (axes >= 4) {
	this->index = index;
	return true;
      }
      else
      {
	buttons = -1;
	axes = -1;
	this->index = -1;
	qCritical("Gamepad has less than 4 axes");
      }
    }
    else
    {
      buttons = -1;
      axes = -1;
      this->index = -1;
      qCritical("Unable to open Gamepad!");
    }
  }

  return false;
}

/**********************************************************************/
void SDLGamepad::setTickRate(qint16 ms)
{
  tick = ms;
}

/**********************************************************************/
void SDLGamepad::updateAxes()
{
  if(gamepad)
  {
    QListInt16 values;
    SDL_JoystickUpdate();

    for(qint8 i = 0; i < axes; i++)
    {
      qint16 value = SDL_JoystickGetAxis(gamepad, i);

      if(value > -NULL_RANGE && value < NULL_RANGE)
        value = 0;

      values.append(value);
    }

    emit axesValues(values);
  }
}

/**********************************************************************/
void SDLGamepad::updateButtons()
{
  if(gamepad)
  {
    SDL_JoystickUpdate();

    for(qint8 i = 0; i < buttons; i++)
    {
      qint16 state = SDL_JoystickGetButton(gamepad, i);

      if(buttonStates.at(i) != state)
      {
        if(state > 0)
          emit buttonState((ButtonNumber)i, true);
        else
          emit buttonState((ButtonNumber)i, false);

        buttonStates.replace(i, state);
      }
    }
  }
}

/**********************************************************************/
void SDLGamepad::quit()
{
  loop = false;
}

/**********************************************************************/
qint16 SDLGamepad::getAxes()
{
  return axes;
}

/**********************************************************************/
qint16 SDLGamepad::getButtons()
{
  return buttons;
}
