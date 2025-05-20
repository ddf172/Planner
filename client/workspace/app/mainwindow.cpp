#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Initialize the UI
    initializeUI();
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

void MainWindow::on_btn_Groups_clicked() { switchToPage("groupsPage"); }
void MainWindow::on_btn_Teachers_clicked() { switchToPage("teachersPage"); }
void MainWindow::on_btn_Rooms_clicked() { switchToPage("roomsPage"); }
void MainWindow::on_btn_Constraints_clicked() { switchToPage("constraintsPage"); }
void MainWindow::on_btn_Checkout_clicked() { switchToPage("checkoutPage"); }
void MainWindow::on_btn_Subjects_clicked() { switchToPage("subjectsPage"); }
void MainWindow::on_btn_TimeBlocks_clicked() { switchToPage("timeBlocksPage"); }


void MainWindow::initializeUI()
{
    // Set the initial page to "groupsPage"
    ui->stackedWidget->setCurrentIndex(0);

    // Load the JSON data from the file
    QString path = QCoreApplication::applicationDirPath() + "/data/input.json";
    std::string filename = path.toStdString();
    if (readFromJsonFile(filename)) {
        qDebug() << "Data successfully loaded from" << path;
    } else {
        qWarning() << "Failed to load data from" << path;
    }
}


bool MainWindow::writeToJsonFile(const std::string &filename) {
    try {
        // Create a JSON object to hold all data
        json j;
        j["groups"] = groups;
        j["teachers"] = teachers;
        j["rooms"] = rooms;
        j["subjects"] = subjects;
        j["timeBlocks"] = timeBlocks;

        // Open file for writing
        std::ofstream file(filename);
        if (!file.is_open()) {
            qWarning() << "Failed to open file:" << QString::fromStdString(filename);
            return false;
        }

        // Write formatted JSON to file (with indentation)
        file << j.dump(4);

        file.close();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Error writing to JSON file:" << e.what();
        return false;
    }
}


void MainWindow::on_pushButtonCheckoutSubmit_clicked()
{
    // Create /data directory in build folder if it doesn't exist
    QDir dir(QCoreApplication::applicationDirPath() + "/data");
    if (!dir.exists()) {
        dir.mkpath(".");
    }

    QString path = QCoreApplication::applicationDirPath() + "/data/input.json";
    std::string filename = path.toStdString();
    if (writeToJsonFile(filename)) {
        qDebug() << "Data successfully written to" << path;
    } else {
        qWarning() << "Failed to write data to" << path;
    }
}


bool MainWindow::readFromJsonFile(const std::string &filename) {
    try {
        // Open file for reading
        std::ifstream file(filename);
        if (!file.is_open()) {
            qWarning() << "Failed to open file:" << QString::fromStdString(filename);
            return false;
        }
        // Parse JSON data
        json j;
        file >> j;
        file.close();
        // Deserialize data into vectors
        groups = j["groups"].get<std::vector<Group>>();
        teachers = j["teachers"].get<std::vector<Teacher>>();
        rooms = j["rooms"].get<std::vector<Room>>();
        subjects = j["subjects"].get<std::vector<Class>>();
        timeBlocks = j["timeBlocks"].get<std::vector<TimeBlock>>();
        return true;
    } catch (const std::exception& e) {
        qWarning() << "Error reading from JSON file:" << e.what();
        return false;
    }
}


void MainWindow::on_pushButtonGroupSubmit_clicked()
{
    // Read input from the form
    QString groupName = ui->lineEditGroupName->text();
    QString groupId = ui->lineEditGroupId->text();

    // Create a new Group object
    Group newGroup;
    newGroup.name = groupName.toStdString();
    newGroup.id = groupId.toStdString();
    newGroup.parentGroupId = ""; // Not handled in this form, set as empty
    newGroup.constraints = {};   // Not handled in this form, set as empty

    // Add to the groups vector
    groups.push_back(newGroup);

    // Optionally, update the preview label
    ui->labelGroupPrev->addItem(
        QString("ID: %1\nName: %2").arg(groupId, groupName)
    );

    // Optionally, clear the form fields
    ui->lineEditGroupName->clear();
    ui->lineEditGroupId->clear();
}


