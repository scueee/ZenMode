#include "mainwidget.h"
#include "globalshortcut.h"
#include <QApplication>
#include <QTextEdit>
#include <QHBoxLayout>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QKeyEvent>
#include <QSystemTrayIcon>
#include <QBrush>
#include <QTextFrame>
#include <QResizeEvent>
#include <QPainter>
#include <QScrollBar>
#include <QLabel>
#include <QClipboard>
#include <QFileInfo>
#include <QSettings>

MainWidget::MainWidget(QWidget *parent) : QWidget(parent)
{
    //读取 config.ini 配置
    QSettings *iniRead = new QSettings("config.ini", QSettings::IniFormat);
    text_width_percent = iniRead->value("/set/text_width_percent").toDouble();
    font_name = iniRead->value("/set/font_name").toString();
    font_size = iniRead->value("/set/font_size").toInt();
    bool ok;
    font_color = QRgb(iniRead->value("/set/font_color").toString().toInt(&ok,16));
    bg_win = QRgb(iniRead->value("/set/bg_win").toString().toInt(&ok,16));
    bg_text = QRgb(iniRead->value("/set/bg_text").toString().toInt(&ok,16));
    delete iniRead;

    QHBoxLayout *lay = new QHBoxLayout(this);
    QFont text_font(font_name,font_size,QFont::Normal);
    QFont font("黑体",16,QFont::Normal);
    QScrollBar *bar = new QScrollBar(this);

    edit = new QTextEdit(this);
    edit->setTextColor(QColor(font_color));
    edit->setFrameShape(QFrame::NoFrame);
    edit->setAcceptRichText(false);
    edit->setFont(text_font);
    edit->setVerticalScrollBar(bar);
    lay->addWidget(edit);

    //文本框背景透明
    QPalette pl = edit->palette();
    pl.setBrush(QPalette::Base,QBrush(QColor(255,0,0,0)));
    edit->setPalette(pl);

    //字数统计
    QLabel *sumLabel = new QLabel(this);
    sumLabel->setStyleSheet("color:#666666");
    sumLabel->setFont(font);
    sumLabel->move(20,10);
    connect(edit,&QTextEdit::textChanged,[=]{
        char str[20];
        sprintf(str,"%d",edit->document()->characterCount());
        sumLabel->setText(str);
    });

    //打字机滚动
    connect(edit,&QTextEdit::cursorPositionChanged,[=]{
        //光标所在行数等于总行数时保持滚动条于最下方
        if (edit->textCursor().blockNumber()+1 == edit->document()->lineCount()){
            bar->setValue(bar->maximum());
        }
    });

    //右键菜单
    QMenu *rMenu = new QMenu(this);
    QAction *copyAct = new QAction("复制",this);
    QAction *pasteAct = new QAction("粘贴",this);
    QAction *showAct = new QAction("全屏显示",this);
    QAction *minAct = new QAction("退出全屏",this);
    QAction *copyAllAct = new QAction("复制全文",this);
    QAction *outputAct = new QAction("导出TXT",this);

    rMenu->addAction(copyAct);
    rMenu->addAction(pasteAct);
    rMenu->addSeparator();
    rMenu->addAction(showAct);
    rMenu->addAction(minAct);
    rMenu->addAction(copyAllAct);
    rMenu->addAction(outputAct);
    rMenu->setToolTipsVisible(true);
    outputAct->setToolTip("导出至程序所在目录");
    showAct->setToolTip("快捷键F11");

    connect(copyAct,&QAction::triggered,[=]{
        edit->copy();
    });
    connect(pasteAct,&QAction::triggered,[=]{
        edit->paste();
    });
    connect(showAct,&QAction::triggered,[=]{
        showFullScreen();
        setWindowState(Qt::WindowFullScreen);
    });
    connect(minAct,&QAction::triggered,[=]{
        showMaximized();
        setWindowState(Qt::WindowMaximized);
    });
    connect(copyAllAct,&QAction::triggered,[=]{
        clipboard->setText(edit->toPlainText());
    });
    connect(outputAct,&QAction::triggered,[=]{
        QFile file("out.txt");
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text))
            return;
        QTextStream out(&file);
        out.setCodec("utf-8");
        out << QString(edit->toPlainText()).toUtf8();
        file.close();
    });
    edit->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(edit,&QTextEdit::customContextMenuRequested,[=]{
        rMenu->exec(QCursor::pos());
    });

    //系统托盘
    QIcon icon(":/logo.ico");
    QSystemTrayIcon *tray = new QSystemTrayIcon(this);
    QMenu *tMenu = new QMenu(this);
    QAction *hideAct = new QAction("隐藏窗口",this);
    QAction *quitAct = new QAction("退出程序",this);

    tMenu->addAction(copyAllAct);
    tMenu->addAction(outputAct);
    tMenu->addSeparator();
    tMenu->addAction(hideAct);
    tMenu->addAction(showAct);
    tMenu->addAction(quitAct);
    tMenu->setToolTipsVisible(true);
    tray->setIcon(icon);
    tray->setToolTip("单击显示，中击全屏");
    tray->setContextMenu(tMenu);
    tray->show();

    connect(hideAct,&QAction::triggered,[=]{
        hide();
    });
    connect(quitAct,SIGNAL(triggered()),qApp,SLOT(quit()));
    connect(tray,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(iconIsActived(QSystemTrayIcon::ActivationReason)));
}

