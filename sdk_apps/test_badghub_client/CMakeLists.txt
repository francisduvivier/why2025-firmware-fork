project(TestBadgehubClient)
cmake_minimum_required(VERSION 3.5)

find_package(SDL3 REQUIRED)
find_package(CURL REQUIRED)

add_executable(badgehub-client)
target_sources(badgehub-client PUBLIC 
                ./main.c
                ./cJSON.c
                ./utils.c
                ./app_card.c
                ./test_drawing_helper.c
                ./badgehub_client.c
                ./cJSON_Utils.c
)
target_include_directories(badgehub-client PUBLIC ./)
target_link_libraries(badgehub-client PUBLIC SDL3 CURL::libcurl)
