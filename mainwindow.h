#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "downloader.h"
#include "QListWidgetItem"
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QDateTime>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

struct weatherDatapoint
{
    QString startTime;
    QString endTime;
    double probability;

};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    //QVector<weatherDatapoint> weatherforecast;
    QJsonArray forecastArray;

private:
    Ui::MainWindow *ui;
    Downloader *downloader;
    QMap<QString, QJsonValue> jsonValues;
    void showDatainList();
    void inigetForecastURL(const QString &url);

    struct WeatherDataPoint {
        QDateTime startTime;
        QDateTime endTime;
        double temperature;
        QString windSpeed;
        double probabilityOfPrecipitation;
        double relativeHumidity;
        QString shortForecast;
    };


    QJsonArray getForecastArray(const QString &url);
    QString forecastURL;
    QVector<weatherDatapoint> weatherData;
public slots:
    void enable_button();
    void show_json();
    void onlistchanged(QListWidgetItem *item);
    void process_forecast_data();
    void getForecastURL();
    void getWeatherPrediction();
    ;

private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listWidget_contents_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_ProcessJson_clicked();
    void on_lineEdit_cursorPositionChanged(int arg1, int arg2);
};

#endif // MAINWINDOW_H
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
