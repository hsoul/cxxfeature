aux_source_directory(. TOOLS_SRC)

add_executable(clang_address_sanitizer_test clang_AddressSanitizer_test.c)

# TARGET_LINK_LIBRARIES(
#     clang_address_sanitizer_test
# )

target_compile_options(clang_address_sanitizer_test
    PUBLIC
        -O0
        -fsanitize=address
        -fno-omit-frame-pointer #可以在错误信息中提取更有意义的栈帧信息
        # 为了获取完美的栈帧信息，可能需要禁用内联（仅使用 -O1）和跟踪调用消除（-fno-optimize-sibling-calls）
)

target_link_options(clang_address_sanitizer_test
    PUBLIC
        # -fuse-ld=lld # use lld 链接器链接目标文件
        -fsanitize=address
)