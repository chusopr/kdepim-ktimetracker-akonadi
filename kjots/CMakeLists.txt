project(kjots)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

if("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  find_package(KDE4 REQUIRED)
  find_package(Akonadi REQUIRED)
  find_package(KdepimLibs 4.4.60 REQUIRED)
  find_package(Grantlee NO_MODULE REQUIRED)
  include(KDE4Defaults)
  include(MacroLibrary)
  include(CheckIncludeFiles)

  set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )
  include_directories( ../akonadi/akonadi_next )

  set(KJOTS_BUILD_KONTACT_PLUGIN FALSE)
else("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")
  option(KJOTS_BUILD_KONTACT_PLUGIN "Build kjots with support for kontact" TRUE)
endif("${CMAKE_SOURCE_DIR}" STREQUAL "${CMAKE_CURRENT_SOURCE_DIR}")

include_directories(
  ${KDE4_INCLUDES}
  ${KDEPIMLIBS_INCLUDE_DIRS}
  ${Boost_INCLUDE_DIRS}
  ${Grantlee_INCLUDE_DIRS}
)

set(kjotsLibs ${KDE4_KIO_LIBS})

if(KJOTS_BUILD_KONTACT_PLUGIN)
  add_definitions(-DKJOTSKONTACTPLUGIN)
  set(kjotsLibs ${kjotsLibs} kdepim)
endif(KJOTS_BUILD_KONTACT_PLUGIN)

add_subdirectory( icons )
add_subdirectory( plasmoid )

#########################################################################
# Code common to the kjots application and the kontact plugin
# Don't make it a static library, this code needs to be compiled
# with -fPIC for the part and without -fPIC for the executable
#########################################################################

set(kjots_config_SRCS kjotsconfigdlg.cpp)

kde4_add_kcfg_files(kjots_config_SRCS KJotsSettings.kcfgc )

kde4_add_ui_files(kjots_config_SRCS confpagemisc.ui)


set (kjots_common_SRCS
     aboutdata.cpp
     kjotslockattribute.cpp
     kjotslockjob.cpp
     kjotsedit.cpp
     kjotstreeview.cpp
     kjotsbookmarks.cpp
     kjotsreplacenextdialog.cpp
     kjotsmodel.cpp
     kjotssortproxymodel.cpp
     kjotswidget.cpp
     kjotsbrowser.cpp
     kjotslinkdialog.cpp
     localresourcecreator.cpp
     kjotsbookshelfentryvalidator.cpp
     knowitimporter.cpp ${kjots_config_SRCS}
)

#########################################################################
# Auto creation for local notes.
#########################################################################

macro(add_resource_iface _kcfgFile _ifaceName _className)
  kcfg_generate_dbus_interface(${_kcfgFile} ${_ifaceName})
  string(TOLOWER ${_className} _codeFile)
  set_source_files_properties( ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml PROPERTIES INCLUDE "metatype.h")
  qt4_add_dbus_interface(kjots_common_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/${_ifaceName}.xml ${_codeFile} ${_className}
  )
endmacro(add_resource_iface)

add_resource_iface( ${CMAKE_CURRENT_SOURCE_DIR}/maildirresource.kcfg
                    org.kde.Akonadi.Maildir.Settings MaildirSettings )

add_custom_target(kjots_settings_xml ALL
  DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/org.kde.Akonadi.Maildir.Settings.xml
  SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/maildirresource.kcfg
)

#########################################################################
# D-Bus interface
#########################################################################

qt4_generate_dbus_interface( kjotswidget.h org.kde.KJotsWidget.xml OPTIONS -m )
qt4_add_dbus_interfaces(kjots_common_SRCS
    ${CMAKE_CURRENT_BINARY_DIR}/org.kde.KJotsWidget.xml)


#########################################################################
# Main Application SECTION
#########################################################################

set(kjots_SRCS
   main.cpp
   KJotsMain.cpp
   ${kjots_common_SRCS}
)

kde4_add_executable(kjots ${kjots_SRCS})

target_link_libraries(kjots
  ${kjotsLibs}
  ${KDE4_KCMUTILS_LIBS}
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${Grantlee_CORE_LIBRARIES}
  ${Grantlee_GUI_LIBRARIES}
  ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
  akonadi_next
)

if(KJOTS_BUILD_KONTACT_PLUGIN)
  target_link_libraries(kjots ${KDEPIMLIBS_KONTACTINTERFACE_LIBS} )
endif(KJOTS_BUILD_KONTACT_PLUGIN)


install(TARGETS kjots  ${INSTALL_TARGETS_DEFAULT_ARGS} )


########### install files ###############

install( PROGRAMS Kjots.desktop  DESTINATION ${XDG_APPS_INSTALL_DIR})
install( FILES kjots.kcfg  DESTINATION ${KCFG_INSTALL_DIR})
install( FILES kjotsui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

install(DIRECTORY themes DESTINATION ${DATA_INSTALL_DIR}/kjots PATTERN *.svn EXCLUDE )




# If building kontact plugin:
if ( KJOTS_BUILD_KONTACT_PLUGIN  )
  #########################################################################
  # KPART SECTION
  #########################################################################

  # This part is loaded by kontact if kjots is embedded in it.

  set(kjotspart_PART_SRCS
    kjotspart.cpp
    ${kjots_common_SRCS}
  )

  kde4_add_plugin(kjotspart ${kjotspart_PART_SRCS}
  )


  target_link_libraries(kjotspart
    ${KDE4_KCMUTILS_LIBS}
    ${KDE4_KPARTS_LIBS}
    kdepim
    ${KDEPIMLIBS_KMIME_LIBS}
    ${KDEPIMLIBS_AKONADI_LIBS}
    ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
    ${KDEPIMLIBS_KMIME_LIBS}
    ${Grantlee_CORE_LIBRARIES}
    ${Grantlee_GUI_LIBRARIES}
    ${KDEPIMLIBS_KPIMTEXTEDIT_LIBS}
    akonadi_next
  )

  install( FILES kjotspart.desktop  DESTINATION ${SERVICES_INSTALL_DIR} )
  install( FILES kjotspartui.rc  DESTINATION ${DATA_INSTALL_DIR}/kjots)

  install(TARGETS kjotspart  DESTINATION ${PLUGIN_INSTALL_DIR} )


  ###########################################################################
  set(kcm_kjots_PART_SRCS kcm_kjots.cpp ${kjots_config_SRCS} )

  kde4_add_plugin(kcm_kjots ${kcm_kjots_PART_SRCS})

  target_link_libraries(kcm_kjots  ${KDE4_KDEUI_LIBS} ${KDE4_KCMUTILS_LIBS})

  install(TARGETS kcm_kjots DESTINATION ${PLUGIN_INSTALL_DIR})


  install(FILES
        kjots_config_misc.desktop
        DESTINATION ${SERVICES_INSTALL_DIR})



endif ( KJOTS_BUILD_KONTACT_PLUGIN )
