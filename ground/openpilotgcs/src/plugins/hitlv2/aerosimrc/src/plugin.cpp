/**
 ******************************************************************************
 *
 * @file       plugin.cpp
 * @author     The OpenPilot Team, http://www.openpilot.org Copyright (C) 2010-2012.
 * @addtogroup 3rdParty Third-party integration
 * @{
 * @addtogroup AeroSimRC AeroSimRC proxy plugin
 * @{
 * @brief AeroSimRC simulator to HITL proxy plugin
 *****************************************************************************/
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#include "plugin.h"
#include "udpconnect.h"
#include "qdebughandler.h"
#include "enums.h"
#include "settings.h"

bool isFirstRun = true;
QString debugInfo(DBG_BUFFER_MAX_SIZE, ' ');
QString pluginFolder(MAX_PATH, ' ');
QString outputFolder(MAX_PATH, ' ');

QList<quint16> videoModes;
QTime ledTimer;

UdpSender *sndr;
UdpReceiver *rcvr;

const float RAD2DEG = (float)(180.0 / M_PI);
const float DEG2RAD = (float)(M_PI / 180.0);

//extern "C" int __stdcall DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
extern "C" int __stdcall DllMain(void*, quint32 fdwReason, void*)
{
    switch (fdwReason) {
    case 0:
//        qDebug() << hinstDLL << "DLL_PROCESS_DETACH " << lpvReserved;
// free resources here
        rcvr->stop();
        rcvr->wait(500);
        delete rcvr;
        delete sndr;
        qDebug("------");
        break;
    case 1:
//        qDebug() << hinstDLL << " DLL_PROCESS_ATTACH " << lpvReserved;
        break;
    case 2:
//        qDebug() << hinstDLL << "DLL_THREAD_ATTACH " << lpvReserved;
        break;
    case 3:
//        qDebug() << hinstDLL << "DLL_THREAD_DETACH " << lpvReserved;
        break;
    }
    return true;
}

SIM_DLL_EXPORT void AeroSIMRC_Plugin_ReportStructSizes(quint32 *sizeSimToPlugin,
                                                       quint32 *sizePluginToSim,
                                                       quint32 *sizePluginInit)
{
    // debug redirection
    qInstallMsgHandler(myQDebugHandler);

    qDebug() << "AeroSIMRC_Plugin_ReportStructSizes";
    *sizeSimToPlugin = sizeof(simToPlugin);
    *sizePluginToSim = sizeof(pluginToSim);
    *sizePluginInit  = sizeof(pluginInit);
    qDebug() << "sizeSimToPlugin = " << *sizeSimToPlugin;
    qDebug() << "sizePluginToSim = " << *sizePluginToSim;
    qDebug() << "sizePluginInit  = " << *sizePluginInit;
}

SIM_DLL_EXPORT void AeroSIMRC_Plugin_Init(pluginInit *p)
{
    qDebug() << "AeroSIMRC_Plugin_Init begin";

    pluginFolder = p->strPluginFolder;
    outputFolder = p->strOutputFolder;

    ledTimer.restart();

    Settings *ini = new Settings(pluginFolder);
    ini->read();

    videoModes = ini->getVideoModes();

    sndr = new UdpSender(ini->getOutputMap(), ini->isFromTX());
    sndr->init(ini->remoteHost(), ini->remotePort());

    rcvr = new UdpReceiver(ini->getInputMap(), ini->isToRX());
    rcvr->init(ini->localHost(), ini->localPort());

    // run thread
    rcvr->start();

    delete ini;

    qDebug() << "AeroSIMRC_Plugin_Init done";
}

//-----------------------------------------------------------------------------

void Run_Command_Reset(/*const simToPlugin *stp,
                               pluginToSim *pts*/)
{
    // Print some debug info, although it will only be seen during one frame
    debugInfo.append("\nRESET");
}

void Run_Command_WindowSizeAndPos(const simToPlugin *stp,
                                        pluginToSim *pts)
{
    static quint8 snSequence = 0;
    quint8 idx = snSequence * 4;

    if (snSequence >= videoModes.at(0)) {   // set fullscreen
        pts->newScreenX = 0;
        pts->newScreenY = 0;
        pts->newScreenW = stp->screenW;
        pts->newScreenH = stp->screenH;
        snSequence = 0;
    } else {                                // set video mode from config
        pts->newScreenX = videoModes.at(idx + 1);
        pts->newScreenY = videoModes.at(idx + 2);
        pts->newScreenW = videoModes.at(idx + 3);
        pts->newScreenH = videoModes.at(idx + 4);
        snSequence++;
    }
}

