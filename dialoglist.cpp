#include "dialoglist.h"
#include "ui_dialoglist.h"
#include <QToolButton>
#include <widget.h>
#include <QMessageBox>

DialogList::DialogList(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::DialogList)
{
    ui->setupUi(this);
    //设置标题
    setWindowTitle("QQ");
    //设置图标
    setWindowIcon(QPixmap(":/images/qq.png"));

    // 准备图标
    QList<QString>nameList;
    nameList << "哥只是个传说" << "忆梦如澜" <<"快乐MM"<<"王子不是我"<<"心如止水"
             <<"一路繁华的夏"<<"落水无痕"<<"断点"<<"果冻先森";


    QStringList iconNameList; //图标资源列表
    iconNameList << "a1"<< "a2" <<"a3" <<"a4"<< "a5"
                 <<"a6"<<"a7"<<"a8"<<"a9";
    QVector<QToolButton*> vToolBtn;

    for (int i = 0; i < 9; ++i) {
        //设置头像
        QToolButton* btn = new QToolButton;
        btn->setText(nameList[i]);
        QString str = QString(":/headImage/%1.jpg").arg(iconNameList.at(i));
        btn->setIcon(QPixmap(str));
        btn->setIconSize(QSize(79, 77));
        //设置按钮风格 透明
        btn->setAutoRaise(true);
        //设置文字和图片一起显示
        btn->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
        //加到垂直布局中
        ui->vLayout->addWidget(btn);
        //容器保存住9个按钮方便以后再次操作
        vToolBtn.push_back(btn);
        //9个标识的默认初始化
        isShow.push_back(false);
    }

    //对9个按钮 进行添加信号槽
    for (int i =0; i < vToolBtn.size(); i++) {
        connect(vToolBtn[i], &QToolButton::clicked, [=]() { // 当isShow不是成员变量时，不加mutable isShow[i]会报错
            // 如果被打开了，就不要再次打开
            if (isShow[i]) {
                QString str = QString("%1窗口已经被打开了").arg(vToolBtn[i]->text());
                QMessageBox::warning(this, "警告",str);
                return;
            }
            isShow[i] = true;
            //弹出聊天对话框
            //构造聊天窗口的时候 告诉这个窗口它的名字 参数1 顶层方式弹出 参数2 窗口名字
            //注意 Widget构造函数默认并没有这两个参数，需要自己修改
           Widget* widget = new Widget(0, vToolBtn[i]->text()); // 0代表以顶层方式弹出
           widget->setWindowTitle(vToolBtn[i]->text());
           widget->setWindowIcon(vToolBtn[i]->icon());
           widget->show();

           connect(widget, &Widget::closeWideget, [=]() {
              isShow[i] = false;
           });
        });
    }

}

DialogList::~DialogList()
{
    delete ui;
}