void MainWidget::resizeEvent(QResizeEvent *){
    //重绘
    update();
    //根据窗口宽调整两侧 Margin
    QTextDocument *doc = edit->document();
    QTextFrame *rootFrame = doc->rootFrame();
    QTextFrameFormat frameformat;
    frameformat.setLeftMargin(int(width()*(1-text_width_percent)/2)+20);
    frameformat.setRightMargin(int(width()*(1-text_width_percent)/2)+20);
    frameformat.setBottomMargin(height()/2);
    rootFrame->setFrameFormat(frameformat);
}

void MainWidget::paintEvent(QPaintEvent *){
    //绘制窗口背景
    QPainter p(this);
    p.setPen(Qt::NoPen);
    p.setBrush(QColor(bg_win));
    p.drawRect(rect());
    //绘制 TextEdit 背景
    QPainter painter(this);
    painter.fillRect(QRect(int(width()*(1-text_width_percent)/2),0,int(width()*text_width_percent),height()),QBrush(bg_text));
}

void MainWidget::keyPressEvent(QKeyEvent *event){
    if(event->key() == Qt::Key_F11){
        if(MainWidget::isFullScreen()){
            hide();
        } else {
            showFullScreen();
            setWindowState(Qt::WindowFullScreen);
        }
    }
}

//托盘图标:单击最大化，中击全屏
void MainWidget::iconIsActived(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason){
    case QSystemTrayIcon::Trigger:{
        showMaximized();
        setWindowState(Qt::WindowMaximized);
        break;
    }
    case QSystemTrayIcon::MiddleClick:{
        showFullScreen();
        setWindowState(Qt::WindowFullScreen);
        break;
    }
    default:
        break;
    }
}

void MainWidget::closeEvent(QCloseEvent *event){
    hide();
    event->ignore();
}

void MainWidget::activated()
{
    if(MainWidget::isFullScreen()){
        hide();
    } else {
        showFullScreen();
        setWindowState(Qt::WindowFullScreen);
    }
}

int main(int argc,char*argv[])
{

    QApplication app(argc,argv);
    MainWidget w;

    w.setWindowTitle("ZenMode");
    w.showMaximized();

    GlobalShortCut *shortcut = new GlobalShortCut("F11",&w);
    QObject::connect(shortcut,SIGNAL(activated()),&w,SLOT(activated()));

    return app.exec();
}
