project(game)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( game game game.cpp )
target_include_directories( game PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( game ${CMAKE_SOURCE_DIR}/../ricardian )