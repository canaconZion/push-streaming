#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QFileDialog>
#include <thread>
#include <QInputDialog>
#include <QtAVWidgets/WidgetRenderer.h>
#include <QtAV/AVPlayer.h>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);



    play_btn = ui->play_btn;
    select_btn = ui->select_btn;
    in_url_btn = ui->in_url_btn;
    player_frame = ui->player_frame;

    renderer = new QtAV::WidgetRenderer(this);
    player = new QtAV::AVPlayer(this);
    player->setRenderer(renderer);

    QVBoxLayout *layout = new QVBoxLayout(player_frame);
    layout->addWidget(renderer);
    player_frame->setLayout(layout);


    show_fname_lab = ui->show_fname;
    show_out_url_lab = ui->show_out;

    connect(play_btn,&QPushButton::clicked,this,&MainWindow::push_thread);
    connect(select_btn,&QPushButton::clicked,this,&MainWindow::select_file);
    connect(in_url_btn,&QPushButton::clicked,this,&MainWindow::input_out_url);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::select_file()
{
    QString source_path = "D:/video/videos";
    QString s = QFileDialog::getOpenFileName(
                this, QStringLiteral("选择要播放的文件"),
                source_path,
                QStringLiteral("视频文件 (*.flv *.rmvb *.avi *.MP4 *.mkv);;") + QStringLiteral("音频文件 (*.mp3 *.wma *.wav);;") + QStringLiteral("所有文件 (*.*)"));
    if (!s.isEmpty())
    {
        get_file = s;
        qDebug() << get_file;
        show_fname_lab->setText(get_file);
    }
    else
    {
        qDebug() << "Don't get video";
    }
}


void MainWindow::input_out_url()
{
    bool ok;
    QString text = QInputDialog::getText(nullptr, "打开媒体", "请输入网络URL:", QLineEdit::Normal, QString(), &ok);

    if (ok && !text.isEmpty())
    {
        qDebug() << "User input: " << text;
        get_url = text;
    }
    else
    {
        get_url = "rtmp://101.43.26.36:1935/live/test";
        qDebug() << "User cancelled the input. Use defaule";
    }
    show_out_url_lab->setText(get_url);
}


void MainWindow::push_thread()
{
    std::thread([&](MainWindow *push){
        push->push_video();
    },this).detach();

    player->play(get_url);
}


void MainWindow::video_player()
{
    using namespace QtAV;
    WidgetRenderer renderer;
    renderer.show();
    Widgets::registerRenderers();
    AVPlayer player;
    player.setRenderer(&renderer);
    player.play("D:/video/videos/cat.flv");
}


