# PREPARE LIST OF SOURCES
# -------------------------------------

set (caffelib_SOURCES
  caffe_encoder.cc
  caffe_encoder_utils.cc
  augmentation_helper.cc
  netpool/caffe_netinst.cc
  netpool/caffe_netpool.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (caffelib_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs)

# COMPILE TARGETS
# -------------------------------------

add_library(caffelib STATIC ${caffelib_SOURCES})

# LINK LIBRARIES
# -------------------------------------

target_link_libraries(caffelib ${caffelib_LIBRARIES})

# INSTALL TARGETS
# -------------------------------------

install(TARGETS
  caffelib
  DESTINATION "${CMAKE_SOURCE_DIR}/bin/lib")
