#include "mainwindow.h"
#include "ui_mainwindow.h"
#define PORT 5555
#define CONPORT 5544
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    clientSocket = new QTcpSocket(this);
    connect(clientSocket,SIGNAL(readyRead()),this,SLOT(clientReady()));
    connect(clientSocket,SIGNAL(disconnected()),this,SLOT(clientDisconnect()));
    connect(clientSocket, &QIODevice::bytesWritten,
            this, &MainWindow::updateProgress);
    startserver();
    ui->progressBar->setValue(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    server_status = 0;
    db.close();
}

void MainWindow::startserver()
{
    tcpServer = new QTcpServer(this);
    connect(tcpServer, SIGNAL(newConnection()), this, SLOT(newuser()));
    if (!tcpServer->listen(QHostAddress::Any, PORT) && !server_status) {
        qDebug() <<  QObject::tr("Unable to start the server: %1.").arg(tcpServer->errorString());
        ui->textinfo->append(tcpServer->errorString());
    } else {
        server_status=1;
        qDebug() << tcpServer->isListening() << "TCPSocket listen on port";
        ui->textinfo->append(QString::fromUtf8("Сервер запущен!"));
        qDebug() << QString::fromUtf8("Сервер запущен!");
    }
}

void MainWindow::on_startButton_clicked()
{

}

void MainWindow::on_stopButton_clicked()
{
    if(server_status){
        QTextStream os(socket);
        os.setAutoDetectUnicode(true);
        os << QDateTime::currentDateTime().toString() << "\n";
        socket->close();
        socket->deleteLater();
        tcpServer->close();
        ui->textinfo->append(QString::fromUtf8("Сервер остановлен!"));
        qDebug() << QString::fromUtf8("Сервер остановлен!");
        server_status=0;
    }
}

void MainWindow::updateProgress(qint64 numBytes)
{
    currSize += int(numBytes);
    ui->progressBar->setMaximum(totalSize);
    ui->progressBar->setValue(currSize);
}

void MainWindow::on_sendButton_clicked()
{
    QString filename = QFileDialog::getOpenFileName(this, "Открыть файл...", "");
    QFile file(filename);
    socket->write(getFileName(filename).toUtf8());
    totalSize = QFileInfo(filename).size();
    currSize=0;
    QByteArray qba;
    qba.setNum(totalSize);
    socket->write("|");
    socket->write(qba);
    socket->write("|");
    file.open(QIODevice::ReadOnly);
    QByteArray q = file.readAll();
    socket->write(q);
    socket->write("|");
    insertToDB(QString::number(PORT), QString::number(CONPORT), getFileName(filename), totalSize, "10-10-2010");
}

QString MainWindow::getFileName(QString filename)
{
    return filename.mid(filename.lastIndexOf("/")+1);
}

void MainWindow::saveFile(QString name, QByteArray data)
{
    data.remove(data.lastIndexOf('|'), 1);
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить файл как...", name);
    qDebug() << "DATA: " << data.mid(data.lastIndexOf("|")+1);
    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    QTextStream stream(&file);
    stream << data.mid(data.lastIndexOf("|")+1);
}

void MainWindow::clientReady()
{
    if (clientSocket->waitForConnected(1000))
    {
        clientSocket->waitForReadyRead(1000);
        data = clientSocket->readAll();
        //qDebug()<<"Data: "<<data;
        if(data.at(data.size()-1)=='|')
        {
            qDebug()<<"ENDED";
            saveFile(data.left(data.indexOf('|')),data);
        }

    }
}

void MainWindow::clientDisconnect()
{
    server_status=1;
    clientSocket->deleteLater();
}

void MainWindow::insertToDB(QString sender,QString receiver,QString filename,long int filesize, QString date)
{
    qDebug() << "Пытаюсь подключиться к БД.";
        db = QSqlDatabase::addDatabase("QSQLITE");
        db.setDatabaseName("database.db");
        if(!db.open())
        {
            qDebug() << "Что-то пошло не так!";
            qDebug() << db.lastError().text();
        }
        else
        {
            QSqlQuery query("", db);
            //query.prepare("insert into actions values ('a', 'b', 'c', '10', '12-12-2012');");
            query.prepare("insert into actions values (?, ?, ?, ?, ?);");
            query.bindValue(0, sender);
            query.bindValue(1, receiver);
            query.bindValue(2, filename);
            query.bindValue(3, QString::number(filesize));
            query.bindValue(4, date);
            bool b = query.exec();
                /*if (!b) {
                    qDebug() << "Не удается создать таблицу.";
                    qDebug() << query.lastQuery();
                    qDebug() << query.lastError().text();
                }
                else
                {
                    qDebug() << "Insert successfull .";
                    qDebug() << query.lastQuery();
                }*/
        }
}


void MainWindow::newuser()
{
    if(server_status == 1)
    {
                ui->textinfo->append(QString::fromUtf8("У нас новое соединение!"));
                QTcpSocket* clientSocket=tcpServer->nextPendingConnection();
                socket=clientSocket;
                connect(socket,SIGNAL(readyRead()),this, SLOT(slotReadClient()));
                server_status = 2;
                socket->write("Connection successfully!");
                qDebug()<<"Client connected";
    }
}

void MainWindow::on_connectButton_clicked()
{
    clientSocket->connectToHost("127.0.0.1",CONPORT);
}
