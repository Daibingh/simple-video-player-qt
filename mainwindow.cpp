#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->tableWidget->setColumnWidth(0, 50);
    ui->tableWidget->setColumnWidth(1, 650);

    player = new Player;
    player->moveToThread(&workerThread); // 将工作对象与新线程关联
    connect(&workerThread, &QThread::finished, player, &QObject::deleteLater);
    connect(this, &MainWindow::play, player, &Player::run); // 连接启动耗时操作的槽函数
    connect(player, &Player::img_ready, this, &MainWindow::show_img); // 连接耗时操作完成的信号
    connect(player, &Player::finished, this, &MainWindow::handle_finished);
    connect(player, &Player::send_pos, this, &MainWindow::handle_pos);
    workerThread.start(); // 启动新线程的事件循环

    def_path = ".";
}

MainWindow::~MainWindow()
{
    workerThread.quit(); // 成功退出线程的事件循环
    workerThread.wait(); // 阻塞当前线程，直到新线程结束运行（子线程先结束，父线程再结束）

    delete ui;
    cout<<"~MainWindow"<<endl;
}

void MainWindow::updateTable()
{
    ui->tableWidget->setRowCount(fileList.size());
    for (int i=0; i<fileList.size(); i++)
    {
        ui->tableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(i+1)));
        ui->tableWidget->setItem(i, 1, new QTableWidgetItem(fileList[i]));
    }
}

QStringList MainWindow::getFileList(QString path)
{
    QDir recoredDir(path);
    QStringList nameFilters;
    nameFilters << "*.avi" << "*.mp4";
    QStringList list =  recoredDir.entryList(nameFilters, QDir::Files);
    for (int i=0; i<list.size(); i++)
    {
        list[i] = path + '/' + list[i];
    }
    return list;
}

void MainWindow::on_actionopen_folder_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    def_path,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty())
    {
        return;
    }
    def_path = dir;
    qDebug()<<dir;
    fileList = getFileList(dir);
    updateTable();

    foreach (const QString s, fileList)
    {
        qDebug()<<s;
    }

}

void MainWindow::on_tableWidget_doubleClicked(const QModelIndex &index)
{
    if (fileList.isEmpty())
    {
        return;
    }
    QString file = ui->tableWidget->item(index.row(), 1)->text();
    qDebug() << file;
    if (isBusy)
    {
        player->next = true;

    }
    else
    {

        player->next = false;
        emit play(file);
        isBusy = true;
        ui->pushButton_2->setText(tr("Pause"));
    }
    //    qDebug()<<isBusy;
}

void MainWindow::show_img(QImage img)
{
    ui->label->setPixmap(QPixmap::fromImage(img));
}

void MainWindow::handle_finished()
{
    if ( player->next )
    {
        player->next = false;
        int index = ui->tableWidget->currentRow();
        emit play(fileList[index]);
        isBusy = true;
        ui->pushButton_2->setText(tr("Pause"));
    }
    else
    {
        isBusy = false;
        ui->pushButton_2->setText(tr("Play"));
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if (fileList.isEmpty())
    {
        return;
    }
    if (ui->pushButton_2->text() == tr("Play"))
    {
        if (player->pause)
        {
//            cout<<"------"<<endl;
            player->pause = false;
//            cout<<player->pause<<endl;
            ui->pushButton_2->setText(tr("Pause"));
        }
        else
        {
            ui->pushButton_2->setText(tr("Pause"));
            int index = ui->tableWidget->currentRow();
            emit play(fileList[index]);
            isBusy = true;
        }
    }
    else
    {
        ui->pushButton_2->setText(tr("Play"));
        player->pause = true;
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    player->pause = false;
    player->quit = true;
    event->accept();
    cout<<"closeEvent"<<endl;
}

void MainWindow::on_pushButton_clicked()
{
    if (fileList.isEmpty())
    {
        return;
    }
    int index = ui->tableWidget->currentRow();
    index--;
    index = index < 0 ? 0 : index;

    ui->tableWidget->setCurrentCell(index, 1);

    if (isBusy)
    {
        player->next = true;

    }
    else
    {

        player->next = false;
        emit play(fileList[index]);
        isBusy = true;
        ui->pushButton_2->setText(tr("Pause"));
    }
}

void MainWindow::on_pushButton_3_clicked()
{
    if (fileList.isEmpty())
    {
        return;
    }
    int index = ui->tableWidget->currentRow();
    index++;
    index = index < fileList.size() ? index : fileList.size()-1;

    ui->tableWidget->setCurrentCell(index, 1);

    if (isBusy)
    {
        player->next = true;

    }
    else
    {

        player->next = false;
        emit play(fileList[index]);
        isBusy = true;
        ui->pushButton_2->setText(tr("Pause"));
    }
}

void MainWindow::handle_pos(double pos)
{
    int v = (int)(pos*10000);
    cout<<v<<endl;
    ui->horizontalSlider->setValue(v);
}

void MainWindow::on_actionadd_folder_triggered()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    def_path,
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if (dir.isEmpty())
    {
        return;
    }
    def_path = dir;
    qDebug()<<dir;
    QStringList list = getFileList(dir);
    foreach (QString s, list)
    {
        fileList.append(s);
    }
    updateTable();
}

void MainWindow::on_actionopen_file_triggered()
{
    QString video_file = QFileDialog::getOpenFileName(this, tr("Open File"),
                                              def_path,
                                              tr("Videos (*.avi *.mp4)"));
    qDebug()<<video_file;
    if (video_file.isEmpty())
    {
        return;
    }
    fileList.clear();
    fileList.append(video_file);
    updateTable();
}

void MainWindow::on_actionadd_file_triggered()
{
    QString video_file = QFileDialog::getOpenFileName(this, tr("Open File"),
                                              def_path,
                                              tr("Videos (*.avi *.mp4)"));
    qDebug()<<video_file;
    if (video_file.isEmpty())
    {
        return;
    }
    fileList.append(video_file);
    updateTable();
}
