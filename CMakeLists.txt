cmake_minimum_required(VERSION 3.10)
project(ZSNES)

set(CMAKE_VERBOSE_MAKEFILE ON)

project(zsnes LANGUAGES C CXX ASM_NASM)

# Set the default build type to Release if not specified
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()


# Set default values and gather required package information
set(PREFIX "/usr" CACHE PATH "Default install path prefix")
find_package(PkgConfig REQUIRED)
pkg_check_modules(SDL REQUIRED sdl)
pkg_check_modules(LIBPNG REQUIRED libpng)


# Set Standards
set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 14)
enable_language(ASM_NASM)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

# Find necessary packages
include(FindPkgConfig)
pkg_search_module(SDL1 REQUIRED sdl)
pkg_search_module(PNG REQUIRED libpng)

# Include Parsegen.cmake to build and check parsegen
include(Parsegen.cmake)

# Include source files
include(SourceFiles.cmake)

# zsnes-specific compiler and linker flags
set(ZSNES_COMMON_FLAGS
    -m32
    -pthread
    -fPIE
    -O1
    -march=pentium-mmx
    -fno-inline
    -fno-pic
    -mtune=generic
    -mmmx
    -D_FORTIFY_SOURCE=2
    -mno-sse
    -mno-sse2
    -ffunction-sections
    -fdata-sections
    -Wfatal-errors
    -w
    -rdynamic
)

# Custom Commands and Targets for parsegen
#set(PARSEGEN_FLAGS "-DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/libpng16")
#set(PARSEGEN_FLAGS "-DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/libpng16 -gcc \"cc -m32\"")

# Convert the ZSNES_COMMON_FLAGS to a single string with space separation
#string(JOIN " " ZSNES_COMMON_FLAGS_STRING "${ZSNES_COMMON_FLAGS}")

# Custom Commands and Targets for parsegen
#set(PARSEGEN_FLAGS "${ZSNES_COMMON_FLAGS_STRING} -DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/libpng16 -gcc \"cc -m32\"")

# set(PARSEGEN_FLAGS "-DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/libpng16 -gcc \"cc -m32\"")

# Custom Commands and Targets for parsegen
#set(PARSEGEN_FLAGS "-DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/libpng16")
set(PARSEGEN_FLAGS "-DNO_AO -DNO_DEBUGGER -D__OPENGL__ -D__UNIXSDL__ -I/usr/include/SDL -D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/libpng16 -gcc \"cc -m32\"")
set(PSR_FILES "cfg.psr" "input.psr" "md.psr")
foreach(PSR ${PSR_FILES})
    string(REPLACE ".psr" "" BASE_NAME ${PSR})
    add_custom_command(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.h ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.o
        COMMAND ${PARSEGEN_BINARY} ${PARSEGEN_FLAGS} -gcc cc -compile -cheader ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.h -fname ${BASE_NAME} ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.o ${PSR}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        DEPENDS parsegen
    )
    list(APPEND GENERATED_FILES ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.h ${CMAKE_CURRENT_BINARY_DIR}/${BASE_NAME}.o)
endforeach()

add_custom_target(generate_files DEPENDS ${GENERATED_FILES})

# Main target
add_executable(${PROJECT_NAME} ${ZSNES_C_SRC} ${ZSNES_ASM_SRC} ${ZSNES_HDR} ${GENERATED_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${SDL1_INCLUDE_DIRS} ${PNG_INCLUDE_DIRS})

set(ZSNES_ASM_NASM_FLAGS
    -O1
    -w-orphan-labels
    -felf32
    -DELF
    -I${CMAKE_CURRENT_SOURCE_DIR}/
    -I${CMAKE_CURRENT_SOURCE_DIR}/cpu/
    -I${CMAKE_CURRENT_SOURCE_DIR}/video/
    -I${CMAKE_CURRENT_SOURCE_DIR}/chips/
)

set(ZSNES_C_FLAGS
    ${ZSNES_COMMON_FLAGS}
    -std=gnu99
    -fcommon
)

set(ZSNES_CXX_FLAGS
    ${ZSNES_COMMON_FLAGS}
    -std=gnu++14
)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<COMPILE_LANGUAGE:C>:${ZSNES_C_FLAGS}>
    $<$<COMPILE_LANGUAGE:CXX>:${ZSNES_CXX_FLAGS}>
    $<$<COMPILE_LANGUAGE:ASM_NASM>:${ZSNES_ASM_NASM_FLAGS}>
)

# Make sure the main target depends on the generated files
add_dependencies(${PROJECT_NAME} generate_files)

target_link_libraries(zsnes ${SDL_LIBRARIES} ${LIBPNG_LIBRARIES} z dl X11 GL)

# Linker flags
set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "-m32 -pie -L/usr/lib32")

# Additional configurations
add_compile_definitions(NO_AO NO_DEBUGGER __OPENGL__ __UNIXSDL__)

set_property(SOURCE ${ZSNES_ASM_SRC} PROPERTY LANGUAGE ASM_NASM)

function(check_object_file file)
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND echo "Checking file: ${file}"
        COMMAND file ${file}
    )
endfunction()

set(CMAKE_ASM_NASM_COMPILER /usr/bin/nasm)

enable_language(ASM_NASM)

if(CMAKE_ASM_NASM_COMPILER_LOADED)
    set(CMAKE_ASM_NASM_COMPILER nasm)
endif()

set(CMAKE_ASM_NASM_COMPILE_OBJECT "<CMAKE_ASM_NASM_COMPILER> <INCLUDES> <FLAGS> -o <OBJECT> <SOURCE>")

set(CMAKE_ASM_NASM_FLAGS "${CMAKE_ASM_NASM_FLAGS} -felf32" CACHE STRING "" FORCE)

foreach(ASM_SRC ${ZSNES_ASM_SRC})
    # Convert the .asm source path to a corresponding .o path in the build directory
    string(REPLACE ".asm" ".asm.o" OBJ_FILE "${CMAKE_CURRENT_BINARY_DIR}/${ASM_SRC}")
    check_object_file(${OBJ_FILE})
endforeach()

# Installation
install(TARGETS zsnes DESTINATION ${PREFIX}/bin)

# Define the dependencies for each asm file
set(cpu_dma_dependencies cpu/dma.asm cpu/regs.mac cpu/regsw.mac macros.mac)
set(cpu_dsp_dependencies cpu/dsp.asm macros.mac)

# Now, set the dependencies on each asm file
set_source_files_properties(cpu/dma.asm PROPERTIES OBJECT_DEPENDS "${cpu_dma_dependencies}")
set_source_files_properties(cpu/dsp.asm PROPERTIES OBJECT_DEPENDS "${cpu_dsp_dependencies}")

set(cpu_dma_dependencies cpu/regs.mac cpu/regsw.mac macros.mac)
set(cpu_dsp_dependencies macros.mac)
set(cpu_dspproc_dependencies macros.mac cpu/firtable.inc)

set_source_files_properties(cpu/dma.asm PROPERTIES OBJECT_DEPENDS "${cpu_dma_dependencies}")
set_source_files_properties(cpu/dsp.asm PROPERTIES OBJECT_DEPENDS "${cpu_dsp_dependencies}")
set_source_files_properties(cpu/dspproc.asm PROPERTIES OBJECT_DEPENDS "${cpu_dspproc_dependencies}")

set(UNIVERSAL_ASM_DEPENDENCIES macros.mac)

foreach(ASM_SRC ${ZSNES_ASM_SRC})
    set_source_files_properties(${ASM_SRC} PROPERTIES OBJECT_DEPENDS "${UNIVERSAL_ASM_DEPENDENCIES}")
endforeach()
