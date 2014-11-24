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

    connect(&pingTimer, SIGNAL(timeout()), this, SLOT(sendPing()));

    this->openConnection();
}

MainWindow::~MainWindow()
{

}
void MainWindow::openConnection(){
    qDebug() << "Open connection";
    webSocket.open(QUrl(QStringLiteral("ws://isthetoiletfree.fastmonkeys.com/hasfreesocket")));
    QNetworkRequest request;
    request.setUrl(QUrl("http://isthetoiletfree.fastmonkeys.com"));
    request.setRawHeader("User-Agent", "ToiletWatch 1.0");

    netManager.get(request);
}
void MainWindow::httpFinished(QNetworkReply*r){
    if(r->error() == 0){
        qDebug() << "Http request ok";
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
        qDebug() << "Http request failed " << r->error();
        this->setState(OFFLINE);
    }
    r->deleteLater();
}
void MainWindow::onConnected()
{
    qDebug() << "WebSocket connected";
    connect(&webSocket, &QWebSocket::textMessageReceived,
            this, &MainWindow::onTextMessageReceived);
    pingTimer.start(20000);
}
void MainWindow::onDisconnect()
{
    qDebug() << "WebSocket disconnected";
    this->setState(OFFLINE);
    pingTimer.stop();
}
void MainWindow::onError(QAbstractSocket::SocketError socketError)
{
    if(socketError == QAbstractSocket::RemoteHostClosedError){
        qDebug() << "WebSocket connection reset";
        webSocket.abort();
        this->openConnection();
        return;
    }
    qDebug() << "WebSocket error " << socketError;
    this->setState(OFFLINE);
}
void MainWindow::onTextMessageReceived(QString message)
{
    qDebug() << "WebSocket received:" << message;
    if(message.contains("yes"))
        this->setState(FREE);
    else if(message.contains("no"))
        this->setState(OCCUPIED);
    else{
        qDebug() << message;
        this->setState(OFFLINE);
    }
}
void MainWindow::sendPing()
{
    qDebug() << "Sending ping";
    //Avoid Heroku conection timeouts
    webSocket.sendTextMessage("Ping");
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
