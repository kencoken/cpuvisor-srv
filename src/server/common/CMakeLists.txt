# PREPARE LIST OF SOURCES
# -------------------------------------

set (visor_common_SOURCES
  generic_zmq_server.cc
  proto_parse.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (visor_common_LIBRARIES
  ${GLOG_LIBRARIES}
  ${ZeroMQ_LIBRARIES}
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
