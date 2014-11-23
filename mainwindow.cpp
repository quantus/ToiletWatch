#include "mainwindow.h"
#include "ui_mainwindow.h"

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
}

MainWindow::~MainWindow()
{

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
