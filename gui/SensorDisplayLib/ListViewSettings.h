/* This file is part of the KDE project
   Copyright ( C ) 2003 Nadeem Hasan <nhasan@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (  at your option ) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#ifndef LISTVIEWSETTINGS_H
#define LISTVIEWSETTINGS_H

#include <QDialog>

class QColor;

class Ui_ListViewSettingsWidget;

class ListViewSettings : public QDialog
{
  Q_OBJECT

  public:

    explicit ListViewSettings( QWidget *parent=nullptr, const QString &name=QString() );
    ~ListViewSettings() override;

    QString title() const;
    QColor textColor() const;
    QColor backgroundColor() const;
    QColor gridColor() const;

    void setTitle( const QString & );
    void setTextColor( const QColor & );
    void setBackgroundColor( const QColor & );
    void setGridColor( const QColor & );

  private:

    Ui_ListViewSettingsWidget *m_settingsWidget;
};

#endif // LISTVIEWSETTINGS_H

/* vim: et sw=2 ts=2
*/