void Run_Command_MoveToNextWaypoint(const simToPlugin *stp,
                                          pluginToSim *pts)
{
    static quint8 snSequence = 0;

    switch(snSequence) {
    case 0:
        pts->newPosX = stp->wpAX;
        pts->newPosY = stp->wpAY;
        pts->newPosZ = 100.0;
        break;
    case 1:
        pts->newPosX = stp->wpBX;
        pts->newPosY = stp->wpBY;
        pts->newPosZ = 100.0;
        break;
    case 2:
        pts->newPosX = stp->wpCX;
        pts->newPosY = stp->wpCY;
        pts->newPosZ = 100.0;
        break;
    case 3:
        pts->newPosX = stp->wpDX;
        pts->newPosY = stp->wpDY;
        pts->newPosZ = 100.0;
        break;
    case 4:
        pts->newPosX = stp->wpHomeX;
        pts->newPosY = stp->wpHomeY;
        pts->newPosZ = 100.0;
        break;
    default:
        qFatal("Run_Command_MoveToNextWaypoint switch error");
    }
    pts->modelOverrideFlags = 0;
    pts->modelOverrideFlags |= OVR_POS;

    snSequence++;
    if(snSequence > 4)
        snSequence = 0;
}

void Run_BlinkLEDs(const simToPlugin *stp,
                         pluginToSim *pts)
{
    if ((stp->simMenuStatus & MenuEnable) != 0) {
        pts->newMenuStatus |= MenuLedGreen;
        int timeout;
        quint8 armed;
        quint8 mode;
        rcvr->getFlighStatus(armed, mode);
        debugInfo.append(QString("armed: %1, mode: %2\n").arg(armed).arg(mode));

        if (armed == 0)         // disarm
            timeout = 1000;
        else if (armed == 1)    // arming
            timeout = 40;
        else if (armed == 2)    // armed
            timeout = 100;
        else                    // unknown
            timeout = 2000;
        if (ledTimer.elapsed() > timeout) {
            ledTimer.restart();
            pts->newMenuStatus ^= MenuLedBlue;
        }

        if (mode == 6) {
            pts->newMenuStatus |= MenuFMode3;
            pts->newMenuStatus |= MenuFMode2;
            pts->newMenuStatus |= MenuFMode1;
        } else if (mode == 5) {
            pts->newMenuStatus |= MenuFMode3;
            pts->newMenuStatus |= MenuFMode2;
            pts->newMenuStatus &= ~MenuFMode1;
        } else if (mode == 4) {
            pts->newMenuStatus |= MenuFMode3;
            pts->newMenuStatus &= ~MenuFMode2;
            pts->newMenuStatus |= MenuFMode1;
        } else if (mode == 3) {
            pts->newMenuStatus |= MenuFMode3;
            pts->newMenuStatus &= ~MenuFMode2;
            pts->newMenuStatus &= ~MenuFMode1;
        } else if (mode == 2) {
            pts->newMenuStatus &= ~MenuFMode3;
            pts->newMenuStatus |= MenuFMode2;
            pts->newMenuStatus &= ~MenuFMode1;
        } else if (mode == 1) {
            pts->newMenuStatus &= ~MenuFMode3;
            pts->newMenuStatus &= ~MenuFMode2;
            pts->newMenuStatus |= MenuFMode1;
        } else /*(mode == 0)*/ {
            pts->newMenuStatus &= ~MenuFMode3;
            pts->newMenuStatus &= ~MenuFMode2;
            pts->newMenuStatus &= ~MenuFMode1;
        }
    } else {
        pts->newMenuStatus = 0;
    }
}

