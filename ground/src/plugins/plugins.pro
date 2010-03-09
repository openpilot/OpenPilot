# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_welcome

# Blank Template Plugin, not compiled by default
#SUBDIRS += plugin_donothing
#plugin_donothing.subdir = donothing
#plugin_donothing.depends = plugin_coreplugin

# Core Plugin
plugin_coreplugin.subdir = coreplugin

# Welcome Plugin
plugin_welcome.subdir = welcome
plugin_welcome.depends = plugin_coreplugin


