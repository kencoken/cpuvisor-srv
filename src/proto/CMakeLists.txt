file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})

add_library(protodefs STATIC ${ProtoSources} ${ProtoHeaders})

target_include_directories(protodefs PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(protodefs ${Boost_LIBRARIES} ${PROTOBUF_LIBRARY})
