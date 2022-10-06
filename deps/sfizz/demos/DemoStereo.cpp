// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/Panning.h"
#include "ui_DemoStereo.h"
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QButtonGroup>
#include <QDebug>
#include <jack/jack.h>
#include <memory>

///
struct jack_delete {
    void operator()(jack_client_t *x) const noexcept { jack_client_close(x); }
};

typedef std::unique_ptr<jack_client_t, jack_delete> jack_client_u;

///
class DemoApp : public QApplication {
public:
    DemoApp(int &argc, char **argv);
    bool initSound();
    void initWindow();

private:
    static int processAudio(jack_nframes_t nframes, void *cbdata);

private:
    void valueChangedWidth(int value);
    void valueChangedPan(int value);

private:
    QMainWindow *fWindow = nullptr;
    Ui::DemoStereoWindow fUi;

    static constexpr int widthMin = -100;
    static constexpr int widthMax = +100;

    static constexpr int panMin = -100;
    static constexpr int panMax = +100;

    int fWidth = 100;
    int fPan = 0;

    std::unique_ptr<float[]> fTmpWidthEnvelope;
    std::unique_ptr<float[]> fTmpPositionEnvelope;
    std::unique_ptr<float[]> fTmpBuffer1;

    jack_client_u fClient;
    jack_port_t *fPorts[4] = {};
};

DemoApp::DemoApp(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName(tr("Sfizz Stereo"));
}

bool DemoApp::initSound()
{
    jack_client_t *client = jack_client_open(
        applicationName().toUtf8().data(), JackNoStartServer, nullptr);
    if (!client) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot open JACK audio."));
        return false;
    }

    fClient.reset(client);

    uint32_t bufsize = jack_get_buffer_size(client);
    fTmpWidthEnvelope.reset(new float[bufsize]);
    fTmpPositionEnvelope.reset(new float[bufsize]);
    fTmpBuffer1.reset(new float[bufsize]);

    fPorts[0] = jack_port_register(client, "in_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    fPorts[1] = jack_port_register(client, "in_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    fPorts[2] = jack_port_register(client, "out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    fPorts[3] = jack_port_register(client, "out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

    if (!(fPorts[0] && fPorts[1] && fPorts[2] && fPorts[3])) {
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
    QMainWindow *window = new QMainWindow;
    fWindow = window;
    fUi.setupUi(window);
    window->setWindowTitle(applicationDisplayName());

    fUi.valWidth->setRange(widthMin, widthMax);
    fUi.valPan->setRange(panMin, panMax);
    fUi.spinWidth->setRange(widthMin, widthMax);
    fUi.spinPan->setRange(panMin, panMax);

    fUi.valWidth->setValue(fWidth);
    fUi.valPan->setValue(fPan);
    fUi.spinWidth->setValue(fWidth);
    fUi.spinPan->setValue(fPan);

    connect(
        fUi.valWidth, &QSlider::valueChanged,
        this, [this](int value) { valueChangedWidth(value); });
    connect(
        fUi.spinWidth, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedWidth(value); });
    connect(
        fUi.valPan, &QSlider::valueChanged,
        this, [this](int value) { valueChangedPan(value); });
    connect(
        fUi.spinPan, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedPan(value); });

    window->adjustSize();
    window->setFixedSize(window->size());

    window->show();
}

int DemoApp::processAudio(jack_nframes_t nframes, void *cbdata)
{
    DemoApp *self = reinterpret_cast<DemoApp *>(cbdata);

    absl::Span<float> leftBuffer {
        reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[2], nframes)),
        nframes};
    absl::Span<float> rightBuffer {
        reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[3], nframes)),
        nframes};

    std::copy_n(
        reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[0], nframes)),
        nframes, leftBuffer.begin());
    std::copy_n(
        reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[1], nframes)),
        nframes, rightBuffer.begin());

    absl::Span<float> widthEnvelope{self->fTmpWidthEnvelope.get(), nframes};
    absl::Span<float> positionEnvelope{self->fTmpPositionEnvelope.get(), nframes};
    absl::Span<float> tempSpan1{self->fTmpBuffer1.get(), nframes};

    std::fill(widthEnvelope.begin(), widthEnvelope.end(), self->fWidth * 0.01f);
    std::fill(positionEnvelope.begin(), positionEnvelope.end(), self->fPan * 0.01f);

    using namespace sfz;
    width(widthEnvelope, leftBuffer, rightBuffer);
    pan(positionEnvelope, leftBuffer, rightBuffer);
    return 0;
}

void DemoApp::valueChangedWidth(int value)
{
    fUi.valWidth->blockSignals(true);
    fUi.valWidth->setValue(value);
    fUi.valWidth->blockSignals(false);

    fUi.spinWidth->blockSignals(true);
    fUi.spinWidth->setValue(value);
    fUi.spinWidth->blockSignals(false);

    fWidth = value;
}

void DemoApp::valueChangedPan(int value)
{
    fUi.valPan->blockSignals(true);
    fUi.valPan->setValue(value);
    fUi.valPan->blockSignals(false);

    fUi.spinPan->blockSignals(true);
    fUi.spinPan->setValue(value);
    fUi.spinPan->blockSignals(false);

    fPan = value;
}

int main(int argc, char *argv[])
{
    DemoApp app(argc, argv);

    if (!app.initSound())
        return 1;

    app.initWindow();

    return app.exec();
}
