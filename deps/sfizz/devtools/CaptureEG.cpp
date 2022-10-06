// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "CaptureEG.h"
#include "ui_CaptureEG.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QMouseEvent>
#include <QDrag>
#include <QMimeData>
#include <QTimer>
#include <QStandardPaths>
#include <QFileInfo>
#include <QDir>
#include <QDebug>
#include <sndfile.hh>
#include <cmath>

Application::Application(int& argc, char *argv[])
    : QApplication(argc, argv), _ui(new Ui::MainWindow)
{
    setApplicationName("SfizzCaptureEG");
}

Application::~Application()
{
}

bool Application::init()
{
    _cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    if (_cacheDir.isEmpty()) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot determine the cache directory."));
        return false;
    }
    QDir(_cacheDir).mkpath(".");

    ///
    jack_client_t* client = jack_client_open(
        applicationName().toUtf8().data(), JackNoStartServer, nullptr);

    if (!client) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot register a new JACK client."));
        return false;
    }
    _client.reset(client);

    std::string clientName = jack_get_client_name(client);

    if (!(_portAudioIn = jack_port_register(client, "audio_in", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0)) ||
        !(_portMidiOut = jack_port_register(client, "midi_out", JACK_DEFAULT_MIDI_TYPE, JackPortIsOutput, 0)))
    {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot register the JACK client ports."));
        return false;
    }

    jack_set_process_callback(client, &processAudio, this);

    if (jack_activate(client) != 0) {
        QMessageBox::critical(nullptr, tr("Error"), tr("Cannot activate the JACK client."));
        return false;
    }

    // Try to connect Dimension if it exists
    {
        const char** synthAudioPorts = jack_get_ports(client, "^Dimension Pro:", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput);
        const char** synthMidiPorts = jack_get_ports(client, "^Dimension Pro:", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput);
        if (synthAudioPorts && *synthAudioPorts)
            jack_connect(client, *synthAudioPorts, jack_port_name(_portAudioIn));
        if (synthMidiPorts && *synthMidiPorts)
            jack_connect(client, jack_port_name(_portMidiOut), *synthMidiPorts);
        jack_free(synthAudioPorts);
        jack_free(synthMidiPorts);
    }

    // Allocate capture buffer (capacity 30 seconds)
    double sampleRate = jack_get_sample_rate(client);
    _captureCapacity = static_cast<size_t>(std::ceil(30.0 * sampleRate));
    _captureBuffer.reset(new float[_captureCapacity]);
    // Always capture a minimum of 0.5 seconds (ensure not stopping too early)
    _captureMinFrames = static_cast<size_t>(std::ceil(0.5 * sampleRate));

    ///
    QMainWindow* window = new QMainWindow;
    _window = window;
    _ui->setupUi(window);
    window->setWindowTitle(applicationDisplayName());
    window->adjustSize();
    window->setFixedSize(window->size());
    window->show();

    _ui->dragFileLabel->setDragFilePath(getSfzPath());
    _ui->dragFileLabel->setPixmap(
        QIcon::fromTheme("text-x-generic").pixmap(_ui->dragFileLabel->size()));

    _ui->releaseTimeVal->setRange(0.0, 10.0);
    _ui->releaseTimeVal->setValue(5.0);

    _ui->internalGainVal->setRange(0.1, 2.0);
    _ui->internalGainVal->setValue(0.342); // default to match Dimension

    _ui->saveButton->setEnabled(false);

    connect(
        _ui->envelopeEdit, &QPlainTextEdit::textChanged,
        this, [this]() { onSfzTextChanged(); });

    connect(
        _ui->captureButton, &QPushButton::clicked,
        this, [this]() { engageCapture(); });

    connect(
        _ui->saveButton, &QPushButton::clicked,
        this, [this]() { saveCapture(); });

    _sfzUpdateTimer = new QTimer;
    _sfzUpdateTimer->setInterval(500);
    _sfzUpdateTimer->setSingleShot(true);
    connect(_sfzUpdateTimer, &QTimer::timeout, this, [this]() { performSfzUpdate(); });

    _idleTimer = new QTimer;
    _idleTimer->setInterval(50);
    _idleTimer->setSingleShot(false);
    connect(_idleTimer, &QTimer::timeout, this, [this]() { performIdleChecks(); });

    _idleTimer->start();

    onSfzTextChanged();

    return true;
}

int Application::processAudio(unsigned numFrames, void* arg)
{
    auto* self = static_cast<Application*>(arg);

    const float* audioIn = static_cast<float*>(jack_port_get_buffer(self->_portAudioIn, numFrames));
    void* midiOut = jack_port_get_buffer(self->_portMidiOut, numFrames);

    jack_midi_clear_buffer(midiOut);

    if (self->_captureStatus != CaptureEngaged)
        return 0;

    bool over = false;

    size_t captureIndex = self->_captureFill;
    const size_t captureCapacity = self->_captureCapacity;
    const size_t captureMinFrames = self->_captureMinFrames;
    float* captureBuffer = self->_captureBuffer.get();

    constexpr float silentThreshold = 1e-4; // -80 dB

    long tt = self->_framesLeftToTrigger;
    long tr = self->_framesLeftToRelease;

    for (size_t i = 0; i < numFrames && !over; ++i) {
        if (tt == 0) {
            const unsigned char noteOn[3] = {0x90, 69, 127};
            jack_midi_event_write(midiOut, i, noteOn, sizeof(noteOn));
        }
        if (tr == 0) {
            const unsigned char noteOff[3] = {0x90, 69, 0};
            jack_midi_event_write(midiOut, i, noteOff, sizeof(noteOff));
        }
        --tt;
        --tr;
        if (captureIndex == captureCapacity)
            over = true;
        else {
            captureBuffer[captureIndex++] = audioIn[i];
            if (tr < 0 && captureIndex >= captureMinFrames && audioIn[i] < silentThreshold)
                over = true;
        }
    }

    self->_captureFill = captureIndex;
    self->_framesLeftToTrigger = tt;
    self->_framesLeftToRelease = tr;

    if (over)
        self->_captureStatus = CaptureOver;

    return 0;
}

