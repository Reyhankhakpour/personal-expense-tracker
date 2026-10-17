#include <fstream>
#include <sstream>
#include <string>
#include <QComboBox>
#include "mainwindow.h"
#include "./ui_mainwindow.h"
#include <QLabel>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QDebug>
#include <QTableWidget>
#include <QHeaderView>
#include <QTableWidgetItem>
#include <QDate>
#include <QPainter>
#include <QMap>
#include <QScrollArea>

class NumericTableWidgetItem : public QTableWidgetItem
{
public:
    using QTableWidgetItem::QTableWidgetItem;

    bool operator<(const QTableWidgetItem &other) const override
    {
        return text().toDouble() < other.text().toDouble();
    }
};
class DonutChart : public QWidget
{
public:
    explicit DonutChart(QWidget *parent = nullptr)
        : QWidget(parent)
    {
        setMinimumSize(300, 180);
    }

    void setValues(const QMap<QString, double> &newValues)
    {
        values = newValues;
        update();
    }

protected:
    void paintEvent(QPaintEvent *) override
    {
        QPainter painter(this);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillRect(rect(), QColor("#FFF0F5"));

        QRectF rect(
            (width() - 180) / 2.0,
            20,
            180,
            180
            );

        double total = 0;

        for (double amount : values)
            total += amount;

        if (total <= 0) {
            painter.setPen(QColor("#A78BBA"));
            painter.drawText(
                rect,
                Qt::AlignCenter,
                "No expenses yet"
                );
            return;
        }

        QStringList colors = {
            "#C4B5FD",
            "#FDA4AF",
            "#A7E8D4",
            "#F8D9A0",
            "#A7D8F0",
            "#E9B8E8"
        };

        int startAngle = 90 * 16;
        int colorIndex = 0;

        for (auto it = values.cbegin();
             it != values.cend(); ++it) {

            int spanAngle =
                qRound(it.value() / total * 360 * 16);

            painter.setBrush(
                QColor(colors[colorIndex % colors.size()])
                );
            painter.setPen(Qt::NoPen);

            painter.drawPie(
                rect,
                startAngle,
                -spanAngle
                );

            startAngle -= spanAngle;
            colorIndex++;
        }

        painter.setBrush(QColor("#FFF8FC"));
        painter.drawEllipse(rect.adjusted(
            48, 48, -48, -48
            ));

        painter.setPen(QColor("#6D527C"));
        painter.drawText(
            rect.adjusted(10, 65, -10, -65),
            Qt::AlignCenter,
            QString("Total\n$%1").arg(total, 0, 'f', 2)
            );
    }

private:
    QMap<QString, double> values;
};

QMap<QString, double> calculateCategoryTotals(QTableWidget *table);

