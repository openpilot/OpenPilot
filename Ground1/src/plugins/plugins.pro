# USE .subdir AND .depends !
# OTHERWISE PLUGINS WILL BUILD IN WRONG ORDER (DIRECTORIES ARE COMPILED IN PARALLEL)

TEMPLATE  = subdirs

SUBDIRS   = plugin_coreplugin \
            plugin_welcome
            
plugin_coreplugin.subdir = coreplugin

plugin_welcome.subdir = welcome
plugin_welcome.depends = plugin_coreplugin

