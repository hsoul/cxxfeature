# aux_source_directory(. TOOLS_SRC)

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

add_executable(test_thread_analysis test_thread_analysis.cc)

target_link_libraries(test_thread_analysis
    pthread
    # asan
)

target_compile_options(test_thread_analysis
    PUBLIC
        -O0
        -Wthread-safety
)

add_executable(test_access_private test_access_private.cc)

target_include_directories(test_access_private
    PUBLIC "${CMAKE_SOURCE_DIR}/" 
)