#ifndef DCC_ANALYZER_RESULTS
#define DCC_ANALYZER_RESULTS

#include <AnalyzerResults.h>

class DCCAnalyzer;
class DCCAnalyzerSettings;

class DCCAnalyzerResults : public AnalyzerResults
{
public:
	DCCAnalyzerResults( DCCAnalyzer* analyzer, DCCAnalyzerSettings* settings );
	virtual ~DCCAnalyzerResults();

	virtual void GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base );
	virtual void GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id );

	virtual void GenerateFrameTabularText(U64 frame_index, DisplayBase display_base );
	virtual void GeneratePacketTabularText( U64 packet_id, DisplayBase display_base );
	virtual void GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base );

	enum FrameType { FT_Preamble, FT_Data, FT_Checksum };
protected: //functions

protected:  //vars
	DCCAnalyzerSettings* mSettings;
	DCCAnalyzer* mAnalyzer;
};

#endif //DCC_ANALYZER_RESULTS
