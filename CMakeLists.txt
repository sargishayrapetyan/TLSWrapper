cmake_minimum_required(VERSION 3.7)
project(TLS_WRAPPER)

set ( CMAKE_PREFIX_PATH /usr/lib/x86_64-linux-gnu/cmake/ )
set(CMAKE_BUILD_TYPE Debug)
file(GLOB tls_wrapper "src/*.cpp")

#find_package(OpenSSL REQUIRED)
add_executable(TLS_WRAPPER ${tls_wrapper}) 

set (CMAKE_CXX_FLAGS "-g -O3 -std=c++14 -Wall -lpthread -Wextra")
target_compile_options(TLS_WRAPPER PRIVATE -std=c++14 -g -O3 -ggdb3 -no-pie -Wall -Wpedantic -Wextra -Wno-deprecated-declarations -fpermissive) #-Werror)
#INCLUDE_DIRECTORIES( ${LIBCRYPTO_INCLUDE_DIR})
target_link_libraries(TLS_WRAPPER PRIVATE ${OPENSSL_SSL_LIBRARY} ${OPENSSL_CRYPTO_LIBRARY} )
#message(${OPENSSL_CRYPTO_LIBRARY})

#target_link_libraries(kms PRIVATE cpprestsdk::cpprest Threads::Threads)
#target_link_libraries(kms PRIVATE ${AWSSDK_LINK_LIBRARIES})
target_include_directories(TLS_WRAPPER PUBLIC "inc/")
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)
