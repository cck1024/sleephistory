cmake_minimum_required(VERSION 3.16.3) #on host it was 4.1
project(sleephistory)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)

include(CheckIPOSupported)
check_ipo_supported(RESULT lto_supported OUTPUT error)
if(lto_supported)
    set(CMAKE_INTERPROCEDURAL_OPTIMIZATION TRUE)
    message(STATUS "LTO is enabled")
endif()

include(FetchContent)

FetchContent_Declare(ftxui
        GIT_REPOSITORY https://github.com/ArthurSonzogni/ftxui
        GIT_TAG v5.0.0 #Hostsystem: GIT_TAG main
)

FetchContent_MakeAvailable(ftxui)

FetchContent_Declare(fmt
        GIT_REPOSITORY https://github.com/fmtlib/fmt
        GIT_TAG 10.2.1 # oder aktuellste Version
)
FetchContent_MakeAvailable(fmt)

add_executable(sleephistory src/main.cpp
        src/JournalTUI.h
        src/timehelpers.h
        src/JournalBackend.h
        src/Structures.h
        src/parser.h
        src/JournalCLI.h
        src/Help.h
        src/Requirements.h
        src/debugging.h
        src/Colors.h
        src/timehelpers.cpp
        src/Structures.cpp
        src/Colors.cpp
        src/parser.cpp
        src/JournalTUI.cpp
        src/JournalCLI.cpp
        src/JournalBackend.cpp)

target_link_libraries(sleephistory PRIVATE ftxui::screen ftxui::dom ftxui::component fmt::fmt) # fmt ersetzt std::format (für kompatibilitaet)