/**
 * @internal
 *
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
#ifndef SDLGAMEPAD_H
#define SDLGAMEPAD_H

/**********************************************************************/
#include <SDL/SDL.h>
#undef main

/**********************************************************************/
#include <QThread>
#include "sdlgamepad_global.h"

/**
 * The Axis range that is treated as null.
 *
 * This is a sort of callibration mechanism for the physical gamepad.
 * SDL axis values greater than -NULL_RANGE and smaller than +NULL_RANGE
 * will be treated as null.
 */
#define NULL_RANGE 2800

/**
 * The default tick rate of refreshing the SDL info.
 *
 * This is the default ms value in the thread method to sleep. If you
 * dont set a sleep rate you processor will have a much higher load!
 *
 * @see SDLGamepad::setTickRate()
 */
#define MIN_RATE 10

/**
 * Axis enumeration.
 *
 * This enumeration can be used to have a more readable code for
 * dealing with axes numbers. Up to 10 axes are supported. That should
 * be more than enough for every gamepad out there.
 */
enum AxisNumber
{
  AXIS_1,
  AXIS_2,
  AXIS_3,
  AXIS_4,
  AXIS_5,
  AXIS_6,
  AXIS_7,
  AXIS_8,
  AXIS_9
};

/**
 * Button enumeration.
 *
 * This enumeration is uesed internaly in the singal for the buttons.
 * Be aware that you need to register this enumeration with
 * qRegisterMetaType when you want to use the signal buttonState with
 * queued connections.
 *
 * @see SDLGamepad::buttonState()
 */
enum ButtonNumber
{
  BUTTON_01,
  BUTTON_02,
  BUTTON_03,
  BUTTON_04,
  BUTTON_05,
  BUTTON_06,
  BUTTON_07,
  BUTTON_08,
  BUTTON_09,
  BUTTON_10,
  BUTTON_11,
  BUTTON_12,
  BUTTON_13,
  BUTTON_14,
  BUTTON_15,
  BUTTON_16,
  BUTTON_17,
  BUTTON_18,
  BUTTON_19,
  BUTTON_20
};

/**
 * A typedef for qRegisterMetaType().
 *
 * This typedef is used to int the signal axesValues. You need this if
 * you want to use the signal with queued connections, because you
 * can not register QList<qint16> directly.
 *
 * @see SDLGamepad::axisValues()
 */
typedef QList<qint16> QListInt16;

/**
 * A class for communication with a sdl gamepad.
 *
 * This class inherts QThread. The run method will look for new button
 * states via SDL function calls and emit signals for every button if
 * the state of the button has changed. It will also emit signals with
 * the current axes values. The default sleep after this two tasks is
 * 10 milliseconds.
 *
 * @author Manuel Blanquett (mail.nalla@gmail.com)
 * @version 1.0
 * @date 2009
 */
class SDLGAMEPADSHARED_EXPORT SDLGamepad : public QThread
{
  Q_OBJECT

  public:

    /**
     * Class constructor.
     *
     * Some private variables will be initzialized here and the tick
     * rate will be set to MIN_RATE.
     */
    SDLGamepad();

    /**
     * Class deconstructor.
     *
     * The SDL gamepad will be cracefully closed. After that the SDL
     * system will be terminated.
     */
    ~SDLGamepad();

    /**
     * Implemented virtual run method from QThread.
     *
     * The base of operation so to speak. Very abstract this method
     * does the following:
     * - refresh SDL information
     * - emit signals
     * - sleep tickrate
     */
    void run();

    /**
     * Getter method for the axis count.
     *
     * This class member retunrs the actual number of axes present
     * on the currently opened gamepad. If no gamepad is present after
     * setting a new one with setGamepad() the value will be -1. This
     * is also the default value.
     */
    qint16 getAxes();

    /**
     * Getter method for the button count.
     *
     * This class member retunrs the actual number of buttons present
     * on the currently opened gamepad. If no gamepad is present after
     * setting a new one with setGamepad() the value will be -1. This
     * is also the default value.
     */
    qint16 getButtons();

  public slots:

    /**
     * Init the SDL system and set up gamepad.
     *
     * You have to connect to the gamepads signal if you want to
     * receive the gamepads present in the SDL system.
     *
     * @see gamepads()
     * @return True if the initzialisation was successfull.
     */
    bool init();

