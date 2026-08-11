cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#use this command:
#cmake -DCMAKE_TOOLCHAIN_FILE=../cmake/gcc-arm-none-eabi.cmake ..
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME Riverdi_50STM32U5G9)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    STM32CubeIDE/Application/User/gui/can_types.cpp
    Drivers/BSP/Components/mx25lm51245g/mx25lm51245g.c
    # TouchGFX GUI sources
    TouchGFX/gui/src/model/Model.cpp
    TouchGFX/gui/src/common/FrontendApplication.cpp
    TouchGFX/gui/src/screen1_screen/Screen1Presenter.cpp
    TouchGFX/gui/src/screen1_screen/Screen1View.cpp
    
    # TouchGFX generated sources
    TouchGFX/target/generated/OSWrappers.cpp
    TouchGFX/generated/gui_generated/src/screen1_screen/Screen1ViewBase.cpp
    TouchGFX/generated/gui_generated/src/common/FrontendApplicationBase.cpp
    TouchGFX/generated/fonts/src/ApplicationFontProvider.cpp
    TouchGFX/generated/fonts/src/CachedFont.cpp
    TouchGFX/generated/fonts/src/CompressedFontCache.cpp
    TouchGFX/generated/fonts/src/CompressedUnmappedFontCache.cpp
    TouchGFX/generated/fonts/src/Font_verdana_10_4bpp_0.cpp
    TouchGFX/generated/fonts/src/Font_verdana_20_4bpp_0.cpp
    TouchGFX/generated/fonts/src/Font_verdana_40_4bpp_0.cpp
    TouchGFX/generated/fonts/src/Font_verdana_80_4bpp_0.cpp
    TouchGFX/generated/fonts/src/FontCache.cpp
    TouchGFX/generated/fonts/src/GeneratedFont.cpp
    TouchGFX/generated/fonts/src/Kerning_verdana_10_4bpp.cpp
    TouchGFX/generated/fonts/src/Kerning_verdana_20_4bpp.cpp
    TouchGFX/generated/fonts/src/Kerning_verdana_40_4bpp.cpp
    TouchGFX/generated/fonts/src/Kerning_verdana_80_4bpp.cpp
    TouchGFX/generated/fonts/src/Table_verdana_10_4bpp.cpp
    TouchGFX/generated/fonts/src/Table_verdana_20_4bpp.cpp
    TouchGFX/generated/fonts/src/Table_verdana_40_4bpp.cpp
    TouchGFX/generated/fonts/src/Table_verdana_80_4bpp.cpp
    TouchGFX/generated/fonts/src/UnmappedDataFont.cpp
    TouchGFX/generated/fonts/src/VectorFontRendererBuffers.cpp


    TouchGFX/generated/images/src/BitmapDatabase.cpp
    TouchGFX/generated/images/src/SVGDatabase.cpp
    TouchGFX/generated/images/src/image_backgroundRPMbar.cpp
    TouchGFX/generated/images/src/image_filledRPMbar.cpp


    TouchGFX/generated/texts/src/Texts.cpp
    TouchGFX/generated/texts/src/TypedTextDatabase.cpp
    TouchGFX/generated/texts/src/LanguageGb.cpp
    
    
    # TouchGFX target sources
    TouchGFX/target/CortexMMCUInstrumentation.cpp
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
    Drivers/BSP/Components/mx25lm51245g
    Middlewares/ST/touchgfx/framework/include
    Core/Inc
    TouchGFX/gui/include
    TouchGFX/gui/include/gui
    TouchGFX/generated/gui_generated/include
    #TouchGFX/platform/os
    TouchGFX/generated/fonts/include
    TouchGFX/generated/images/include
    TouchGFX/generated/texts/include
    TouchGFX/generated/videos/include
)

# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx
    ${CMAKE_SOURCE_DIR}/Middlewares/ST/touchgfx/lib/core/cortex_m33/gcc/libtouchgfx-float-abi-hard.a
    # Add user defined libraries
)
