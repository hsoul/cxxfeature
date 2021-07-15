aux_source_directory(. CXX_FEATURE_SRC)

file (GLOB_RECURSE BASE_SRC 
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    base/*.cpp
    base/*.h
)

add_executable(cxxfeature ${CXX_FEATURE_SRC} ${BASE_SRC})

TARGET_LINK_LIBRARIES(
    cxxfeature
    pthread
    base
)

target_compile_options(cxxfeature
    PUBLIC
        "-DTEST_OPTION"
)

target_include_directories(cxxfeature
    PUBLIC "${CMAKE_SOURCE_DIR}/" 
)