set(base_SRCS
  CountDownLatch.cpp
  Exception.cpp
  CurrentThread.cpp
  Thread.cpp
  )

add_library(serverlib_base ${base_SRCS})
target_link_libraries(serverlib_base pthread rt)

install(TARGETS serverlib_base DESTINATION lib)

file(GLOB HEADERS "*.h")
install(FILES ${HEADERS} DESTINATION include/serverlib_base/base)





# add_library(libserver_base ${LIB_SRC})
# target_link_libraries(libserver_base pthread rt)

# set_target_properties(libserver_base PROPERTIES OUTPUT_NAME "server_base")
