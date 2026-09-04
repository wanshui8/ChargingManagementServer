QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    main.cpp \
    src/db/dbmanager.cpp \
    src/db/migrationmanager.cpp \
    src/dto/sysadmin.cpp \
    src/dto/chargingstation.cpp \
    src/dto/chargingpile.cpp \
    src/dto/appuser.cpp \
    src/dto/chargeorder.cpp \
    src/dto/walletrecharge.cpp \
    src/dto/operationlog.cpp \
    src/dao/sysadmindao.cpp \
    src/dao/chargingstationdao.cpp \
    src/dao/chargingpiledao.cpp \
    src/dao/appuserdao.cpp \
    src/dao/chargeorderdao.cpp \
    src/dao/walletrechargedao.cpp \
    src/dao/operationlogdao.cpp \
    src/service/login.cpp \
    src/ui/mainwindow.cpp \
    widget.cpp

HEADERS += \
    src/db/dbmanager.h \
    src/db/migrationmanager.h \
    src/dto/sysadmin.h \
    src/dto/chargingstation.h \
    src/dto/chargingpile.h \
    src/dto/appuser.h \
    src/dto/chargeorder.h \
    src/dto/walletrecharge.h \
    src/dto/operationlog.h \
    src/dao/sysadmindao.h \
    src/dao/chargingstationdao.h \
    src/dao/chargingpiledao.h \
    src/dao/appuserdao.h \
    src/dao/chargeorderdao.h \
    src/dao/walletrechargedao.h \
    src/dao/operationlogdao.h \
    src/service/login.h \
    src/ui/mainwindow.h \
    widget.h

FORMS += \
    widget.ui \
    src/ui/mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
