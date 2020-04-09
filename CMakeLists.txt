project(myjavap)
add_compile_options(-std=c++11 -g)

aux_source_directory(. DIR_SRCS)
add_executable(myjavap ${DIR_SRCS})

