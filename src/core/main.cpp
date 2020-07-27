#include <QtDebug>
#include <QWidget>
#include <QComboBox>
#include <QVBoxLayout>
#include <QRadioButton>
#include <QApplication>
#include <QTableView>

#include "systemtextinputmanager.h"

int main(int argc, char **argv)
{
    QApplication a(argc, argv);

    SystemTextInputManager::instance();

    QWidget window;

    QTableView tableView;
    tableView.setModel(SystemTextInputManager::instance());
    tableView.show();

    return  a.exec();
}
