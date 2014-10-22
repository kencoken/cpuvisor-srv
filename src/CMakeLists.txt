# PREPARE LIST OF SOURCES
# -------------------------------------

set (cpuvisor_netlib_SOURCES
  cpuvisor_netlib.cc)

set (cpuvisor_preproc_SOURCES
  cpuvisor_preproc.cc
  directencode/caffe_encoder.cc
  directencode/caffe_encoder_utils.cc
  directencode/augmentation_helper.cc
  server/util/io.cc)

set (cpuvisor_service_SOURCES
  cpuvisor_service.cc
  server/zmq_server.cc
  server/base_server.cc
  directencode/caffe_encoder.cc
  directencode/caffe_encoder_utils.cc
  directencode/augmentation_helper.cc
  server/util/image_downloader.cc
  server/util/io.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (cpuvisor_netlib_LIBRARIES
  ${Boost_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${CPPNETLIB_LIBRARIES})

set (cpuvisor_preproc_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs)

set (cpuvisor_service_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Liblinear_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${ZeroMQ_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  ${CPPNETLIB_LIBRARIES}
  protodefs)

# COMPILE TARGETS
# -------------------------------------

add_executable(cpuvisor_netlib ${cpuvisor_netlib_SOURCES})
add_executable(cpuvisor_preproc ${cpuvisor_preproc_SOURCES})
add_executable(cpuvisor_service ${cpuvisor_service_SOURCES})

# LINK LIBRARIES
# -------------------------------------

target_link_libraries(cpuvisor_netlib ${cpuvisor_netlib_LIBRARIES})
target_link_libraries(cpuvisor_preproc ${cpuvisor_preproc_LIBRARIES})
target_link_libraries(cpuvisor_service ${cpuvisor_service_LIBRARIES})

# INSTALL TARGETS
# -------------------------------------

install(TARGETS
  cpuvisor_netlib
  cpuvisor_preproc
  cpuvisor_service
  DESTINATION "${CMAKE_SOURCE_DIR}/bin")
