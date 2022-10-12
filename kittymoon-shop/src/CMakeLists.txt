project(shop)

set(EOSIO_WASM_OLD_BEHAVIOR "Off")
find_package(eosio.cdt)

add_contract( shop shop shop.cpp )
target_include_directories( shop PUBLIC ${CMAKE_SOURCE_DIR}/../include )
target_ricardian_directory( shop ${CMAKE_SOURCE_DIR}/../ricardian )