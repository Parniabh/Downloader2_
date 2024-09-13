#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCharts/QChart>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QValueAxis>
#include <QSplitter>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)

    , chart(new QtCharts::QChart())
    , chartView(new QtCharts::QChartView(chart, this))
    , series(new QtCharts::QLineSeries())
    , timer(new QTimer(this))
{
    ui->setupUi(this);
    ui->ProcessJson->setEnabled(false);

    inigetForecastURL("https://api.weather.gov/points/39.7456,-97.0892");
    connect(ui->ProcessJson, SIGNAL(clicked()), this, SLOT(show_json()));
    connect(ui->listWidget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(onlistchanged(QListWidgetItem*)));
    connect(timer, &QTimer::timeout, this, &MainWindow::updateWeatherData);


}



MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setupChart()
{
    series = new QtCharts::QLineSeries();

    for (int i = 0; i < weatherData.count(); i++)
    {
        QDateTime momentInTime = weatherData[i].startTime;
        series->append(momentInTime.toMSecsSinceEpoch(), weatherData[i].probabilityOfPrecipitation);
    }

    chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->legend()->hide();
    chart->setTitle("Probability of Precipitation Over Time");

    axisX = new QtCharts::QDateTimeAxis;
    axisX->setTickCount(10);
    axisX->setFormat("dd-MM-yyyy HH:mm");
    axisX->setTitleText("Date");
    chart->addAxis(axisX, Qt::AlignBottom);
    axisX->setLabelsAngle(-90);
    series->attachAxis(axisX);

    axisY = new QtCharts::QValueAxis;
    axisY->setLabelFormat("%i");
    axisY->setTitleText("Probability of Precipitation (%)");
    axisY->setRange(0, 100);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chartView = new QtCharts::QChartView(chart);
   chartView->setRenderHint(QPainter::Antialiasing);



    QVBoxLayout *layout = new QVBoxLayout(this);

    QSplitter *splitter = new QSplitter(Qt::Vertical, this);

    splitter->addWidget(ui->listWidget);
    splitter->addWidget(chartView);
    layout->addWidget(splitter);
    splitter->setStretchFactor(0, 1);
    splitter->setStretchFactor(1, 7);
    centralWidget()->setLayout(layout);
    ui->chartLayout->addWidget(chartView);
    resize(800, 2000);

    show();
}

void MainWindow::updateChartData(const QVector<QPair<QDateTime, double>> &data)
{
    series->clear();

    for (const auto &point : data) {
        qreal time = point.first.toMSecsSinceEpoch();
        series->append(time, point.second);
    }

    if (!data.isEmpty()) {
        qDebug()<<data.first().first<<":"<<data.last().first;
        axisX->setRange(data.first().first, data.last().first);
        axisY->setRange(0, 100);
    }
}

void MainWindow::plotForecast()
{
    QVector<QPair<QDateTime, double>> forecastData;

    for (const auto &dataPoint : weatherData) {
        forecastData.append(qMakePair(dataPoint.startTime, dataPoint.probabilityOfPrecipitation));
    }

    updateChartData(forecastData);
}

void MainWindow::enable_button()
{
    ui->ProcessJson->setEnabled(true);
}