    /**
     * Exiting the thread.
     *
     * This is a overwritten method from QThread to gracefully end the
     * execution of this thread. You should connect the QT App's
     * aboutToQuit signal with this.
     */
    void quit();

    /**
     * Change the default tickrate.
     *
     * You can change the tickrate at any time. So you can change the
     * thread sleep dynamicly accordingly to your application's state.
     *
     * @param ms The new tickrate in milliseconds.
     */
    void setTickRate(qint16 ms);

    /**
     * Change the active gamepad.
     *
     * You can set active gamepad here. The first gamepad to be
     * activaed is 0. If the gamepad could not be activated a qCritical
     * will be printed and false is returned.
     *
     * @see gamepads()
     * @param index The new gamepad by index.
     * @return True if the gamepad was successfully changed.
     */
    bool setGamepad(qint16 index);

  private:

    /**
     * Variable to control thread.
     *
     * This class member variable is false at construction time. If
     * the sdl init was successfull it will be set to true. The quit
     * slot will false it again.
     *
     * @see quit()
     */
    bool loop;

    /**
     * Get new axes information from the SDL system.
     *
     * This class member is called from the run method to ask the SDL
     * system for new axes values. Afterwords those values are emitted
     * via the axesValues signal.
     *
     * @see run()
     * @see axesValues()
     */
    void updateAxes();

    /**
     * Get new button information from the SDL system.
     *
     * This class member is called from the run method to ask the SDL
     * system for new button states. If changed, the states are will be
     * emitted via the buttonState signal.
     *
     * @see run()
     * @see buttonState()
     */
    void updateButtons();

    /**
     * Number of buttons.
     *
     * If a new gamepad is opened via SDL the acutal button count is
     * saved in this class member to be used in control statements.
     */
    qint16 buttons;

    /**
     * Number of axes.
     *
     * If a new gamepad is opened via SDL the acutal axes count is
     * saved in this class member to be used in control statements.
     */
    qint16 axes;

    /**
     * The tickrate.
     *
     * A variable to be used in the run method to sleep after getting
     * and emitting SDL information.
     *
     * @see run()
     */
    qint16 tick;

    /**
     * SDL_Joystick index.
     *
     * A variable that holds the SDL_Joystick index of the currently
     * opend SDL_Joystick Object.
     */
    qint16 index;

    /**
     * SDL_Joystick object.
     *
     * This represents the currently opend SDL_Joystick object.
     */
    SDL_Joystick *gamepad;

    /**
     * A QList to store the current button states.
     *
     * This list stores the current states of all avaliable buttons.
     */
    QList<qint16> buttonStates;

  signals:

    /**
     * A signal that emitts the number of gamepads present in SDL.
     *
     * This signal is emitted in the init slot, so you need connect to
     * this bevore you call init! You get a quint8 with the number of
     * gamepads currently present in the SDL system. The first gamepad
     * will be opened at default. You can set another gamepad with the
     * setGamepad slot. If you have 5 gamepads in the system you need
     * to use 0-4 in the setGamepad slot.
     *
     * @see setGamepad()
     * @param count The number of gamepads present in SDL.
     */
    void gamepads(quint8 count);

    /**
     * A signal that emitts the current state of a gamepad button.
     *
     * You can connect to this signal to receive the state of the
     * gamepad buttons. The states are not pressed at default. If the
     * first pressed state will occur within SDL you will get a signal.
     * You will get another signal if you release the button. With
     * every tick the states will be reread from the SDL system.
     *
     * @param number A ButtonNumber enum value.
     * @param pressed A bool value wether the button is pressed or not.
     */
    void buttonState(ButtonNumber number, bool pressed);

    /**
     * A signal that emitts the current values of the gamepad axes.
     *
     * You can connect to this signal to receive the values of the
     * gamepad axes. Unlike the button signal, this signal is thrown
     * with every tick. You will get a QListInt16 containing the value
     * of every present axis in a QList.
     *
     * @see QListInt16
     * @param values A QListInt16 Type containing all axes values.
     */
    void axesValues(QListInt16 values);
};

/**********************************************************************/
#endif // SDLGAMEPAD_H
