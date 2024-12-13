#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
      this->resize(1300, 637);

    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("app_data.db");
    if (!db.open()) {
        qDebug() << "Ошибка при подключении к базе данных:" << db.lastError();
    } else {
        qDebug() << "Подключение к базе данных успешно!";
    }

    createTable();
    loadAllData(columnWidths);

    // Проверим, пуста ли база данных после того как окно отобразилось
    QTimer::singleShot(0, this, &MainWindow::checkDatabaseEmpty);  // Время 0, чтобы выполнить после отображения окна
    connect(ui->searchButton, &QPushButton::clicked, this, &MainWindow::onSearchButtonClicked);

}

MainWindow::~MainWindow()
{
    delete ui;
}
//создание базы и проверка
void MainWindow::createTable() {
    QSqlQuery query;
    bool success = query.exec("CREATE TABLE IF NOT EXISTS app_data ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "title TEXT, "
                              "issn TEXT, "
                              "field TEXT, "
                              "inclusion_date TEXT)");

    if (!success) {
        qDebug() << "Ошибка при создании таблицы:" << query.lastError();
    } else {
        qDebug() << "Таблица успешно создана или уже существует.";
    }
}
bool MainWindow::isDatabaseEmpty()
{
    QSqlQuery query;
    if (!query.exec("SELECT COUNT(*) FROM app_data")) {
        qDebug() << "Ошибка выполнения запроса:" << query.lastError().text();
        return true;
    }

    if (query.next()) {
        return query.value(0).toInt() == 0;
    }
    return true;
}

void MainWindow::checkDatabaseEmpty()
{
    if (isDatabaseEmpty()) {
        showDatabaseEmptyMessage();  // Если база данных пуста, покажем сообщение с кнопкой для заполнения
    }
}

void MainWindow::showDatabaseEmptyMessage()
{
    QMessageBox msgBox;
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText("База данных пуста.");
    msgBox.setInformativeText("Нажмите 'Заполнить базу данных', чтобы добавить данные.");
    QPushButton *fillButton = msgBox.addButton("Заполнить базу данных", QMessageBox::ActionRole);
    msgBox.exec();

    // Если пользователь нажмет на кнопку "Заполнить базу данных"
    if (msgBox.clickedButton() == fillButton) {
        on_fillDatabaseButton_clicked();  // Открываем окно для заполнения базы данных
    }
}

void MainWindow::on_fillDatabaseButton_clicked()
{
      this->resize(1300, 1020);
}

//Поиск
void MainWindow::on_columnSelector_activated(int index)
{
    // Проверяем, что индекс валиден
    if (index < 0) {
        return;
    }
    // Определяем запрос в зависимости от выбранного столбца
    switch (index) {
    case 1: // "Название"
        columnToSearch = "title";
        break;
    case 2: // "ISSN"
        columnToSearch = "issn";
        break;
    case 3: // "Специальность"
        columnToSearch = "field";
        break;
    case 4: // "Дата включения"
        columnToSearch = "inclusion_date";
        break;
    }
}

void MainWindow::onSearchButtonClicked()
{
    QString searchQuery = ui->searchInput->text().trimmed();

    if (searchQuery.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Введите запрос для поиска.");
        return;
    }

    if (columnToSearch.isEmpty()) {
        QMessageBox::warning(this, "Ошибка", "Выберите критерий для поиска.");
        return;
    }


    // Формируем SQL запрос с учетом выбранного столбца
    QString queryStr = QString("SELECT title, issn, field, inclusion_date FROM app_data WHERE %1 LIKE :searchTerm")
                           .arg(columnToSearch);


    QSqlQuery query;
    query.prepare(queryStr);

    // Привязываем значение параметра
    query.bindValue(":searchTerm", "%" + searchQuery + "%");

    if (!query.exec()) {
        return;
    }

    ui->resultsTable->clearContents();
    ui->resultsTable->setRowCount(0);
    ui->resultsTable->setColumnCount(4);
    ui->resultsTable->setHorizontalHeaderLabels({"Название", "ISSN", "Специальность", "Дата включения"});

    QVector<int> columnWidths = {200, 100, 500, 150};
    if (columnWidths.size() == ui->resultsTable->columnCount()) {
        for (int i = 0; i < columnWidths.size(); ++i) {
            ui->resultsTable->setColumnWidth(i, columnWidths[i]);
        }
    }

    int row = 0;
    while (query.next()) {
        QString title = query.value(0).toString();
        QString issn = query.value(1).toString();
        QString field = query.value(2).toString();
        QString date = query.value(3).toString();

        ui->resultsTable->insertRow(row);
        ui->resultsTable->setItem(row, 0, new QTableWidgetItem(title));
        ui->resultsTable->setItem(row, 1, new QTableWidgetItem(issn));
        ui->resultsTable->setItem(row, 2, new QTableWidgetItem(field));
        ui->resultsTable->setItem(row, 3, new QTableWidgetItem(date));

        row++;
    }

    if (row == 0) {
        QMessageBox::information(this, "Результаты поиска", "Поиск не дал результатов.");
    } else {
        qDebug() << "Найдено записей:" << row;
    }

    ui->resultsTable->viewport()->update();
}

//запись

void MainWindow::on_CloseButton_clicked()
{
     this->resize(1300, 637);
}


void MainWindow::on_LoadTxtButton_clicked()
{
    // Открытие диалогового окна для выбора текстового файла
    QString filePath = QFileDialog::getOpenFileName(this, "Открыть текстовый файл", "", "Text Files (*.txt)");
    if (!filePath.isEmpty()) {
        loadDataFromTxt(filePath);
    }
}

