
include_directories(
  ${CMAKE_BINARY_DIR}/akregator/interfaces
  ${CMAKE_SOURCE_DIR}/akregator/interfaces
)

########### next target ###############

set(kcm_akrgeneralconfig_SRCS akregator_config_general.cpp)

kde4_add_ui_files(kcm_akrgeneralconfig_SRCS settings_general.ui)
kde4_add_plugin(akregator_config_general ${kcm_akrgeneralconfig_SRCS})

target_link_libraries(akregator_config_general
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_general.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_general DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrappearanceconfig_SRCS akregator_config_appearance.cpp)

kde4_add_ui_files(kcm_akrappearanceconfig_SRCS settings_appearance.ui)
kde4_add_plugin(akregator_config_appearance ${kcm_akrappearanceconfig_SRCS})

target_link_libraries(akregator_config_appearance
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_appearance.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_appearance DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrarchiveconfig_SRCS akregator_config_archive.cpp)

kde4_add_ui_files(kcm_akrarchiveconfig_SRCS settings_archive.ui)
kde4_add_plugin(akregator_config_archive ${kcm_akrarchiveconfig_SRCS})

target_link_libraries(akregator_config_archive
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_archive.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_archive DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akrbrowserconfig_SRCS akregator_config_browser.cpp)

kde4_add_ui_files(kcm_akrbrowserconfig_SRCS settings_browser.ui)
kde4_add_plugin(akregator_config_browser ${kcm_akrbrowserconfig_SRCS})

target_link_libraries(akregator_config_browser
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_browser.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_browser DESTINATION ${PLUGIN_INSTALL_DIR})

########### next target ###############

set(kcm_akradvancedconfig_SRCS
  akregator_config_advanced.cpp
  settings_advanced.cpp
)

kde4_add_ui_files(kcm_akradvancedconfig_SRCS settings_advancedbase.ui)
kde4_add_plugin(akregator_config_advanced ${kcm_akradvancedconfig_SRCS})

target_link_libraries(akregator_config_advanced
  akregatorinterfaces
  ${KDE4_KIO_LIBS}
)

install(FILES akregator_config_advanced.desktop DESTINATION ${SERVICES_INSTALL_DIR})
install(TARGETS akregator_config_advanced DESTINATION ${PLUGIN_INSTALL_DIR})
