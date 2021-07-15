aux_source_directory(. APUE_LIB_SRC)

#message(STATUS "${CMAKE_SOURCE_DIR}/apue/include/") 

add_library(apue ${APUE_LIB_SRC})

target_include_directories(apue
    PUBLIC "${CMAKE_SOURCE_DIR}/recipes/apue/include/" 
)

# target_compile_options(rmain
#     PUBLIC
#         "-DTEST_OPTION"
# )