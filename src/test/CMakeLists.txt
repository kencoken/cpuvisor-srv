set (test_SOURCES test.cc
  ../directencode/caffe_encoder.cc
  ../directencode/caffe_encoder_utils.cc
  ../directencode/augmentation_helper.cc
  ../directencode/netpool/caffe_netinst.cc
  ../directencode/netpool/caffe_netpool.cc
  ../classification/svm/liblinear.cc
  ../server/util/io.cc
  ../server/util/preproc.cc
  ../server/util/feat_util.cc)
if (MATEXP_DEBUG)
  list (APPEND test_SOURCES ../server/util/debug/matfileutils.cc)
  list (APPEND test_SOURCES ../server/util/debug/matfileutils_cpp.cc)
endif(MATEXP_DEBUG)

set (test_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Liblinear_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs)
if (MATEXP_DEBUG)
  list (APPEND test_LIBRARIES ${MATIO_LIBRARIES})
endif(MATEXP_DEBUG)

add_executable(test_exec ${test_SOURCES})

target_link_libraries(test_exec ${test_LIBRARIES})

install(TARGETS
  test_exec
  DESTINATION ${CMAKE_SOURCE_DIR}/bin/test)