void MainWindow::loadDataFromTxt(const QString &filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::critical(this, "Ошибка",
                              QString("Не удалось открыть файл TXT: %1").arg(file.errorString()));
        return;
    }

    QTextStream in(&file);
    QString line;
    int addedRecords = 0; // Счётчик добавленных записей
    int skippedRecords = 0; // Счётчик пропущенных записей

    while (!in.atEnd()) {
        line = in.readLine();

        // Разделим строку на части с помощью разделителя "|"
        QStringList parts = line.split("|", Qt::KeepEmptyParts);

        // Обрабатываем строку только если она содержит хотя бы 1 часть
        if (parts.size() >= 4) {
            QString title = parts[0].trimmed();
            QString issn = parts[1].trimmed();
            QString field = parts[2].trimmed();
            QString inclusionDate = parts[3].trimmed();

            // Установим значения по умолчанию, если поля пустые
            if (issn.isEmpty()) issn = " "; // Используем "N/A" как значение по умолчанию для пустого ISSN
            if (field.isEmpty()) field = " "; // Используем "Unknown" для пустого поля field
            if (inclusionDate.isEmpty()) inclusionDate = " "; // Используем "0000-00-00" как пустую дату

            // Вставка данных в базу
            if (insertDataToDb(title, issn, field, inclusionDate)) {
                addedRecords++; // Увеличиваем счётчик при успешной вставке
            } else {
                skippedRecords++; // Если вставка не удалась, увеличиваем счётчик пропущенных строк
                qDebug() << "Ошибка записи в базу для строки:" << line;
            }
        } else {
            skippedRecords++; // Увеличиваем счётчик пропущенных строк
            qDebug() << "Пропущена некорректная строка:" << line;
        }
    }

    file.close();

    // Отображение сообщения с количеством добавленных и пропущенных записей
    QMessageBox::information(this, "Загрузка завершена",
                             QString("Файл успешно обработан.\nДобавлено записей: %1\nПропущено строк: %2")
                                 .arg(addedRecords)
                                 .arg(skippedRecords));
}



bool MainWindow::insertDataToDb(const QString &title, const QString &issn, const QString &field, const QString &inclusionDate) {
    // Проверяем, существует ли запись с такими же данными
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM app_data WHERE title = :title AND issn = :issn AND field = :field AND inclusion_date = :date");
    checkQuery.bindValue(":title", title);
    checkQuery.bindValue(":issn", issn);
    checkQuery.bindValue(":field", field);
    checkQuery.bindValue(":date", inclusionDate);

    if (!checkQuery.exec()) {
        return false;
    }

    checkQuery.next();
    int count = checkQuery.value(0).toInt();

    if (count > 0) {
        // Если запись с такими данными уже существует, можно её пропустить
        return false;
    }

    // Если запись не существует, добавляем новую
    QSqlQuery insertQuery;
    insertQuery.prepare("INSERT INTO app_data (title, issn, field, inclusion_date) "
                        "VALUES (:title, :issn, :field, :date)");
    insertQuery.bindValue(":title", title);
    insertQuery.bindValue(":issn", issn);
    insertQuery.bindValue(":field", field);
    insertQuery.bindValue(":date", inclusionDate);

    if (!insertQuery.exec()) {
        return false;
    } else {
        loadAllData(columnWidths);
        return true; // Успешно добавлено
    }
}

//вывод содержания базы
void MainWindow::loadAllData(const QVector<int> &columnWidths) {
    // Очистка таблицы перед загрузкой всех данных
    ui->tableWidgetResults->clearContents();
    ui->tableWidgetResults->setRowCount(0);
    ui->tableWidgetResults->setColumnCount(4);
    QStringList headers = {"Наименование издания ", "ISSN", "Направление", "Дата\nвключения "};
    ui->tableWidgetResults->setHorizontalHeaderLabels(headers);

    // Настройка размеров колонок, если переданы значения
    if (columnWidths.size() == ui->tableWidgetResults->columnCount()) {
        for (int i = 0; i < columnWidths.size(); ++i) {
            ui->tableWidgetResults->setColumnWidth(i, columnWidths[i]);
        }
    } else {
        // Если размеры не заданы, используем поведение по умолчанию
        ui->tableWidgetResults->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    }

    QSqlQuery query("SELECT title, issn, field, inclusion_date FROM app_data");

    int row = 0;
    while (query.next()) {
        // Извлекаем данные из запроса
        QString title = query.value(0).toString();
        QString issn = query.value(1).toString();
        QString field = query.value(2).toString();
        QString date = query.value(3).toString();

        // Вставляем новую строку и заполняем данными
        ui->tableWidgetResults->insertRow(row);
        ui->tableWidgetResults->setItem(row, 0, new QTableWidgetItem(title));
        ui->tableWidgetResults->setItem(row, 1, new QTableWidgetItem(issn));
        ui->tableWidgetResults->setItem(row, 2, new QTableWidgetItem(field));
        ui->tableWidgetResults->setItem(row, 3, new QTableWidgetItem(date));

        row++;
    }

    // Выводим в отладчик количество загруженных записей
    qDebug() << "Загружено записей:" << row;
}



void MainWindow::on_CleanButton_clicked()
{
    // Создаём диалоговое окно с подтверждением
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Очистка базы данных",
        "Вы уверены, что хотите очистить всю базу данных? Это действие необратимо.",
        QMessageBox::Yes | QMessageBox::No
        );

    // Если пользователь нажал "Да"
    if (reply == QMessageBox::Yes) {
        QSqlQuery query;
        if (!query.exec("DELETE FROM app_data")) { // Удаляем все записи из таблицы
        } else {
            QMessageBox::information(this, "Очистка завершена", "База данных была успешно очищена.");
            loadAllData(columnWidths); // Перезагружаем данные, чтобы обновить интерфейс
        }
    }
    // Если пользователь нажал "Нет" или закрыл окно, ничего не делаем
}

