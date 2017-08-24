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
        PKGCONFIG += jack fftw3f
    }
}

HEADERS += \
    src/AppConfig.h \
    src/AudioPlayer.h \
    src/AudioWorker.h \
    src/Clipboard.h \
    src/DataEntryWindow.h \
    src/editor_global.h \
    src/interactive/AnalysisWindow.h \
    src/interactive/Audio.h \
    src/interactive/FFTW.h \
    src/interactive/InteractiveVTMWindow.h \
    src/interactive/JackRingbuffer.h \
    src/interactive/ParameterLineEdit.h \
    src/interactive/ParameterSlider.h \
    src/interactive/ProgramConfiguration.h \
    src/interactive/qcustomplot/qcustomplot.h \
    src/interactive/SignalDFT.h \
    src/IntonationParametersWindow.h \
    src/IntonationWidget.h \
    src/IntonationWindow.h \
    src/JackClient.h \
    src/MainWindow.h \
    src/ParameterWidget.h \
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
    src/ParameterModificationWidget.h \
    src/ParameterModificationWindow.h \
    src/ParameterModificationSynthesis.h

SOURCES += \
    src/AudioPlayer.cpp \
    src/AudioWorker.cpp \
    src/Clipboard.cpp \
    src/DataEntryWindow.cpp \
    src/interactive/AnalysisWindow.cpp \
    src/interactive/Audio.cpp \
    src/interactive/FFTW.cpp \
    src/interactive/InteractiveVTMWindow.cpp \
    src/interactive/JackRingbuffer.cpp \
    src/interactive/ParameterLineEdit.cpp \
    src/interactive/ParameterSlider.cpp \
    src/interactive/ProgramConfiguration.cpp \
    src/interactive/qcustomplot/qcustomplot.cpp \
    src/interactive/SignalDFT.cpp \
    src/IntonationParametersWindow.cpp \
    src/IntonationWidget.cpp \
    src/IntonationWindow.cpp \
    src/JackClient.cpp \
    src/main.cpp \
    src/MainWindow.cpp \
    src/ParameterWidget.cpp \
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
    src/ParameterModificationWidget.cpp \
    src/ParameterModificationWindow.cpp \
    src/ParameterModificationSynthesis.cpp

FORMS += \
    ui/DataEntryWindow.ui \
    ui/interactive/AnalysisWindow.ui \
    ui/IntonationParametersWindow.ui \
    ui/IntonationWindow.ui \
    ui/MainWindow.ui \
    ui/PostureEditorWindow.ui \
    ui/PrototypeManagerWindow.ui \
    ui/RuleManagerWindow.ui \
    ui/RuleTesterWindow.ui \
    ui/SynthesisWindow.ui \
    ui/TransitionEditorWindow.ui \
    ui/ParameterModificationWindow.ui

INCLUDEPATH += \
    src \
    src/interactive/ \
    src/interactive/qcustomplot \
    src/qt_model

unix {
    !macx {
        QMAKE_CXXFLAGS += -Wall -Wextra

        exists(../gama_tts/CMakeLists.txt) {
            message(Using local GamaTTS)
            INCLUDEPATH += \
                ../gama_tts/src \
                ../gama_tts/src/text_parser \
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
