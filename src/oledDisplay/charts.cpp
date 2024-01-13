#include "charts.h"

float temperatureArray[TEMP_CHART_READINGS];

void initTempGraph()
{
    display.clearDisplay();
    display.setChartCoordinates(0, SCREEN_HEIGHT);
    display.setChartWidthAndHeight(SCREEN_WIDTH, (SCREEN_HEIGHT / 1.5) - 3);
    display.setXIncrement(4);
    display.setAxisDivisionsInc(5, 5);
    display.setYLimits(0, 40);
    display.setYLimitLabels("0", "40");
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
}

const unsigned long interval = 30000; // Interval in milliseconds (30 seconds)
unsigned long previousMillis = 0;     // Will store last time the function was called

void loopTempGraph()
{
    unsigned long currentMillis = millis(); // Get the current time
    display.fillRect(20, 0, SCREEN_WIDTH, 20, SSD1306_BLACK);
    centerText("Temp: " + String(temperature), (SCREEN_HEIGHT / 3) - 5);
    if (currentMillis - previousMillis >= interval)
    {
        display.clearDisplay();
        display.drawChart();
        // Draw the updated chart
        for (int i = 0; i < TEMP_CHART_READINGS; i++)
        {
            display.updateChart(temperatureArray[i]);
        }
        previousMillis = currentMillis;
    }
    display.display();
}