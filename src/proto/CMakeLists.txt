file(GLOB ProtoFiles "${CMAKE_CURRENT_SOURCE_DIR}/*.proto")
PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${ProtoFiles})
PROTOBUF_GENERATE_PYTHON(ProtoSourcesPython ${ProtoFiles})

add_library(protodefs STATIC ${ProtoSources} ${ProtoHeaders})

target_include_directories(protodefs PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(protodefs
  ${Boost_LIBRARIES}
  ${PROTOBUF_LIBRARY})

add_custom_target(pyprotobuf
    DEPENDS ${ProtoSourcesPython}
)

add_dependencies(protodefs pyprotobuf)

# INSTALL TARGETS
# -------------------------------------

install(FILES
  ${ProtoSourcesPython}
  DESTINATION "${CMAKE_SOURCE_DIR}/proto")
