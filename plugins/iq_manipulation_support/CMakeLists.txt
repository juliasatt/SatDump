cmake_minimum_required(VERSION 3.12)
project(iq_manipulation_support)

file(GLOB_RECURSE iq_manipulation_support_CPPS *.cpp)
add_library(iq_manipulation_support SHARED ${iq_manipulation_support_CPPS})
target_link_libraries(iq_manipulation_support PUBLIC satdump_core)
target_include_directories(iq_manipulation_support PUBLIC src)

install(TARGETS iq_manipulation_support DESTINATION ${CMAKE_INSTALL_LIBDIR}/satdump/plugins)
