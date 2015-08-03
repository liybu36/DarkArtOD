
art_make(
  LIB_LIBRARIES 
    darkart_Products 
    ${DARKSIDECORE_DATA} 
    ${ART_UTILITIES} 
    ${CETLIB}
  EXCLUDE 
    VetoAnalyzer_module.cc
  MODULE_LIBRARIES 
    darkart_ArtModules_VetoPMT_service 
    #darkart_ArtModules
    darkart_TpcReco
    bprinter
    ${DARKSIDECORE_ARTSERVICES} 
    ${DARKSIDECORE_ARTSERVICES_DBINTERFACE_SERVICE}
    ${ARTDAQ-CORE_DATA}
    ${ART_FRAMEWORK_SERVICES_REGISTRY} 
    ${FHICLCPP}
    ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
    ${ART_FRAMEWORK_SERVICES_OPTIONAL}
    ${ROOT_HIST}
    ${ROOT_TREE}
    ${ROOT_GRAF}
    ${MF_MESSAGELOGGER}
    ${MF_UTILITIES}
    ${CETLIB}
    ${PQXX}
    ${ROOT_RINT}
    ${ROOT_THREAD}
    ${ROOT_CORE}
    ${ROOT_MATHCORE}
    ${ROOT_MATRIX}
    ${ROOT_GPAD}
    ${ROOT_GUI}
)

simple_plugin(VetoAnalyzer "module"
  darkart_Products
  ${ART_FRAMEWORK_SERVICES_OPTIONAL_TFILESERVICE_SERVICE}
  ${ART_FRAMEWORK_SERVICES_OPTIONAL}
  ${ART_FRAMEWORK_SERVICES_REGISTRY}
  ${ROOT_HIST}
)

install_headers()
install_source() 

	 # SERVICE_LIBRARIES darkart_ArtServices ${FHILCPP}
#         EXCLUDE ProtoVetoMerger_module.cc VetoPMT_service.cc

