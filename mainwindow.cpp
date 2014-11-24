#include "mainwindow.h"
#include <QtWidgets/QAction>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QApplication>
#include <QNetworkReply>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent)
{    
    QAction *quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit()));

    notifyAction = new QAction("", this);
    connect(notifyAction, SIGNAL(triggered()), this, SLOT(onNotifyClick()));
    QMenu *trayIconMenu = new QMenu(this);
    trayIconMenu->addAction(notifyAction);
    trayIconMenu->addSeparator();
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
    connect(&reconnectTimer, SIGNAL(timeout()), this, SLOT(openConnection()));
    reconnectTimer.setSingleShot(true);

    this->openConnection();
}

MainWindow::~MainWindow()
{

}
void MainWindow::openConnection(){
    qDebug() << "Http request";
    QNetworkRequest request;
    request.setUrl(QUrl("http://isthetoiletfree.fastmonkeys.com"));
    request.setRawHeader("User-Agent", "ToiletWatch 1.0");
    netManager.get(request);
}
void MainWindow::httpFinished(QNetworkReply*r){
    r->deleteLater();
    if(r->error() == 0){
        qDebug() << "Http request ok";
        QByteArray data=r->readAll();
        QString str(data);
        if(str.contains(">no<"))
            this->setState(OCCUPIED);
        else if(str.contains(">yes<"))
            this->setState(FREE);
        else {
            qDebug() << "Http content parse error " << str;
            goto error;
        }
        qDebug() << "WebSocket connection";
        webSocket.open(QUrl(QStringLiteral("ws://isthetoiletfree.fastmonkeys.com/hasfreesocket")));
        return;
    }
    else {
        qDebug() << "Http request failed " << r->error();
    }
error:
    this->setState(OFFLINE);
    reconnectTimer.start(10000);
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
    qDebug() << "WebSocket error " << socketError;
    this->setState(OFFLINE);
    webSocket.abort();
    reconnectTimer.start(10000);
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
void MainWindow::onNotifyClick(){
    setState(this->state == WAITING?OCCUPIED:WAITING);
}

void MainWindow::setState(states s){
    switch (s) {
    case FREE:
        if(this->state == WAITING)
            trayIcon->showMessage("The toilet is now free", "", QSystemTrayIcon::Information);
        notifyAction->setEnabled(false);
        notifyAction->setText(tr("&Notify when free"));
        trayIcon->setIcon(QIcon(":/toilet.png"));
        break;
    case OCCUPIED:
        notifyAction->setEnabled(true);
        notifyAction->setText(tr("&Notify when free"));
        trayIcon->setIcon(QIcon(":/toilet_red.png"));
        break;
    case WAITING:
        notifyAction->setEnabled(true);
        notifyAction->setText(tr("&Remove notification"));
        trayIcon->setIcon(QIcon(":/toilet_red.png"));
        break;
    case OFFLINE:
        notifyAction->setEnabled(false);
        notifyAction->setText(tr("&Notify when free"));
        trayIcon->setIcon(QIcon(":/toilet_transparent.png"));
        break;
    }
    this->state = s;
}
