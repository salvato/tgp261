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

#include "communicationmodule.h"

#include <QThread>
#include <QDebug>
#include <QDateTime>
#include <QMessageBox>
#include <QCoreApplication>


CommunicationModule::CommunicationModule(QObject *parent)
    : QObject(parent)
    , sFilename(QString("/dev/ttyUSB0"))
    , serialPortName(sFilename)
    , receivedData(QString())
{
    serialPort.setPortName(serialPortName);
// Data Format:
//    1 start bit
//    8 data bits
//    No parity bit
//    1 stop bit
//    No hardware handshake
//    19200 baud (default ?) can be set to 9600 or 38400
    serialPort.setBaudRate(QSerialPort::Baud19200);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);
    if(serialPort.isOpen()) // Already Open ?
        serialPort.close(); // Close it !
    //serialPort.setParent(this);
}


CommunicationModule::~CommunicationModule() {
    if(serialPort.isOpen()) {
        serialPort.close();
    }
}


bool
CommunicationModule::startConnection() {
    if(!serialPort.open(QIODevice::ReadWrite)) {
        QMessageBox msgBox;
        msgBox.setWindowTitle(QCoreApplication::applicationName());
        msgBox.setIcon(QMessageBox::Critical);
        msgBox.setText(QString("Error Opening Device File.\n%1")
                       .arg(serialPort.errorString()));
        msgBox.setInformativeText(sFilename);
        msgBox.setStandardButtons(QMessageBox::Close);
        msgBox.setDefaultButton(QMessageBox::Close);
        msgBox.exec();
        return false;
    }
    connect(&serialPort, SIGNAL(readyRead()),
            this, SLOT(onNewDataAvailable()));
    return true;
}

void
CommunicationModule::onNewDataAvailable() {
    receivedData += serialPort.readAll();
    int lineLen = receivedData.indexOf('\n');
    if(lineLen < 0) return;
    lineLen++;
    emit newData(receivedData.left(lineLen).remove('\r').remove('\n'));
    receivedData.remove(0, lineLen);
}


QString
CommunicationModule::BlockingQuery(QString queryString) {
    sCommand = queryString.remove('\r').remove('\n');
    sCommand.append("\r\n");
//    qDebug() << sCommand;
    semaphore.lock();
    if(serialPort.write(queryString.toLocal8Bit()) != queryString.length()) {
        semaphore.unlock();
        return  QString();
    }
    serialPort.flush();
    // Some time is needed from a request to a response
    QThread::msleep(200);
    // Read the instrument.
    QString sAnswer = QString(serialPort.readLine()).remove('\r').remove('\n');
    semaphore.unlock();
    return sAnswer;
}


bool
CommunicationModule::writeCommand(QString sCommand) {
    sCommand = sCommand.remove('\r').remove('\n');
    sCommand.append("\r\n");
//    qDebug() << sCommand;
    semaphore.lock();
    if(serialPort.write(sCommand.toLocal8Bit()) != sCommand.length()) {
        semaphore.unlock();
        return false;
    }
    serialPort.flush();
    semaphore.unlock();
    return true;
}


QString
CommunicationModule::Query(QString queryString) {
//    qDebug() << queryString;
    semaphore.lock();
    if(!queryString.endsWith('\n'))
        queryString.append('\n');
    if(serialPort.write(queryString.toLocal8Bit()) != queryString.length()) {
        semaphore.unlock();
        return QString();
    }
    serialPort.flush();
    // Some time is needed from a request to a response
    QThread::msleep(200);
    // Read the instrument.
    QString sString = QString(serialPort.readLine()).remove('\r').remove('\n');
    semaphore.unlock();
    return sString;
}


QByteArray
CommunicationModule::BinaryQuery(QString queryString) {
//    qDebug() << queryString;
    semaphore.lock();
    if(!queryString.endsWith('\n'))
        queryString.append('\n');
    if(serialPort.write(queryString.toLocal8Bit()) != queryString.length()) {
        semaphore.unlock();
        return QByteArray();
    }
    serialPort.flush();
    // Some time is needed from a request to a response
    QThread::msleep(200);
    // Read the instrument.
    QByteArray sBinaryBlock = serialPort.readLine();
    semaphore.unlock();
    return sBinaryBlock;
}

