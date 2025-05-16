#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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

private:
    Ui::MainWindow *ui;
    void switchToPage(const QString &pageName);
};

#endif // MAINWINDOW_H
