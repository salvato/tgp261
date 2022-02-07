/* MIT License

 Copyright (c) 2022 Gabriele Salvato

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
*/

// To ensure a static name to the usb->rs232 interface,
// create a new udev rule file:
// $ sudo nano /etc/udev/rules.d/90-tgp261.rules

// Then insert the following:
// SUBSYSTEM=="usb", ATTR{idVendor}=="04f2", ATTR{idProduct}=="b52b", GROUP="dialout", MODE="0666", SYMLINK+="tgp261"
// changing "04F2", "B52B" and "rov" to suit your interface values.

// Then reload the udev rules:
// $ sudo service udev restart



#include "mainwindow.h"

#include <QApplication>

int
main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    QApplication::addLibraryPath(".");

    QCoreApplication::setOrganizationDomain("Gabriele.Salvato");
    QCoreApplication::setOrganizationName("Gabriele.Salvato");
    QCoreApplication::setApplicationName("Oscilloscope");
    QCoreApplication::setApplicationVersion("0.0.1");

    MainWindow* pMainWindow;
    pMainWindow = new MainWindow();

    pMainWindow->show();

    int result = a.exec();

    return result;
}
