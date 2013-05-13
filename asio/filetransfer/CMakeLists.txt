add_executable(download download.cc)
target_link_libraries(download ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
