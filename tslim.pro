QT       += core gui widgets
QT += charts

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    Battery.cpp \
    BolusSettings.cpp \
    Calculator.cpp \
    ControlIQ.cpp \
    Logger.cpp \
    Person.cpp \
    RangeMonitor.cpp \
    TimedSettings.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    Battery.h \
    BolusSettings.h \
    Calculator.h \
    ControlIQ.h \
    Logger.h \
    Person.h \
    RangeMonitor.h \
    TimedSettings.h \
    mainwindow.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