void InfoText(const simToPlugin *stp,
                    pluginToSim *pts)
{
    debugInfo.append(
                QString(
                    "Plugin Folder = %1\n"
                    "Output Folder = %2\n"
                    "nStructSize = %3  "
                    "fIntegrationTimeStep = %4\n"
                    "\n"
                    "Aileron   TX = %5  RX = %6  RCMD TX = %7  RX = %8\n"
                    "Elevator  TX = %9  RX = %10  RCMD TX = %11  RX = %12\n"
                    "Throttle  TX = %13  RX = %14  RCMD TX = %15  RX = %16\n"
                    "Rudder    TX = %17  RX = %18  RCMD TX = %19  RX = %20\n"
                    "Channel5  TX = %21  RX = %22  RCMD TX = %23  RX = %24\n"
                    "Channel6  TX = %25  RX = %26  RCMD TX = %27  RX = %28\n"
                    "Channel7  TX = %29  RX = %30  RCMD TX = %31  RX = %32\n"
                    "PluginCh1 TX = %33  RX = %34  RCMD TX = %35  RX = %36\n"
                    "PluginCh2 TX = %37  RX = %38  RCMD TX = %39  RX = %40\n"
                    "FPVCamPan TX = %41  RX = %42  RCMD TX = %43  RX = %44\n"
                    "FPVCamTil TX = %45  RX = %46  RCMD TX = %47  RX = %48\n"
                    "\n"
                    "MenuItems = %49\n"
                    // Model data
                    "\n"
                    "fPosX,Y,Z    = (%50, %51, %52)\n"
                    "fVelX,Y,Z    = (%53, %54, %55)\n"
                    "fAngVelX,Y,Z = (%56, %57, %58)\n"
                    "fAccelX,Y,Z  = (%59, %60, %61)\n"
                    "\n"
                    "Lat, Long    = %62, %63\n"
                    "fHeightAboveTerrain = %64\n"
                    "\n"
                    "fHeading = %65   fPitch = %66   fRoll = %67\n"
                    )
                .arg(pluginFolder)
                .arg(outputFolder)
                .arg(stp->structSize)
                .arg(1.0 / stp->simTimeStep, 4, 'f', 1)
                .arg(stp->chSimTX[Ch1Aileron], 5, 'f', 2)
                .arg(stp->chSimRX[Ch1Aileron], 5, 'f', 2)
                .arg(pts->chNewTX[Ch1Aileron], 5, 'f', 2)
                .arg(pts->chNewRX[Ch1Aileron], 5, 'f', 2)
                .arg(stp->chSimTX[Ch2Elevator], 5, 'f', 2)
                .arg(stp->chSimRX[Ch2Elevator], 5, 'f', 2)
                .arg(pts->chNewTX[Ch2Elevator], 5, 'f', 2)
                .arg(pts->chNewRX[Ch2Elevator], 5, 'f', 2)
                .arg(stp->chSimTX[Ch3Throttle], 5, 'f', 2)
                .arg(stp->chSimRX[Ch3Throttle], 5, 'f', 2)
                .arg(pts->chNewTX[Ch3Throttle], 5, 'f', 2)
                .arg(pts->chNewRX[Ch3Throttle], 5, 'f', 2)
                .arg(stp->chSimTX[Ch4Rudder], 5, 'f', 2)
                .arg(stp->chSimRX[Ch4Rudder], 5, 'f', 2)
                .arg(pts->chNewTX[Ch4Rudder], 5, 'f', 2)
                .arg(pts->chNewRX[Ch4Rudder], 5, 'f', 2)
                .arg(stp->chSimTX[Ch5], 5, 'f', 2)
                .arg(stp->chSimRX[Ch5], 5, 'f', 2)
                .arg(pts->chNewTX[Ch5], 5, 'f', 2)
                .arg(pts->chNewRX[Ch5], 5, 'f', 2)
                .arg(stp->chSimTX[Ch6], 5, 'f', 2)
                .arg(stp->chSimRX[Ch6], 5, 'f', 2)
                .arg(pts->chNewTX[Ch6], 5, 'f', 2)
                .arg(pts->chNewRX[Ch6], 5, 'f', 2)
                .arg(stp->chSimTX[Ch7], 5, 'f', 2)
                .arg(stp->chSimRX[Ch7], 5, 'f', 2)
                .arg(pts->chNewTX[Ch7], 5, 'f', 2)
                .arg(pts->chNewRX[Ch7], 5, 'f', 2)
                .arg(stp->chSimTX[Ch23Plugin1], 5, 'f', 2)
                .arg(stp->chSimRX[Ch23Plugin1], 5, 'f', 2)
                .arg(pts->chNewTX[Ch23Plugin1], 5, 'f', 2)
                .arg(pts->chNewRX[Ch23Plugin1], 5, 'f', 2)
                .arg(stp->chSimTX[Ch24Plugin2], 5, 'f', 2)
                .arg(stp->chSimRX[Ch24Plugin2], 5, 'f', 2)
                .arg(pts->chNewTX[Ch24Plugin2], 5, 'f', 2)
                .arg(pts->chNewRX[Ch24Plugin2], 5, 'f', 2)
                .arg(stp->chSimTX[Ch12FPVCamPan], 5, 'f', 2)
                .arg(stp->chSimRX[Ch12FPVCamPan], 5, 'f', 2)
                .arg(pts->chNewTX[Ch12FPVCamPan], 5, 'f', 2)
                .arg(pts->chNewRX[Ch12FPVCamPan], 5, 'f', 2)
                .arg(stp->chSimTX[Ch13FPVCamTilt], 5, 'f', 2)
                .arg(stp->chSimRX[Ch13FPVCamTilt], 5, 'f', 2)
                .arg(pts->chNewTX[Ch13FPVCamTilt], 5, 'f', 2)
                .arg(pts->chNewRX[Ch13FPVCamTilt], 5, 'f', 2)
                .arg(stp->simMenuStatus)
                .arg(stp->posX, 5, 'f', 2)
                .arg(stp->posY, 5, 'f', 2)
                .arg(stp->posZ, 5, 'f', 2)
                .arg(stp->velX, 5, 'f', 2)
                .arg(stp->velY, 5, 'f', 2)
                .arg(stp->velZ, 5, 'f', 2)
                .arg(stp->angVelXm, 5, 'f', 2)
                .arg(stp->angVelYm, 5, 'f', 2)
                .arg(stp->angVelZm, 5, 'f', 2)
                .arg(stp->accelXm, 5, 'f', 2)
                .arg(stp->accelYm, 5, 'f', 2)
                .arg(stp->accelZm, 5, 'f', 2)
                .arg(stp->latitude, 5, 'f', 2)
                .arg(stp->longitude, 5, 'f', 2)
                .arg(stp->AGL, 5, 'f', 2)
                .arg(stp->heading*RAD2DEG, 5, 'f', 2)
                .arg(stp->pitch*RAD2DEG, 5, 'f', 2)
                .arg(stp->roll*RAD2DEG, 5, 'f', 2)
    );
}

