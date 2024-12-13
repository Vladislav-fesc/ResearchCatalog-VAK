#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QMessageBox>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>// Подключаем QTimer
#include <QSqlRecord>
#include <QSqlError>
#include <QFile>
#include <QFileDialog>



QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_fillDatabaseButton_clicked();  // Слот для кнопки "Заполнить базу данных"
    void onSearchButtonClicked();          // Слот для кнопки "Искать"
    void on_CloseButton_clicked();
    void on_LoadTxtButton_clicked();

    void on_CleanButton_clicked();

    void on_columnSelector_activated(int index);

private:
    Ui::MainWindow *ui;
    QSqlDatabase db;

    // Методы
    void createTable();
    bool initializeDatabase();              // Метод для инициализации базы данных
    bool isDatabaseEmpty();                 // Метод для проверки, пуста ли база данных
    void showDatabaseEmptyMessage();        // Метод для отображения сообщения, если база данных пуста
    void checkDatabaseEmpty();              // Метод для проверки базы данных и отображения сообщения
    bool insertDataToDb(const QString &title, const QString &issn, const QString &field, const QString &inclusionDate) ;
    void loadDataFromTxt(const QString &filePath);
    void loadAllData(const QVector<int> &columnWidths);

    QVector<int> columnWidths = {500, 100, 500, 150};
    QString columnToSearch;

};

#endif // MAINWINDOW_H
