cmake_minimum_required(VERSION 3.27)
project(crow-web-hasher)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Crow CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)

# Needed for duplicate source
# find_package(nlohmann_json CONFIG REQUIRED)
# find_package(inja CONFIG REQUIRED)

find_package(xxHash CONFIG REQUIRED)

add_executable(crow-web-hasher src/main.cpp)


target_link_libraries(crow-web-hasher PRIVATE Crow::Crow)
# Needed for duplicate source
# target_link_libraries(crow-web-hasher PRIVATE nlohmann_json::nlohmann_json)
# target_link_libraries(crow-web-hasher PRIVATE pantor::inja)
target_link_libraries(crow-web-hasher PRIVATE xxHash::xxhash)
target_link_libraries(crow-web-hasher PRIVATE OpenSSL::SSL OpenSSL::Crypto)