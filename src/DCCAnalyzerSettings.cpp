#include "DCCAnalyzerSettings.h"
#include <AnalyzerHelpers.h>


DCCAnalyzerSettings::DCCAnalyzerSettings()
:	mInputChannel( UNDEFINED_CHANNEL ),
	mStrictTiming( false )
{
	mInputChannelInterface.reset( new AnalyzerSettingInterfaceChannel() );
	mInputChannelInterface->SetTitleAndTooltip( "DCC", "DCC data line" );
	mInputChannelInterface->SetChannel( mInputChannel );

	mStrictTimingInterface.reset( new AnalyzerSettingInterfaceBool() );
	mStrictTimingInterface->SetTitleAndTooltip("", "Enable to verify Command Station transmission conformance");
	mStrictTimingInterface->SetCheckBoxText("Display strict timing errors");
	mStrictTimingInterface->SetValue(mStrictTiming);

	AddInterface( mInputChannelInterface.get() );
	AddInterface(mStrictTimingInterface.get());

	AddExportOption( 0, "Export as text/csv file" );
	AddExportExtension( 0, "text", "txt" );
	AddExportExtension( 0, "csv", "csv" );

	ClearChannels();
	AddChannel( mInputChannel, "DCC", false );
}

DCCAnalyzerSettings::~DCCAnalyzerSettings()
{
}

bool DCCAnalyzerSettings::SetSettingsFromInterfaces()
{
	mInputChannel = mInputChannelInterface->GetChannel();
	mStrictTiming = mStrictTimingInterface->GetValue();

	ClearChannels();
	AddChannel( mInputChannel, "DCC", true );

	return true;
}

void DCCAnalyzerSettings::UpdateInterfacesFromSettings()
{
	mInputChannelInterface->SetChannel( mInputChannel );
	mStrictTimingInterface->SetValue(mStrictTiming);
}

void DCCAnalyzerSettings::LoadSettings( const char* settings )
{
	SimpleArchive text_archive;
	text_archive.SetString( settings );

	text_archive >> mInputChannel;
	text_archive >> mStrictTiming;
	
	ClearChannels();
	AddChannel( mInputChannel, "DCC", true );

	UpdateInterfacesFromSettings();
}

const char* DCCAnalyzerSettings::SaveSettings()
{
	SimpleArchive text_archive;

	text_archive << mInputChannel;
	text_archive << mStrictTiming;

	return SetReturnString( text_archive.GetString() );
}
