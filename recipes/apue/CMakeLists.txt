add_executable(1-3-ls 1-3-ls.c)
target_link_libraries(1-3-ls apue)

add_executable(1-8-perror 1-8-perror.c)
target_link_libraries(1-8-perror apue)

add_executable(1-9-uid 1-9-uid.c)
target_link_libraries(1-9-uid apue)

add_executable(1-10-sig 1-10-sig.c)
target_link_libraries(1-10-sig apue)


add_subdirectory(apue_lib)
