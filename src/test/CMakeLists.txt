set (test_SOURCES test.cc
  ../directencode/caffe_encoder.cc
  ../directencode/caffe_encoder_utils.cc
  ../directencode/augmentation_helper.cc
  ../classification/svm/liblinear.cc
  ../server/util/io.cc
  ../server/util/preproc.cc
  ../server/util/feat_util.cc)

set (test_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Liblinear_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs)

add_executable(test_exec ${test_SOURCES})

target_link_libraries(test_exec ${test_LIBRARIES})

install(TARGETS
  test_exec
  DESTINATION ${CMAKE_SOURCE_DIR}/bin/test)
