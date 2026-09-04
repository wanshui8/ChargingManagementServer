# 共享协议库源文件，由 server.pro / client.pro 通过 include(common/common.pri) 引入
INCLUDEPATH += $$PWD

HEADERS += $$PWD/protocol.h \
    $$PWD/datatypes.h

SOURCES += $$PWD/protocol.cpp