void MainWindow::on_pushButtonTimeBlockSubmit_clicked()
{
    // Read input from the form
    QList<QListWidgetItem*> selectedDays = ui->listWidgetTimeBlockDay->selectedItems();
    QString day = selectedDays.isEmpty() ? "" : selectedDays.first()->text();
    QString startStr = ui->lineEditTimeBlockStart->text();
    QString endStr = ui->lineEditTimeBlockEnd->text();

    // Generate a simple ID (could be improved)
    QString id = QString("%1_%2_%3").arg(day, startStr, endStr);

    // Parse start and end times as integers
    int start = startStr.toInt();
    int end = endStr.toInt();
    int duration = end - start;

    // Create a new TimeBlock object
    TimeBlock newTimeBlock;
    newTimeBlock.id = id.toStdString();
    newTimeBlock.day = day.toStdString();
    newTimeBlock.start = start;
    newTimeBlock.end = end;
    newTimeBlock.duration = duration;

    // Add to the timeBlocks vector
    timeBlocks.push_back(newTimeBlock);

    // Optionally, update the preview label
    ui->labelTimeBlockPrev->setText(
        QString("ID: %1\nDay: %2\nStart: %3\nEnd: %4\nDuration: %5")
            .arg(id)
            .arg(day)
            .arg(start)
            .arg(end)
            .arg(duration)
    );

    // Optionally, clear the form fields
    ui->listWidgetTimeBlockDay->clearSelection();
    ui->lineEditTimeBlockStart->clear();
    ui->lineEditTimeBlockEnd->clear();
}

void MainWindow::on_pushButtonRoomSubmit_clicked()
{
    // Read input from the form
    QString roomId = ui->lineEditRoomId->text();
    QString roomName = ui->lineEditRoomName->text();

    // Get selected features
    std::set<std::string> features;
    for (QListWidgetItem* item : ui->listWidgetRoomFeatures->selectedItems()) {
        features.insert(item->text().toStdString());
    }

    // Create a new Room object
    Room newRoom;
    newRoom.id = roomId.toStdString();
    newRoom.name = roomName.toStdString();
    newRoom.features = features;

    // Add to the rooms vector
    rooms.push_back(newRoom);

    // Optionally, update the preview label
    QString featuresStr;
    for (const auto& f : features) {
        featuresStr += QString::fromStdString(f) + " ";
    }
    ui->labelRoomPrev->setText(
        QString("ID: %1\nName: %2\nFeatures: %3")
            .arg(roomId)
            .arg(roomName)
            .arg(featuresStr.trimmed())
    );

    // Optionally, clear the form fields
    ui->lineEditRoomId->clear();
    ui->lineEditRoomName->clear();
    ui->listWidgetRoomFeatures->clearSelection();
}

void MainWindow::on_pushButtonTeacherSubmit_clicked()
{
    // Read input from the form
    QString teacherId = ui->lineEditTeacherId->text();
    QString teacherName = ui->lineEditTeacherName->text();

    // Get selected subjects (as a single string, or you can adapt to your model)
    QStringList selectedSubjects;
    for (QListWidgetItem* item : ui->listWidgetTeacherSubjects->selectedItems()) {
        selectedSubjects << item->text();
    }
    QString subject = selectedSubjects.join(", ");

    // Create a new Teacher object
    Teacher newTeacher;
    newTeacher.id = teacherId.toStdString();
    newTeacher.name = teacherName.toStdString();
    newTeacher.subject = subject.toStdString();
    newTeacher.constraints = {}; // Not handled in this form

    // Add to the teachers vector
    teachers.push_back(newTeacher);

    // Optionally, update the preview label
    ui->labelTeacherPrev->setText(
        QString("ID: %1\nName: %2\nSubjects: %3")
            .arg(teacherId)
            .arg(teacherName)
            .arg(subject)
    );

    // Optionally, clear the form fields
    ui->lineEditTeacherId->clear();
    ui->lineEditTeacherName->clear();
    ui->listWidgetTeacherSubjects->clearSelection();
}

void MainWindow::on_pushButtonSubjectSubmit_clicked()
{
    // Read input from the form
    QString subjectName = ui->lineEditSubjectName->text();

    // Get selected room features
    QStringList roomFeatures;
    for (QListWidgetItem* item : ui->listWidgetSubjectConstraintRoom->selectedItems()) {
        roomFeatures << item->text();
    }

    // Get selected teacher subjects
    QStringList teacherSubjects;
    for (QListWidgetItem* item : ui->listWidgetSubjectConstraintTeacher->selectedItems()) {
        teacherSubjects << item->text();
    }

    // Generate a simple ID (could be improved)
    QString id = subjectName;

    // Create a new Class object
    Class newClass;
    newClass.id = id.toStdString();
    newClass.name = subjectName.toStdString();
    newClass.subject = subjectName.toStdString();
    newClass.difficulty = 0; // Not handled in this form
    newClass.constraints = {}; // Not handled in this form

    // Add to the subjects vector
    subjects.push_back(newClass);

    // Optionally, update the preview label
    ui->labelSubjectPrev->setText(
        QString("ID: %1\nName: %2\nRoom features: %3\nTeacher subjects: %4")
            .arg(id)
            .arg(subjectName)
            .arg(roomFeatures.join(", "))
            .arg(teacherSubjects.join(", "))
    );

    // Optionally, clear the form fields
    ui->lineEditSubjectName->clear();
    ui->listWidgetSubjectConstraintRoom->clearSelection();
    ui->listWidgetSubjectConstraintTeacher->clearSelection();
}