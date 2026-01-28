cmake_minimum_required(VERSION 3.5)

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

add_executable(kuio main.c)

target_link_libraries(kuio
  SceFios2KernelForDriver_stub
  SceIofilemgrForDriver_stub
  SceThreadmgrForDriver_stub
  SceSblACMgrForDriver_stub
  SceSysmemForDriver_stub
  SceSysclibForDriver_stub
)

set_target_properties(kuio
  PROPERTIES LINK_FLAGS "-nostdlib"
  COMPILE_FLAGS "-D__VITA_KERNEL__"
)

add_custom_target(kuio.skprx ALL
  COMMAND vita-elf-create -s -e ${CMAKE_SOURCE_DIR}/kuio.yml kuio kuio.velf ${CMAKE_SOURCE_DIR}/taihen.json
  COMMAND vita-make-fself -c kuio.velf kuio.skprx
)

add_dependencies(kuio.skprx kuio)

vita_create_stubs(stubs kuio ${CMAKE_SOURCE_DIR}/kuio.yml KERNEL)

install(FILES ${CMAKE_BINARY_DIR}/stubs/libkuio_stub.a DESTINATION lib)
install(FILES ${CMAKE_BINARY_DIR}/stubs/libkuio_stub_weak.a DESTINATION lib)
install(FILES ${CMAKE_SOURCE_DIR}/include/kuio.h DESTINATION include)
