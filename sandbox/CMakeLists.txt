
# set the path for all test files
set(SANDBOX_PATH ${PROJECT_SOURCE_DIR}/sandbox)
#
# Add your sandbox applications here
#
set (SANDBOX_SRC
  TestTransfer.cpp
  TestFakeSocketHandler.cpp
  TestConnection.cpp
  TestFlakyConnection.cpp
  )
# Add test executable target
foreach(SRC_LINE ${SANDBOX_SRC})
  string(REPLACE ".cpp" "" FILE_LINE ${SRC_LINE})

  add_executable(${FILE_LINE} ${SRC_LINE})
  target_link_libraries(
    ${FILE_LINE}
    LINK_PUBLIC
    ${PROJECT_NAME}-static
    ${Boost_SYSTEM_LIBRARY}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARIES}
    gcrypt
    glog
    gflags
    ${COREFOUNDATION_LIBRARY}
    )
    add_test(${FILE_LINE} ${FILE_LINE})
endforeach()
