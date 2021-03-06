set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_BINARY_DIR})

add_definitions(-DMAIL_DATA_DIR="\\"${CMAKE_CURRENT_SOURCE_DIR}/data\\"")

include_directories(
  ${Boost_INCLUDE_DIRS}
  ${CMAKE_SOURCE_DIR}/libkleo
  ${CMAKE_SOURCE_DIR}/templateparser/
  ${CMAKE_BINARY_DIR}/templateparser/
)

# convenience macro to add qtest unit tests
macro(add_templateparser_unittest _source)
  set(_test ${_source})
  get_filename_component(_name ${_source} NAME_WE)
  kde4_add_unit_test(${_name} TESTNAME templateparser-${_name} ${_test})
  target_link_libraries(${_name}
    kleo
    templateparser
    messageviewer
    ${QGPGME_LIBRARIES}
    ${QT_QTTEST_LIBRARY}
    ${QT_QTCORE_LIBRARY}
    ${QT_QTWEBKIT_LIBRARY}
    ${KDE4_KIO_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  )
endmacro()

add_templateparser_unittest(templateparsertest.cpp)
