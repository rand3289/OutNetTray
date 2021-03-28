#include <qapplication.h>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QUdpSocket>
#include <QNetworkDatagram>
#include <iostream>
using namespace std;
// system tray icon to display notifications
// to test: in bash: echo "This is my data" > /dev/udp/127.0.0.1/3000
// https://doc.qt.io/qt-5/qudpsocket.html


class Server: public QObject {
    QSystemTrayIcon& tray;
    QUdpSocket& sock;
    QIcon icon; // TODO: reference main icon?
public:
    Server(QSystemTrayIcon& trayIcon, QUdpSocket& socket): tray(trayIcon), sock(socket), icon("icon.xpm") {}

    void read(){ //        while( sock.hasPendingDiagrams() ){
        QNetworkDatagram data = sock.receiveDatagram();
        QString msg = QString::fromStdString( data.data().toStdString() );
        tray.showMessage("Title", msg, icon, 10*1000);
    }
};


int main( int argc, char **argv ) {
    QApplication app( argc, argv );

    if (! QSystemTrayIcon::isSystemTrayAvailable() ){
        cerr << "System Tray is not available on your system!  Exiting." << endl;
        return 1;
    }
    if (! QSystemTrayIcon::supportsMessages() ){
        cerr << "System Tray application does not support messages on your system!  Exiting." << endl;
        return 2;
    }

    QSystemTrayIcon tray;
    QIcon icon("icon.xpm");
    tray.setIcon(icon);
    tray.setToolTip("OutNet");
    tray.show();

    int PORT = 12778;
    QUdpSocket sock;
    if( !sock.bind(QHostAddress::LocalHost, PORT) ){
        cerr << "Error binding to port " << PORT << " Exiting." << endl;
        return 3;
    }

    Server server(tray, sock);
    QObject::connect(&sock, &QUdpSocket::readyRead, &server, &Server::read);

    return app.exec();
}
