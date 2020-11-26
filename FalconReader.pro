QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17
QMAKE_CXXFLAGS += -std=c++17
QMAKE_CXXFLAGS += -static -static-libgcc -static-libstdc++ -lpthread
QMAKE_LFLAGS += -static -static-libgcc -static-libstdc++ -lpthread

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    contextwindow.cpp \
    filereadertask.cpp \
    main.cpp \
    mainwindow.cpp \
    options.cpp \
    tabcontent.cpp

HEADERS += \
    contextwindow.h \
    filereadertask.h \
    mainwindow.h \
    options.h \
    settings.h \
    tabcontent.h

FORMS += \
    contextwindow.ui \
    mainwindow.ui \
    options.ui \
    tabcontent.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
