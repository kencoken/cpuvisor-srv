# PREPARE LIST OF SOURCES
# -------------------------------------

set (visorcommon_SOURCES
  generic_zmq_server.cc
  proto_parse.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (visorcommon_LIBRARIES
  ${GLOG_LIBRARIES}
  ${ZeroMQ_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs)

# COMPILE TARGETS
# -------------------------------------

add_library(visorcommon STATIC ${visorcommon_SOURCES})

# LINK LIBRARIES
# -------------------------------------

target_link_libraries(visorcommon ${visorcommon_LIBRARIES})

# INSTALL TARGETS
# -------------------------------------

install(TARGETS
  visorcommon
  DESTINATION "${CMAKE_SOURCE_DIR}/bin/lib")
