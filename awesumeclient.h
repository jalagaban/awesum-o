#ifndef AWESUMECLIENT_H
#define AWESUMECLIENT_H

#include <QDialog>
#include <QTcpSocket>
#include <QThread>
#include <QString>
#include <QTimer>
#include "world.h"
class ConnectThread : public QThread
{
private:
    QTcpSocket *socket;
    bool success;
    QString hostname, usr, pwd, errorMsg;
public:
    ConnectThread(QString initH, QString initU, QString initP):
        hostname(initH), usr(initU), pwd(initP){}
    void run();
    bool getResult() {return success; }
    QString getMsg() {return errorMsg; }
    QTcpSocket* getSocket() {return socket;}
    QString getUsr() {return usr;}
};

namespace Ui {
class AwesumeClient;
}

class AwesumeClient : public QDialog
{
    Q_OBJECT
    QTimer* SCORE;
public:
    explicit AwesumeClient(QWidget *parent = 0);
    ~AwesumeClient();
    bool getConnection(){return success;}
    void timerHit();
private slots:
    void on_btnConnect_clicked();
    void dataReceived();
    void serverDisconnected();
    void on_btnSend_clicked();
    void connectFinished();

private:
    Ui::AwesumeClient *ui;
    QTcpSocket *socket;
    ConnectThread *thread;
    bool success;
};

#endif // AWESUMECLIENT_H
