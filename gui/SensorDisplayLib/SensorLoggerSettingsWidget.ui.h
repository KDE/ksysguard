/****************************************************************************
** ui.h extension file, included from the uic-generated form implementation.
**
** If you wish to add, delete or rename functions or slots use
** Qt Designer which will update this file, preserving your code. Create an
** init() function in place of a constructor, and a destroy() function in
** place of a destructor.
*****************************************************************************/


QString SensorLoggerSettingsWidget::title()
{
    return m_title->text();
}


QColor SensorLoggerSettingsWidget::foregroundColor()
{
    return m_foregroundColor->color();
}


QColor SensorLoggerSettingsWidget::backgroundColor()
{
    return m_backgroundColor->color();
}


QColor SensorLoggerSettingsWidget::alarmColor()
{
    return m_alarmColor->color();
}


void SensorLoggerSettingsWidget::setTitle( const QString &t )
{
    m_title->setText(t);
}


void SensorLoggerSettingsWidget::setForegroundColor( const QColor &c )
{
    m_foregroundColor->setColor(c);
}


void SensorLoggerSettingsWidget::setBackgroundColor( const QColor &c )
{
    m_backgroundColor->setColor(c);
}


void SensorLoggerSettingsWidget::setAlarmColor( const QColor &c )
{
    m_alarmColor->setColor(c);
}
