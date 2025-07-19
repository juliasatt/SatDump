cmake_minimum_required(VERSION 3.12)
project(baseband_toolbox_app)

if(BUILD_GUI)
    file(GLOB_RECURSE baseband_toolbox_app_CPPS *.cpp)
    add_library(baseband_toolbox_app SHARED ${baseband_toolbox_app_CPPS})
    target_link_libraries(baseband_toolbox_app PUBLIC satdump_core satdump_interface)
    target_include_directories(baseband_toolbox_app PUBLIC . ../../src-interface)

    install(TARGETS baseband_toolbox_app DESTINATION ${CMAKE_INSTALL_LIBDIR}/satdump/plugins)
endif()
