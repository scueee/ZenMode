#ifndef MAINWIDGET_H
#define MAINWIDGET_H

#include <QWidget>
#include <QSystemTrayIcon>
#include <QTextEdit>
#include <QClipboard>

class MainWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MainWidget(QWidget *parent = nullptr);

signals:

public slots:
    void iconIsActived(QSystemTrayIcon::ActivationReason reason);

protected:
    void keyPressEvent(QKeyEvent *event);
    void paintEvent(QPaintEvent *);
    void resizeEvent(QResizeEvent *);
    void closeEvent(QCloseEvent *event);

private:
    QTextEdit *edit;
    QClipboard *clipboard;

    double text_width_percent = 0.5;
    QString font_name = "宋体";
    int font_size = 18;
    QRgb font_color = 0x000000;
    QRgb bg_win = 0xeeeeee;
    QRgb bg_text = 0xdddddd;
};

#endif // MAINWIDGET_H
