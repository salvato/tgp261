/* MIT License

// Copyright (c) 2021 Gabriele Salvato

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
*/

#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "plot2d.h"

#include <QSettings>
#include <QFile>
#include <QDir>
#include <QFileDialog>
#include <QMessageBox>


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , pTgp261(new tgp261())
    , pOutputFile(nullptr)
    , pPlotMeasurements(nullptr)
    , sBaseDir(QDir::homePath())
    , sOutFileName("data.dat")
    , bRunning(false)
{
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    ui->setupUi(this);
    restoreSettings();
    ui->editInfo->setPlainText(sSampleInfo);
    ui->editPath->setText(sBaseDir);
    ui->editFileName->setText(sOutFileName);
    pTgp261 = new tgp261();
    connect(pTgp261, SIGNAL(dataReady(QString)),
            this, SLOT(onNewData(QString)));
}


MainWindow::~MainWindow() {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    ui->statusbar->showMessage("Saving Configuration...");
    QCoreApplication::processEvents();
    saveSettings();
    if(pTgp261) {
        disconnect(pTgp261);
        delete pTgp261;
    }
    pTgp261 = nullptr;
    if(pPlotMeasurements)
        delete pPlotMeasurements;
    pPlotMeasurements = nullptr;
    delete ui;
    QApplication::restoreOverrideCursor();
}


void
MainWindow::closeEvent(QCloseEvent *event) {
    Q_UNUSED(event)
    if(pOutputFile) {
        if(pOutputFile->isOpen())
            pOutputFile->close();
        pOutputFile->deleteLater();
        pOutputFile = nullptr;
    }
    if(pPlotMeasurements)
        delete pPlotMeasurements;
    pPlotMeasurements = nullptr;
}


void
MainWindow::restoreSettings() {
    QSettings settings;
    sSampleInfo    = settings.value("FileTabSampleInfo", "").toString();
    sBaseDir       = settings.value("FileTabBaseDir", sBaseDir).toString();
    sOutFileName   = settings.value("FileTabOutFileName", sOutFileName).toString();
}


void
MainWindow::saveSettings() {
    QSettings settings;
    sSampleInfo = ui->editInfo->toPlainText();
    settings.setValue("FileTabSampleInfo", sSampleInfo);
    settings.setValue("FileTabBaseDir", sBaseDir);
    settings.setValue("FileTabOutFileName", sOutFileName);
}


void
MainWindow::show() {
    QMainWindow::show();
    ui->statusbar->showMessage("Initializing TGP261 ...");
    QCoreApplication::processEvents();
    if(!pTgp261->Init()) {
        ui->statusbar->showMessage("Unable to Initialize TGP261 ...");
        ui->buttonStart->setText("Connect");
        QCoreApplication::processEvents();
    }
    else
        ui->statusbar->showMessage("TGP261 Initialized");

    pPlotMeasurements = new Plot2D(nullptr, "Pressure [mbar] vs Time [s]");
    pPlotMeasurements->setMaxPoints(3000);
    pPlotMeasurements->SetLimits(0.0, 1.0, 0.1, 1.0, true, true, false, false);
    // Datasets
    pPlotMeasurements->NewDataSet(0,                   //Id
                                  3,                   //Pen Width
                                  QColor(255, 255, 64),// Color
                                  Plot2D::ipoint,      // Symbol
                                  "Unused"             // Title
                       );
    pPlotMeasurements->NewDataSet(1,                   //Id
                                  3,                   //Pen Width
                                  QColor(208, 208, 255),// Color
                                  Plot2D::ipoint,      // Symbol
                                  "Saved"              // Title
                       );
    pPlotMeasurements->SetShowDataSet(0, true);
    pPlotMeasurements->SetShowTitle(0, true);
    pPlotMeasurements->SetShowDataSet(1, true);
    pPlotMeasurements->SetShowTitle(1, true);
    pPlotMeasurements->UpdatePlot();
    pPlotMeasurements->show();
    startMeasuringTime = QDateTime::currentDateTime();
    QApplication::restoreOverrideCursor();
}


void
MainWindow::onNewData(QString sData) {
    double y = sData.toDouble();
    currentTime = QDateTime::currentDateTime();
    double x = startMeasuringTime.secsTo(currentTime);
    if(bRunning) {
        QString sData = QString("%1 %2\n")
                                .arg(x, 12, 'g', 6, ' ')
                                .arg(y, 12, 'g', 6, ' ');
        pOutputFile->write(sData.toLocal8Bit());
        pOutputFile->flush();
        if(pPlotMeasurements) {
            pPlotMeasurements->NewPoint(1, x, y);
        }
    }
    else {
        if(pPlotMeasurements) {
            pPlotMeasurements->NewPoint(0, x, y);
        }
    }
    if(pPlotMeasurements) {
        pPlotMeasurements->UpdatePlot();
    }
}


