// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#pragma once
#include <QApplication>
#include <QLabel>
#include <QString>
#include <jack/jack.h>
#include <jack/midiport.h>
#include <memory>
#include <atomic>
class QMainWindow;
namespace Ui { class MainWindow; }

class Application : public QApplication {
public:
    Application(int& argc, char *argv[]);
    ~Application();
    bool init();

private:
    static int processAudio(unsigned numFrames, void* arg);

private:
    QMainWindow* _window = nullptr;
    std::unique_ptr<Ui::MainWindow> _ui;

    QTimer* _sfzUpdateTimer = nullptr;
    QTimer* _idleTimer = nullptr;
    QString _cacheDir;

    QString getSfzPath() const;
    QString getSamplePath() const;

    void onSfzTextChanged();
    void engageCapture();
    void saveCapture();

    void saveSoundFile(const QString& path, int format);
    void savePlotData(const QString& path);

    void performSfzUpdate();
    void performIdleChecks();

private:
    // capture status
    enum CaptureStatus { CaptureIdle, CaptureEngaged, CaptureOver };
    std::atomic<CaptureStatus> _captureStatus { CaptureIdle };
    //
    bool _capturing = false;
    long _framesLeftToTrigger = 0;
    long _framesLeftToRelease = 0;
    std::unique_ptr<float[]> _captureBuffer;
    size_t _captureCapacity = 0;
    size_t _captureMinFrames = 0;
    size_t _captureFill = 0;

private:
    struct jack_client_delete {
        void operator()(jack_client_t* x) const noexcept { jack_client_close(x); }
    };

    std::unique_ptr<jack_client_t, jack_client_delete> _client;
    jack_port_t* _portAudioIn = nullptr;
    jack_port_t* _portMidiOut = nullptr;
};

//------------------------------------------------------------------------------

class DragFileLabel : public QLabel {
public:
    using QLabel::QLabel;

    void setDragFilePath(const QString& path);

    void mousePressEvent(QMouseEvent *event) override;

private:
    QString _dragFilePath;
};
