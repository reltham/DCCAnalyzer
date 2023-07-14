#ifndef DCC_ANALYZER_SETTINGS
#define DCC_ANALYZER_SETTINGS

#include <AnalyzerSettings.h>
#include <AnalyzerTypes.h>

class DCCAnalyzerSettings : public AnalyzerSettings
{
public:
	DCCAnalyzerSettings();
	virtual ~DCCAnalyzerSettings();

	virtual bool SetSettingsFromInterfaces();
	void UpdateInterfacesFromSettings();
	virtual void LoadSettings( const char* settings );
	virtual const char* SaveSettings();

	
	Channel mInputChannel;
	bool mStrictTiming;

protected:
	std::unique_ptr< AnalyzerSettingInterfaceChannel >	mInputChannelInterface;
	std::unique_ptr< AnalyzerSettingInterfaceBool > mStrictTimingInterface;
};

#endif //DCC_ANALYZER_SETTINGS