SIM_DLL_EXPORT void AeroSIMRC_Plugin_Run(const simToPlugin *stp,
                                               pluginToSim *pts)
{
    debugInfo = "---\n";
    // By default do not change the Menu Items of type CheckBox
    pts->newMenuStatus = stp->simMenuStatus;
    // Extract Menu Commands from Flags
    bool isReset  = (stp->simMenuStatus & MenuCmdReset) != 0;
    bool isEnable = (stp->simMenuStatus & MenuEnable) != 0;
    bool isTxON   = (stp->simMenuStatus & MenuTx) != 0;
    bool isRxON   = (stp->simMenuStatus & MenuRx) != 0;
    bool isScreen = (stp->simMenuStatus & MenuScreen) != 0;
    bool isNextWp = (stp->simMenuStatus & MenuNextWpt) != 0;
    // Run commands
    if (isReset) {
        Run_Command_Reset(/*stp, pts*/);
    } else if (isScreen) {
        Run_Command_WindowSizeAndPos(stp, pts);
    } else if (isNextWp) {
        Run_Command_MoveToNextWaypoint(stp, pts);
    } else {
        Run_BlinkLEDs(stp, pts);
        if (isEnable) {
            if (isTxON)
                sndr->sendDatagram(stp);
            if (isRxON)
                rcvr->setChannels(pts);
        }

        // network lag
        debugInfo.append(QString("out: %1, inp: %2, delta: %3\n")
                         .arg(sndr->pcks() - 1)
                         .arg(rcvr->pcks())
                         .arg(sndr->pcks() - rcvr->pcks() - 1)
                         );
    }

    // debug info is shown on the screen
    InfoText(stp, pts);
    pts->dbgInfoText = debugInfo.toAscii();
    isFirstRun = false;
}