void MainWindow::show_json()
{
    QJsonObject jsonobject = downloader->loadedJson.object();
    qDebug() << jsonobject;

    forecastArray = jsonobject.value("properties").toObject().value("periods").toArray();

    QString forecastUrl = jsonobject.value("properties").toObject().value("forecastHourly").toString();
    qDebug() << "Forecast URL: " << forecastUrl;
    connect(downloader, SIGNAL(download_finished_sgnl()), this, SLOT(process_forecast_data()));

    if (!forecastUrl.isEmpty()) {

        disconnect(downloader, SIGNAL(download_finished_sgnl()), this, SLOT(enable_button()));


        downloader->setUrl(forecastUrl);
        downloader->execute();

}

    showDatainList();
    setupChart();
    plotForecast();
}


        void MainWindow::showDatainList() {
            ui->listWidget_contents->clear();
            for (const auto &dataPoint : weatherData) {

                QString itemText = QString("Start Time: %1, End Time: %2, Temperature: %3, windSpeed: %4, Probability of Precipitation: %5, Relative Humidity: %6, shortForecast: %7")
                                       .arg(dataPoint.startTime.toString(Qt::ISODate))
                                       .arg(dataPoint.endTime.toString(Qt::ISODate))
                                       .arg(dataPoint.temperature)
                                       .arg(dataPoint.windSpeed)
                                       .arg(dataPoint.probabilityOfPrecipitation)
                                       .arg(dataPoint.relativeHumidity)
                                       .arg(dataPoint.shortForecast)
                    ;

                ui->listWidget_contents->addItem(itemText);
            }
        }





void MainWindow::process_forecast_data()
{
    connect(downloader, &Downloader::download_finished_sgnl, this, &MainWindow::process_forecast_data);

    //connect(downloader, SIGNAL(download_finished_sgnl()), this, SLOT(process_forecast_data()));

    QJsonObject forecastJson = downloader->loadedJson.object();
    qDebug() << forecastJson;

   QJsonValue forecastHourly = forecastJson.value("properties").toObject().value("periods");

    if (forecastHourly.isArray())
   {
   QJsonArray hourlyForecastArray = forecastHourly.toArray();
       ui->listWidget->clear();
        weatherData.clear();

        for (const QJsonValue &value : hourlyForecastArray) {
            QJsonObject forecastObj = value.toObject();
            weatherDatapoint dataPoint;

            dataPoint.startTime = QDateTime::fromString(forecastObj.value("startTime").toString(), Qt::ISODate);
            dataPoint.endTime = QDateTime::fromString(forecastObj.value("endTime").toString(), Qt::ISODate);

            if (!dataPoint.startTime.isValid() || !dataPoint.endTime.isValid()) {
                qDebug() << "Invalid date-time format in JSON.";
            }

            dataPoint.temperature = forecastObj.value("temperature").toDouble();
            dataPoint.windSpeed = forecastObj.value("windSpeed").toString();
            dataPoint.probabilityOfPrecipitation = forecastObj.value("probabilityOfPrecipitation").toObject().value("value").toDouble();
            dataPoint.relativeHumidity = forecastObj.value("relativeHumidity").toObject().value("value").toDouble();
            dataPoint.shortForecast = forecastObj.value("shortForecast").toString();


            qDebug() << "Weather Data Size:" << weatherData.size();
            for (const auto &dataPoint : weatherData) {
                qDebug() << "Start Time:" << dataPoint.startTime;
                qDebug() << "End Time:" << dataPoint.endTime;
                qDebug() << "Temperature:" << dataPoint.temperature;
                qDebug() << "Wind Speed:" << dataPoint.windSpeed;
                qDebug() << "Probability of Precipitation:" << dataPoint.probabilityOfPrecipitation;
                qDebug() << "Relative Humidity:" << dataPoint.relativeHumidity;
                qDebug() << "Short Forecast:" << dataPoint.shortForecast;
            }

            weatherData.append(dataPoint);
        }

        plotForecast();
    } else {
        qDebug() << "Unexpected data format for forecast hourly";
        qDebug() << "Calling showDatainList()";
        showDatainList();

    }
}

