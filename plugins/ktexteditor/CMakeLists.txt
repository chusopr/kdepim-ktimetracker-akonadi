
include_directories( ${KDE4_INCLUDE_DIR} )

########### next target ###############

set(ktexteditorkabcbridge_PART_SRCS ktexteditorkabcbridge.cpp )

kde4_add_plugin(ktexteditorkabcbridge  ${ktexteditorkabcbridge_PART_SRCS})

target_link_libraries(ktexteditorkabcbridge  ${KDE4_KDEUI_LIBS} ${KDEPIMLIBS_KABC_LIBS}
                                             ${KDEPIMLIBS_KPIMIDENTITIES_LIBS} ${KDE4_KCMUTILS_LIBS})

install(TARGETS ktexteditorkabcbridge  DESTINATION ${PLUGIN_INSTALL_DIR})
