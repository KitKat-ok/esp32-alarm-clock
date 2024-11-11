#include "charts.h"

float temperatureArray[TEMP_CHART_READINGS];
float lightArray[TEMP_CHART_READINGS];

const unsigned long interval = 30000; // Interval in milliseconds (30 seconds)
unsigned long previousMillis = 0;     // Will store last time the function was called

OLED_SSD1306_Chart chart;

void initTempGraph()
{
    previousMillis = 0;
    display.clearDisplay();
    chart.setChartCoordinates(0, SCREEN_HEIGHT - 5);
    chart.setChartWidthAndHeight(SCREEN_WIDTH, (SCREEN_HEIGHT / 1.5) - 3);
    chart.setXIncrement(2);
    chart.setAxisDivisionsInc(5, 5);
    chart.setYLimits(0.00, 3000.0);
    chart.setYLimitLabels(0, 30);
    chart.setYLabelsVisible(true);
    chart.setPointGeometry(POINT_GEOMETRY_NONE);
    chart.setPlotMode(SINGLE_PLOT_MODE);
    chart.setLineThickness(NORMAL_LINE);
    display.clearDisplay();
    chart.drawChart();
    // Draw the updated chart
    for (int i = 0; i < TEMP_CHART_READINGS; i++)
    {
        chart.updateChart(temperatureArray[i]);
    }
    manager.oledDisplay();
}

void loopTempGraph()
{
    unsigned long currentMillis = millis(); // Get the current time
    display.fillRect(20, 0, SCREEN_WIDTH, 20, SSD1306_BLACK);
    centerText("Temp: " + String(temperature), (SCREEN_HEIGHT / 3) - 5);
    delay(10);
    manager.oledDisplay();
    if (currentMillis - previousMillis >= interval)
    {
        display.clearDisplay();
        centerText("Temp: " + String(temperature), (SCREEN_HEIGHT / 3) - 5);
        chart.drawChart();
        // Draw the updated chart
        for (int i = 0; i < TEMP_CHART_READINGS; i++)
        {
            chart.updateChart(temperatureArray[i] * 100);
        }
        previousMillis = currentMillis;
        delay(10);
    }
}

void initLightGraph()
{
    previousMillis = 0;
    display.clearDisplay();
    chart.setChartCoordinates(0, SCREEN_HEIGHT - 5);
    chart.setChartWidthAndHeight(SCREEN_WIDTH, (SCREEN_HEIGHT / 1.5) - 3);
    chart.setXIncrement(2);
    chart.setAxisDivisionsInc(0.1, 0.1);
    chart.setYLimits(0.00, 80.00);
    chart.setYLimitLabels(0, 80);
    chart.setYLabelsVisible(true);
    chart.setPointGeometry(POINT_GEOMETRY_NONE);
    chart.setPlotMode(SINGLE_PLOT_MODE);
    chart.setLineThickness(NORMAL_LINE);
    display.clearDisplay();
    chart.drawChart();
    // Draw the updated chart
    for (int i = 0; i < TEMP_CHART_READINGS; i++)
    {
        chart.updateChart(lightArray[i]);
    }
    chart.drawChart();
}

void loopLightGraph()
{
    unsigned long currentMillis = millis(); // Get the current time
    display.fillRect(20, 0, SCREEN_WIDTH, 20, SSD1306_BLACK);
    centerText("lux: " + String(lightMeter.readLightLevel()), (SCREEN_HEIGHT / 3) - 5);
    delay(10);
    manager.oledDisplay();
    if (currentMillis - previousMillis >= interval)
    {
        display.clearDisplay();
        centerText("lux: " + String(lightMeter.readLightLevel()), (SCREEN_HEIGHT / 3) - 5);
        chart.drawChart();
        // Draw the updated chart
        for (int i = 0; i < TEMP_CHART_READINGS; i++)
        {
            chart.updateChart(lightArray[i] * 100);
        }
        previousMillis = currentMillis;
    }
}
