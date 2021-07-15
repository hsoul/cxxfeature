aux_source_directory(. TOOLS_SRC)

add_executable(clang_address_sanitizer_test clang_AddressSanitizer_test.c)

# TARGET_LINK_LIBRARIES(
#     clang_address_sanitizer_test
# )

set(CLANG_LD -fuse-ld=lld)

target_compile_options(clang_address_sanitizer_test
    PUBLIC
        -fsanitize=address 
        -fno-omit-frame-pointer
        ${CLANG_LD}
)

# target_include_directories(cxxfeature
#     PUBLIC "${CMAKE_SOURCE_DIR}/" 
# )