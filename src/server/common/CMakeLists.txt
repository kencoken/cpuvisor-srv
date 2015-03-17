# PREPARE LIST OF SOURCES
# -------------------------------------

set (visor_common_SOURCES
  server/common/generic_zmq_server.cc
  server/common/proto_parse.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (visor_common_LIBRARIES
  ${GLOG_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs)

# COMPILE TARGETS
# -------------------------------------

add_library(visor_common SHARED ${visor_common_SOURCES})

# LINK LIBRARIES
# -------------------------------------

target_link_libraries(visor_common ${visor_common_LIBRARIES})

# INSTALL TARGETS
# -------------------------------------

install(TARGETS
  visor_common
  DESTINATION "${CMAKE_SOURCE_DIR}/bin/lib")
