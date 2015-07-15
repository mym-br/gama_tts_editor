#-------------------------------------------------
#
# Project created by QtCreator 2014-12-13T12:47:12
#
#-------------------------------------------------

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets
}
CONFIG += c++11

unix {
    !macx {
        CONFIG += link_pkgconfig
        PKGCONFIG += sndfile portaudiocpp
    }
}

TARGET = gs_editor
TEMPLATE = app

SOURCES += src/main.cpp \
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
    src/IntonationWindow.cpp \
    src/IntonationWidget.cpp \
    src/Synthesis.cpp \
    src/IntonationParametersWindow.cpp \
    src/AudioPlayer.cpp \
    src/AudioWorker.cpp

HEADERS += src/MainWindow.h \
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
    src/IntonationWindow.h \
    src/IntonationWidget.h \
    src/Synthesis.h \
    src/IntonationParametersWindow.h \
    src/AudioPlayer.h \
    src/AudioWorker.h

FORMS += ui/MainWindow.ui \
    ui/DataEntryWindow.ui \
    ui/SynthesisWindow.ui \
    ui/PrototypeManagerWindow.ui \
    ui/TransitionEditorWindow.ui \
    ui/PostureEditorWindow.ui \
    ui/RuleManagerWindow.ui \
    ui/RuleTesterWindow.ui \
    ui/IntonationWindow.ui \
    ui/IntonationParametersWindow.ui

INCLUDEPATH += src \
    src/qt_model

unix {
    !macx {
        QMAKE_CXXFLAGS += -Wall -Wextra -march=native

        exists(../gnuspeech_sa/CMakeLists.txt) {
            message(Using local Gnuspeech-SA)
            INCLUDEPATH += \
                ../gnuspeech_sa/src \
                ../gnuspeech_sa/src/trm \
                ../gnuspeech_sa/src/trm_control_model
            LIBS += -L../gnuspeech_sa-build -lgnuspeechsa
        } else {
            message(Using system Gnuspeech-SA)
            PKGCONFIG += gnuspeechsa
        }

        isEmpty(INSTALL_PREFIX) {
            INSTALL_PREFIX = /usr/local
        }
        target.path = $${INSTALL_PREFIX}/bin
        dataset.path = $${INSTALL_PREFIX}/share/gnuspeech/gs_editor
        dataset.files = data
        INSTALLS = target dataset
    }
}

win32 {
    INCLUDEPATH += \
        ../gnuspeech_sa/src \
        ../gnuspeech_sa/src/trm \
        ../gnuspeech_sa/src/trm_control_model \
        ../pa_stable_v19_20140130/portaudio/include \
        ../pa_stable_v19_20140130/portaudio/bindings/cpp/include \
        "c:/Program Files (x86)/Mega-Nerd/libsndfile/include"
    LIBS += \
        $$_PRO_FILE_PWD_/../pa_stable_v19_20140130/portaudio-cpp-build/portaudio-cpp.lib \
        $$_PRO_FILE_PWD_/../pa_stable_v19_20140130/portaudio-build/Release/portaudio_x86.lib \
        $$_PRO_FILE_PWD_/../gnuspeech_sa-build/Release/gnuspeechsa.lib \
        "c:/Program Files (x86)/Mega-Nerd/libsndfile/lib/libsndfile-1.lib" \
        -L"c:/Program Files (x86)/Microsoft SDKs/Windows/v7.1A/Lib"
}

DEPENDPATH += $${INCLUDEPATH}

MOC_DIR = tmp
OBJECTS_DIR = tmp
UI_DIR = tmp
