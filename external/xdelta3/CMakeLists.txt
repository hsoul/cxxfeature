file(GLOB SOURCES *.c)

add_library(xdelta3 STATIC ${SOURCES})

set(xdelta3_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}")

target_include_directories(xdelta3 PUBLIC
    "$<BUILD_INTERFACE:${xdelta3_INCLUDE_DIR}>"
)

set_target_properties(xdelta3 PROPERTIES
        COMPILE_FLAGS
		"-O3"
        )

install(TARGETS xdelta3
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        COMPONENT library
        )

file(GLOB INCLUDES *.h)
install(FILES ${INCLUDES} DESTINATION include)
