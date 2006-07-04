######################################################################
# Automatically generated by qmake (2.00a) Mon Apr 10 14:28:07 2006
######################################################################

TEMPLATE = app
TARGET += 
DEPENDPATH += .
INCLUDEPATH += ../compat ../common . ..

# Input
HEADERS += form.h \
           formcreator.h \
           formgui.h \
           guielement.h \
           guihandler.h \
           guihandlerdialogs.h \
           guihandlerflat.h \
           hints.h \
           input.h \
           kresult.h \
           list.h \
           listmodel.h \
           mainwindow.h \
           manager.h \
           reference.h \
           select1.h \
           textarea.h \
           xformscommon.h \
           xmlbuilder.h \
           prefs.h
SOURCES += form.cpp \
           formcreator.cpp \
           formgui.cpp \
           guielement.cpp \
           guihandler.cpp \
           guihandlerdialogs.cpp \
           guihandlerflat.cpp \
           hints.cpp \
           input.cpp \
           kresult.cpp \
           list.cpp \
           listmodel.cpp \
           main.cpp \
           mainwindow.cpp \
           manager.cpp \
           reference.cpp \
           select1.cpp \
           textarea.cpp \
           xformscommon.cpp \
           xmlbuilder.cpp \
           prefs.cpp

QT += xml

LIBS += -L../kxml_compiler -lkschema
LIBS += -L../schema -lschema
LIBS += -L../common -lkxmlcommon
LIBS += -L../compat -lcompat

DUMMY_MOCS = formgui.moc guihandlerdialogs.moc guihandlerflat.moc \
  input.moc list.moc mainwindow.moc remotefile.moc

system( touch $$DUMMY_MOCS )
