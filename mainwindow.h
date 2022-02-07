#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QDateTime>
#include <QFile>

#include "tgp261.h"
#include "plot2d.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void show();

public slots:
    void onNewData(QString sData);

protected:
    void closeEvent(QCloseEvent*) Q_DECL_OVERRIDE;
    void saveSettings();
    void restoreSettings();
    bool checkFileName();
    bool prepareOutputFile(QString sBaseDir, QString sFileName);
    void writeFileHeader();

private slots:
    void on_buttonPath_clicked();
    void on_buttonStart_clicked();

private:
    Ui::MainWindow *ui;
    QSettings    settings;
    tgp261*      pTgp261;
    QFile*       pOutputFile;
    Plot2D*      pPlotMeasurements;
    QDateTime    currentTime;
    QDateTime    startMeasuringTime;
    QDateTime    dateStart;
    QString      sSampleInfo;
    QString      sBaseDir;
    QString      sOutFileName;
    bool         bRunning;
};
