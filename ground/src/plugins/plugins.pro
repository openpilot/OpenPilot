# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin

# Blank Template Plugin, not compiled by default
#SUBDIRS += plugin_donothing
#plugin_donothing.subdir = donothing
#plugin_donothing.depends = plugin_coreplugin

# Core Plugin
plugin_coreplugin.subdir = coreplugin

# Welcome Plugin
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

# UAVObjects plug-in
SUBDIRS += plugin_uavobjects
plugin_uavobjects.subdir = uavobjects
plugin_uavobjects.depends = plugin_coreplugin

# UAVTalk plug-in
SUBDIRS += plugin_uavtalk
plugin_uavtalk.subdir = uavtalk
plugin_uavtalk.depends = plugin_uavobjects
plugin_uavtalk.depends += plugin_coreplugin

# Empty UAVGadget
plugin_emptygadget.subdir = emptygadget
plugin_emptygadget.depends = plugin_coreplugin
SUBDIRS += plugin_emptygadget

# Map UAVGadget
plugin_map.subdir = map
plugin_map.depends = plugin_coreplugin
SUBDIRS += plugin_map

# Scope UAVGadget
plugin_scope.subdir = scope
plugin_scope.depends = plugin_coreplugin
SUBDIRS += plugin_scope

# ModelView UAVGadget
plugin_modelview.subdir = modelview
plugin_scope.depends = plugin_coreplugin
SUBDIRS += plugin_modelview

# UAVObject Browser Gadget
plugin_uavobjectbrowser.subdir = uavobjectbrowser
plugin_uavobjectbrowser.depends = plugin_coreplugin
plugin_uavobjectbrowser.depends = plugin_uavobjects
SUBDIRS += plugin_uavobjectbrowser

#Uploader Gadget
plugin_uploader.subdir = uploader
plugin_uploader.depends = plugin_coreplugin
SUBDIRS += plugin_uploader

#Airspeed Dial Gadget
plugin_airspeed.subdir = airspeed
plugin_airspeed.depends = plugin_coreplugin
SUBDIRS += plugin_airspeed

#Linear Dial Gadget
plugin_lineardial.subdir = lineardial
plugin_lineardial.depends = plugin_coreplugin
SUBDIRS += plugin_lineardial

#System health Gadget
plugin_systemhealth.subdir = systemhealth
plugin_systemhealth.depends = plugin_coreplugin
SUBDIRS += plugin_systemhealth


#Config Gadget
plugin_config.subdir = config
plugin_config.depends = plugin_coreplugin
SUBDIRS += plugin_config

