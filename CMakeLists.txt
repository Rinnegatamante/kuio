cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(kuio)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wl,-q -Wall -O3 -std=gnu99")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -fno-rtti -fno-exceptions")

include_directories(
)

link_directories(
  ${CMAKE_CURRENT_BINARY_DIR}
)

add_definitions(-DRELEASE)

add_executable(kuio
  main.c
)

target_link_libraries(kuio
  taihenForKernel_stub
  SceLibKernel_stub
  SceSysmemForDriver_stub
  SceThreadmgrForDriver_stub
  SceIofilemgrForDriver_stub
  k
  gcc
)

set_target_properties(kuio
  PROPERTIES LINK_FLAGS "-nostdlib"
  COMPILE_FLAGS "-D__VITA_KERNEL__"
)

vita_create_self(kuio.skprx kuio CONFIG ${CMAKE_SOURCE_DIR}/kuio.yml)

vita_create_stubs(stubs kuio ${CMAKE_SOURCE_DIR}/kuio.yml KERNEL)
