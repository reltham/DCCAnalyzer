#ifndef DCC_ANALYZER_H
#define DCC_ANALYZER_H

#include <Analyzer.h>
#include "DCCAnalyzerResults.h"
#include "DCCSimulationDataGenerator.h"

class DCCAnalyzerSettings;
class ANALYZER_EXPORT DCCAnalyzer : public Analyzer2
{
public:
	DCCAnalyzer();
	virtual ~DCCAnalyzer();
	virtual void WorkerThread();

	virtual U32 GenerateSimulationData( U64 newest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channels );
	virtual U32 GetMinimumSampleRateHz();

	virtual const char* GetAnalyzerName() const;
	virtual bool NeedsRerun();

protected: //vars
	std::unique_ptr< DCCAnalyzerSettings > mSettings;
	std::unique_ptr< DCCAnalyzerResults > mResults;
	AnalyzerChannelData* mSerial;

	DCCSimulationDataGenerator mSimulationDataGenerator;
	bool mSimulationInitialized;

	enum DCCBitState { BS_NONE, BS_0, BS_1 };
	//typedef enum DCCBitTiming { BT_OUTOFSPEC, BT_RELAXED, BT_STRICT };
	enum DCCDecoderState { DS_IDLE, DS_PREAMBLE, DS_HALFSTART, DS_DATABYTE, DS_STARTEND };
	typedef struct {
		U64 bit0min;
		U64 bit0max;
		U64 bit0summax;
		U64 bit1min;
		U64 bit1max;
		U64 bit1diffmax;
		U32 preamble_count_min;
	} BitTimingFilterType;

	//Serial analysis vars:
	U32 mSampleRateHz;
	U64 mPrevEdge;
	DCCDecoderState mState;

	void SetupResults();
	DCCBitState DetermineHalfBitType(U64 bitlen, BitTimingFilterType* filter);
};

extern "C" ANALYZER_EXPORT const char* __cdecl GetAnalyzerName();
extern "C" ANALYZER_EXPORT Analyzer* __cdecl CreateAnalyzer( );
extern "C" ANALYZER_EXPORT void __cdecl DestroyAnalyzer( Analyzer* analyzer );

#endif //DCC_ANALYZER_H
