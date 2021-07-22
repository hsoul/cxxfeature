aux_source_directory(. ALGORITHM_SRC)


add_executable(algorithm ${ALGORITHM_SRC})

# TARGET_LINK_LIBRARIES(
#     hamming_weight
# )

# target_compile_options(cxxfeature
#     PUBLIC
#         "-DTEST_OPTION"
# )

# target_include_directories(cxxfeature
#     PUBLIC "${CMAKE_SOURCE_DIR}/" 
# )