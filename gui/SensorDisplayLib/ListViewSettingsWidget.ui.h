/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


QString ListViewSettingsWidget::title()
{
    return m_title->text();
}


QColor ListViewSettingsWidget::gridColor()
{
    return m_gridColor->color();
}


QColor ListViewSettingsWidget::backgroundColor()
{
    return m_backgroundColor->color();
}


QColor ListViewSettingsWidget::textColor()
{
    return m_textColor->color();
}


void ListViewSettingsWidget::setTitle( const QString &t )
{
    m_title->setText(t);
}


void ListViewSettingsWidget::setBackgroundColor( const QColor &c )
{
    m_backgroundColor->setColor(c);
}


void ListViewSettingsWidget::setTextColor( const QColor &c )
{
    m_textColor->setColor(c);
}


void ListViewSettingsWidget::setGridColor( const QColor &c )
{
    m_gridColor->setColor(c);
}