QMap<QString, double> calculateFilteredTotals(
    QTableWidget *table,
    const QString &selectedCategory
    );
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("Personal Expense Tracker");
    resize(1100, 800);


    QWidget *central = new QWidget(this);

    central->setStyleSheet(
        "QWidget {"
        "background-color: #FFF8FC;"
        "color: #51445F;"
        "font-family: 'Segoe UI';"
        "font-size: 14px;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(central);
    layout->setSizeConstraint(QLayout::SetMinimumSize);

    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(central);
    scrollArea->setStyleSheet(
        "QScrollArea { border: none; }"
        );
    setCentralWidget(scrollArea);


    QLabel *title = new QLabel("Personal Expense Tracker");
    title->setStyleSheet(
        "font-size: 28px;"
        "font-weight: bold;"
        "color: #8B5CF6;"
        "padding: 8px 0px;"
        );
    layout->addWidget(title);

    QLabel *subtitle = new QLabel("Your money, under control.");
    subtitle->setStyleSheet(
        "font-size: 14px;"
        "color: #A78BBA;"
        "padding-bottom: 10px;"
        );
    layout->addWidget(subtitle);

    QHBoxLayout *cards = new QHBoxLayout();

    QLabel *total = new QLabel("Total Expenses\n$0.00");
    monthlyLabel = new QLabel("This Month\n$0.00");
    QLabel *transactions = new QLabel("Transactions\n0");

    QLabel *items[] = {total, monthlyLabel, transactions};

    QString cardColors[] = {
        "#FCE7F3",
        "#EDE9FE",
        "#DDF7F0"
    };

    for (int i = 0; i < 3; i++) {
        items[i]->setMinimumHeight(100);
        items[i]->setAlignment(Qt::AlignCenter);

        items[i]->setStyleSheet(
            QString(
                "background-color: %1;"
                "border: 1px solid #F3D9EB;"
                "border-radius: 18px;"
                "font-size: 18px;"
                "font-weight: bold;"
                "color: #6D527C;"
                "padding: 12px;"
                ).arg(cardColors[i])
            );

        cards->addWidget(items[i]);
    }


    layout->addLayout(cards);

    QLabel *chartTitle = new QLabel("✨ Expense Breakdown");
    chartTitle->setStyleSheet(
        "font-size: 20px;"
        "font-weight: bold;"
        "color: #8B5CF6;"
        "padding-top: 8px;"
        );

    layout->addWidget(chartTitle);

    DonutChart *chart = new DonutChart();
    chart->setMinimumSize(300, 180);

    QHBoxLayout *chartLayout = new QHBoxLayout();

    chartLayout->addWidget(chart);

    QVBoxLayout *legendLayout = new QVBoxLayout();

    chartLayout->addLayout(legendLayout);

    QStringList categories = {
        "Food",
        "Ingredients",
        "Transport",
        "Rent",
        "Shopping",
        "Other"
    };

    QStringList colors = {
        "#C4B5FD",
        "#FDA4AF",
        "#A7E8D4",
        "#F8D9A0",
        "#A7D8F0",
        "#E9B8E8"
    };

    for (int i = 0; i < categories.size(); i++) {
        QLabel *legendItem = new QLabel();

        legendItem->setText(
            QString("●  %1").arg(categories[i])
            );

        legendItem->setStyleSheet(
            QString("color: %1; font-size: 15px; padding: 5px;")
                .arg(colors[i])
            );

        legendLayout->addWidget(legendItem);
    }

    legendLayout->addStretch();

    layout->addLayout(chartLayout);

    QLineEdit *amountInput = new QLineEdit();
    amountInput->setPlaceholderText("Enter amount...");
    QLineEdit *nameInput = new QLineEdit();
    nameInput->setPlaceholderText("Expense name...");
    QComboBox *categoryInput = new QComboBox();

    categoryInput->addItem("Food");
    categoryInput->addItem("Ingredients");
    categoryInput->addItem("Transport");
    categoryInput->addItem("Rent");
    categoryInput->addItem("Shopping");
    categoryInput->addItem("Other");
    QString inputStyle =
        "QLineEdit, QComboBox {"
        "background-color: white;"
        "border: 1px solid #E9D5FF;"
        "border-radius: 10px;"
        "padding: 10px;"
        "color: #51445F;"
        "}"

        "QLineEdit:focus, QComboBox:focus {"
        "border: 2px solid #C4B5FD;"
        "}";

    nameInput->setStyleSheet(inputStyle);
    amountInput->setStyleSheet(inputStyle);
    categoryInput->setStyleSheet(inputStyle);

    QPushButton *addButton = new QPushButton("Add Expense");

    QPushButton *deleteButton =
        new QPushButton("Delete Selected Expense");
    QPushButton *editButton =
        new QPushButton("Edit Selected Expense");
    QPushButton *cancelEditButton =
        new QPushButton("Cancel Edit");

    addButton->setStyleSheet(
        "QPushButton {"
        "background-color: #C4B5FD;"
        "color: #49376B;"
        "border: none;"
        "border-radius: 12px;"
        "padding: 12px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #B39AFB;"
        "}"
        );

    deleteButton->setStyleSheet(
        "QPushButton {"
        "background-color: #FDA4AF;"
        "color: #713743;"
        "border: none;"
        "border-radius: 12px;"
        "padding: 10px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #FB8797;"
        "}"
        );
    editButton->setStyleSheet(
        "QPushButton {"
        "background-color: #A7E8D4;"
        "color: #355E55;"
        "border: none;"
        "border-radius: 12px;"
        "padding: 10px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #8BDCC5;"
        "}"
        );
    cancelEditButton->setStyleSheet(
        "QPushButton {"
        "background-color: #E9D5FF;"
        "color: #6D527C;"
        "border: none;"
        "border-radius: 12px;"
        "padding: 10px;"
        "font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "background-color: #D8B4FE;"
        "}"
        );


    layout->addWidget(nameInput);
    layout->addWidget(amountInput);
    layout->addWidget(categoryInput);
    layout->addWidget(addButton);
    layout->addWidget(editButton);
    layout->addWidget(cancelEditButton);
    layout->addWidget(deleteButton);

    table = new QTableWidget();

    table->setColumnCount(4);
    table->setMinimumHeight(200);
    table->setMaximumHeight(300);

    QComboBox *filterInput = new QComboBox();

    filterInput->addItem("All Categories");
    filterInput->addItem("Food");
    filterInput->addItem("Ingredients");
    filterInput->addItem("Transport");
    filterInput->addItem("Rent");
    filterInput->addItem("Shopping");
    filterInput->addItem("Other");

    filterInput->setStyleSheet(
        "QComboBox {"
        "background-color: #F3E8FF;"
        "border: 1px solid #D8B4FE;"
        "border-radius: 10px;"
        "padding: 8px;"
        "color: #6D527C;"
        "}"
        );


    QStringList headers;
    headers << "Expense Name"
            << "Amount"
            << "Category"
            << "Date";

    table->setHorizontalHeaderLabels(headers);

    table->horizontalHeader()->setStretchLastSection(true);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);
    table->horizontalHeader()->setSectionsClickable(true);

    table->setStyleSheet(
        "QTableWidget {"
        "background-color: #FFF5E8;"
        "alternate-background-color: #FCEBDD;"
        "gridline-color: #EAD8C8;"
        "border: 1px solid #EAD8C8;"
        "border-radius: 10px;"
        "color: #655568;"
        "selection-background-color: #E8DDFB;"
        "selection-color: #49376B;"
        "}"

        "QTableWidget::item {"
        "padding: 8px;"
        "}"

        "QHeaderView::section {"
        "background-color: #F3E4D7;"
        "color: #6D527C;"
        "padding: 8px;"
        "border: none;"
        "font-weight: bold;"
        "}"
        );

    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);


    table->setAlternatingRowColors(true);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setSortingEnabled(true);

    layout->addWidget(filterInput);

    layout->addWidget(table);


    connect(filterInput, &QComboBox::currentTextChanged,
            this, [this, filterInput, total, chart]() {

                QString selectedCategory = filterInput->currentText();

                for (int i = 0; i < table->rowCount(); i++) {
                    QString category = table->item(i, 2)->text();

                    bool showRow =
                        selectedCategory == "All Categories" ||
                        category == selectedCategory;

                    table->setRowHidden(i, !showRow);
                }

                QMap<QString, double> filteredTotals =
                    calculateFilteredTotals(table, selectedCategory);

                double filteredTotal = 0;

                for (double amount : filteredTotals) {
                    filteredTotal += amount;
                }

                chart->setValues(filteredTotals);

                total->setText(
                    QString("Total Expenses\n$%1")
                        .arg(filteredTotal, 0, 'f', 2)
                    );
            });



    connect(addButton, &QPushButton::clicked, this,
            [this, nameInput, amountInput, categoryInput, total, transactions,chart,filterInput]() {
                QString text = amountInput->text();

                bool ok;
                double amount = text.toDouble(&ok);

                if (!ok || amount <= 0) {
                    qDebug() << "Please enter a valid amount.";
                    return;
                }
                QString name = nameInput->text();

                if (name.isEmpty()) {
                    qDebug() << "Please enter an expense name.";
                    return;
                }
                QString category = categoryInput->currentText();

                QString date = QDate::currentDate().toString("yyyy-MM-dd");

                int row = table->rowCount();
                table->insertRow(row);
                transactions->setText(
                    QString("Transactions\n%1").arg(table->rowCount())
                    );

                table->setItem(row, 0, new QTableWidgetItem(name));
                table->setItem(row, 1,
                               new NumericTableWidgetItem(
                                   QString::number(amount, 'f', 2)));
                table->setItem(row, 2, new QTableWidgetItem(category));
                table->setItem(row, 3, new QTableWidgetItem(date));

                std::ofstream file("expenses.txt", std::ios::app);

                if (file.is_open()) {
                    file << name.toStdString() << ","
                         << amount << ","
                         << category.toStdString() << ","
                         << date.toStdString() << "\n";

                    file.close();
                }

                nameInput->clear();
                amountInput->clear();

                totalAmount += amount;

                transactions->setText(
                    QString("Transactions\n%1").arg(table->rowCount())
                    );

                updateMonthlyTotal();

                QString selectedCategory = filterInput->currentText();

                for (int i = 0; i < table->rowCount(); i++) {
                    QString category = table->item(i, 2)->text();

                    bool showRow =
                        selectedCategory == "All Categories" ||
                        category == selectedCategory;

                    table->setRowHidden(i, !showRow);
                }

                QMap<QString, double> filteredTotals =
                    calculateFilteredTotals(table, selectedCategory);

                double filteredTotal = 0;

                for (double amount : filteredTotals) {
                    filteredTotal += amount;
                }

                total->setText(
                    QString("Total Expenses\n$%1")
                        .arg(filteredTotal, 0, 'f', 2)
                    );

                chart->setValues(filteredTotals);

                qDebug() << "Total:" << totalAmount;
            });
    connect(editButton, &QPushButton::clicked, this,
            [this, nameInput, amountInput, categoryInput,
             editButton, total, transactions, chart, filterInput]() {

                if (editingRow == -1) {

                    int row = table->currentRow();

                    if (row < 0) {
                        qDebug() << "Please select an expense first.";
                        return;
                    }

                    nameInput->setText(table->item(row, 0)->text());
                    amountInput->setText(table->item(row, 1)->text());

                    int categoryIndex =
                        categoryInput->findText(table->item(row, 2)->text());

                    if (categoryIndex >= 0) {
                        categoryInput->setCurrentIndex(categoryIndex);
                    }

                    editingRow = row;
                    editButton->setText("Save Changes");

                    return;
                }

                QString name = nameInput->text().trimmed();
                QString amountText = amountInput->text().trimmed();

                bool ok;
                double amount = amountText.toDouble(&ok);

                if (name.isEmpty() || !ok || amount <= 0) {
                    qDebug() << "Enter a valid name and amount.";
                    return;
                }

                table->item(editingRow, 0)->setText(name);
                table->setItem(editingRow, 1,
                               new NumericTableWidgetItem(
                                   QString::number(amount, 'f', 2)));
                table->item(editingRow, 2)->setText(
                    categoryInput->currentText());

                totalAmount = 0;

                for (int i = 0; i < table->rowCount(); i++) {
                    totalAmount += table->item(i, 1)->text().toDouble();
                }

                std::ofstream outFile("expenses.txt");

                for (int i = 0; i < table->rowCount(); i++) {
                    outFile << table->item(i, 0)->text().toStdString() << ","
                            << table->item(i, 1)->text().toStdString() << ","
                            << table->item(i, 2)->text().toStdString() << ","
                            << table->item(i, 3)->text().toStdString() << "\n";
                }

                outFile.close();

                transactions->setText(
                    QString("Transactions\n%1").arg(table->rowCount()));

                updateMonthlyTotal();

                QString selectedCategory = filterInput->currentText();

                for (int i = 0; i < table->rowCount(); i++) {
                    QString category = table->item(i, 2)->text();

                    bool showRow =
                        selectedCategory == "All Categories" ||
                        category == selectedCategory;

                    table->setRowHidden(i, !showRow);
                }

                QMap<QString, double> filteredTotals =
                    calculateFilteredTotals(table, selectedCategory);

                double filteredTotal = 0;

                for (double value : filteredTotals) {
                    filteredTotal += value;
                }

                total->setText(
                    QString("Total Expenses\n$%1").arg(filteredTotal, 0, 'f', 2));

                chart->setValues(filteredTotals);

                editingRow = -1;
                editButton->setText("Edit Selected Expense");

                nameInput->clear();
                amountInput->clear();

                qDebug() << "Expense updated successfully.";
            });
    connect(cancelEditButton, &QPushButton::clicked, this,
            [this, nameInput, amountInput, editButton]() {

                editingRow = -1;

                nameInput->clear();
                amountInput->clear();

                editButton->setText("Edit Selected Expense");

                qDebug() << "Editing cancelled.";
            });
    connect(deleteButton, &QPushButton::clicked, this,[this,total,transactions,chart,filterInput]() {
        int row = table->currentRow();

        if (row < 0) {
            qDebug() << "Please select an expense first.";
            return;
        }

        table->removeRow(row);
        transactions->setText(
            QString("Transactions\n%1").arg(table->rowCount())
            );

        totalAmount = 0;

        for (int i = 0; i < table->rowCount(); i++) {
            totalAmount += table->item(i, 1)->text().toDouble();
        }
        updateMonthlyTotal();

        QString selectedCategory = filterInput->currentText();

        for (int i = 0; i < table->rowCount(); i++) {
            QString category = table->item(i, 2)->text();

            bool showRow =
                selectedCategory == "All Categories" ||
                category == selectedCategory;

            table->setRowHidden(i, !showRow);
        }

        QMap<QString, double> filteredTotals =
            calculateFilteredTotals(table, selectedCategory);

        double filteredTotal = 0;

        for (double amount : filteredTotals) {
            filteredTotal += amount;
        }

        total->setText(
            QString("Total Expenses\n$%1")
                .arg(filteredTotal, 0, 'f', 2)
            );

        chart->setValues(filteredTotals);
        std::ofstream outFile("expenses.txt");

        for (int i = 0; i < table->rowCount(); i++) {
            QString name = table->item(i, 0)->text();
            QString amount = table->item(i, 1)->text();
            QString category = table->item(i, 2)->text();
            QString date = table->item(i, 3)->text();

            outFile << name.toStdString() << ","
                    << amount.toStdString() << ","
                    << category.toStdString() << ","
                    << date.toStdString() << "\n";

        }

        outFile.close();
    });
    std::ifstream file("expenses.txt");

    if (file.is_open()) {
        std::string line;

        while (std::getline(file, line)) {
            std::stringstream ss(line);

            std::string name;
            std::string amountText;
            std::string category = "Other";
            std::string date = "";

            if (std::getline(ss, name, ',') &&
                std::getline(ss, amountText, ',')) {

                std::string savedCategory;
                std::string savedDate;

                if (std::getline(ss, savedCategory, ',')) {
                    category = savedCategory;
                }

                if (std::getline(ss, savedDate)) {
                    date = savedDate;
                }

                bool ok;
                double amount =
                    QString::fromStdString(amountText).toDouble(&ok);

                if (ok) {
                    int row = table->rowCount();
                    table->insertRow(row);

                    table->setItem(row, 0,
                                   new QTableWidgetItem(
                                       QString::fromStdString(name)));

                    table->setItem(row, 1,
                                   new NumericTableWidgetItem(
                                       QString::number(amount, 'f', 2)));
                    table->setItem(row, 2,
                                   new QTableWidgetItem(
                                       QString::fromStdString(category)));
                    table->setItem(row, 3,
                                   new QTableWidgetItem(QString::fromStdString(date)));

                    totalAmount += amount;
                }
            }
        }

        file.close();
    }

    total->setText(
        QString("Total Expenses\n$%1").arg(totalAmount, 0, 'f', 2)
        );
    transactions->setText(
        QString("Transactions\n%1").arg(table->rowCount())
        );
    updateMonthlyTotal();
    qDebug() << "Rows loaded:" << table->rowCount();
    qDebug() << "Category totals:" << calculateCategoryTotals(table);
    chart->setValues(calculateCategoryTotals(table));


}
void MainWindow::updateMonthlyTotal()
{
    double monthlyTotal = 0;

    QString currentMonth =
        QDate::currentDate().toString("yyyy-MM");

    for (int i = 0; i < table->rowCount(); i++) {
        QString date = table->item(i, 3)->text();

        if (date.startsWith(currentMonth)) {
            monthlyTotal += table->item(i, 1)->text().toDouble();
        }
    }

    monthlyLabel->setText(
        QString("This Month\n$%1").arg(monthlyTotal, 0, 'f', 2)
        );
}
QMap<QString, double> calculateCategoryTotals(QTableWidget *table)
{
    QMap<QString, double> totals;

    for (int i = 0; i < table->rowCount(); i++) {
        QString category = table->item(i, 2)->text();
        double amount = table->item(i, 1)->text().toDouble();

        totals[category] += amount;
    }

    return totals;

}
QMap<QString, double> calculateFilteredTotals(
    QTableWidget *table,
    const QString &selectedCategory)
{
    QMap<QString, double> totals;

    for (int i = 0; i < table->rowCount(); i++) {
        QString category = table->item(i, 2)->text();
        double amount = table->item(i, 1)->text().toDouble();

        if (selectedCategory == "All Categories" ||
            category == selectedCategory) {
            totals[category] += amount;
        }
    }

    return totals;
}
MainWindow::~MainWindow()
{
    delete ui;
}
