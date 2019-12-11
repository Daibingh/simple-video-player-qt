#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QTableWidgetItem>
#include <iostream>
#include <QThread>
#include <QObject>
#include <opencv2/opencv.hpp>
#include <QImage>
#include <QCloseEvent>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}


class Player: public QObject
{
    Q_OBJECT
public:
    bool next = false;
    bool pause = false;
    bool quit = false;

public slots:
    void run(const QString &file)
    {
        VideoCapture cap(file.toStdString());
        if (!cap.isOpened())
        {
            qDebug() << "not open!";
            return;
        }
//        cout<<cap.get(cv::CAP_PROP_FRAME_COUNT)<<endl;
        double num_frames = cap.get(cv::CAP_PROP_FRAME_COUNT);
        Mat frame;
        while(cap.read(frame) && !next && !quit)
        {
            while(pause)
            {
                QThread::msleep(100);
//                cout<<pause<<endl;
            }
            double pos = cap.get(cv::CAP_PROP_POS_FRAMES)+1;
            emit send_pos(pos / num_frames);
            cvtColor(frame, frame, CV_BGR2RGB);
            emit img_ready(QImage(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_RGB888));
            QThread::msleep(30);
        }
        frame.release();
        cap.release();
        emit finished();
    }

signals:
    void img_ready(QImage img);
    void finished();
    void send_pos(double pos);

};



class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void updateTable();

    QStringList getFileList(QString path);

    QStringList fileList;
    QThread workerThread; // 新线程
    Player* player;

signals:
    void play(const QString& file);

private slots:
    void on_actionopen_folder_triggered();
    void on_tableWidget_doubleClicked(const QModelIndex &index);
    void show_img(QImage img);
    void handle_finished();


    void on_pushButton_2_clicked();

    void closeEvent(QCloseEvent *event);

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void handle_pos(double pos);

    void on_actionadd_folder_triggered();

    void on_actionopen_file_triggered();


    void on_actionadd_file_triggered();

private:
    Ui::MainWindow *ui;
    bool isBusy = false;
    QString def_path;
};

#endif // MAINWINDOW_H
