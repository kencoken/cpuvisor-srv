# PREPARE LIST OF SOURCES
# -------------------------------------

set (cpuvisor_service_SOURCES cpuvisor_service.cc)

# PREPARE LIST OF LIBRARIES
# -------------------------------------

set (cpuvisor_service_LIBRARIES
  ${Boost_LIBRARIES}
  ${OpenCV_LIBRARIES}
  ${Liblinear_LIBRARIES}
  ${Caffe_LIBRARIES})

# COMPILE TARGETS
# -------------------------------------

add_executable(cpuvisor_service ${cpuvisor_service_SOURCES})

# LINK LIBRARIES
# -------------------------------------

target_link_libraries(cpuvisor_service ${cpuvisor_service_LIBRARIES})

# INSTALL TARGETS
# -------------------------------------

install(TARGETS
  cpuvisor_service
  DESTINATION "${CMAKE_SOURCE_DIR}/bin")
