#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QtWebSockets/QWebSocket>
#include <QNetworkAccessManager>
#include <QTimer>

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
    void sendPing();
    void openConnection();
    void onNotifyClick();

private:
    enum states {FREE, OCCUPIED, WAITING, OFFLINE};
    states state;
    void setState(states s);

    QSystemTrayIcon *trayIcon;
    QAction *notifyAction;
    QWebSocket webSocket;
    QNetworkAccessManager netManager;
    QTimer pingTimer, reconnectTimer;
};

#endif // MAINWINDOW_H
