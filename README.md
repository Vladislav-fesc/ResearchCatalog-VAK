# Percs-eLibrary-Parser

Эта практика/проект представляет собой парсер для автоматического сбора и хранения показателей авторов с сайта научной электронной библиотеки **eLibrary.ru**.

## Описание

Парсер позволяет извлекать информацию о публикациях, индексах и других показателях авторов, используя их уникальный идентификатор (ID) на сайте **eLibrary.ru**. С помощью этого парсера можно собирать данные о научных показателях авторов, включая количество статей, индекс цитируемости и другие метрики.

Парсер использует **Selenium** для автоматизации работы с веб-страницами и получения данных с динамически загружаемых страниц.

## Требования

Для запуска парсера необходимы следующие инструменты и библиотеки:

- Python 3.x
- Selenium
- WebDriver для вашего браузера (в данном случае ChromeDriver)
- Библиотека для управления драйверами `webdriver_manager`

## Установка

1. Клонируйте репозиторий:

    ```bash
   git clone https://github.com/Kvelmi/Percs-eLibrary-Parser
   ```

2. Перейдите в каталог проекта:
       
    ```bash
   cd Percs-eLibrary-Parser
   ```

3. Установите все зависимости из файла:
      
    ```bash
   pip install -r requirements.txt
   ```

## Использование

1. Запустите скрипт:

    ```bash
   python main.py
   ```

2. Введите user_id автора, данные которого необходимо спарсить

3. Если появится CAPTCHA, то программа уведомит, что после её прохождения необходимо нажать ENTER, чтобы произошла переадресация на целевую страницу

4. Программа автоматически получит данные о основных показателях автора, выведет их в консоль, запишет в БД, а также произведёт экспорт в .txt файл

### Пример вывода

```yaml
    Введите user_id: 12345
Основные показатели автора:
Количество публикаций: 100
Индекс цитируемости: 50
Общее количество ссылок: 200
Данные были добавлены в БД

