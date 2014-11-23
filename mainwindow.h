#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    enum states {FREE, OCCUPIED, WAITING, OFFLINE};
    states state;
    void setState(states s);

    QSystemTrayIcon *trayIcon;
};

#endif // MAINWINDOW_H
