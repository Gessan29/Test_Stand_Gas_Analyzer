QT       += core gui serialport printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    func_acm.cpp \
    main.cpp \
    mainwindow.cpp \
    protocol_parser.cpp \
    qcustomplot.cpp \
    test_mcu.cpp \
    uart.cpp


HEADERS += \
    customdialog.h \
    func_acm.h \
    mainwindow.h \
    protocol_parser.h \
    qcustomplot.h \
    test_mcu.h \
    uart.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
