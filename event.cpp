#include "event.h"
#include <QColorDialog>

Events::Events(QList<int> _l, int _i, QDate _d, QString _path, QWidget *parent):QDialog(parent){
    ui.setupUi(this);
    list = _l;
    item = _i;
    dat = _d;
    appPath = _path;
    ui.checkBox_period->hide();
    loadIcons();

    createEvent();

    connect(ui.pushButton_close, SIGNAL(clicked()), this, SLOT(close()));
    connect(ui.pushButton_save, SIGNAL(clicked()), this, SLOT(saveEvent()));
    connect(ui.pushButton_del, SIGNAL(clicked()), this, SLOT(deleteEvent()));

    connect(ui.pushButton_toFirst, SIGNAL(clicked()), this, SLOT(toFirst()));
    connect(ui.pushButton_toLast, SIGNAL(clicked()), this, SLOT(toLast()));
    connect(ui.pushButton_toNext, SIGNAL(clicked()), this, SLOT(toNext()));
    connect(ui.pushButton_toPrev, SIGNAL(clicked()), this, SLOT(toPrev()));

    connect(ui.checkBox_period, SIGNAL(clicked()), this, SLOT(setPeriodicly()));
    connect(ui.checkBox_full_day, SIGNAL(clicked(bool)), this, SLOT(setFullDay(bool)));

    connect(ui.toolButton_color, SIGNAL(clicked()), this, SLOT(changeColor()));
    connect(ui.comboBox_rem, SIGNAL(currentIndexChanged(int)), this, SLOT(setRemindDateTime(int)));
}

void Events::createEvent(){
    QSqlQuery query(QString("select events.name, events.date_s, events.time_s, events.time_e, events.note, events.color, "
                            "events.rem_date, events.rem_time "
                             "from events "
                             "where events.id = %1").arg(list.at(item)));
    query.next();
    ui.lineEdit_name->setText(query.value(0).toString());
    ui.dateEdit_main_start->setDate(query.value(1).toDate());
    ui.timeEdit_main_start->setTime(query.value(2).toTime());
    ui.timeEdit_main_end->setTime(query.value(3).toTime());
    ui.textEdit_note->setPlainText(query.value(4).toString());
    col.setNamedColor(query.value(5).toString());
    ui.toolButton_color->setStyleSheet(QString("background-color: %1;").arg(query.value(5).toString()));
    if (!query.value(6).isNull()){
        ui.groupBox_rem->setChecked(true);
        ui.comboBox_rem->setCurrentIndex(4);
        ui.dateEdit_rem->setDate(query.value(6).toDate());
        ui.timeEdit_rem->setTime(query.value(7).toTime());
    } else {
        ui.groupBox_rem->setChecked(false);
        ui.comboBox_rem->setCurrentIndex(4);
        setRemindDateTime(4);
    }
    //full day
    if (ui.timeEdit_main_start->time().toString("H:mm") == "0:00" and
            ui.timeEdit_main_end->time().toString("H:mm") == "23:59"){
        ui.checkBox_full_day->setChecked(true);
        ui.timeEdit_main_start->setVisible(false);
        ui.timeEdit_main_end->setVisible(false);
        ui.label_time->setVisible(false);
    } else {
        ui.checkBox_full_day->setChecked(false);
        ui.timeEdit_main_start->setVisible(true);
        ui.timeEdit_main_end->setVisible(true);
        ui.label_time->setVisible(true);
    }

    if (list.size() == 0){
        ui.pushButton_toFirst->setEnabled(false);
        ui.pushButton_toNext->setEnabled(false);
        ui.pushButton_toLast->setEnabled(false);
        ui.pushButton_toPrev->setEnabled(false);
        ui.pushButton_del->setEnabled(false);
    } else {
        ui.pushButton_toFirst->setEnabled(true);
        ui.pushButton_toNext->setEnabled(true);
        ui.pushButton_toLast->setEnabled(true);
        ui.pushButton_toPrev->setEnabled(true);
        ui.pushButton_del->setEnabled(true);
    }
    if (item == 0 and list.size() > 0){
        ui.pushButton_toFirst->setEnabled(false);
        ui.pushButton_toPrev->setEnabled(false);
    } else if (item > 0 and list.size() > 0) {
        ui.pushButton_toFirst->setEnabled(true);
        ui.pushButton_toPrev->setEnabled(true);
    }
    if (item == list.size() -1 and list.size() > 0){
        ui.pushButton_toNext->setEnabled(false);
        ui.pushButton_toLast->setEnabled(false);
    } else if (item < list.size() -1 and list.size() > 0) {
        ui.pushButton_toNext->setEnabled(true);
        ui.pushButton_toLast->setEnabled(true);
    }
}

