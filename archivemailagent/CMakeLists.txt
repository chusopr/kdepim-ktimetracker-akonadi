project(archivemailagent)

include_directories(
   ${CMAKE_SOURCE_DIR}/libkdepim
)

set(archivemailagent_SRCS
   archivemailkernel.cpp
   archivemailagent.cpp
   archivemailmanager.cpp
   archivemaildialog.cpp
   archivemailinfo.cpp
   addarchivemaildialog.cpp
   archivejob.cpp
   archivemailagentutil.cpp
)

qt4_add_dbus_adaptor(archivemailagent_SRCS org.freedesktop.Akonadi.ArchiveMailAgent.xml archivemailagent.h ArchiveMailAgent)

#include_directories(${Boost_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/mailimporter )
set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${KDE4_ENABLE_EXCEPTIONS}" )

kde4_add_ui_files(archivemailagent_SRCS ui/archivemailwidget.ui )

kde4_add_executable(akonadi_archivemail_agent ${archivemailagent_SRCS})

target_link_libraries(akonadi_archivemail_agent
  ${KDEPIMLIBS_AKONADI_LIBS}
  ${KDEPIMLIBS_KPIMIDENTITIES_LIBS}
  ${KDEPIMLIBS_KMIME_LIBS}
  ${KDEPIMLIBS_AKONADI_KMIME_LIBS}
  ${KDE4_KIO_LIBS}
  mailcommon
)

if (Q_WS_MAC)
  set_target_properties(akonadi_archivemail_agent PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/Info.plist.template)
  set_target_properties(akonadi_archivemail_agent PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER "org.kde.Akonadi.archivemail")
  set_target_properties(akonadi_archivemail_agent PROPERTIES MACOSX_BUNDLE_BUNDLE_NAME "KDE Akonadi Email Archiver")
endif (Q_WS_MAC)

install(TARGETS akonadi_archivemail_agent ${INSTALL_TARGETS_DEFAULT_ARGS} )

install(FILES archivemailagent.desktop DESTINATION "${CMAKE_INSTALL_PREFIX}/share/akonadi/agents")
install(FILES akonadi_archivemail_agent.notifyrc DESTINATION "${DATA_INSTALL_DIR}/akonadi_archivemail_agent" )


