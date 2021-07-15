aux_source_directory(. BASE_SRC)

add_library(base ${BASE_SRC})

# target_include_directories(apue
#     INTERFACE "${CMAKE_SOURCE_DIR}/recipes/apue/include/" 
# )