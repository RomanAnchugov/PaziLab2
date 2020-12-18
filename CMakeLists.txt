cmake_minimum_required(VERSION 3.17)
project(PaziLab2)

set(CMAKE_CXX_STANDARD 20)

include_directories(/usr/local/Cellar/libgcrypt/1.8.6/include)
link_directories(/usr/local/Cellar/libgcrypt/1.8.6/lib)
link_libraries(gcrypt)

include_directories(/usr/local/Cellar/libgpg-error/1.39/include)
link_directories(/usr/local/Cellar/libgpg-error/1.39/lib)
link_libraries(gpg-error)

add_executable(PaziLab2 main.cpp)