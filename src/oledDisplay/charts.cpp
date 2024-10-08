#include "charts.h"

float temperatureArray[TEMP_CHART_READINGS];
float lightArray[TEMP_CHART_READINGS];

const unsigned long interval = 30000; // Interval in milliseconds (30 seconds)
unsigned long previousMillis = 0;     // Will store last time the function was called

void initTempGraph()
{
    previousMillis = 0;
    display.clearDisplay();
    display.setChartCoordinates(0, SCREEN_HEIGHT - 5);
    display.setChartWidthAndHeight(SCREEN_WIDTH, (SCREEN_HEIGHT / 1.5) - 3);
    display.setXIncrement(2);
    display.setAxisDivisionsInc(5, 5);
    display.setYLimits(0.00, 3000.0);
    display.setYLimitLabels("0", "30");
    display.setYLabelsVisible(true);
    display.setPointGeometry(POINT_GEOMETRY_NONE);
    display.setPlotMode(SINGLE_PLOT_MODE);
    display.setLineThickness(NORMAL_LINE);
    display.clearDisplay();
    display.drawChart();
    // Draw the updated chart
    for (int i = 0; i < TEMP_CHART_READINGS; i++)
    {
        display.updateChart(temperatureArray[i]);
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
        display.drawChart();
        // Draw the updated chart
        for (int i = 0; i < TEMP_CHART_READINGS; i++)
        {
            display.updateChart(temperatureArray[i] * 100);
        }
        previousMillis = currentMillis;
        delay(10);
    }
}

void initLightGraph()
{
    previousMillis = 0;
    display.clearDisplay();
    display.setChartCoordinates(0, SCREEN_HEIGHT - 5);
    display.setChartWidthAndHeight(SCREEN_WIDTH, (SCREEN_HEIGHT / 1.5) - 3);
    display.setXIncrement(2);
    display.setAxisDivisionsInc(0.1, 0.1);
    display.setYLimits(0.00, 80.00);
    display.setYLimitLabels("0", "80");
    display.setYLabelsVisible(true);
    display.setPointGeometry(POINT_GEOMETRY_NONE);
    display.setPlotMode(SINGLE_PLOT_MODE);
    display.setLineThickness(NORMAL_LINE);
    display.clearDisplay();
    display.drawChart();
    // Draw the updated chart
    for (int i = 0; i < TEMP_CHART_READINGS; i++)
    {
        display.updateChart(lightArray[i]);
    }
    display.drawChart();
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
        display.drawChart();
        // Draw the updated chart
        for (int i = 0; i < TEMP_CHART_READINGS; i++)
        {
            display.updateChart(lightArray[i] * 100);
        }
        previousMillis = currentMillis;
    }
}