void Events::saveEvent(){
    QString err;
    if (!ui.lineEdit_name->text().isEmpty()){
        QString qtext(QString("update events set name = \"%1\", date_s = \'%2\', time_s = \'%3\', time_e = \'%4\', "
                              "note = \'%5\', color = \'%6\'")
                      .arg(ui.lineEdit_name->text())
                      .arg(ui.dateEdit_main_start->date().toString("yyyy-MM-dd"))
                      .arg(ui.timeEdit_main_start->time().toString("hh:mm:ss"))
                      .arg(ui.timeEdit_main_end->time().toString("hh:mm:ss"))
                      .arg(ui.textEdit_note->toPlainText())
                      .arg(col.name()));
        if (ui.groupBox_rem->isChecked()){
            qtext.append(QString(", rem_date = \'%1\', rem_time = \'%2:00\' ")
                         .arg(ui.dateEdit_rem->date().toString("yyyy-MM-dd"))
                         .arg(ui.timeEdit_rem->time().toString("hh:mm")));
        }
        qtext.append(QString(" where events.id = \'%1\'")
                     .arg(list.at(item)));
        QSqlQuery query(qtext);
        query.exec();
        err.append(query.lastError().text());
        if (err.size() == 1){
            ui.lineEdit_status->setText("Saved...");
        } else {
            ui.lineEdit_status->setText(err);
        }
    } else {
        ui.lineEdit_status->setText("No title!!!");
    }
}

void Events::deleteEvent(){
    QMessageBox messa;
    messa.setText("Delete?");
    QPushButton *yes = messa.addButton(QMessageBox::Yes);
    QPushButton *no = messa.addButton(QMessageBox::No);
    messa.exec();
    if (messa.clickedButton() == yes){
        QSqlQuery del(QString("delete from events where events.id = \'%1\' ").arg(list.at(item)));
        del.exec();
        close();
    } else if (messa.clickedButton() == no){
        messa.close();
    }
}

void Events::toFirst(){
    item = 0;
    createEvent();
}

void Events::toLast(){
    item = list.size() - 1;
    createEvent();
}

void Events::toNext(){
    item = item + 1;
    createEvent();
}

void Events::toPrev(){
    item = item - 1;
    createEvent();
}

void Events::setFullDay(bool x){
    if (x == true){
        ui.timeEdit_main_start->setTime(QTime::fromString("00:00:00", "hh:mm:ss"));
        ui.timeEdit_main_start->hide();
        ui.timeEdit_main_end->setTime(QTime::fromString("23:59:00", "hh:mm:ss"));
        ui.timeEdit_main_end->hide();
        ui.label_time->hide();
    } else if (x == false){
        ui.timeEdit_main_start->setVisible(true);
        ui.timeEdit_main_end->setVisible(true);
        ui.label_time->setVisible(true);
    }
}
void Events::setPeriodicly(){

}

void Events::changeColor(){
    col = QColorDialog::getColor(Qt::white, this);
    ui.toolButton_color->setStyleSheet(QString("background-color: %1;").arg(col.name()));
}

void Events::loadIcons(){
    QIcon iDel(QDir::toNativeSeparators(QString("%1/icons/delete.png").arg(appPath)));
    ui.pushButton_del->setIcon(iDel);
    QIcon iClose(QDir::toNativeSeparators(QString("%1/icons/close_a.png").arg(appPath)));
    ui.pushButton_close->setIcon(iClose);
    QIcon iOk(QDir::toNativeSeparators(QString("%1/icons/ok.png").arg(appPath)));
    ui.pushButton_save->setIcon(iOk);
    QIcon iToFirst(QDir::toNativeSeparators(QString("%1/icons/toFirst.png").arg(appPath)));
    ui.pushButton_toFirst->setIcon(iToFirst);
    QIcon iToNext(QDir::toNativeSeparators(QString("%1/icons/toNext3.png").arg(appPath)));
    ui.pushButton_toNext->setIcon(iToNext);
    QIcon iToPrev(QDir::toNativeSeparators(QString("%1/icons/toPrev.png").arg(appPath)));
    ui.pushButton_toPrev->setIcon(iToPrev);
    QIcon iToLast(QDir::toNativeSeparators(QString("%1/icons/toLast.png").arg(appPath)));
    ui.pushButton_toLast->setIcon(iToLast);
    QIcon iWin(QDir::toNativeSeparators(QString("%1/icons/bookmark.png").arg(appPath)));
    setWindowIcon(iWin);
}

void Events::setRemindDateTime(int c){
    int m = 0;
    if (c < 4) {
        if (c == 0){
            m = 15;
        } else if (c == 1){
            m = 30;
        } else if (c == 2){
            m = 60;
        } else if (c == 3){
            m = 180;
        }
        QDateTime dm;
        dm.setDate(ui.dateEdit_main_start->date());
        dm.setTime(ui.timeEdit_main_start->time());
        qint64 ms = dm.toUTC().toMSecsSinceEpoch() - (m * 60 * 1000);
        QDateTime dt(QDateTime::fromMSecsSinceEpoch(ms));
        ui.dateEdit_rem->setDate(dt.date());
        ui.timeEdit_rem->setTime(dt.time());
    } else if (c == 4){
        ui.dateEdit_rem->setDate(ui.dateEdit_main_start->date());
        ui.timeEdit_rem->setTime(ui.timeEdit_main_start->time());
    }
}
