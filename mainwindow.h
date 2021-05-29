#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>
#include <QObject>
#include <QByteArray>
#include <QDebug>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QDateTime>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSql>

namespace Ui {
    class MainWindow;
}

class QTcpServer;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QSqlDatabase db;

private slots:
    void startserver();
    void on_startButton_clicked();
    void on_stopButton_clicked();
    void newuser();
    void on_sendButton_clicked();
    void clientReady();
    void clientDisconnect();
    void on_connectButton_clicked();
    void updateProgress(qint64 numBytes);

private:
    Ui::MainWindow *ui;
    QTcpServer *tcpServer;
    int server_status;
    qint64 totalSize;
    qint64 currSize;
    QByteArray data;
    QTcpSocket *socket;
    QTcpSocket *clientSocket;
    QString getFileName(QString filename);
    void sendFile(QString fileName);
    void saveFile (QString name, QByteArray data);
    void insertToDB(QString sender,QString receiver,QString filename,long int filesize, QString date);
};

#endif // MAINWINDOW_H
