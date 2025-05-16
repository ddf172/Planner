#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

}

MainWindow::~MainWindow()
{
    delete ui; // Ensure proper cleanup of the UI
}

void MainWindow::switchToPage(const QString &pageName) {
    for (int i = 0; i < ui->stackedWidget->count(); ++i) {
        QWidget *page = ui->stackedWidget->widget(i);
        if (page->objectName() == pageName) {
            ui->stackedWidget->setCurrentIndex(i);
            return;
        }
    }

    qWarning() << "Page not found:" << pageName;
}

void MainWindow::on_btn_Groups_clicked()
{
     switchToPage("groupsPage");
}


void MainWindow::on_btn_Teachers_clicked()
{
    switchToPage("teachersPage");
}

void MainWindow::on_btn_Rooms_clicked()
{
    switchToPage("roomsPage");
}


void MainWindow::on_btn_Constraints_clicked()
{
    switchToPage("constraintsPage");
}




void MainWindow::on_btn_Checkout_clicked()
{
    switchToPage("checkoutPage");
}


void MainWindow::on_btn_Subjects_clicked() {
    switchToPage("subjectsPage");
}


void MainWindow::on_btn_TimeBlocks_clicked()
{
    switchToPage("timeBlocksPage");
}

