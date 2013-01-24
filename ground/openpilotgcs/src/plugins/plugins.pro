# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin

# Blank Template Plugin, not compiled by default
#SUBDIRS += plugin_donothing
#plugin_donothing.subdir = donothing
#plugin_donothing.depends = plugin_coreplugin

# Core plugin
plugin_coreplugin.subdir = coreplugin

# Empty UAVGadget - Default for new splits
plugin_emptygadget.subdir = emptygadget
plugin_emptygadget.depends = plugin_coreplugin
SUBDIRS += plugin_emptygadget

# Debug Gadget plugin
plugin_debuggadget.subdir = debuggadget
plugin_debuggadget.depends = plugin_coreplugin
SUBDIRS += plugin_debuggadget

# Welcome plugin
plugin_welcome.subdir = welcome
plugin_welcome.depends = plugin_coreplugin
SUBDIRS += plugin_welcome

# RawHID connection plugin
SUBDIRS += plugin_rawhid
plugin_rawhid.subdir = rawhid
plugin_rawhid.depends = plugin_coreplugin

# Serial port connection plugin
SUBDIRS += plugin_serial
plugin_serial.subdir = serialconnection
plugin_serial.depends = plugin_coreplugin

# UAVObjects plugin
SUBDIRS += plugin_uavobjects
plugin_uavobjects.subdir = uavobjects
plugin_uavobjects.depends = plugin_coreplugin

# UAVTalk plugin
SUBDIRS += plugin_uavtalk
plugin_uavtalk.subdir = uavtalk
plugin_uavtalk.depends = plugin_uavobjects
plugin_uavtalk.depends += plugin_coreplugin

# OPMap UAVGadget
plugin_opmap.subdir = opmap
plugin_opmap.depends = plugin_coreplugin
plugin_opmap.depends += plugin_uavobjects
plugin_opmap.depends += plugin_uavobjectutil
plugin_opmap.depends += plugin_uavtalk
SUBDIRS += plugin_opmap

# Scope UAVGadget
plugin_scope.subdir = scope
plugin_scope.depends = plugin_coreplugin
plugin_scope.depends += plugin_uavobjects
plugin_scope.depends += plugin_uavtalk
SUBDIRS += plugin_scope

# UAVObject Browser gadget
plugin_uavobjectbrowser.subdir = uavobjectbrowser
plugin_uavobjectbrowser.depends = plugin_coreplugin
plugin_uavobjectbrowser.depends += plugin_uavobjects
SUBDIRS += plugin_uavobjectbrowser

# ModelView UAVGadget
plugin_modelview.subdir = modelview
plugin_modelview.depends = plugin_coreplugin
plugin_modelview.depends += plugin_uavobjects
SUBDIRS += plugin_modelview

#Qt 4.8.0 / phonon may crash on Mac, fixed in Qt 4.8.1, QTBUG-23128
macx:contains(QT_VERSION, ^4\\.8\\.0): CONFIG += disable_notify_plugin

# Notify gadget
!disable_notify_plugin {
    plugin_notify.subdir = notify
    plugin_notify.depends = plugin_coreplugin
    plugin_notify.depends += plugin_uavobjects
    plugin_notify.depends += plugin_uavtalk
    SUBDIRS += plugin_notify
}

# Uploader gadget
plugin_uploader.subdir = uploader
plugin_uploader.depends = plugin_coreplugin
plugin_uploader.depends += plugin_uavobjects
plugin_uploader.depends += plugin_uavtalk
plugin_uploader.depends += plugin_rawhid
plugin_uploader.depends += plugin_uavobjectutil
SUBDIRS += plugin_uploader

# Dial gadget
plugin_dial.subdir = dial
plugin_dial.depends = plugin_coreplugin
plugin_dial.depends += plugin_uavobjects
SUBDIRS += plugin_dial

# Linear Dial gadget
plugin_lineardial.subdir = lineardial
plugin_lineardial.depends = plugin_coreplugin
plugin_lineardial.depends += plugin_uavobjects
SUBDIRS += plugin_lineardial

# System Health gadget
plugin_systemhealth.subdir = systemhealth
plugin_systemhealth.depends = plugin_coreplugin
plugin_systemhealth.depends += plugin_uavobjects
plugin_systemhealth.depends += plugin_uavtalk
SUBDIRS += plugin_systemhealth

# Config gadget
plugin_config.subdir = config
plugin_config.depends = plugin_coreplugin
plugin_config.depends += plugin_uavtalk
plugin_config.depends += plugin_uavobjects
plugin_config.depends += plugin_uavobjectutil
plugin_config.depends += plugin_uavobjectwidgetutils
plugin_config.depends += plugin_uavsettingsimportexport
SUBDIRS += plugin_config

# GPS Display gadget
plugin_gpsdisplay.subdir = gpsdisplay
plugin_gpsdisplay.depends = plugin_coreplugin
plugin_gpsdisplay.depends += plugin_uavobjects
SUBDIRS += plugin_gpsdisplay

