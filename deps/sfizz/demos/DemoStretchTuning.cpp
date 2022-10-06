// SPDX-License-Identifier: BSD-2-Clause

// This code is part of the sfizz library and is licensed under a BSD 2-clause
// license. You should have receive a LICENSE.md file along with the code.
// If not, contact the sfizz maintainers at https://github.com/sfztools/sfizz

#include "sfizz/Tuning.h"
#include "ui_DemoStretchTuning.h"
#include <QApplication>
#include <QMainWindow>
#include <QPainter>
#include <QPainterPath>
#include <QDebug>
#include <functional>
#include <cmath>

class DataPlotWidget;

///
class DemoApp : public QApplication {
public:
    DemoApp(int& argc, char** argv);
    void initWindow();

private:
    void updateStretch();

private:
    QMainWindow* fWindow = nullptr;
    DataPlotWidget* fDataPlot = nullptr;
    Ui::DemoStretchTuningWindow fUi;

    sfz::StretchTuning fTuning;
};

///
class DataPlotWidget : public QWidget {
public:
    explicit DataPlotWidget(QWidget* parent = nullptr);
    void setYRange(float y1, float y2);
    void setYFunction(std::function<qreal(qreal)> fun);

protected:
    void paintEvent(QPaintEvent* event) override;

    float yRange1_ = 0.0f;
    float yRange2_ = 0.0f;
    std::function<qreal(qreal)> yFunction_;
};

DataPlotWidget::DataPlotWidget(QWidget* parent)
    : QWidget(parent)
{
}

void DataPlotWidget::setYRange(float y1, float y2)
{
    yRange1_ = y1;
    yRange2_ = y2;
}

void DataPlotWidget::setYFunction(std::function<qreal(qreal)> fun)
{
    yFunction_ = fun;
    repaint();
}

void DataPlotWidget::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.fillRect(rect(), Qt::white);

    const qreal yRange1 = yRange1_;
    const qreal yRange2 = yRange2_;
    std::function<qreal(qreal)> yFunction = yFunction_;

    if (yRange1 == yRange2 || !yFunction)
        return;

    int w = width();
    int h = height();

    auto yPointToCoord = [h, yRange1, yRange2](qreal y) {
        qreal r = (y - yRange1) / (yRange2 - yRange1);
        return (1 - r) * (h - 1);
    };

    QPainterPath path;
    path.moveTo(0.0, yPointToCoord(yFunction(0.0)));
    for (int x = 1; x < w; ++x) {
        qreal wRatio = x * (qreal(1.0) / (w - 1));
        path.lineTo(x, yPointToCoord(yFunction(wRatio * 127)));
    }

    painter.strokePath(path, QPen(Qt::red, 1.0));
}

///
DemoApp::DemoApp(int& argc, char** argv)
    : QApplication(argc, argv)
{
    setApplicationName(tr("Sfizz Stretch Tuning"));
}

void DemoApp::initWindow()
{
    QMainWindow* window = new QMainWindow;
    fWindow = window;
    fUi.setupUi(window);

    QVBoxLayout* lPlot = new QVBoxLayout;
    lPlot->setContentsMargins(QMargins());
    DataPlotWidget* dataPlot = new DataPlotWidget;
    fDataPlot = dataPlot;
    lPlot->addWidget(dataPlot);
    fUi.frmPlot->setLayout(lPlot);

    dataPlot->setYRange(-50.0, +50.0);

    connect(
        fUi.valStretch, &QSlider::valueChanged,
        this, [this]() { updateStretch(); });

    fUi.valStretch->setValue(fUi.valStretch->maximum() / 2);

    window->setWindowTitle(applicationDisplayName());
    window->show();
}

void DemoApp::updateStretch()
{
    qreal stretch = qreal(fUi.valStretch->value()) / fUi.valStretch->maximum();

    sfz::StretchTuning& tuning = fTuning;
    tuning = sfz::StretchTuning::createRailsbackFromRatio(stretch);

    fDataPlot->setYFunction([this](qreal x) -> qreal {
        float ratio = fTuning.getRatioForFractionalKey(x);
        float cents = 1200.0f * std::log2(ratio);
        return cents;
    });
}

int main(int argc, char *argv[])
{
    DemoApp app(argc, argv);

    app.initWindow();

    return app.exec();
}
