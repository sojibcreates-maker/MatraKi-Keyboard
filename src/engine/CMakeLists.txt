## Platform depended things
if (OS_LINUX)
    ## Platform is Linux
    if (ENABLE_IBUS)
    add_subdirectory(ibus)
endif()
endif (OS_LINUX)

## Include subdirectories
add_subdirectory(riti)