# Primary Flight Display (PFD) gadget
plugin_pfd.subdir = pfd
plugin_pfd.depends = plugin_coreplugin
plugin_pfd.depends += plugin_uavobjects
SUBDIRS += plugin_pfd

# QML viewer gadget
plugin_qmlview.subdir = qmlview
plugin_qmlview.depends = plugin_coreplugin
plugin_qmlview.depends += plugin_uavobjects
SUBDIRS += plugin_qmlview

# PathAction Editor gadget
plugin_pathactioneditor.subdir = pathactioneditor
plugin_pathactioneditor.depends = plugin_coreplugin
plugin_pathactioneditor.depends += plugin_uavobjects
SUBDIRS += plugin_pathactioneditor

# Primary Flight Display (PFD) gadget, QML version
plugin_pfdqml.subdir = pfdqml
plugin_pfdqml.depends = plugin_coreplugin
plugin_pfdqml.depends += plugin_uavobjects
SUBDIRS += plugin_pfdqml

# IP connection plugin
plugin_ipconnection.subdir = ipconnection
plugin_ipconnection.depends = plugin_coreplugin
SUBDIRS += plugin_ipconnection

#HITL Simulation gadget
plugin_hitl.subdir = hitl
plugin_hitl.depends = plugin_coreplugin
plugin_hitl.depends += plugin_uavobjects
plugin_hitl.depends += plugin_uavtalk
SUBDIRS += plugin_hitl

# Export and Import GCS Configuration
plugin_importexport.subdir = importexport
plugin_importexport.depends = plugin_coreplugin
SUBDIRS += plugin_importexport

# Telemetry data logging plugin
plugin_logging.subdir = logging
plugin_logging.depends = plugin_coreplugin
plugin_logging.depends += plugin_uavobjects
plugin_logging.depends += plugin_uavtalk
plugin_logging.depends += plugin_scope
SUBDIRS += plugin_logging

# GCS Control of UAV gadget
plugin_gcscontrol.subdir = gcscontrol
plugin_gcscontrol.depends = plugin_coreplugin
plugin_gcscontrol.depends += plugin_uavobjects
SUBDIRS += plugin_gcscontrol

# Antenna tracker
#plugin_antennatrack.subdir = antennatrack
#plugin_antennatrack.depends = plugin_coreplugin
#plugin_antennatrack.depends += plugin_uavobjects
#SUBDIRS += plugin_antennatrack

# Scope OpenGL Gadget
#plugin_scopeogl.subdir = scopeogl
#plugin_scopeogl.depends = plugin_coreplugin
#plugin_scopeogl.depends += plugin_uavobjects
#SUBDIRS += plugin_scopeogl

# UAV Object Utility plugin
plugin_uavobjectutil.subdir = uavobjectutil
plugin_uavobjectutil.depends = plugin_coreplugin
plugin_uavobjectutil.depends += plugin_uavobjects
SUBDIRS += plugin_uavobjectutil

# OSG Earth View plugin
OSG {
    plugin_osgearthview.subdir = osgearthview
    plugin_osgearthview.depends = plugin_coreplugin
    plugin_osgearthview.depends += plugin_uavobjects
    plugin_osgearthview.depends += plugin_uavobjectwidgetutils
    SUBDIRS += plugin_osgearthview
}

# Magic Waypoint gadget
plugin_magicwaypoint.subdir = magicwaypoint
plugin_magicwaypoint.depends = plugin_coreplugin
plugin_magicwaypoint.depends = plugin_uavobjects
SUBDIRS += plugin_magicwaypoint

# UAV Settings Import/Export plugin
plugin_uavsettingsimportexport.subdir = uavsettingsimportexport
plugin_uavsettingsimportexport.depends = plugin_coreplugin
plugin_uavsettingsimportexport.depends += plugin_uavobjects
plugin_uavsettingsimportexport.depends += plugin_uavobjectutil
SUBDIRS += plugin_uavsettingsimportexport

# UAV Object Widget Utility plugin
plugin_uavobjectwidgetutils.subdir = uavobjectwidgetutils
plugin_uavobjectwidgetutils.depends = plugin_coreplugin
plugin_uavobjectwidgetutils.depends += plugin_uavobjects
plugin_uavobjectwidgetutils.depends += plugin_uavobjectutil
plugin_uavobjectwidgetutils.depends += plugin_uavsettingsimportexport
plugin_uavobjectwidgetutils.depends += plugin_uavtalk
SUBDIRS += plugin_uavobjectwidgetutils

# Setup Wizard plugin
plugin_setupwizard.subdir = setupwizard
plugin_setupwizard.depends = plugin_coreplugin
plugin_setupwizard.depends += plugin_uavobjectutil
plugin_setupwizard.depends += plugin_config
plugin_setupwizard.depends += plugin_uploader
SUBDIRS += plugin_setupwizard

# Junsi Powerlog plugin
#plugin_powerlog.subdir = powerlog
#plugin_powerlog.depends = plugin_coreplugin
#plugin_powerlog.depends += plugin_rawhid
#SUBDIRS += plugin_powerlog