int MainWindow::push_video()
{
    qDebug() << "Start push video.";
    AVOutputFormat *ofmt = NULL;
    //输入对应一个AVFormatContext，输出对应一个AVFormatContext
    //（Input AVFormatContext and Output AVFormatContext）
    AVFormatContext *ifmt_ctx = NULL, *ofmt_ctx = NULL;
    AVPacket pkt;
    int ret, i;
    int videoindex=-1;
    int frame_index=0;
    int64_t start_time=0;
    if(!get_file.isEmpty() && !get_url.isEmpty())
    {
        QByteArray byteArray =get_file.toUtf8();
        in_filename = byteArray.data();
        QByteArray byte_url =get_url.toUtf8();
        out_filename = byte_url.data();
        //        in_filename = get_file.toUtf8().data();
        //        out_filename = get_url.toUtf8().data();
    }
    else
    {
        qDebug() << "Please input source file and rtmp url";
        goto end;
    }
    av_register_all();
    //Network
    avformat_network_init();
    //输入（Input）
    if ((ret = avformat_open_input(&ifmt_ctx, in_filename, 0, 0)) < 0) {
        printf( "Could not open input file.");
        qDebug() << "Could not open input file." << in_filename;
        goto end;
    }
    if ((ret = avformat_find_stream_info(ifmt_ctx, 0)) < 0) {
        printf( "Failed to retrieve input stream information");
        goto end;
    }

    for(i=0; i<ifmt_ctx->nb_streams; i++)
        if(ifmt_ctx->streams[i]->codec->codec_type==AVMEDIA_TYPE_VIDEO){
            videoindex=i;
            break;
        }

    av_dump_format(ifmt_ctx, 0, in_filename, 0);

    //输出（Output）

    avformat_alloc_output_context2(&ofmt_ctx, NULL, "flv", out_filename); //RTMP
    //avformat_alloc_output_context2(&ofmt_ctx, NULL, "mpegts", out_filename);//UDP

    if (!ofmt_ctx) {
        printf( "Could not create output context\n");
        ret = AVERROR_UNKNOWN;
        goto end;
    }
    ofmt = ofmt_ctx->oformat;
    for (i = 0; i < ifmt_ctx->nb_streams; i++) {
        //根据输入流创建输出流（Create output AVStream according to input AVStream）
        AVStream *in_stream = ifmt_ctx->streams[i];
        AVStream *out_stream = avformat_new_stream(ofmt_ctx, in_stream->codec->codec);
        if (!out_stream) {
            printf( "Failed allocating output stream\n");
            ret = AVERROR_UNKNOWN;
            goto end;
        }
        //复制AVCodecContext的设置（Copy the settings of AVCodecContext）
        ret = avcodec_copy_context(out_stream->codec, in_stream->codec);
        if (ret < 0) {
            printf( "Failed to copy context from input to output stream codec context\n");
            goto end;
        }
        out_stream->codec->codec_tag = 0;
        if (ofmt_ctx->oformat->flags & AVFMT_GLOBALHEADER)
            out_stream->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }
    //Dump Format------------------
    av_dump_format(ofmt_ctx, 0, out_filename, 1);
    //打开输出URL（Open output URL）
    if (!(ofmt->flags & AVFMT_NOFILE)) {
        ret = avio_open(&ofmt_ctx->pb, out_filename, AVIO_FLAG_WRITE);
        if (ret < 0) {
            printf( "Could not open output URL '%s'", out_filename);
            goto end;
        }
    }
    //写文件头（Write file header）
    ret = avformat_write_header(ofmt_ctx, NULL);
    if (ret < 0) {
        printf( "Error occurred when opening output URL\n");
        goto end;
    }

    start_time=av_gettime();
    while (1) {
        AVStream *in_stream, *out_stream;
        //获取一个AVPacket（Get an AVPacket）
        ret = av_read_frame(ifmt_ctx, &pkt);
        if (ret < 0)
            break;
        //FIX：No PTS (Example: Raw H.264)
        //Simple Write PTS
        if(pkt.pts==AV_NOPTS_VALUE){
            //Write PTS
            AVRational time_base1=ifmt_ctx->streams[videoindex]->time_base;
            //Duration between 2 frames (us)
            int64_t calc_duration=(double)AV_TIME_BASE/av_q2d(ifmt_ctx->streams[videoindex]->r_frame_rate);
            //Parameters
            pkt.pts=(double)(frame_index*calc_duration)/(double)(av_q2d(time_base1)*AV_TIME_BASE);
            pkt.dts=pkt.pts;
            pkt.duration=(double)calc_duration/(double)(av_q2d(time_base1)*AV_TIME_BASE);
        }
        //Important:Delay
        if(pkt.stream_index==videoindex){
            AVRational time_base=ifmt_ctx->streams[videoindex]->time_base;
            AVRational time_base_q={1,AV_TIME_BASE};
            int64_t pts_time = av_rescale_q(pkt.dts, time_base, time_base_q);
            int64_t now_time = av_gettime() - start_time;
            if (pts_time > now_time)
                av_usleep(pts_time - now_time);

        }

        in_stream  = ifmt_ctx->streams[pkt.stream_index];
        out_stream = ofmt_ctx->streams[pkt.stream_index];
        /* copy packet */
        //转换PTS/DTS（Convert PTS/DTS）
        pkt.pts = av_rescale_q_rnd(pkt.pts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.dts = av_rescale_q_rnd(pkt.dts, in_stream->time_base, out_stream->time_base, (AVRounding)(AV_ROUND_NEAR_INF|AV_ROUND_PASS_MINMAX));
        pkt.duration = av_rescale_q(pkt.duration, in_stream->time_base, out_stream->time_base);
        pkt.pos = -1;
        //Print to Screen
        if(pkt.stream_index==videoindex){
            printf("Send %8d video frames to output URL\n",frame_index);
            frame_index++;
        }
        //ret = av_write_frame(ofmt_ctx, &pkt);
        ret = av_interleaved_write_frame(ofmt_ctx, &pkt);

        if (ret < 0) {
            printf( "Error muxing packet\n");
            break;
        }

        av_free_packet(&pkt);

    }
    //写文件尾（Write file trailer）
    av_write_trailer(ofmt_ctx);
end:
    avformat_close_input(&ifmt_ctx);
    /* close output */
    if (ofmt_ctx && !(ofmt->flags & AVFMT_NOFILE))
        avio_close(ofmt_ctx->pb);
    avformat_free_context(ofmt_ctx);
    if (ret < 0 && ret != AVERROR_EOF) {
        printf( "Error occurred.\n");
        return -1;
    }
    return 0;
}
