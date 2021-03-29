#include <qapplication.h>
#include <QSystemTrayIcon>
#include <QIcon>
#include <QUdpSocket>
#include <cstdlib> // strtol()
#include <iostream> // cerr
using namespace std;
// System tray icon to display notifications.
// This program requires QT to compile.
// To test using bash shell:  echo test,title,,3, | tr , \\0 > /dev/udp/127.0.0.1/12778
// echo "test,title,,30,xdg-open http://127.0.0.1:4444>/dev/null," | tr , \\0 > /dev/udp/127.0.0.1/12778


class Server: public QObject {
    QSystemTrayIcon& tray;
    QUdpSocket& sock;
    QIcon icon; // TODO: use/delete/make it a reference to main icon?
    const char* action = nullptr;
    char buff[1025];
public:
    Server(QSystemTrayIcon& trayIcon, QUdpSocket& socket): tray(trayIcon), sock(socket), icon("icon.xpm") {}

    void msgClick() {
        if( nullptr != action){
            system(action);
            action = nullptr;
        }
// This is a security problem as any local process can execute anything under your user ID.
// Take only port number instead ???
//        tray.showMessage("CLICK", action, icon, 5*1000);
//        system("xdg-open http://127.0.0.1:4444 > /dev/null");
    }

    void read() {
        int rdSize = sock.readDatagram(buff, sizeof(buff)-1); // pendingDatagramSize()
        char* end = buff+rdSize; // at this point buff should contain 5 strings: msg, title, icon, timeout, action
        *end = 0; // null terminate the buffer

        const char* msg = buff;
        const char* title = "TITLE";
        const char* iconFile = "icon.xpm";
        const char* timeoutStr = "10";

        if(msg+strlen(msg) < end){
            title = msg+strlen(msg)+1;
            if(title < end){
                iconFile = title+strlen(title)+1;
                if(iconFile < end){
                    timeoutStr = iconFile+strlen(iconFile)+1;
                    if(timeoutStr < end){
                        action = timeoutStr+strlen(timeoutStr)+1;
                        if(action+strlen(action) >= end){
                            action = nullptr;
                        }
                    }
                }
            }
        }

        int timeout = strtol(timeoutStr, nullptr, 10); // base 10
        timeout = timeout < 1 ? 10 : timeout; // message should be visible for at least one second

        if( strlen(iconFile) < 5 ){ iconFile = "icon.xpm"; } // minimum file name length "a.xpm"
        QIcon iconObj(iconFile);

        tray.showMessage(title, msg, iconObj, timeout*1000);
    } // read()
}; // class Server


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

    int PORT = 12778;
    QUdpSocket sock;
    if( !sock.bind(QHostAddress::LocalHost, PORT) ){
        cerr << "Error binding to port " << PORT << " Exiting." << endl;
        return 3;
    }

    QSystemTrayIcon tray;
    QIcon icon("icon.xpm");
    tray.setIcon(icon);
    tray.setToolTip("OutNet");
    tray.show();

    Server server(tray, sock);
    QObject::connect(&sock, &QUdpSocket::readyRead, &server, &Server::read); // connect signal for socket readyRead
    QObject::connect(&tray, &QSystemTrayIcon::messageClicked, &server, &Server::msgClick); // for messageClicked

    return app.exec();
}
