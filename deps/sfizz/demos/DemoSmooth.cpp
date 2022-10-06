// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/Smoothers.h"
#include "sfizz/MathHelpers.h"
#include "ui_DemoSmooth.h"
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QButtonGroup>
#include <QDebug>
#include <jack/jack.h>
#include <memory>
#include <cmath>

///
struct jack_delete {
    void operator()(jack_client_t* x) const noexcept { jack_client_close(x); }
};

typedef std::unique_ptr<jack_client_t, jack_delete> jack_client_u;

///
class DemoApp : public QApplication {
public:
    DemoApp(int& argc, char** argv);
    bool initSound();
    void initWindow();

private:
    static int processAudio(jack_nframes_t nframes, void* cbdata);

private:
    void valueChangedSmooth(int value);

private:
    QMainWindow* fWindow = nullptr;
    Ui::DemoSmoothWindow fUi;

    static constexpr int smoothMin = 0;
    static constexpr int smoothMax = 100;

    int fSmooth = 0;

    sfz::Smoother fFilter;

    jack_client_u fClient;
    jack_port_t* fPorts[2] = {};
};

DemoApp::DemoApp(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName(tr("Sfizz Smooth"));
}

bool DemoApp::initSound()
{
    jack_client_t* client = jack_client_open(
        applicationName().toUtf8().data(), JackNoStartServer, nullptr);
    if (!client) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot open JACK audio."));
        return false;
    }

    fClient.reset(client);

    double sampleRate = jack_get_sample_rate(client);
    fFilter.setSmoothing(fSmooth, sampleRate);

    fPorts[0] = jack_port_register(client, "in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    fPorts[1] = jack_port_register(client, "out", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (!(fPorts[0] && fPorts[1])) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot register JACK ports."));
        return false;
    }

    jack_set_process_callback(client, &processAudio, this);

    if (jack_activate(client) != 0) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot activate JACK client."));
        return false;
    }

    return true;
}

void DemoApp::initWindow()
{
    QMainWindow* window = new QMainWindow;
    fWindow = window;
    fUi.setupUi(window);
    window->setWindowTitle(applicationDisplayName());

    fUi.dialSmooth->setRange(smoothMin, smoothMax);
    fUi.spinSmooth->setRange(smoothMin, smoothMax);

    fUi.dialSmooth->setValue(fSmooth);
    fUi.spinSmooth->setValue(fSmooth);

    connect(
        fUi.dialSmooth, &QDial::valueChanged,
        this, [this](int value) { valueChangedSmooth(value); });
    connect(
        fUi.spinSmooth, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedSmooth(value); });

    window->adjustSize();
    window->setFixedSize(window->size());

    window->show();
}

int DemoApp::processAudio(jack_nframes_t nframes, void* cbdata)
{
    DemoApp* self = reinterpret_cast<DemoApp*>(cbdata);

    const float* in = reinterpret_cast<float*>(jack_port_get_buffer(self->fPorts[0], nframes));
    float* out = reinterpret_cast<float*>(jack_port_get_buffer(self->fPorts[1], nframes));

    self->fFilter.process(absl::MakeSpan(in, nframes), absl::MakeSpan(out, nframes));
    return 0;
}

void DemoApp::valueChangedSmooth(int value)
{
    fUi.dialSmooth->blockSignals(true);
    fUi.dialSmooth->setValue(value);
    fUi.dialSmooth->blockSignals(false);

    fUi.spinSmooth->blockSignals(true);
    fUi.spinSmooth->setValue(value);
    fUi.spinSmooth->blockSignals(false);

    fSmooth = value;

    double sampleRate = jack_get_sample_rate(fClient.get());
    fFilter.setSmoothing(fSmooth, sampleRate);
}

int main(int argc, char* argv[])
{
    DemoApp app(argc, argv);

    if (!app.initSound())
        return 1;

    app.initWindow();

    return app.exec();
}
