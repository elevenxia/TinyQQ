#include "widget.h"
#include "ui_widget.h"
#include <QDataStream>
#include <QMessageBox>
#include <QDateTime>
#include <QColorDialog>
#include <QFileDialog>
#include <QFile>
#include <QTextStream>

Widget::Widget(QWidget *parent, QString name) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    //初始化操作
    udpSocket = new QUdpSocket(this);
    //用户名获取
    this->uName = name;
    //端口号
    this->port = 9999;
    //绑定端口号 绑定模式 共享地址，断线重连
    udpSocket->bind(port, QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);

    //发送新用户进入
    sndMsg(UsrEnter);

    //点击发送按钮发送消息
    connect(ui->sendBtn, &QPushButton::clicked, [=](){
        sndMsg(Msg);
    });

    //监听别人发送的数据
    connect(udpSocket, &QUdpSocket::readyRead, this, &Widget::ReceiveMessage);

    //点击退出按钮，实现关闭窗口
    connect(ui->exitBtn, &QPushButton::clicked, [=](){
        this->close();
    });

    ////////////////////// 辅助功能 //////////////////////
    //字体
    connect(ui->fontCbx, &QFontComboBox::currentFontChanged, [=](const QFont& font){
       ui->msgTxtEdit->setCurrentFont(font);
       ui->msgTxtEdit->setFocus();
    });

    //字号
    void(QComboBox::*cbxsignal)(const QString& text) = &QComboBox::currentIndexChanged;
    connect(ui->sizeCbx, cbxsignal, [=](const QString& text){
        ui->msgTxtEdit->setFontPointSize(text.toDouble());
        ui->msgTxtEdit->setFocus();
    });

    //加粗
    connect(ui->boldTBtn, &QToolButton::clicked, [=](bool isCheck){
        if (isCheck) {
            ui->msgTxtEdit->setFontWeight(QFont::Bold);
        } else {
            ui->msgTxtEdit->setFontWeight(QFont::Normal);
        }
    });

    //倾斜
    connect(ui->italicTBtn, &QToolButton::clicked, [=](bool check){
       ui->msgTxtEdit->setFontItalic(check);
    });

    //下划线
    connect(ui->underlineTBtn, &QToolButton::clicked, [=](bool check){
       ui->msgTxtEdit->setFontUnderline(check);
    });

    //字体颜色
    connect(ui->colorTBtn, &QToolButton::clicked, [=](){
        QColor color = QColorDialog::getColor(Qt::red);
        ui->msgTxtEdit->setTextColor(color);
    });

    //清空聊天记录
    connect(ui->clearTBtn, &QToolButton::clicked, [=](){
        ui->msgBrowser->clear();
    });

    //保存聊天记录
    connect(ui->saveTBtn, &QToolButton::clicked, [=](){
        if (ui->msgBrowser->document()->isEmpty()) {
            QMessageBox::warning(this, "警告", "内容不能为空");
            return;
        } else {
            QString path = QFileDialog::getSaveFileName(this, "保存记录", "聊天记录", "(*.txt)");
            if (path.isEmpty()) {
                QMessageBox::warning(this, "警告", "路径不能为空");
                return;
            } else {
                QFile file(path);
                //打开模式加换行操作
                file.open(QIODevice::WriteOnly | QIODevice::Text);
                QTextStream stream(&file);
                stream << ui->msgBrowser->toPlainText();
                file.close();
            }}
    });
}


//关闭事件
void Widget::closeEvent(QCloseEvent* e) {
    emit this->closeWideget();

    sndMsg(UsrLeft); //用户离开
    //断开套接字
    udpSocket->close();
    udpSocket->destroyed();

    QWidget::closeEvent(e);
}

void Widget::sndMsg(Widget::MsgType type) //广播UDP消息
{
    //发送的消息分为三种类型
    //发送的数据做分段处理，第一段 枚举类型， 第二段 用户名， 第三段 具体内容
    QByteArray array;
    QDataStream stream(&array, QIODevice::WriteOnly);

    stream << type  << getUsr(); //将第一段内容和第二段用户名添加到流中

    switch (type) {
    case Msg: //普通消息发送
        if (ui->msgTxtEdit->toPlainText() == "") //判断如果用户没有输入内容，不发送任何消息
        {
            QMessageBox::warning(this, "警告", "发送内容不能为空");
            return;
        }
        //第三段，具体说的话
        stream << getMsg();
        break;
    case UsrEnter: //新用户进入消息

        break;
    case UsrLeft: //用户离开消息

        break;
    default:
        break;
    }

    //书写报文 广播发送
    udpSocket->writeDatagram(array, QHostAddress::Broadcast, port);
}

void Widget::usrEnter(QString username)
{
    //更新右侧TableWidget
    bool isEmpty1 = ui->usrTblWidget->findItems(username, Qt::MatchExactly).isEmpty();
    if (isEmpty1) {
        QTableWidgetItem* usr = new QTableWidgetItem(username);
        //插入行
        ui->usrTblWidget->insertRow(0);
        ui->usrTblWidget->setItem(0, 0, usr);

        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(QString("%1 上线了").arg(username));

        //在线人数更新
        ui->usrNumLbl->setText(QString("在线用户：%1").arg(ui->usrTblWidget->rowCount()));

        //把自身的信息广播出去
        sndMsg(UsrEnter);
    }
}

void Widget::usrLeft(QString usrname, QString time) //处理用户离开
{
    //更新右侧tableWidget
    bool isEmpty = ui->usrTblWidget->findItems(usrname, Qt::MatchExactly).isEmpty();
    if (!isEmpty) {
        int row = ui->usrTblWidget->findItems(usrname, Qt::MatchExactly).first()->row();
        ui->usrTblWidget->removeRow(row);

        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::gray);
        ui->msgBrowser->append(QString("%1 于 %2 离开了").arg(usrname).arg(time));

        //在线人数更新
        ui->usrNumLbl->setText(QString("在线用户：%1").arg(ui->usrTblWidget->rowCount()));

    }
}

QString Widget::getUsr() //获取用户名
{
    return this->uName;
}

QString Widget::getMsg() //获取聊天消息
{
    QString str = ui->msgTxtEdit->toHtml();

    //清空输入框
    ui->msgTxtEdit->clear();
    ui->msgTxtEdit->setFocus();

    return str;
}

void Widget::ReceiveMessage() //接受UDP消息
{
    //拿到数据报文
    //获取长度
    qint64 size = udpSocket->pendingDatagramSize();

    QByteArray array = QByteArray(10000, 0);
    udpSocket->readDatagram(array.data(), size);

    //解析数据
    //第一段 类型， 第二段 用户名， 第三段 具体内容
    QDataStream stream(&array, QIODevice::ReadOnly);

    int msgType; //读取到类型
    QString usrName;
    QString msg;

    //获取当前时间
   QString time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    stream >> msgType;

    switch (msgType) {
    case Msg: // 普通聊天
        stream >> usrName >> msg;

        //追加聊天记录
        ui->msgBrowser->setTextColor(Qt::blue);
        ui->msgBrowser->append("[" + usrName + "]" + time);
        ui->msgBrowser->append(msg);

        break;
    case UsrEnter:
        stream >> usrName;
        usrEnter(usrName);


        break;
    case UsrLeft:
        stream >> usrName;
        usrLeft(usrName, time);
        break;
    default:
        break;
    }
}
Widget::~Widget()
{
    delete ui;
}
