#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QtWebSockets/QWebSocket>
#include <QNetworkAccessManager>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void onConnected();
    void onDisconnect();
    void onError(QAbstractSocket::SocketError socketError);
    void onTextMessageReceived(QString message);
    void httpFinished(QNetworkReply*);

private:
    enum states {FREE, OCCUPIED, WAITING, OFFLINE};
    states state;
    void setState(states s);
    void openConnection();

    QSystemTrayIcon *trayIcon;
    QWebSocket webSocket;
    QNetworkAccessManager netManager;

};

#endif // MAINWINDOW_H
