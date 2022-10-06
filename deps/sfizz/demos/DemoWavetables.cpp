// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/Wavetables.h"
#include "sfizz/MathHelpers.h"
#include "ui_DemoWavetables.h"
#include <QApplication>
#include <QMainWindow>
#include <QMessageBox>
#include <QButtonGroup>
#include <QDebug>
#include <jack/jack.h>
#include <atomic>
#include <memory>
#include <cstring>

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
    void valueChangedWave(int value);
    void valueChangedQuality(int value);
    void buttonClickedPlaySweep();

private:
    QMainWindow* fWindow = nullptr;
    Ui::DemoWavetablesWindow fUi;

    sfz::WavetableMulti fMulti[4];
    sfz::WavetableOscillator fOsc;
    unsigned fWavePlaying = 0;
    std::atomic<int> fNewWavePending { -1 };
    std::atomic<int> fNewQualityPending { -1 };
    std::atomic<bool> fStartNewSweep { false };

    static constexpr float sweepMin = 0.0;
    static constexpr float sweepMax = 136.0;
    static constexpr float sweepDuration = 3.0;

    float fSweepCurrent = sweepMax;
    float fSweepIncrement = 0.0;

    std::unique_ptr<float[]> fTmpFrequency;
    std::unique_ptr<float[]> fTmpDetune;

    jack_client_u fClient;
    jack_port_t* fPorts[2] = {};
};

constexpr float DemoApp::sweepMin;
constexpr float DemoApp::sweepMax;
constexpr float DemoApp::sweepDuration;

DemoApp::DemoApp(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName(tr("Sfizz Wavetables"));
}

bool DemoApp::initSound()
{
    jack_client_t* client = jack_client_open(
        applicationName().toUtf8().data(), JackNoStartServer, nullptr);
    if (!client) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot open JACK audio."));
        return false;
    }

    double sampleRate = jack_get_sample_rate(client);
    fOsc.init(sampleRate);
    fSweepIncrement = ((sweepMax - sweepMin) / (sweepDuration * sampleRate));

    unsigned bufferSize = jack_get_buffer_size(client);
    fTmpFrequency.reset(new float[bufferSize]);
    fTmpDetune.reset(new float[bufferSize]);

    fMulti[0] = sfz::WavetableMulti::createForHarmonicProfile(
        sfz::HarmonicProfile::getSine(), sfz::config::amplitudeSine, 2048);
    fMulti[1] = sfz::WavetableMulti::createForHarmonicProfile(
        sfz::HarmonicProfile::getTriangle(), sfz::config::amplitudeTriangle, 2048);
    fMulti[2] = sfz::WavetableMulti::createForHarmonicProfile(
        sfz::HarmonicProfile::getSaw(), sfz::config::amplitudeSaw, 2048);
    fMulti[3] = sfz::WavetableMulti::createForHarmonicProfile(
        sfz::HarmonicProfile::getSquare(), sfz::config::amplitudeSquare, 2048);

    fClient.reset(client);

    fPorts[0] = jack_port_register(client, "out_left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    fPorts[1] = jack_port_register(client, "out_right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);

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

    fUi.valWave->addItem(tr("1 - Sine"));
    fUi.valWave->addItem(tr("2 - Triangle"));
    fUi.valWave->addItem(tr("3 - Saw"));
    fUi.valWave->addItem(tr("4 - Square"));

    fUi.valQuality->addItem(tr("1 - Nearest"));
    fUi.valQuality->addItem(tr("2 - Linear"));
    fUi.valQuality->addItem(tr("3 - High"));
    fUi.valQuality->addItem(tr("4 - Dual-High"));

    fUi.valQuality->setCurrentIndex(fOsc.quality());

    connect(
        fUi.valWave, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) { valueChangedWave(index); });

    connect(
        fUi.valQuality, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this](int index) { valueChangedQuality(index); });

    connect(
        fUi.btnPlaySweep, &QPushButton::clicked,
        this, [this]() { buttonClickedPlaySweep(); });

    window->adjustSize();
    window->setFixedSize(window->size());

    window->show();
}

int DemoApp::processAudio(jack_nframes_t nframes, void* cbdata)
{
    DemoApp* self = reinterpret_cast<DemoApp*>(cbdata);

    sfz::WavetableOscillator& osc = self->fOsc;

    int newWave = self->fNewWavePending.exchange(-1);
    if (newWave != -1)
        self->fWavePlaying = newWave;

    int newQuality = self->fNewQualityPending.exchange(-1);
    if (newQuality != -1)
        osc.setQuality(newQuality);

    osc.setWavetable(&self->fMulti[self->fWavePlaying]);

    float* left = reinterpret_cast<float*>(
        jack_port_get_buffer(self->fPorts[0], nframes));
    float* right = reinterpret_cast<float*>(
        jack_port_get_buffer(self->fPorts[1], nframes));

    // sweep the pitch of the oscillator
    float* frequency = self->fTmpFrequency.get();
    float sweepCurrent = self->fSweepCurrent;
    if (self->fStartNewSweep.exchange(false))
        sweepCurrent = sweepMin;
    float sweepIncrement = self->fSweepIncrement;
    for (unsigned i = 0; i < nframes; ++i) {
        frequency[i] = 440.0f * std::pow(2.0f, (sweepCurrent - 69.0f) * (1.0f / 12.0f));
        sweepCurrent = std::min(sweepMax, sweepCurrent + sweepIncrement);
    }
    self->fSweepCurrent = sweepCurrent;

    // fill the detune value
    float* detune = self->fTmpDetune.get();
    std::fill(detune, detune + nframes, 1.0f);

    // compute oscillator
    osc.processModulated(frequency, detune, left, nframes);
    std::memcpy(right, left, nframes * sizeof(float));

    return 0;
}

void DemoApp::valueChangedWave(int value)
{
    fNewWavePending.store(value);
}

void DemoApp::valueChangedQuality(int value)
{
    fNewQualityPending.store(value);
}

void DemoApp::buttonClickedPlaySweep()
{
    fStartNewSweep.store(true);
}

int main(int argc, char* argv[])
{
    DemoApp app(argc, argv);

    if (!app.initSound())
        return 1;

    app.initWindow();

    return app.exec();
}