QString Application::getSfzPath() const
{
    return _cacheDir + "/CaptureEG.sfz";
}

QString Application::getSamplePath() const
{
    return _cacheDir + "/CaptureEG.wav";
}

void Application::onSfzTextChanged()
{
    _ui->dragFileLabel->setEnabled(false);
    _sfzUpdateTimer->start();
}

void Application::engageCapture()
{
    if (_captureStatus != CaptureIdle)
        return;

    _ui->saveButton->setEnabled(false);

    const double sampleRate = jack_get_sample_rate(_client.get());

    _framesLeftToTrigger = 0;
    _framesLeftToRelease = static_cast<size_t>(std::ceil(sampleRate * _ui->releaseTimeVal->value()));
    _captureFill = 0;

    _captureStatus = CaptureEngaged;
}

void Application::saveCapture()
{
    if (_captureStatus != CaptureIdle)
        return;

    QString filePath = QFileDialog::getSaveFileName(
        _window, tr("Save data"), QString(), tr("Sound files (*.wav *.flac);;Data files (*.dat)"));
    if (filePath.isEmpty())
        return;

    QString fileSuffix = QFileInfo(filePath).suffix();
    if (fileSuffix.compare("wav", Qt::CaseInsensitive) == 0)
        saveSoundFile(filePath, SF_FORMAT_WAV);
    else if (fileSuffix.compare("flac", Qt::CaseInsensitive) == 0)
        saveSoundFile(filePath, SF_FORMAT_FLAC);
    else
        savePlotData(filePath);
}

void Application::saveSoundFile(const QString& path, int format)
{
    const float *data = _captureBuffer.get();
    size_t size = _captureFill;
    double sampleRate = jack_get_sample_rate(_client.get());
    double scaleFactor = 1.0 / _ui->internalGainVal->value();
    size_t captureLatency = jack_get_buffer_size(_client.get());

    SndfileHandle snd(path.toUtf8().data(), SFM_WRITE, format|SF_FORMAT_PCM_16, 1, sampleRate);

    for (size_t i = captureLatency; i < size; ++i) {
        float sample = scaleFactor * data[i];
        sample = std::max(-1.0f, std::min(+1.0f, sample));
        snd.write(&sample, 1);
    }

    snd.writeSync();

    if (snd.error())
        QFile::remove(path);
}

void Application::savePlotData(const QString& path)
{
    QFile file(path);
    file.open(QFile::WriteOnly|QFile::Truncate);
    QTextStream stream(&file);

    const float *data = _captureBuffer.get();
    size_t size = _captureFill;
    double sampleRate = jack_get_sample_rate(_client.get());
    double scaleFactor = 1.0 / _ui->internalGainVal->value();
    size_t captureLatency = jack_get_buffer_size(_client.get());

    for (size_t i = captureLatency; i < size; ++i)
        stream << ((i - captureLatency) / sampleRate)
               << ' ' << (scaleFactor * data[i]) << '\n';
}

void Application::performSfzUpdate()
{
    const QString sfzPath = getSfzPath();
    const QString samplePath = getSamplePath();

    QString code;
    code += "<region>\n";
    code += "key=69\n";
    code += "sample="; code += QFileInfo(samplePath).fileName(); code += "\n";
    code += _ui->envelopeEdit->toPlainText();

    QFile sfzFile(sfzPath);
    sfzFile.open(QFile::WriteOnly|QFile::Truncate);
    sfzFile.write(code.toUtf8());
    sfzFile.close();

    if (!QFile::exists(samplePath)) {
        // generate all-1s sound file of 30 seconds length
        constexpr float sampleRate = 44100.0;
        constexpr float duration = 30.0;
        constexpr size_t channels = 2;
        size_t frames = static_cast<size_t>(std::ceil(sampleRate * duration));

        SndfileHandle snd(
            samplePath.toUtf8().data(),
            SFM_WRITE, SF_FORMAT_PCM_16|SF_FORMAT_WAV, 2, sampleRate);

        float frameData[channels];
        for (size_t i = 0; i < channels; ++i)
            frameData[i] = 1.0;

        for (size_t i = 0; i < frames; ++i)
            snd.writef(frameData, 1);

        snd.writeSync();

        if (snd.error())
            QFile::remove(samplePath);
    }

    _ui->dragFileLabel->setEnabled(true);
}

void Application::performIdleChecks()
{
    if (_captureStatus == CaptureOver) {
        _captureStatus = CaptureIdle;
        _ui->saveButton->setEnabled(true);
    }
}

//------------------------------------------------------------------------------

void DragFileLabel::setDragFilePath(const QString& path)
{
    _dragFilePath = path;
}

void DragFileLabel::mousePressEvent(QMouseEvent *event)
{
    if (!_dragFilePath.isEmpty() && event->button() == Qt::LeftButton && rect().contains(event->pos())) {
        QMimeData *mimeData = new QMimeData;
        mimeData->setUrls(QList<QUrl>() << QUrl::fromLocalFile(_dragFilePath));

        QDrag *drag = new QDrag(this);
        drag->setMimeData(mimeData);
        drag->exec();
        drag->deleteLater();

        event->accept();
        return;
    }

    QLabel::mousePressEvent(event);
}

//------------------------------------------------------------------------------

int main(int argc, char* argv[])
{
    Application app(argc, argv);

    if (!app.init())
        return 1;

    return app.exec();
}
