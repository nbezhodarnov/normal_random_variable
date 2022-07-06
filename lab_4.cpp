#include <algorithm>
#include <iostream>
#include <vector>
#include <math.h>

#include "QCustomPlot/qcustomplot.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QDialog>

static int y = 0;
static const unsigned long Y_VALUE_LIMIT = (unsigned long)1 << (sizeof(y) * 8 - 1);

// Генератор псевдослучайных чисел (равномерное распределение на отрезке [0, 1])
double rnd() {
    y *= 843314861;
    y += 453816693;
    if (y < 0) {
        y += Y_VALUE_LIMIT;
    }
    return double(y) / double(Y_VALUE_LIMIT - 1);
}

// Генератор псевдослучайных чисел (нормальное распределение с математическим ожиданием expection и среднеквадратическим отклонением standard_deviation)
double gsv(const double &expection, const double &standard_deviation) {
    if (standard_deviation <= 0) {
        std::cerr << "Non-positive dispersion!\n";
        return 0.;
    }
    double teta = sqrt(-2 * log(rnd()));
    double fi = (2.515517 + 0.802853 * teta + 0.010328 * pow(teta, 2)) / (1 + 1.432788 * teta + 0.1869269 * pow(teta, 2) + 0.001308 * pow(teta, 3)) - teta;
    return fi * standard_deviation + expection;
}

// Класс гистограммы
class BarGraph {
public:
    BarGraph(const std::vector<double>&, const double&);
    double getValue(const double&);
    void ShowPlot();

private:
    std::vector<double> bar_borders;
    std::vector<double> bar_heights;
};

// Конструктор класса гистограммы
BarGraph::BarGraph(const std::vector<double> &section, const double &h) {
    if (section.size() < 2) {
        return;
    }
    std::vector<double> sorted_section = section;
    std::sort(sorted_section.begin(), sorted_section.end());
    const unsigned int k = std::ceil((sorted_section[sorted_section.size() - 1] - sorted_section[0]) / h);
    bar_borders.resize(k + 1);
    bar_heights.resize(k);
    unsigned int index = 0;
    bar_borders[0] = sorted_section[0];
    for (unsigned int i = 1; i < bar_borders.size(); i++) {
        bar_borders[i] = sorted_section[0] + i * h;
        bar_heights[i - 1] = 0;
        while (index < sorted_section.size() && sorted_section[index] < bar_borders[i]) {
            bar_heights[i - 1]++;
            index++;
        }
        bar_heights[i - 1] /= sorted_section.size() * h;
    }
}

// Функция вычисления значения гистограммы в точке x
double BarGraph::getValue(const double &x) {
    if (x < bar_borders[0] || x > bar_borders[bar_borders.size() - 1]) {
        return 0.;
    }
    for (unsigned int i = 1; i < bar_borders.size(); i++) {
        if (x <= bar_borders[i]) {
            return bar_heights[i - 1];
        }
    }
    return 0;
}

// Функция создания окна с построением графика гистограммы
void BarGraph::ShowPlot() {
    QDialog window;
    QHBoxLayout layout;
    QCustomPlot plot;
    plot.addGraph();

    double x = bar_borders[0], step = (bar_borders[1] - bar_borders[0]) * 0.01;
    for (unsigned int index = 1; index < bar_borders.size(); index++) {
        while (x <= bar_borders[index]) {
            plot.graph(0)->addData(x, bar_heights[index - 1]);
            x += step;
        }
    }

    plot.xAxis->setRange(bar_borders[0], bar_borders[bar_borders.size() - 1]);
    plot.yAxis->setRange(0, 1);
    layout.addWidget(&plot);
    window.setLayout(&layout);
    window.resize(600, 400);
    window.exec();
}

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);
    double expection = 5, standard_deviation = 2, h = 0.001;
    unsigned int N = 1000000;

    // Генерация выборки из N элементов и проверка правила трёх сигм
    std::vector<double> selection(N);
    double left_border = expection - 3 * standard_deviation, right_border = expection + 3 * standard_deviation;
    unsigned int inside_border_elements_count = 0;
    for (unsigned int i = 0; i < selection.size(); i++) {
        selection[i] = gsv(expection, standard_deviation);
        if (left_border < selection[i] && selection[i] < right_border) {
            inside_border_elements_count++;
        }
    }
    std::cout << "There has been genereated a selection of " << N << " values of normal distribution with expection = " << expection << " and standard deviation = " << standard_deviation << '\n';

    // Вывод результата проверки правила трёх сигм
    std::cout << "Probability of getting a random number inside (" << expection << " - 3 * " << standard_deviation << ", " << expection << " + 3 * " << standard_deviation << "): " << inside_border_elements_count / (double)N << '\n';
    if (inside_border_elements_count / (double)N >= 0.997) {
        std::cout << "3-sigma rule is met.\n";
    } else {
        std::cout << "3-sigma rule is not met.\n";
    }

    // Построение гистограммы и её графика
    std::cout << "Now the application will create a window with bargraph created by generated selection.\n";
    BarGraph bar_graph(selection, h);
    bar_graph.ShowPlot();
    return 0;
}
