# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_welcome \

# Blank Template Plugin, not compiled by default
#SUBDIRS += plugin_donothing
#plugin_donothing.subdir = donothing
#plugin_donothing.depends = plugin_coreplugin

# Core Plugin
plugin_coreplugin.subdir = coreplugin

# Welcome Plugin
plugin_welcome.subdir = welcome
plugin_welcome.depends = plugin_coreplugin

# UAVObjects plug-in
SUBDIRS += plugin_uavobjects
plugin_uavobjects.subdir = uavobjects
plugin_uavobjects.depends = plugin_coreplugin

# UAVTalk plug-in
SUBDIRS += plugin_uavtalk
plugin_uavtalk.subdir = uavtalk
plugin_uavtalk.depends = plugin_uavobjects
plugin_uavtalk.depends += plugin_coreplugin


