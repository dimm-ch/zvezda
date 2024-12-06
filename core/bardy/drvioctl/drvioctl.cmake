set(PROJECT_INCLUDE_DIRS ${PROJECT_INCLUDE_DIRS}
    ${PROJECT_SOURCE_DIR}/drvioctl/ambp
    ${PROJECT_SOURCE_DIR}/drvioctl/ambpex
    ${PROJECT_SOURCE_DIR}/drvioctl/ambzynq
    ${PROJECT_SOURCE_DIR}/drvioctl/axizynq
    ${PROJECT_SOURCE_DIR}/drvioctl/pex6678
)
set(PROJECT_INCLUDES ${PROJECT_INCLUDES}
    ${PROJECT_SOURCE_DIR}/drvioctl/ambp/ddwambp.h
    ${PROJECT_SOURCE_DIR}/drvioctl/ambpex/ambpexregs.h
    ${PROJECT_SOURCE_DIR}/drvioctl/ambpex/ddwambpex.h
    ${PROJECT_SOURCE_DIR}/drvioctl/ambzynq/zynqregs.h
    ${PROJECT_SOURCE_DIR}/drvioctl/ambzynq/ddzynq.h
    ${PROJECT_SOURCE_DIR}/drvioctl/axizynq/axizynqregs.h
    ${PROJECT_SOURCE_DIR}/drvioctl/axizynq/ddaxizynq.h
    ${PROJECT_SOURCE_DIR}/drvioctl/pex6678/ddw6678pex.h
    ${PROJECT_SOURCE_DIR}/drvioctl/pex6678/ti6678hw.h
    ${PROJECT_SOURCE_DIR}/drvioctl/pex6678/icrCopy.h
    ${PROJECT_SOURCE_DIR}/drvioctl/pex6678/pcieLocalReset_6678.h
)
