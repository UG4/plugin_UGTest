cmake_minimum_required(VERSION 2.6)

# Name of your plugin and sources.
set(pluginName	UGTest)

project(UG_PLUGIN_${pluginName})

add_subdirectory(system_testing)

add_subdirectory(unit_testing)