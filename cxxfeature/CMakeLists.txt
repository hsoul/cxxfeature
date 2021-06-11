aux_source_directory(. SRC)

add_executable(rmain ${SRC})

TARGET_LINK_LIBRARIES(
    rmain
    pthread
)

target_compile_options(rmain
    PUBLIC
        "-DTEST_OPTION"
)

# add_subdirectory(net)