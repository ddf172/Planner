#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "ScheduleData.hpp"
#include <vector>
#include <string>

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

private:
    Ui::MainWindow *ui;
    std::vector<TimeBlock> timeBlocks;
    std::vector<Class> subjects;
    std::vector<Room> rooms;
    std::vector<Teacher> teachers;
    std::vector<Group> groups;
    std::vector<Constraint> constraints;


    void switchToPage(const QString &pageName);
};

#endif // MAINWINDOW_H
