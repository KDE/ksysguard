/****************************************************************************
** Form interface generated from reading ui file 'MultiMeterSettings.ui'
**
** Created: Tue Jul 18 02:10:57 2000
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef MULTIMETERSETTINGS_H
#define MULTIMETERSETTINGS_H

#include <klocale.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSpinBox;

class MultiMeterSettings : public QDialog
{ 
    Q_OBJECT

public:
    MultiMeterSettings( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~MultiMeterSettings();

    QGroupBox* GroupBox1;
    QCheckBox* lowerLimitActive;
    QLabel* TextLabel2;
    QSpinBox* lowerLimit;
    QGroupBox* GroupBox1_2;
    QCheckBox* upperLimitActive;
    QLabel* TextLabel2_2;
    QSpinBox* upperLimit;
    QPushButton* okButton;
    QPushButton* applyButton;
    QPushButton* cancelButton;
    QLabel* TextLabel1;
    QLineEdit* title;
    QCheckBox* showUnit;

public slots:
    virtual void applyClicked();

protected:
    QHBoxLayout* hbox;
    QHBoxLayout* hbox_2;
    QGridLayout* grid;
    QGridLayout* grid_2;
    QGridLayout* grid_3;
};

#endif // MULTIMETERSETTINGS_H
