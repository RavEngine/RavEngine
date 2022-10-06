// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/SfzFilter.h"
#include "sfizz/MathHelpers.h"
#include "ui_DemoFilters.h"
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
    void valueChangedType(int value);
    void valueChangedCutoff(int value);
    void valueChangedResonance(int value);
    void valueChangedPkShGain(int value);
    void valueChangedBandwidth(int value);
    void valueChangedFilterMode(int value);
    void valueChangedCutoffModSpeed(int value);
    void valueChangedCutoffModRange(int value);

private:
    QMainWindow *fWindow = nullptr;
    Ui::DemoFiltersWindow fUi;

    static constexpr int cutoffMin = 10.0;
    static constexpr int cutoffMax = 20000.0;

    static constexpr int resoMin = 0.0;
    static constexpr int resoMax = 40.0;

    static constexpr int pkshMin = -40.0;
    static constexpr int pkshMax = 40.0;

    static constexpr int bwMin = 1.0;
    static constexpr int bwMax = 10.0;

    static constexpr float lfoRateMin = 0.1;
    static constexpr float lfoRateMax = 10.0;

    static constexpr int cutoffModMin = 0;
    static constexpr int cutoffModMax = 48;

    int fType = sfz::kFilterNone;
    int fCutoff = 500.0;
    int fReso = 0.0;
    int fPksh = 20.0;
    int fBw = 1.0;
    float fCutoffRate = 1.0;
    int fCutoffMod = 24.0;

    float fCutoffLfoPhase = 0.0f;

    sfz::Filter fFilter;
    sfz::FilterEq fFilterEq;

    enum FilterMode {
        kFilterModeMulti,
        kFilterModeEq,
    };
    FilterMode fFilterMode = kFilterModeMulti;

    jack_client_u fClient;
    jack_port_t *fPorts[4] = {};

    std::unique_ptr<float[]> fTempCutoff;
    std::unique_ptr<float[]> fTempReso;
    std::unique_ptr<float[]> fTempBw;
    std::unique_ptr<float[]> fTempPksh;
};

