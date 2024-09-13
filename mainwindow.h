#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "downloader.h"
#include "QListWidgetItem"
#include <QJsonObject>
#include <QJsonArray>
#include <QVector>
#include <QDateTime>
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QListWidgetItem>

#include <QtCharts/QDateTimeAxis>
#include <QTimer>
#include <QSplitter>


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

    QJsonArray forecastArray;

private:
    Ui::MainWindow *ui;
    Downloader *downloader;
    QMap<QString, QJsonValue> jsonValues;
    void showDatainList();
    void inigetForecastURL(const QString &url);
    struct weatherDatapoint
    {
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


    QtCharts::QChart *chart;
    QtCharts::QChartView *chartView;
    QtCharts::QLineSeries *series;
    QtCharts::QDateTimeAxis *axisX;
    QtCharts::QValueAxis *axisY;
    QTimer *timer;

    void setupChart();
    void updateChartData(const QVector<QPair<QDateTime, double>> &data);

public slots:
    void enable_button();
    void show_json();
    void onlistchanged(QListWidgetItem *item);
    void process_forecast_data();
    void getForecastURL();
    void getWeatherPrediction();

    void plotForecast();

private slots:
    void on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_listWidget_contents_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous);
    void on_ProcessJson_clicked();
    void on_comboBox_activated(int index);
    void updateWeatherData();
};
#endif
