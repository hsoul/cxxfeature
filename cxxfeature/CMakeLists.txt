aux_source_directory(. SRC)

file (GLOB_RECURSE BASE_SRC 
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    base/*.cpp
    base/*.h
)

add_executable(rmain ${SRC} ${BASE_SRC})

TARGET_LINK_LIBRARIES(
    rmain
    pthread
)

target_compile_options(rmain
    PUBLIC
        "-DTEST_OPTION"
)

# add_subdirectory(net)