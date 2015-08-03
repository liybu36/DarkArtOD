set(LIB_NAME darkart_ODReco_Tools)
art_make_library(
  SOURCE
    Utilities_service.cc
  LIBRARIES
    ${FHICLCPP}
    ${CETLIB}
    ${MF_MESSAGELOGGER}
)
simple_plugin(Utilities "service" ${LIB_NAME})

install_headers()
install_source()
