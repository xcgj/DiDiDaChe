HEADERS += \
    server.h \
    redis.h

SOURCES += \
    server.cpp \
    main.cpp \
    redis.cpp

QT += sql network
LIBS += -ltufao1 -lhiredis
CONFIG += C++11
