#ifndef LMSENSORS_H
#define LMSENSORS_H

#include <SensorPlugin.h>

class SensorsFeatureSensor;

class LmSensorsPlugin : public SensorPlugin
{
    Q_OBJECT
public:
    LmSensorsPlugin(QObject *parent, const QVariantList &args);
    ~LmSensorsPlugin() override;
    QString providerName() const override;
    void update() override;
private:
    QVector<SensorsFeatureSensor*> m_sensors;
};
#endif
