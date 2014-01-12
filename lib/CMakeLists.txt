add_library(zcut
    zcut.c
    printer.c
)

add_library(zcut_main
    zcut_main.c
    zcut.c
    printer.c
)

install(FILES zcut.h
    DESTINATION include
)
install(TARGETS zcut zcut_main
    ARCHIVE DESTINATION lib
)
