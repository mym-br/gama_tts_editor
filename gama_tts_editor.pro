TEMPLATE = app
TARGET = gama_tts_editor
QT += core gui

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += widgets printsupport
    CONFIG += c++14
} else {
    error(Qt 4 is not supported.)
}

unix {
    !macx {
        CONFIG += link_pkgconfig
        PKGCONFIG += portaudiocpp jack fftw3f
    }
}

HEADERS += \
    src/AppConfig.h \
    src/AudioPlayer.h \
    src/AudioWorker.h \
    src/Clipboard.h \
    src/DataEntryWindow.h \
    src/EventWidget.h \
    src/IntonationParametersWindow.h \
    src/IntonationWidget.h \
    src/IntonationWindow.h \
    src/MainWindow.h \
    src/PostureEditorWindow.h \
    src/PrototypeManagerWindow.h \
    src/qt_model/CategoryModel.h \
    src/qt_model/ParameterModel.h \
    src/qt_model/SymbolModel.h \
    src/RuleManagerWindow.h \
    src/RuleTesterWindow.h \
    src/Synthesis.h \
    src/SynthesisWindow.h \
    src/TransitionEditorWindow.h \
    src/TransitionPoint.h \
    src/TransitionWidget.h \
    ../gama_tts_interactive/src/AnalysisWindow.h \
    ../gama_tts_interactive/src/Audio.h \
    ../gama_tts_interactive/src/FFTW.h \
    ../gama_tts_interactive/src/global.h \
    ../gama_tts_interactive/src/InteractiveVTMWindow.h \
    ../gama_tts_interactive/src/JackClient.h \
    ../gama_tts_interactive/src/JackRingbuffer.h \
    ../gama_tts_interactive/src/ParameterLineEdit.h \
    ../gama_tts_interactive/src/ParameterSlider.h \
    ../gama_tts_interactive/src/ProgramConfiguration.h \
    ../gama_tts_interactive/src/qcustomplot/qcustomplot.h \
    ../gama_tts_interactive/src/SignalDFT.h

SOURCES += \
    src/AudioPlayer.cpp \
    src/AudioWorker.cpp \
    src/Clipboard.cpp \
    src/DataEntryWindow.cpp \
    src/EventWidget.cpp \
    src/IntonationParametersWindow.cpp \
    src/IntonationWidget.cpp \
    src/IntonationWindow.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/PostureEditorWindow.cpp \
    src/PrototypeManagerWindow.cpp \
    src/qt_model/CategoryModel.cpp \
    src/qt_model/ParameterModel.cpp \
    src/qt_model/SymbolModel.cpp \
    src/RuleManagerWindow.cpp \
    src/RuleTesterWindow.cpp \
    src/Synthesis.cpp \
    src/SynthesisWindow.cpp \
    src/TransitionEditorWindow.cpp \
    src/TransitionPoint.cpp \
    src/TransitionWidget.cpp \
    ../gama_tts_interactive/src/AnalysisWindow.cpp \
    ../gama_tts_interactive/src/Audio.cpp \
    ../gama_tts_interactive/src/FFTW.cpp \
    ../gama_tts_interactive/src/InteractiveVTMWindow.cpp \
    ../gama_tts_interactive/src/JackClient.cpp \
    ../gama_tts_interactive/src/JackRingbuffer.cpp \
    ../gama_tts_interactive/src/ParameterLineEdit.cpp \
    ../gama_tts_interactive/src/ParameterSlider.cpp \
    ../gama_tts_interactive/src/ProgramConfiguration.cpp \
    ../gama_tts_interactive/src/qcustomplot/qcustomplot.cpp \
    ../gama_tts_interactive/src/SignalDFT.cpp

FORMS += \
    ui/DataEntryWindow.ui \
    ui/IntonationParametersWindow.ui \
    ui/IntonationWindow.ui \
    ui/MainWindow.ui \
    ui/PostureEditorWindow.ui \
    ui/PrototypeManagerWindow.ui \
    ui/RuleManagerWindow.ui \
    ui/RuleTesterWindow.ui \
    ui/SynthesisWindow.ui \
    ui/TransitionEditorWindow.ui \
    ../gama_tts_interactive/ui/AnalysisWindow.ui

INCLUDEPATH += \
    src \
    src/qt_model \
    ../gama_tts_interactive/src \
    ../gama_tts_interactive/src/qcustomplot

unix {
    !macx {
        QMAKE_CXXFLAGS += -Wall -Wextra

        exists(../gama_tts/CMakeLists.txt) {
            message(Using local GamaTTS)
            INCLUDEPATH += \
                ../gama_tts/src \
                ../gama_tts/src/vtm \
                ../gama_tts/src/vtm_control_model
            LIBS += -L../gama_tts-build -lgamatts
        } else {
            message(Using system GamaTTS)
            PKGCONFIG += gama_tts
        }

        isEmpty(INSTALL_PREFIX) {
            INSTALL_PREFIX = /usr/local
        }
        target.path = $${INSTALL_PREFIX}/bin
        INSTALLS = target
    }
}

MOC_DIR = tmp
OBJECTS_DIR = tmp
UI_DIR = tmp
