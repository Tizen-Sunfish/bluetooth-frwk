CMAKE_MINIMUM_REQUIRED(VERSION 2.6)
PROJECT(bt-service-api C)

SET(SRCS
bt-common.c
bt-adapter.c
bt-device.c
bt-hid.c
bt-network.c
bt-audio.c
bt-oob.c
bt-hdp.c
bt-avrcp.c
bt-gatt.c
bt-telephony.c
bt-opp-client.c
bt-obex-server.c
bt-rfcomm-client.c
bt-rfcomm-server.c
bt-request-sender.c
bt-event-handler.c
bt-telephony-glue.c
bt-gatt-glue.c
)

SET(HEADERS
bluetooth-api.h
bluetooth-hid-api.h
bluetooth-audio-api.h
bluetooth-telephony-api.h
bluetooth-media-control.h)

SET(PREFIX ${CMAKE_INSTALL_PREFIX})
SET(EXEC_PREFIX "\${prefix}")
SET(LIBDIR "\${prefix}/lib")
SET(INCLUDEDIR "\${prefix}/include")
SET(VERSION_MAJOR 1)
SET(VERSION ${VERSION_MAJOR}.0.0)

INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/include)
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR})
INCLUDE_DIRECTORIES(${CMAKE_CURRENT_SOURCE_DIR}/include)

INCLUDE(FindPkgConfig)
pkg_check_modules(packages REQUIRED dlog dbus-glib-1 gobject-2.0 gmodule-2.0 vconf
libxml-2.0 syspopup-caller security-server glib-2.0 gio-unix-2.0)

FOREACH(flag ${packages_CFLAGS})
	SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} ${flag}")
ENDFOREACH(flag)

SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -fvisibility=hidden -Wall")
SET(EXTRA_CFLAGS "${EXTRA_CFLAGS} -g ")

SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${EXTRA_CFLAGS}")

FIND_PROGRAM(UNAME NAMES uname)
EXEC_PROGRAM("${UNAME}" ARGS "-m" OUTPUT_VARIABLE "ARCH")
IF("${ARCH}" STREQUAL "arm")
	ADD_DEFINITIONS("-DTARGET")
	MESSAGE("add -DTARGET")
ENDIF("${ARCH}" STREQUAL "arm")

FIND_PROGRAM(MARSHALTOOL NAMES glib-genmarshal)
EXEC_PROGRAM("${MARSHALTOOL}" ARGS "--prefix=marshal ${CMAKE_CURRENT_SOURCE_DIR}/marshal.list --header > ${CMAKE_CURRENT_SOURCE_DIR}/include/marshal.h")
EXEC_PROGRAM("${MARSHALTOOL}" ARGS "--prefix=marshal ${CMAKE_CURRENT_SOURCE_DIR}/marshal.list --body > ${CMAKE_CURRENT_SOURCE_DIR}/marshal.c")

FIND_PROGRAM(GDBUS_CODE_GEN NAMES gdbus-codegen)
EXEC_PROGRAM("${GDBUS_CODE_GEN}" ARGS "--interface-prefix bt --generate-c-code ${CMAKE_CURRENT_SOURCE_DIR}/bt-gatt-glue ${CMAKE_CURRENT_SOURCE_DIR}/bt-gatt.xml")

FIND_PROGRAM(DBUS_BINDING_TOOL NAMES dbus-binding-tool)
EXEC_PROGRAM("${DBUS_BINDING_TOOL}" ARGS "--prefix=bt ${CMAKE_CURRENT_SOURCE_DIR}/bt-request-service.xml --mode=glib-client --output=${CMAKE_CURRENT_SOURCE_DIR}/include/bt-request-service.h")

FIND_PROGRAM(GDBUS_CODE_GEN NAMES gdbus-codegen)
EXEC_PROGRAM("${GDBUS_CODE_GEN}" ARGS "--interface-prefix bt --generate-c-code ${CMAKE_CURRENT_SOURCE_DIR}/bt-telephony-glue ${CMAKE_CURRENT_SOURCE_DIR}/bt-telephony.xml")

ADD_LIBRARY(bluetooth-api SHARED ${SRCS})
SET_TARGET_PROPERTIES(bluetooth-api PROPERTIES SOVERSION ${VERSION_MAJOR})
SET_TARGET_PROPERTIES(bluetooth-api PROPERTIES VERSION ${VERSION})
TARGET_LINK_LIBRARIES(bluetooth-api ${packages_LDFLAGS})
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/bluetooth-api.pc.in ${CMAKE_CURRENT_BINARY_DIR}/bluetooth-api.pc @ONLY)
INSTALL(FILES ${CMAKE_CURRENT_BINARY_DIR}/bluetooth-api.pc DESTINATION lib/pkgconfig)
INSTALL(TARGETS bluetooth-api DESTINATION lib COMPONENT RuntimeLibraries)

FOREACH(hfile ${HEADERS})
	INSTALL(FILES ${CMAKE_SOURCE_DIR}/include/${hfile} DESTINATION include/bt-service)
ENDFOREACH(hfile)
