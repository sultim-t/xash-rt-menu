#
# Copyright (c) 2015 Pavlo Lavrenenko
#
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#

cmake_minimum_required(VERSION  2.8.0)
project (MAINUI)

if(MAINUI_APPLICATION_MODE)
	set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/")
endif()
include(FWGSLib)

# By default we require C++11 support.
# But it can be overriden with -DMY_COMPILER_SUCKS compiler option
# It's not recommended and may not work(as it's done only for MSVC6 support)
set(CMAKE_CXX_STANDARD 11)

set(MAINUI_LIBRARY xashmenu)
fwgs_fix_default_msvc_settings()
option(MAINUI_USE_CUSTOM_FONT_RENDER "Use custom font rendering" ON)
option(MAINUI_USE_STB "Use stb_truetype.h for rendering(*nix-only)" OFF)
option(MAINUI_RENDER_PICBUTTON_TEXT "Use custom font render to render picbuttons(experimental)" OFF)
option(MAINUI_APPLICATION_MODE "Build mainui_cpp as an application, without engine" OFF)

file(GLOB MAINUI_SOURCES *.cpp font/*.cpp controls/*.cpp) # Common sources for both engine and appmode
include_directories(. controls/ menus/ utl/ font/ model/) # Common includes for both engine and appmode

if(MAINUI_APPLICATION_MODE)
	include_directories(appmode) # appmode only includes
	
	file(GLOB MAINUI_APP_MODE_SOURCES appmode/*.cpp) # appmode only sources
	
	list(APPEND MAINUI_SOURCES ${MAINUI_APP_MODE_SOURCES})
	add_definitions(-DMAINUI_APPLICATION_MODE)
	add_executable(${MAINUI_LIBRARY} ${MAINUI_SOURCES})
	xash_link_sdl2(${MAINUI_LIBRARY})
else()
	if(NOT XASH_SDK)
		set(XASH_SDK "../")
	endif()
	include_directories(${XASH_SDK}/common ${XASH_SDK}/engine gamemode/) # gamemode only sources
	
	file(GLOB MAINUI_GAME_MODE_SOURCES gamemode/*.cpp gamemode/controls/*.cpp gamemode/menus/*.cpp) # gamemode only sources
	
	list(APPEND MAINUI_SOURCES ${MAINUI_GAME_MODE_SOURCES})
	add_definitions(-DMAINUI_GAME_MODE)
	add_library(${MAINUI_LIBRARY} SHARED ${MAINUI_SOURCES})
endif()

if(NOT WIN32 AND NOT MINGW)
	fwgs_add_compile_options(C -Wall -Wextra -Wno-unused-parameter -Wno-unused-variable -std=c++11)
else()
	# Only for MSVC6 compability mode! Newer VC does fail with this define
	# add_definitions(-DMY_COMPILER_SUCKS)
endif()

# Force stb_truetype for Apple devices, as there is no Apple native font renderer
# And freetype&fontconfig isn't used on this platform by default
if(APPLE)
	set(MAINUI_USE_STB TRUE)
endif()

if(MSVC)
	add_definitions(-D_CRT_SECURE_NO_WARNINGS)
endif()

# Font Rendering(FreeType or WinAPI)
if(MAINUI_USE_CUSTOM_FONT_RENDER)
	# Win32 will always use GDI font renderer
	if(NOT WIN32)
		if(MAINUI_USE_STB)
			# Use stbtt
			add_definitions(-DMAINUI_USE_STB)
		else()
			# Use freetype2
			find_package(PkgConfig)
			pkg_check_modules(FC REQUIRED fontconfig)
			include_directories(${FC_INCLUDE_DIRS})
			target_link_libraries(${MAINUI_LIBRARY} ${FC_LIBRARIES})
			add_definitions(-DMAINUI_USE_FREETYPE)
		endif()
	endif()

	add_definitions(-DMAINUI_USE_CUSTOM_FONT_RENDER)
	if(MAINUI_RENDER_PICBUTTON_TEXT)
		add_definitions(-DMAINUI_RENDER_PICBUTTON_TEXT)
	endif()
endif()

# Name library as it named in Unkle Mike's Xash3D
if(WIN32)
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES
		OUTPUT_NAME menu PREFIX "")
endif()

if(XASH_64BIT)
	get_target_property(MAINUI_NAME ${MAINUI_LIBRARY} OUTPUT_NAME)
	if(NOT MAINUI_NAME)
		set(MAINUI_NAME ${MAINUI_LIBRARY})
	endif()
	set_target_properties(${MAINUI_LIBRARY} PROPERTIES 
		OUTPUT_NAME ${MAINUI_NAME}64)
endif()

fwgs_set_default_properties(${MAINUI_LIBRARY})
fwgs_install(${MAINUI_LIBRARY})
