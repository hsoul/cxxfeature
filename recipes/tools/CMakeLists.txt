aux_source_directory(. TOOLS_SRC)

add_executable(clang_address_sanitizer_test clang_AddressSanitizer_test.c)

# TARGET_LINK_LIBRARIES(
#     clang_address_sanitizer_test
# )

target_compile_options(clang_address_sanitizer_test
    PUBLIC
        -O0
        -fsanitize=address
        -fno-omit-frame-pointer
)

target_link_options(clang_address_sanitizer_test
    PUBLIC
        # -fuse-ld=lld
        -fsanitize=address
)