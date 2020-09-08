# Currently STKTransfer is a header only library which means without special
# help its headers will not be installed correctly. All this file does is define
# the installation of the headers

FILE(GLOB HEADERS *.hpp)

INSTALL(FILES ${HEADERS} DESTINATION
${CMAKE_INSTALL_PREFIX}/${${PROJECT_NAME}_INSTALL_INCLUDE_DIR}/stk_transfer/)

FILE(GLOB HEADERS_IMPL ${CMAKE_CURRENT_SOURCE_DIR}/copy_by_id/*.hpp)
FILE(GLOB SOURCES_IMPL  ${CMAKE_CURRENT_SOURCE_DIR}/copy_by_id/*.cpp)

TRIBITS_ADD_LIBRARY(
  stk_transfer_impl
  NOINSTALLHEADERS ${HEADERS_IMPL}
  SOURCES  ${SOURCES_IMPL}
  )

IF (TPL_ENABLE_SICM)
  TARGET_INCLUDE_DIRECTORIES(stk_transfer_impl PRIVATE ${TPL_SICM_INCLUDE_DIRS})
  TARGET_LINK_LIBRARIES(stk_transfer_impl ${TPL_SICM_LIBRARIES})
ENDIF()
