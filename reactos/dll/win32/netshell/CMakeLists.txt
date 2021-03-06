
set_cpp(WITH_RUNTIME)

if(NOT MSVC)
    # HACK: this should be enabled globally!
    add_compile_flags_language("-std=c++11" "CXX")
endif()

remove_definitions(-D_WIN32_WINNT=0x502)
add_definitions(-D_WIN32_WINNT=0x600)

add_definitions(-D_NETSHELL_)

spec2def(netshell.dll netshell.spec)

list(APPEND SOURCE
    netshell.cpp
    shfldr_netconnect.cpp
    enumlist.cpp
    classfactory.cpp
    connectmanager.cpp
    lanconnectui.cpp
    lanstatusui.cpp
    precomp.h)

add_library(netshell SHARED
    ${SOURCE}
    netshell.rc
    ${CMAKE_CURRENT_BINARY_DIR}/netshell_stubs.c
    ${CMAKE_CURRENT_BINARY_DIR}/netshell.def)

set_module_type(netshell win32dll)
target_link_libraries(netshell uuid wine)

add_importlibs(netshell
    msvcrt
    shlwapi
    shell32
    version
    iphlpapi
    ole32
    user32
    advapi32
    setupapi
    ws2_32
    comctl32
    kernel32
    ntdll)

add_pch(netshell precomp.h SOURCE)
add_cd_file(TARGET netshell DESTINATION reactos/system32 FOR all)
