add_executable(rtclient client.cc)
target_link_libraries(rtclient ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
add_executable(rtserver server.cc)
target_link_libraries(rtserver ${Boost_LIBRARIES} ${CMAKE_THREAD_LIBS_INIT})