void MainWindow::onlistchanged(QListWidgetItem *item)
{
    ui->listWidget_contents->clear();
    QJsonValue selectedValue = jsonValues[item->text()];

    if (selectedValue.isString()) {
        ui->lineEdit->setText(selectedValue.toString());
    } else if (selectedValue.isObject()) {
        QJsonObject jsonObject = selectedValue.toObject();
        foreach (const QString &key, jsonObject.keys()) {
            QString valueStr = jsonObject.value(key).toString();
            ui->listWidget_contents->addItem(key + ": " + valueStr);
        }
    } else if (selectedValue.isArray()) {
        QJsonArray jsonArray = selectedValue.toArray();
        for (int i = 0; i < jsonArray.size(); ++i) {
            QJsonObject jsonObject = jsonArray[i].toObject();
            foreach (const QString &key, jsonObject.keys()) {
                QString valueStr = jsonObject.value(key).toString();
                ui->listWidget_contents->addItem(key + ": " + valueStr);
            }
        }
    } else if (selectedValue.isBool()) {
        ui->lineEdit->setText("Boolean value: " + QString::number(selectedValue.toBool()));
    } else {
        ui->lineEdit->setText("Unsupported JSON value type");
    }
}

void MainWindow::inigetForecastURL(const QString &url)
{
    downloader = new Downloader(url);
    connect(downloader, SIGNAL(download_finished_sgnl()), this, SLOT(getForecastURL()));
}

void MainWindow::getForecastURL()
{
    forecastURL = downloader->loadedJson.object().value("properties").toObject().value("forecastHourly").toString();
    delete downloader;
    downloader = new Downloader(forecastURL, this);
    connect(downloader, SIGNAL(download_finished_sgnl()), this, SLOT(getWeatherPrediction()));
}

void MainWindow::getWeatherPrediction()
{
    if (!downloader || downloader->loadedJson.isNull()) {
        qDebug() << "No valid JSON data loaded.";
        return;
    }

    QJsonObject jsonObject = downloader->loadedJson.object();
    qDebug() << jsonObject;

    QJsonObject propObj = jsonObject.value("properties").toObject();
    qDebug() << propObj;

    QJsonValue periodsObj = propObj.value("periods");
    qDebug() << periodsObj;

    QJsonArray periodsArray = periodsObj.toArray();
    qDebug() << periodsArray;



    for (const QJsonValue &value : periodsArray) {
        QJsonObject forecastObj = value.toObject();

       weatherDatapoint dataPoint;

        qDebug()<<forecastObj.value("startTime").toString();
        dataPoint.startTime = QDateTime::fromString(forecastObj.value("startTime").toString(), Qt::ISODate);
        dataPoint.endTime = QDateTime::fromString(forecastObj.value("endTime").toString(), Qt::ISODate);
        dataPoint.temperature = forecastObj.value("temperature").toDouble();
        dataPoint.windSpeed = forecastObj.value("windSpeed").toString();
        dataPoint.probabilityOfPrecipitation = forecastObj.value("probabilityOfPrecipitation").toObject().value("value").toDouble();
        dataPoint.relativeHumidity = forecastObj.value("relativeHumidity").toObject().value("value").toDouble();
        dataPoint.shortForecast = forecastObj.value("shortForecast").toString();



        qDebug() << "Start Time:" << dataPoint.startTime;
        qDebug() << "End Time:" << dataPoint.endTime;
        qDebug() << "Temperature:" << dataPoint.temperature;
        qDebug() << "Wind Speed:" << dataPoint.windSpeed;
        qDebug() << "Probability of Precipitation:" << dataPoint.probabilityOfPrecipitation;
        qDebug() << "Relative Humidity:" << dataPoint.relativeHumidity;
        qDebug() << "Short Forecast:" << dataPoint.shortForecast;

        weatherData.append(dataPoint);
    }



    setupChart();
    plotForecast();
    showDatainList();

    timer->start(3600000);
}
void MainWindow::updateWeatherData()
{
    qDebug() << "Updating weather data...";


    downloader->setUrl(forecastURL);
    downloader->execute();
}

void MainWindow::on_listWidget_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{

}

void MainWindow::on_listWidget_contents_currentItemChanged(QListWidgetItem *current, QListWidgetItem *previous)
{

}

void MainWindow::on_ProcessJson_clicked()
{

}

void MainWindow::on_comboBox_activated(int index)
{

}