DemoApp::DemoApp(int &argc, char **argv)
    : QApplication(argc, argv)
{
    setApplicationName(tr("Sfizz Filters"));
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

    double sampleRate = jack_get_sample_rate(client);
    unsigned bufferSize = jack_get_buffer_size(client);

    fFilter.init(sampleRate);
    fFilter.setChannels(2);

    fFilterEq.init(sampleRate);
    fFilterEq.setType(sfz::kEqPeak); // TODO: make a chooser of EQ type
    fFilterEq.setChannels(2);

    fTempCutoff.reset(new float[bufferSize]);
    fTempReso.reset(new float[bufferSize]);
    fTempBw.reset(new float[bufferSize]);
    fTempPksh.reset(new float[bufferSize]);

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

    QComboBox *cbTypes = fUi.comboBox;
    cbTypes->addItem("None", static_cast<int>(sfz::kFilterNone));
    cbTypes->addItem("Apf1p", static_cast<int>(sfz::kFilterApf1p));
    cbTypes->addItem("Bpf1p", static_cast<int>(sfz::kFilterBpf1p));
    cbTypes->addItem("Bpf2p", static_cast<int>(sfz::kFilterBpf2p));
    cbTypes->addItem("Bpf4p", static_cast<int>(sfz::kFilterBpf4p));
    cbTypes->addItem("Bpf6p", static_cast<int>(sfz::kFilterBpf6p));
    cbTypes->addItem("Brf1p", static_cast<int>(sfz::kFilterBrf1p));
    cbTypes->addItem("Brf2p", static_cast<int>(sfz::kFilterBrf2p));
    cbTypes->addItem("Hpf1p", static_cast<int>(sfz::kFilterHpf1p));
    cbTypes->addItem("Hpf2p", static_cast<int>(sfz::kFilterHpf2p));
    cbTypes->addItem("Hpf4p", static_cast<int>(sfz::kFilterHpf4p));
    cbTypes->addItem("Hpf6p", static_cast<int>(sfz::kFilterHpf6p));
    cbTypes->addItem("Lpf1p", static_cast<int>(sfz::kFilterLpf1p));
    cbTypes->addItem("Lpf2p", static_cast<int>(sfz::kFilterLpf2p));
    cbTypes->addItem("Lpf4p", static_cast<int>(sfz::kFilterLpf4p));
    cbTypes->addItem("Lpf6p", static_cast<int>(sfz::kFilterLpf6p));
    cbTypes->addItem("Pink", static_cast<int>(sfz::kFilterPink));
    cbTypes->addItem("Lpf2pSv", static_cast<int>(sfz::kFilterLpf2pSv));
    cbTypes->addItem("Hpf2pSv", static_cast<int>(sfz::kFilterHpf2pSv));
    cbTypes->addItem("Bpf2pSv", static_cast<int>(sfz::kFilterBpf2pSv));
    cbTypes->addItem("Brf2pSv", static_cast<int>(sfz::kFilterBrf2pSv));
    cbTypes->addItem("Lsh", static_cast<int>(sfz::kFilterLsh));
    cbTypes->addItem("Hsh", static_cast<int>(sfz::kFilterHsh));
    cbTypes->addItem("Peq", static_cast<int>(sfz::kFilterPeq));

    cbTypes->setCurrentIndex(cbTypes->findData(fType));
    fUi.lcdType->display(fType);

    connect(
        cbTypes, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, [this, cbTypes](int index) { valueChangedType(cbTypes->itemData(index).toInt()); });

    fUi.dialCutoff->setRange(cutoffMin, cutoffMax);
    fUi.dialResonance->setRange(resoMin, resoMax);
    fUi.dialPkShGain->setRange(pkshMin, pkshMax);
    fUi.dialBandwidth->setRange(bwMin, bwMax);
    fUi.spinCutoff->setRange(cutoffMin, cutoffMax);
    fUi.spinResonance->setRange(resoMin, resoMax);
    fUi.spinPkShGain->setRange(pkshMin, pkshMax);
    fUi.spinBandwidth->setRange(bwMin, bwMax);
    fUi.valCutoffModSpeed->setRange(lfoRateMin * 1e3, lfoRateMax * 1e3);
    fUi.valCutoffModRange->setRange(cutoffModMin, cutoffModMax);

    fUi.dialCutoff->setValue(fCutoff);
    fUi.dialResonance->setValue(fReso);
    fUi.dialPkShGain->setValue(fPksh);
    fUi.dialBandwidth->setValue(fBw);
    fUi.spinCutoff->setValue(fCutoff);
    fUi.spinResonance->setValue(fReso);
    fUi.spinPkShGain->setValue(fPksh);
    fUi.spinBandwidth->setValue(fBw);
    fUi.valCutoffModSpeed->setValue(fCutoffRate * 1e3);
    fUi.valCutoffModRange->setValue(fCutoffMod);
    fUi.lblCutoffModSpeed->setText(QString::number(fCutoffRate));
    fUi.lblCutoffModRange->setText(QString::number(fCutoffMod));

    connect(
        fUi.dialCutoff, &QDial::valueChanged,
        this, [this](int value) { valueChangedCutoff(value); });
    connect(
        fUi.spinCutoff, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedCutoff(value); });
    connect(
        fUi.dialResonance, &QDial::valueChanged,
        this, [this](int value) { valueChangedResonance(value); });
    connect(
        fUi.spinResonance, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedResonance(value); });
    connect(
        fUi.dialPkShGain, &QDial::valueChanged,
        this, [this](int value) { valueChangedPkShGain(value); });
    connect(
        fUi.spinPkShGain, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedPkShGain(value); });
    connect(
        fUi.dialBandwidth, &QDial::valueChanged,
        this, [this](int value) { valueChangedBandwidth(value); });
    connect(
        fUi.spinBandwidth, QOverload<int>::of(&QSpinBox::valueChanged),
        this, [this](int value) { valueChangedBandwidth(value); });
    connect(
        fUi.valCutoffModSpeed, &QSlider::valueChanged,
        this, [this](int value) { valueChangedCutoffModSpeed(value); });
    connect(
        fUi.valCutoffModRange, &QSlider::valueChanged,
        this, [this](int value) { valueChangedCutoffModRange(value); });

    QButtonGroup *grpMode = new QButtonGroup(this);
    grpMode->addButton(fUi.btnMultiMode, kFilterModeMulti);
    grpMode->addButton(fUi.btnEqMode, kFilterModeEq);
    fUi.btnMultiMode->setChecked(true);
    grpMode->setExclusive(true);

    connect(
        grpMode, &QButtonGroup::idToggled, this,
        [this](int id, bool toggled) {
            if (toggled)
                valueChangedFilterMode(id);
        });

    window->adjustSize();
    window->setFixedSize(window->size());

    window->show();
}

