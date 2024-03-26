#include "widget.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),
    tcpSocket(new QTcpSocket(this)),
    sslSocket(new QSslSocket(this)),
    udpSocket(new QUdpSocket(this))
{
    ui->setupUi(this);

    QObject::connect(sslSocket, &QAbstractSocket::connected, this, &Widget::doConnected);
    QObject::connect(sslSocket, &QAbstractSocket::disconnected, this, &Widget::doDisconnected);
    QObject::connect(sslSocket, &QAbstractSocket::readyRead, this, &Widget::doReadyRead);

    QObject::connect(tcpSocket, &QAbstractSocket::connected, this, &Widget::doConnected);
    QObject::connect(tcpSocket, &QAbstractSocket::disconnected, this, &Widget::doDisconnected);
    QObject::connect(tcpSocket, &QAbstractSocket::readyRead, this, &Widget::doReadyRead);

    QObject::connect(udpSocket, &QUdpSocket::readyRead, this, &Widget::doReadyRead);
    ui->pbSend->setEnabled(false);
    ui->pbDisconnect->setEnabled(false);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::doConnected()
{
    QString msg = "Connected to " + sslSocket->peerAddress().toString() + "\r\n";
    ui->pteMessage->insertPlainText(msg);
    ui->pbConnect->setEnabled(false);
    ui->pbSend->setEnabled(true);
    ui->pbDisconnect->setEnabled(true);
}

void Widget::doDisconnected()
{
    QString msg = "Disconnected from " + sslSocket->peerAddress().toString() + "\r\n";
    ui->pteMessage->insertPlainText(msg);
    ui->pbConnect->setEnabled(true);
    ui->pbSend->setEnabled(false);
    ui->pbDisconnect->setEnabled(false);
}

void Widget::doReadyRead()
{
    QByteArray ba;
    if (sslSocket->state() == QAbstractSocket::ConnectedState)
        ba = sslSocket->readAll();
    else if (tcpSocket->state() == QAbstractSocket::ConnectedState)
        ba = tcpSocket->readAll();
    else if (udpSocket->state() == QAbstractSocket::BoundState)
        ba.resize(udpSocket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;
    udpSocket->readDatagram(ba.data(), ba.size(), &sender, &senderPort);
    ui->pteMessage->insertPlainText(ba);
}

void Widget::on_pbConnect_clicked()
{
    QString host = ui->lehost->text();
    quint16 port = ui->leport->text().toUShort();

    if (sslSocket->state() == QAbstractSocket::UnconnectedState)
        sslSocket->connectToHostEncrypted(host, port);
    else
        sslSocket->connectToHost(host, port);
}

void Widget::on_pbDisconnect_clicked()
{
    if (sslSocket->state() == QAbstractSocket::ConnectedState)
        sslSocket->disconnectFromHost();
    else if (tcpSocket->state() == QAbstractSocket::ConnectedState)
        tcpSocket->disconnectFromHost();
    else if (udpSocket->state() == QAbstractSocket::BoundState)
        udpSocket->close();
}

void Widget::on_pbSend_clicked()
{
    QByteArray data = ui->ptSendText->toPlainText().toUtf8();

    if (sslSocket->state() == QAbstractSocket::ConnectedState)
        sslSocket->write(data);
    else if (tcpSocket->state() == QAbstractSocket::ConnectedState)
        tcpSocket->write(data);
    else if (udpSocket->state() == QAbstractSocket::BoundState)
        udpSocket->writeDatagram(data, QHostAddress::LocalHost, 1234); // Change QHostAddress::LocalHost to target address
}

void Widget::on_pbClear_clicked()
{
    ui->pteMessage->clear();
}

void Widget::on_sslCheck_stateChanged(int arg1)
{
    if (arg1 > 0)
    {
        ui->leport->setText("443");
    }
    else
    {
        ui->leport->setText("80");
    }

}
