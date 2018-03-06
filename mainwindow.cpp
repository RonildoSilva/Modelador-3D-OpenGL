#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    //ui->pushButton->setEnabled(false);
    ui->pushButton->setText("Meu botão");
    ui->label->setText("Teste");
    //QKeyEvent * eve1 = new QKeyEvent (QEvent::KeyPress,Qt::Key_N,Qt::NoModifier,"N");
    QKeyEvent * eve1 = new QKeyEvent (QEvent::KeyPress,Qt::Key_N,Qt::NoModifier,"N");
    ui->widget->keyPressEvent(eve1);
}
