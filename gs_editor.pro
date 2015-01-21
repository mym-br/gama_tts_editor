#-------------------------------------------------
#
# Project created by QtCreator 2014-12-13T12:47:12
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += link_pkgconfig

TARGET = gs_editor
TEMPLATE = app


SOURCES += src/main.cpp\
        src/MainWindow.cpp \
    src/DataEntryWindow.cpp \
    src/qt_model/CategoryModel.cpp \
    src/Application.cpp \
    src/qt_model/ParameterModel.cpp \
    src/qt_model/SymbolModel.cpp \
    src/SynthesisWindow.cpp \
    src/EventWidget.cpp \
    src/PrototypeManagerWindow.cpp \
    src/TransitionEditorWindow.cpp \
    src/TransitionWidget.cpp \
    src/TransitionPoint.cpp \
    src/PostureEditorWindow.cpp \
    src/RuleManagerWindow.cpp \
    src/RuleTesterWindow.cpp \
    src/RuleEditorWindow.cpp \
    src/IntonationWindow.cpp \
    src/IntonationWidget.cpp \
    src/Synthesis.cpp \
    src/IntonationParametersWindow.cpp \
    src/AudioPlayer.cpp \
    src/AudioWorker.cpp

HEADERS  += src/MainWindow.h \
    src/DataEntryWindow.h \
    src/qt_model/CategoryModel.h \
    src/Application.h \
    src/AppConfig.h \
    src/qt_model/ParameterModel.h \
    src/qt_model/SymbolModel.h \
    src/LogStreamBuffer.h \
    src/SynthesisWindow.h \
    src/EventWidget.h \
    src/PrototypeManagerWindow.h \
    src/TransitionEditorWindow.h \
    src/TransitionWidget.h \
    src/TransitionPoint.h \
    src/SignalBlocker.h \
    src/PostureEditorWindow.h \
    src/RuleManagerWindow.h \
    src/RuleTesterWindow.h \
    src/RuleEditorWindow.h \
    src/IntonationWindow.h \
    src/IntonationWidget.h \
    src/Synthesis.h \
    src/IntonationParametersWindow.h \
    src/AudioPlayer.h \
    src/AudioWorker.h

FORMS    += ui/MainWindow.ui \
    ui/DataEntryWindow.ui \
    ui/SynthesisWindow.ui \
    ui/PrototypeManagerWindow.ui \
    ui/TransitionEditorWindow.ui \
    ui/PostureEditorWindow.ui \
    ui/RuleManagerWindow.ui \
    ui/RuleTesterWindow.ui \
    ui/RuleEditorWindow.ui \
    ui/IntonationWindow.ui \
    ui/IntonationParametersWindow.ui



#UI_DIR = ui
PKGCONFIG += sndfile portaudiocpp
INCLUDEPATH += src \
    src/qt_model \
    ../gnuspeech_sa/src \
    ../gnuspeech_sa/src/trm \
    ../gnuspeech_sa/src/trm_control_model

DEPENDPATH += $$INCLUDEPATH

QMAKE_CXXFLAGS += -std=c++11 -Wall -Wextra

LIBS += ../gnuspeech_sa-build/libgnuspeechsa.a
