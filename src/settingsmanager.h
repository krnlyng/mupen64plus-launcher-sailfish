#ifndef SETTINGSMANAGER_H
#define SETTINGSMANAGER_H

#include <QObject>
#include <QStringList>

#include <m64p_types.h>



class SettingsManager : public QObject
{
    Q_OBJECT
public:
    SettingsManager();
    ~SettingsManager();
private:
    void addToCurrentSettings(const char *setting_name, m64p_type setting_type);
    QStringList my_current_settings;
    friend void list_parameters_callback(void *context, const char *setting_name, m64p_type setting_type);
    bool settings_initialized;
    m64p_handle config_section;
public slots:
    QString getCurrentPlugin(QString type);
    qint32 getCurrentPluginIndex(QString type);
    QStringList getPossibleSettings(QString plugin_type, QString plugin_name);
    QStringList getPossibleGeneralSettings(QString plugin_type);
    QString getSettingValue(QString plugin_type, QString plugin_name, QString setting_name);
    QString getGeneralSettingValue(QString plugin_type, QString plugin_name, QString setting_name);
    qint32 getSettingType(QString plugin_type, QString plugin_name, QString setting);
    qint32 getGeneralSettingType(QString plugin_type, QString setting);
    QString getSettingHint(QString plugin_type, QString , QString setting);
    QString getGeneralSettingHint(QString plugin_type, QString plugin_name, QString setting);
    qint32 saveSetting(QString plugin_type, QString plugin_name, QString setting, QString value);
    qint32 saveGeneralSetting(QString plugin_type, QString plugin_name, QString setting, QString value);
    qint32 pluginCount(QString plugin_type);
    QString getPlugin(QString plugin_type, qint32 plugin_num);
    void selectPlugin(QString plugin_type, qint32 plugin_num);
    void loadSettings();
    void unloadSettings();
};

#endif // SETTINGSMANAGER_H
