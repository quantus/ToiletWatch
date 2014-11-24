#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{    
    QAction *quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);

    this->setState(OFFLINE);
    trayIcon->show();

    connect(&webSocket, &QWebSocket::connected, this, &MainWindow::onConnected);
    connect(&webSocket, &QWebSocket::disconnected, this, &MainWindow::onDisconnect);
    connect(&webSocket, SIGNAL(error(QAbstractSocket::SocketError)),
                  this, SLOT(onError(QAbstractSocket::SocketError)));

    connect(&netManager, SIGNAL(finished(QNetworkReply*)),
             this, SLOT(httpFinished(QNetworkReply*)));

    this->openConnection();
}

MainWindow::~MainWindow()
{

}
void MainWindow::openConnection(){
    webSocket.open(QUrl(QStringLiteral("ws://isthetoiletfree.fastmonkeys.com/hasfreesocket")));
    QNetworkRequest request;
    request.setUrl(QUrl("http://isthetoiletfree.fastmonkeys.com"));
    request.setRawHeader("User-Agent", "ToiletWatch 1.0");

    netManager.get(request);
}
void MainWindow::httpFinished(QNetworkReply*r){
    if(r->error() == 0){
        QByteArray data=r->readAll();
        QString str(data);
        if(str.contains(">no<"))
            this->setState(OCCUPIED);
        else if(str.contains(">yes<"))
            this->setState(FREE);
        else {
            qDebug() << str;
            this->setState(OFFLINE);
        }
    }
    else {
        qDebug() << r->error();
        this->setState(OFFLINE);
    }
    r->deleteLater();
}
void MainWindow::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(&webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindow::onTextMessageReceived);
}
void MainWindow::onDisconnect()
{
    qDebug() << "Failed";
    this->setState(OFFLINE);
}
void MainWindow::onError(QAbstractSocket::SocketError socketError)
{
    if(socketError == QAbstractSocket::RemoteHostClosedError){
        this->openConnection();
        return;
    }
    qDebug() << "Err " << socketError;
    this->setState(OFFLINE);
}
void MainWindow::onTextMessageReceived(QString message)
{
    qDebug() << "Message received:" << message;
    if(message == "{'hasFree': 'yes'}")
        this->setState(FREE);
    else
        this->setState(OCCUPIED);
}

void MainWindow::setState(states s){
    this->state = s;
    switch (this->state) {
    case FREE:
        trayIcon->setIcon(QIcon(":/toilet.png"));
        break;
    case OCCUPIED:
    case WAITING:
        trayIcon->setIcon(QIcon(":/toilet_red.png"));
        break;
    case OFFLINE:
        trayIcon->setIcon(QIcon(":/toilet_transparent.png"));
        break;
    }
}