void
MainWindow::on_buttonPath_clicked() {
    QFileDialog chooseDirDialog;
    QDir outDir(sBaseDir);
    chooseDirDialog.setFileMode(QFileDialog::Directory);
    chooseDirDialog.setOption(QFileDialog::ShowDirsOnly);
    if(outDir.exists())
        chooseDirDialog.setDirectory(outDir);
    else
        chooseDirDialog.setDirectory(QDir::homePath());
    if(chooseDirDialog.exec() == QDialog::Accepted)
        sBaseDir = chooseDirDialog.selectedFiles().at(0);
    ui->editPath->setText(sBaseDir);
}


bool
MainWindow::checkFileName() {
    sOutFileName = ui->editFileName->text();
    if(sOutFileName == QString()) {
        QMessageBox::information(
                    this,
                    QString("Empty Output Filename"),
                    QString("Please enter a Valid Output File Name"));
        ui->editFileName->setFocus();
        return false;
    }
    if(QDir(sBaseDir).exists(sOutFileName)) {
        QMessageBox* msg = new QMessageBox(
                    QMessageBox::Question,
                    QString("File Exists"),
                    QString("Do you want overwrite\n%1 ?").arg(sOutFileName),
                    QMessageBox::Yes|QMessageBox::No);
        int iAnswer = msg->exec();
        if(iAnswer == QMessageBox::No) {
            ui->editFileName->setFocus();
            return false;
        }
    }
    return true;
}


void
MainWindow::on_buttonStart_clicked() {
    if(ui->buttonStart->text() == QString("Connect")) {
        if(!pTgp261->Init()) {
            ui->statusbar->showMessage("Unable to Initialize TGP261 ...");
        }
        else {
            ui->statusbar->showMessage("TGP261 Initialized");
            ui->buttonStart->setText("Start");
        }
        return;
    }
    if(ui->buttonStart->text() == QString("Stop")) {
        bRunning = false;
        ui->buttonStart->setText("Start");
        ui->buttonPath->setEnabled(true);
        ui->editFileName->setEnabled(true);
        ui->editInfo->setEnabled(true);
        ui->statusbar->showMessage("Measurement Done & File Written");
        return;
    }
    if(checkFileName()) {
        // Open the Output file
        ui->statusbar->showMessage("Opening Output file...");
        if(!prepareOutputFile(sBaseDir, sOutFileName)) {
            ui->statusbar->showMessage("Unable to Open the Output file...");
            ui->buttonStart->setText("Start");
            QApplication::restoreOverrideCursor();
            return;
        }
        writeFileHeader();
        // Init the Plots
        pPlotMeasurements->setWindowTitle(ui->editFileName->text());
        pPlotMeasurements->ClearDataSet(0);
        pPlotMeasurements->ClearDataSet(1);
        pPlotMeasurements->UpdatePlot();
        ui->buttonStart->setText(QString("Stop"));
        ui->statusbar->showMessage("Measure in Progress...");
        ui->buttonPath->setDisabled(true);
        ui->editFileName->setDisabled(true);
        ui->editInfo->setDisabled(true);
        startMeasuringTime = QDateTime::currentDateTime();
        bRunning = true;
    }
}


bool
MainWindow::prepareOutputFile(QString sBaseDir, QString sFileName) {
    if(pOutputFile) {
        pOutputFile->close();
        pOutputFile->deleteLater();
        pOutputFile = Q_NULLPTR;
    }
    pOutputFile = new QFile(sBaseDir + "/" + sFileName);
    if(!pOutputFile->open(QIODevice::Text|QIODevice::WriteOnly)) {
        QMessageBox::critical(this,
                              "Error: Unable to Open Output File",
                              QString("%1/%2")
                              .arg(sBaseDir, sFileName));
        ui->statusbar->showMessage("Unable to Open Output file...");
        return false;
    }
    return true;
}


void
MainWindow::writeFileHeader() {
    // Write the header
    // To cope with the GnuPlot way to handle the comment lines
    // we need a # as a first chraracter in each row.
    pOutputFile->write(QString("%1 %2\n")
                       .arg("#Time[s]", 12)
                       .arg("Pressure[mbar]", 12)
                       .toLocal8Bit());

    QStringList HeaderLines = ui->editInfo->toPlainText().split("\n");
    for(int i=0; i<HeaderLines.count(); i++) {
        pOutputFile->write("# ");
        pOutputFile->write(HeaderLines.at(i).toLocal8Bit());
        pOutputFile->write("\n");
    }
    pOutputFile->flush();
}
