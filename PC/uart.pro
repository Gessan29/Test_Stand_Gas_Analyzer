QT       += core gui serialport printsupport network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    protocol_parser.cpp \
    qcustomplot.cpp \
    uart.cpp \
    um_receiver_base.cpp \
    um_receiver_rs232.cpp \
    um_receiver_udp.cpp \
    um_sender_base.cpp \
    um_sender_rs232.cpp \
    um_sender_udp.cpp


HEADERS += \
    customdialog.h \
    data_logger_defs.h \
    mainwindow.h \
    protocol_parser.h \
    qcustomplot.h \
    uart.h \
    um_defs.h \
    um_protocol_defs.h \
    um_receiver_base.h \
    um_receiver_rs232.h \
    um_receiver_udp.h \
    um_sender_base.h \
    um_sender_rs232.h \
    um_sender_udp.h


FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
