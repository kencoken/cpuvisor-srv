# PREPARE LIST OF SOURCES
# -------------------------------------

set (cpuvisor_testimg_SOURCES
  cpuvisor_testimg.cc
  classification/svm/liblinear.cc
  server/util/io.cc
  server/util/preproc.cc
  server/util/feat_util.cc)
if (MATEXP_DEBUG)
  list (APPEND cpuvisor_testimg_SOURCES server/util/debug/matfileutils.cc)
  list (APPEND cpuvisor_testimg_SOURCES server/util/debug/matfileutils_cpp.cc)
endif(MATEXP_DEBUG)

set (cpuvisor_timeit_SOURCES
  cpuvisor_timeit.cc
  server/util/io.cc)

set (cpuvisor_netlib_SOURCES
  cpuvisor_netlib.cc)

set (cpuvisor_preproc_SOURCES
  cpuvisor_preproc.cc
  classification/svm/liblinear.cc
  server/util/io.cc
  server/util/preproc.cc
  server/util/feat_util.cc)
if (MATEXP_DEBUG)
  list (APPEND cpuvisor_preproc_SOURCES server/util/debug/matfileutils.cc)
  list (APPEND cpuvisor_preproc_SOURCES server/util/debug/matfileutils_cpp.cc)
endif(MATEXP_DEBUG)

set (cpuvisor_service_SOURCES
  cpuvisor_service.cc
  server/cpuvisor/zmq_server.cc
  server/cpuvisor/base_server.cc
  server/cpuvisor/status_notifier.cc
  classification/svm/liblinear.cc
  server/util/image_downloader.cc
  server/util/io.cc
  server/util/feat_util.cc
  server/util/preproc.cc
  server/util/file_util.cc)
if (MATEXP_DEBUG)
  list (APPEND cpuvisor_service_SOURCES server/util/debug/matfileutils.cc)
  list (APPEND cpuvisor_service_SOURCES server/util/debug/matfileutils_cpp.cc)
endif(MATEXP_DEBUG)

set (cpuvisor_preproc_sge_SOURCES
  cpuvisor_preproc_sge.cc
  server/util/io.cc)

set (cpuvisor_combine_chunks_SOURCES
  cpuvisor_combine_chunks.cc
  server/util/io.cc)

set (cpuvisor_inspect_feats_SOURCES
  cpuvisor_inspect_feats.cc
  server/util/io.cc)

set (cpuvisor_add_dset_images_SOURCES
  cpuvisor_add_dset_images.cc
  server/cpuvisor/zmq_client.cc
  server/util/io.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (cpuvisor_testimg_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Liblinear_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon
  caffelib)
if (MATEXP_DEBUG)
  list (APPEND cpuvisor_testimg_LIBRARIES ${MATIO_LIBRARIES})
endif(MATEXP_DEBUG)

set (cpuvisor_timeit_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GLFAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon
  caffelib)

set (cpuvisor_netlib_LIBRARIES
  ${Boost_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${CPPNETLIB_LIBRARIES})

set (cpuvisor_preproc_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Liblinear_LIBRARIES}
  ${Caffe_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon
  caffelib)
if (MATEXP_DEBUG)
  list (APPEND cpuvisor_preproc_LIBRARIES ${MATIO_LIBRARIES})
endif(MATEXP_DEBUG)

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
  protodefs
  visorcommon
  caffelib)
if (MATEXP_DEBUG)
  list (APPEND cpuvisor_service_LIBRARIES ${MATIO_LIBRARIES})
endif(MATEXP_DEBUG)

set (cpuvisor_preproc_sge_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon)

set (cpuvisor_combine_chunks_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon)

set (cpuvisor_inspect_feats_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon)

set (cpuvisor_add_dset_images_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${GLOG_LIBRARIES}
  ${GFLAGS_LIBRARIES}
  ${ZeroMQ_LIBRARIES}
  ${PROTOBUF_LIBRARIES}
  protodefs
  visorcommon)

# COMPILE TARGETS
# -------------------------------------

add_executable(cpuvisor_testimg ${cpuvisor_testimg_SOURCES})
add_executable(cpuvisor_timeit ${cpuvisor_timeit_SOURCES})
add_executable(cpuvisor_netlib ${cpuvisor_netlib_SOURCES})
add_executable(cpuvisor_preproc ${cpuvisor_preproc_SOURCES})
add_executable(cpuvisor_service ${cpuvisor_service_SOURCES})
add_executable(cpuvisor_preproc_sge ${cpuvisor_preproc_sge_SOURCES})
add_executable(cpuvisor_combine_chunks ${cpuvisor_combine_chunks_SOURCES})
add_executable(cpuvisor_inspect_feats ${cpuvisor_inspect_feats_SOURCES})
add_executable(cpuvisor_add_dset_images ${cpuvisor_add_dset_images_SOURCES})

# LINK LIBRARIES
# -------------------------------------

target_link_libraries(cpuvisor_testimg ${cpuvisor_testimg_LIBRARIES})
target_link_libraries(cpuvisor_timeit ${cpuvisor_timeit_LIBRARIES})
target_link_libraries(cpuvisor_netlib ${cpuvisor_netlib_LIBRARIES})
target_link_libraries(cpuvisor_preproc ${cpuvisor_preproc_LIBRARIES})
target_link_libraries(cpuvisor_service ${cpuvisor_service_LIBRARIES})
target_link_libraries(cpuvisor_preproc_sge ${cpuvisor_preproc_sge_LIBRARIES})
target_link_libraries(cpuvisor_combine_chunks ${cpuvisor_combine_chunks_LIBRARIES})
target_link_libraries(cpuvisor_inspect_feats ${cpuvisor_inspect_feats_LIBRARIES})
target_link_libraries(cpuvisor_add_dset_images ${cpuvisor_add_dset_images_LIBRARIES})

# INSTALL TARGETS
# -------------------------------------

install(TARGETS
  cpuvisor_testimg
  cpuvisor_timeit
  cpuvisor_netlib
  cpuvisor_preproc
  cpuvisor_service
  cpuvisor_preproc_sge
  cpuvisor_combine_chunks
  cpuvisor_inspect_feats
  cpuvisor_add_dset_images
  DESTINATION "${CMAKE_SOURCE_DIR}/bin")
