/*
    KSysGuard, the KDE System Guard

    Copyright (c) 1999, 2000 Chris Schlaeger <cs@kde.org>

    This program is free software; you can redistribute it and/or
    modify it under the terms of version 2 of the GNU General Public
    License as published by the Free Software Foundation.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

    $Id$
*/

#ifndef KSG_HOSTCONNECTOR_H
#define KSG_HOSTCONNECTOR_H

#include <kdialogbase.h>

class KComboBox;

class QLabel;
class QRadioButton;
class QSpinBox;

class HostConnector : public KDialogBase
{
  Q_OBJECT

  public:
    HostConnector( QWidget *parent, const char *name = 0 );
    ~HostConnector();

    void setHostNames( const QStringList &list );
    QStringList hostNames() const;

    void setCommands( const QStringList &list );
    QStringList commands() const;

    void setCurrentHostName( const QString &hostName );

    QString currentHostName() const;
    QString currentCommand() const;
    int port() const;

    bool useSsh() const;
    bool useRsh() const;
    bool useDaemon() const;
    bool useCustom() const;

  protected slots:
    virtual void slotHelp();
    void slotHostNameChanged( const QString &_text );
  private:
    KComboBox *mCommands;
    KComboBox *mHostNames;

    QLabel *mHostNameLabel;

    QRadioButton *mUseSsh;
    QRadioButton *mUseRsh;
    QRadioButton *mUseDaemon;
    QRadioButton *mUseCustom;

    QSpinBox *mPort;
};

#endif
