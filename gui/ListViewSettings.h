/****************************************************************************
** Form interface generated from reading ui file './ListViewSettings.ui'
**
** Created: Sun Apr 22 12:21:47 2001
**      by:  The User Interface Compiler (uic)
**
** WARNING! All changes made in this file will be lost!
****************************************************************************/
#ifndef LISTVIEWSETTINGS_H
#define LISTVIEWSETTINGS_H

#include <qvariant.h>
#include <qdialog.h>
class QVBoxLayout; 
class QHBoxLayout; 
class QGridLayout; 
class KColorButton;
class QGroupBox;
class QLabel;
class QPushButton;

class ListViewSettings : public QDialog
{ 
    Q_OBJECT

public:
    ListViewSettings( QWidget* parent = 0, const char* name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~ListViewSettings();

    QGroupBox* GroupBox1;
    QLabel* gridColorLabel;
    KColorButton* gridColorButton;
    QGroupBox* GroupBox2;
    QLabel* textColorLabel;
    KColorButton* textColorButton;
    QPushButton* okButton;
    QPushButton* applyButton;
    QPushButton* cancelButton;

public slots:
    virtual void applyClicked();

protected:
    QGridLayout* ListViewSettingsLayout;
    QGridLayout* GroupBox1Layout;
    QGridLayout* GroupBox2Layout;
    QHBoxLayout* Layout3;
};

#endif // LISTVIEWSETTINGS_H
