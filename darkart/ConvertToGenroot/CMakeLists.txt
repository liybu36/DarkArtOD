# art_make assumes all non-plugin source files go into a library.
# A dictionary is also made if classes_def.xml exists.
#
#  The use of LIB_LIBRARIES will cause the library to be named after the directory.


set(LIB_NAME darkart_ConvertToGenroot)
set(LIB_SOURCES
  GenrootRawEvent.cc
  )

add_library( ${LIB_NAME} SHARED ${LIB_SOURCES} )

# might not be needed - (most of these)

target_link_libraries(${LIB_NAME}
  ${DS50DAQ_DAQ}
  #${DS50DAQ_COMPRESSION}
  #${DS50DAQ_DAQ_DICT}
  ${ARTDAQ_DAQDATA}
  ${ART_PERSISTENCY_COMMON}
  ${ART_PERSISTENCY_PROVENANCE}
  ${ART_FRAMEWORK_ART}
  ${ART_UTILITIES}
  ${FHICLCPP}
  ${CETLIB}
  ${MF_MESSAGELOGGER}
  ${MF_UTILITIES}
  z
  )

simple_plugin(ConvertToGenroot  "module" ${LIB_NAME} )


#art_dictionary(DICTIONARY_LIBRARIES ${LIB_NAME})

install_headers(SUBDIRS detail)
install_source()
install(TARGETS ${LIB_NAME}
  DESTINATION ${flavorqual_dir}/lib )

