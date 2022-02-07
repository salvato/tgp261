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


//    When the TPG 261 is put into operation, it starts trans-
//    mitting measured values in intervals of 1s. As soon as
//    the first character is transferred to the TPG 261, the
//    automatic transmission of measured values stops. After
//    the necessary inquiries or parameter modifications have
//    been made, the transmission of measured values can be
//    started again with the COM command

// Data Format:
//    1 start bit
//    8 data bits
//    No parity bit
//    1 stop bit
//    No hardware handshake
//    9600 baud (default) can be set to 19200 or 38400

// Flow Control:
//    After each ASCII string, the HOST must wait for a report
//    signal (<ACK><CR><LF> or <NAK> <CR><LF>).
//    <ACK> = 0x06
//    <NACK>= 0x15

//    <ETX> = 0x03 (CTRL C) clears the input buffer in the TPG 261

//    <ENQ>  = 0x05 must be transmitted to request the transmission
//    of an ASCII string. Additional strings, according to the
//    last selected mnemonic, are read out by repetitive
//    transmission of <ENQ>

#include <QDebug>
#include <QSettings>

#include "tgp261.h"

tgp261::tgp261(QObject *parent)
    : QObject{parent}
    , bInitialized(false)
{
    pComm = new CommunicationModule();
    connect(pComm, SIGNAL(newData(QString)),
            this, SLOT(onNewData(QString)));
}


void
tgp261::onNewData(QString sData) {
    QStringList sDataList = QStringList(sData.split(","));
    if(sDataList.count() > 3)
        emit dataReady(sDataList.at(1));
}


bool
tgp261::Init() {
    if(!pComm->startConnection()) {
        return false;
    }
    pComm->writeCommand("COM,1"); // Resume Continuous Transmission (if interrupted)
    bInitialized = true;
    emit initialized();
    return true;
}


bool
tgp261::isInitialized() {
    return bInitialized;
}