int DemoApp::processAudio(jack_nframes_t nframes, void *cbdata)
{
    DemoApp *self = reinterpret_cast<DemoApp *>(cbdata);

    const float *ins[2];
    float *outs[2];

    ins[0] = reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[0], nframes));
    ins[1] = reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[1], nframes));
    outs[0] = reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[2], nframes));
    outs[1] = reinterpret_cast<float *>(jack_port_get_buffer(self->fPorts[3], nframes));

    float *tempCutoff = self->fTempCutoff.get();
    float *tempReso = self->fTempReso.get();
    float *tempBw = self->fTempBw.get();
    float *tempPksh = self->fTempPksh.get();

    std::fill(tempCutoff, tempCutoff + nframes, self->fCutoff);
    std::fill(tempReso, tempReso + nframes, self->fReso);
    std::fill(tempBw, tempBw + nframes, self->fBw);
    std::fill(tempPksh, tempPksh + nframes, self->fPksh);

    float cutoffLfoPhase = self->fCutoffLfoPhase;
    float cutoffRate = self->fCutoffRate;
    float cutoffMod = self->fCutoffMod;

    float sampleTime = 1.0 / jack_get_sample_rate(self->fClient.get());

    auto triangleLfo = [](float phase) -> float {
        float y = -4 * phase + 2;
        y = (phase < 0.25f) ? (4 * phase) : y;
        y = (phase > 0.75f) ? (4 * phase - 4) : y;
        return y;
    };

    for (jack_nframes_t i = 0; i < nframes; ++i) {
        float lfo = cutoffMod * triangleLfo(cutoffLfoPhase);
        float modCutoff = tempCutoff[i];
        modCutoff *= std::exp2(lfo * (1.0f / 12.0f));
        modCutoff = clamp(modCutoff, 0.0f, 20000.0f);
        tempCutoff[i] = modCutoff;
        cutoffLfoPhase += cutoffRate * sampleTime;
        cutoffLfoPhase -= (int)cutoffLfoPhase;
    }

    switch (self->fFilterMode) {
    default:
    case kFilterModeMulti:
        self->fFilter.setType(static_cast<sfz::FilterType>(self->fType));
        self->fFilter.processModulated(ins, outs, tempCutoff, tempReso, tempPksh, nframes);
        break;
    case kFilterModeEq:
        self->fFilterEq.processModulated(ins, outs, tempCutoff, tempBw, tempPksh, nframes);
        break;
    }

    self->fCutoffLfoPhase = cutoffLfoPhase;

    return 0;
}

void DemoApp::valueChangedType(int value)
{
    fType = value;
    fUi.lcdType->display(value);
}

void DemoApp::valueChangedCutoff(int value)
{
    fUi.dialCutoff->blockSignals(true);
    fUi.dialCutoff->setValue(value);
    fUi.dialCutoff->blockSignals(false);

    fUi.spinCutoff->blockSignals(true);
    fUi.spinCutoff->setValue(value);
    fUi.spinCutoff->blockSignals(false);

    fCutoff = value;
}

void DemoApp::valueChangedResonance(int value)
{
    fUi.dialResonance->blockSignals(true);
    fUi.dialResonance->setValue(value);
    fUi.dialResonance->blockSignals(false);

    fUi.spinResonance->blockSignals(true);
    fUi.spinResonance->setValue(value);
    fUi.spinResonance->blockSignals(false);

    fReso = value;
}

void DemoApp::valueChangedPkShGain(int value)
{
    fUi.dialPkShGain->blockSignals(true);
    fUi.dialPkShGain->setValue(value);
    fUi.dialPkShGain->blockSignals(false);

    fUi.spinPkShGain->blockSignals(true);
    fUi.spinPkShGain->setValue(value);
    fUi.spinPkShGain->blockSignals(false);

    fPksh = value;
}

void DemoApp::valueChangedBandwidth(int value)
{
    fUi.dialBandwidth->blockSignals(true);
    fUi.dialBandwidth->setValue(value);
    fUi.dialBandwidth->blockSignals(false);

    fUi.spinBandwidth->blockSignals(true);
    fUi.spinBandwidth->setValue(value);
    fUi.spinBandwidth->blockSignals(false);

    fBw = value;
}

void DemoApp::valueChangedFilterMode(int value)
{
    fUi.stackedWidget->setCurrentIndex(value);

    fFilterMode = static_cast<FilterMode>(value);
}

void DemoApp::valueChangedCutoffModSpeed(int value)
{
    fUi.lblCutoffModSpeed->setText(QString::number(value * 1e-3));

    fCutoffRate = value * 1e-3;
}

void DemoApp::valueChangedCutoffModRange(int value)
{
    fUi.lblCutoffModRange->setText(QString::number(value));

    fCutoffMod = value;
}

int main(int argc, char *argv[])
{
    DemoApp app(argc, argv);

    if (!app.initSound())
        return 1;

    app.initWindow();

    return app.exec();
}
