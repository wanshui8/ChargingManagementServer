QT += core gui widgets network webenginewidgets charts

CONFIG += c++11
TARGET = charging_client
TEMPLATE = app

# 共享协议库
include(../common/common.pri)

SOURCES += \
    main.cpp \
    tcpclient.cpp \
    mainwindow.cpp \
    loginpage.cpp \
    stationlistpage.cpp \
    piledetailpage.cpp \
    chargepage.cpp \
    profilepage.cpp \
    mapview.cpp

HEADERS += \
    tcpclient.h \
    mainwindow.h \
    loginpage.h \
    stationlistpage.h \
    piledetailpage.h \
    chargepage.h \
    profilepage.h \
    mapview.h