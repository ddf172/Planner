#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QString>
#include <QDir>
#include <QCoreApplication>

#include "ScheduleData.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_btn_Groups_clicked();
    void on_btn_Teachers_clicked();
    void on_btn_Constraints_clicked();
    void on_btn_Rooms_clicked();
    void on_btn_Checkout_clicked();
    void on_btn_Subjects_clicked();
    void on_btn_TimeBlocks_clicked();

    void on_pushButtonGroupSubmit_clicked();
    void on_pushButtonTeacherSubmit_clicked();
    void on_pushButtonRoomSubmit_clicked();
    void on_pushButtonTimeBlockSubmit_clicked();
    void on_pushButtonSubjectSubmit_clicked();
    void on_pushButtonCheckoutSubmit_clicked();

    void on_btn_labelGroupPrevEdit_clicked();
    void on_btn_labelGroupPrevDelete_clicked();
    void on_btn_labelTeacherPrevEdit_clicked();
    void on_btn_labelTeacherPrevDelete_clicked();
    void on_btn_labelSubjectPrevEdit_clicked();
    void on_btn_labelSubjectPrevDelete_clicked();

private:
    Ui::MainWindow *ui;
    std::vector<TimeBlock> timeBlocks;
    std::vector<Class> subjects;
    std::vector<Room> rooms;
    std::vector<Teacher> teachers;
    std::vector<Group> groups;
    std::vector<Constraint> constraints;

    void initializeUI();
    void switchToPage(const QString &pageName);

    bool writeToJsonFile(const std::string &filename);
    bool readFromJsonFile(const std::string &filename);
};

#endif // MAINWINDOW_H
