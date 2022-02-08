QT += core
QT += gui
QT += serialport
QT += widgets

CONFIG += c++11

DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    AxisFrame.cpp \
    AxisLimits.cpp \
    DataSetProperties.cpp \
    axesdialog.cpp \
    communicationmodule.cpp \
    datastream2d.cpp \
    main.cpp \
    mainwindow.cpp \
    plot2d.cpp \
    plotpropertiesdlg.cpp \
    tgp261.cpp

HEADERS += \
    AxisFrame.h \
    AxisLimits.h \
    DataSetProperties.h \
    axesdialog.h \
    communicationmodule.h \
    datastream2d.h \
    mainwindow.h \
    plot2d.h \
    plotpropertiesdlg.h \
    tgp261.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resources.qrc

DISTFILES += \
    plot.png \
    tgp261.png
