#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QLabel>

#include <QtAVWidgets/WidgetRenderer.h>
//#include <QtAVWidgets>
#include <QtAV/AVPlayer.h>

extern "C"
{
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/time.h"
}

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    QPushButton *play_btn;
    QPushButton *select_btn;
    QPushButton *in_url_btn;
    QString get_file;
    QString get_url;

    char *in_filename;
    char *out_filename;

    int push_video();
    void video_player();

private:
    Ui::MainWindow *ui;
    QtAV::WidgetRenderer *renderer;
    QtAV::AVPlayer *player;

    QLabel *show_fname_lab;
    QLabel *show_out_url_lab;

    QFrame *player_frame;

public slots:
    void push_thread();
    void select_file();
    void input_out_url();
};

#endif // MAINWINDOW_H
