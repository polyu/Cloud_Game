# Install script for directory: E:/150w/rtplib/jrtplib/src

# Set the install prefix
IF(NOT DEFINED CMAKE_INSTALL_PREFIX)
  SET(CMAKE_INSTALL_PREFIX "C:/Program Files/jrtplib")
ENDIF(NOT DEFINED CMAKE_INSTALL_PREFIX)
STRING(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
IF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  IF(BUILD_TYPE)
    STRING(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  ELSE(BUILD_TYPE)
    SET(CMAKE_INSTALL_CONFIG_NAME "Release")
  ENDIF(BUILD_TYPE)
  MESSAGE(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
ENDIF(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)

# Set the component getting installed.
IF(NOT CMAKE_INSTALL_COMPONENT)
  IF(COMPONENT)
    MESSAGE(STATUS "Install component: \"${COMPONENT}\"")
    SET(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  ELSE(COMPONENT)
    SET(CMAKE_INSTALL_COMPONENT)
  ENDIF(COMPONENT)
ENDIF(NOT CMAKE_INSTALL_COMPONENT)

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  FILE(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include/jrtplib3" TYPE FILE FILES
    "E:/150w/rtplib/jrtplib/src/rtcpapppacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcpbyepacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcpcompoundpacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcpcompoundpacketbuilder.h"
    "E:/150w/rtplib/jrtplib/src/rtcppacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcppacketbuilder.h"
    "E:/150w/rtplib/jrtplib/src/rtcprrpacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcpscheduler.h"
    "E:/150w/rtplib/jrtplib/src/rtcpsdesinfo.h"
    "E:/150w/rtplib/jrtplib/src/rtcpsdespacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcpsrpacket.h"
    "E:/150w/rtplib/jrtplib/src/rtcpunknownpacket.h"
    "E:/150w/rtplib/jrtplib/src/rtpaddress.h"
    "E:/150w/rtplib/jrtplib/src/rtpcollisionlist.h"
    "E:/150w/rtplib/jrtplib/src/rtpconfig.h"
    "E:/150w/rtplib/jrtplib/src/rtpdebug.h"
    "E:/150w/rtplib/jrtplib/src/rtpdefines.h"
    "E:/150w/rtplib/jrtplib/src/rtperrors.h"
    "E:/150w/rtplib/jrtplib/src/rtphashtable.h"
    "E:/150w/rtplib/jrtplib/src/rtpinternalsourcedata.h"
    "E:/150w/rtplib/jrtplib/src/rtpipv4address.h"
    "E:/150w/rtplib/jrtplib/src/rtpipv4destination.h"
    "E:/150w/rtplib/jrtplib/src/rtpipv6address.h"
    "E:/150w/rtplib/jrtplib/src/rtpipv6destination.h"
    "E:/150w/rtplib/jrtplib/src/rtpkeyhashtable.h"
    "E:/150w/rtplib/jrtplib/src/rtplibraryversion.h"
    "E:/150w/rtplib/jrtplib/src/rtpmemorymanager.h"
    "E:/150w/rtplib/jrtplib/src/rtpmemoryobject.h"
    "E:/150w/rtplib/jrtplib/src/rtppacket.h"
    "E:/150w/rtplib/jrtplib/src/rtppacketbuilder.h"
    "E:/150w/rtplib/jrtplib/src/rtppollthread.h"
    "E:/150w/rtplib/jrtplib/src/rtprandom.h"
    "E:/150w/rtplib/jrtplib/src/rtprandomrand48.h"
    "E:/150w/rtplib/jrtplib/src/rtprandomrands.h"
    "E:/150w/rtplib/jrtplib/src/rtprandomurandom.h"
    "E:/150w/rtplib/jrtplib/src/rtprawpacket.h"
    "E:/150w/rtplib/jrtplib/src/rtpsession.h"
    "E:/150w/rtplib/jrtplib/src/rtpsessionparams.h"
    "E:/150w/rtplib/jrtplib/src/rtpsessionsources.h"
    "E:/150w/rtplib/jrtplib/src/rtpsourcedata.h"
    "E:/150w/rtplib/jrtplib/src/rtpsources.h"
    "E:/150w/rtplib/jrtplib/src/rtpstructs.h"
    "E:/150w/rtplib/jrtplib/src/rtptimeutilities.h"
    "E:/150w/rtplib/jrtplib/src/rtptransmitter.h"
    "E:/150w/rtplib/jrtplib/src/rtptypes_win.h"
    "E:/150w/rtplib/jrtplib/src/rtptypes.h"
    "E:/150w/rtplib/jrtplib/src/rtpudpv4transmitter.h"
    "E:/150w/rtplib/jrtplib/src/rtpudpv6transmitter.h"
    "E:/150w/rtplib/jrtplib/src/rtpbyteaddress.h"
    "E:/150w/rtplib/jrtplib/src/rtpexternaltransmitter.h"
    )
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

IF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")
  IF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Dd][Ee][Bb][Uu][Gg])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files/jrtplib/lib/jrtplib_d.lib")
    IF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    IF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
FILE(INSTALL DESTINATION "C:/Program Files/jrtplib/lib" TYPE STATIC_LIBRARY FILES "E:/150w/rtplib/jrtplib/src/DEBUG/jrtplib_d.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ee][Aa][Ss][Ee])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files/jrtplib/lib/jrtplib.lib")
    IF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    IF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
FILE(INSTALL DESTINATION "C:/Program Files/jrtplib/lib" TYPE STATIC_LIBRARY FILES "E:/150w/rtplib/jrtplib/src/RELEASE/jrtplib.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Mm][Ii][Nn][Ss][Ii][Zz][Ee][Rr][Ee][Ll])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files/jrtplib/lib/jrtplib.lib")
    IF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    IF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
FILE(INSTALL DESTINATION "C:/Program Files/jrtplib/lib" TYPE STATIC_LIBRARY FILES "E:/150w/rtplib/jrtplib/src/MinSizeRel/jrtplib.lib")
  ELSEIF("${CMAKE_INSTALL_CONFIG_NAME}" MATCHES "^([Rr][Ee][Ll][Ww][Ii][Tt][Hh][Dd][Ee][Bb][Ii][Nn][Ff][Oo])$")
    list(APPEND CMAKE_ABSOLUTE_DESTINATION_FILES
     "C:/Program Files/jrtplib/lib/jrtplib.lib")
    IF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(WARNING "ABSOLUTE path INSTALL DESTINATION : ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_WARN_ON_ABSOLUTE_INSTALL_DESTINATION)
    IF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
        message(FATAL_ERROR "ABSOLUTE path INSTALL DESTINATION forbidden (by caller): ${CMAKE_ABSOLUTE_DESTINATION_FILES}")
    ENDIF (CMAKE_ERROR_ON_ABSOLUTE_INSTALL_DESTINATION)
FILE(INSTALL DESTINATION "C:/Program Files/jrtplib/lib" TYPE STATIC_LIBRARY FILES "E:/150w/rtplib/jrtplib/src/RelWithDebInfo/jrtplib.lib")
  ENDIF()
ENDIF(NOT CMAKE_INSTALL_COMPONENT OR "${CMAKE_INSTALL_COMPONENT}" STREQUAL "Unspecified")

