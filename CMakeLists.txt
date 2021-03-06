cmake_minimum_required (VERSION 2.8.12)
project (EternalTCP)

option(BUILD_TEST "Build all unit test" OFF) # {boolean} Turn on or off unit tests (defaults to off/false).

IF(BUILD_TEST)
  add_subdirectory(${CMAKE_SOURCE_DIR}/cmake/external/googletest)
ENDIF(BUILD_TEST)

# Enable C++-11
if(UNIX)
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -std=c++11")
endif()

# Include external directories.
#add_subdirectory(${CMAKE_SOURCE_DIR}/cmake)

# Add cmake script directory.
LIST(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake")

# Find packages
find_package(Boost COMPONENTS system REQUIRED)
find_package(Threads REQUIRED)
find_package(Glog REQUIRED)
find_package(gflags REQUIRED)
find_package(LibGCrypt REQUIRED)
find_package(Protobuf REQUIRED)

# Apple specific packages
IF (APPLE)
   FIND_LIBRARY(COREFOUNDATION_LIBRARY CoreFoundation)
ELSE (APPLE)
   SET(COREFOUNDATION_LIBRARY "")
ENDIF (APPLE)

PROTOBUF_GENERATE_CPP(
ET_SRCS
ET_HDRS

proto/ET.proto
)

include_directories(
src
${PROTOBUF_INCLUDE_DIRS}
${CMAKE_CURRENT_BINARY_DIR}
)

# Needed to bring in locally build gcrypt on linux
link_directories(
  /usr/local/lib
)

set(${PROJECT_NAME}_SRC

src/BackedReader.cpp
src/BackedWriter.cpp
src/ClientConnection.cpp
src/Connection.cpp
src/CryptoHandler.cpp
src/FakeSocketHandler.cpp
src/FlakyFakeSocketHandler.cpp
src/Headers.cpp
src/ServerClientConnection.cpp
src/ServerConnection.cpp
src/SocketHandler.cpp
src/UnixSocketHandler.cpp

${ET_SRCS} ${ET_HDRS}
)

# Build the static library.
add_library(${PROJECT_NAME}-static STATIC ${${PROJECT_NAME}_SRC})
# Build the shared library.
#add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SRC})

target_link_libraries(
  ${PROJECT_NAME}-static
  )

# Make sure the compiler can find include files for our Hello library
# when other libraries or executables link to Hello
target_include_directories (${PROJECT_NAME}-static PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS ${PROJECT_NAME}-static DESTINATION lib)
install(FILES EternalTCP.hpp DESTINATION include)

PROTOBUF_GENERATE_CPP(
ETERMINAL_SRCS
ETERMINAL_HDRS

proto/ETerminal.proto
)

add_library(
TerminalCommon
STATIC

terminal/ConsoleUtils.cpp
terminal/ProcessHelper.cpp
terminal/SocketUtils.cpp

${ETERMINAL_SRCS} ${ETERMINAL_HDRS}
)
add_dependencies(TerminalCommon EternalTCP-static)

add_executable (Terminal terminal/Terminal.cpp)
target_link_libraries (
Terminal
LINK_PUBLIC
TerminalCommon
EternalTCP-static
${Boost_THREAD_LIBRARY}
${Boost_SYSTEM_LIBRARY}
${CMAKE_THREAD_LIBS_INIT}
${PROTOBUF_LIBRARIES}
gcrypt
glog
gflags
${COREFOUNDATION_LIBRARY}
util
)

add_executable (FlakyTerminal terminal/FlakyTerminal.cpp)
target_link_libraries (
FlakyTerminal
LINK_PUBLIC
TerminalCommon
EternalTCP-static
${Boost_THREAD_LIBRARY}
${Boost_SYSTEM_LIBRARY}
${CMAKE_THREAD_LIBS_INIT}
${PROTOBUF_LIBRARIES}
gcrypt
glog
gflags
${COREFOUNDATION_LIBRARY}
util
)

add_executable (ETServer terminal/TerminalServer.cpp)
target_link_libraries (
ETServer
LINK_PUBLIC
TerminalCommon
EternalTCP-static
${Boost_THREAD_LIBRARY}
${Boost_SYSTEM_LIBRARY}
${CMAKE_THREAD_LIBS_INIT}
${PROTOBUF_LIBRARIES}
${PROTOBUF_LIBRARIES}
gcrypt
glog
gflags
${COREFOUNDATION_LIBRARY}
util
)

add_executable (ETClient terminal/TerminalClient.cpp)
target_link_libraries (
ETClient
LINK_PUBLIC
TerminalCommon
EternalTCP-static
${Boost_THREAD_LIBRARY}
${Boost_SYSTEM_LIBRARY}
${CMAKE_THREAD_LIBS_INIT}
${PROTOBUF_LIBRARIES}
${PROTOBUF_LIBRARIES}
gcrypt
glog
gflags
${COREFOUNDATION_LIBRARY}
util
)


if(BUILD_TEST) # Build unit tests.
  message(STATUS "Enabling test for ${PROJECT_NAME}")
  enable_testing()
  add_subdirectory(sandbox)
  add_subdirectory(test)
endif()
