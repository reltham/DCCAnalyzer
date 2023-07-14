#include "DCCAnalyzer.h"
#include "DCCAnalyzerSettings.h"
#include <AnalyzerChannelData.h>

DCCAnalyzer::DCCAnalyzer()
:	Analyzer2(),  
	mSettings( new DCCAnalyzerSettings() ),
	mSimulationInitialized( false )
{
	SetAnalyzerSettings( mSettings.get() );
}

DCCAnalyzer::~DCCAnalyzer()
{
	KillThread();
}

void DCCAnalyzer::SetupResults()
{
	mResults.reset(new DCCAnalyzerResults(this, mSettings.get()));
	SetAnalyzerResults(mResults.get());
	mResults->AddChannelBubblesWillAppearOn(mSettings->mInputChannel);
}

DCCAnalyzer::DCCBitState DCCAnalyzer::DetermineHalfBitType(U64 bitlen, BitTimingFilterType* filter)
{
	if (bitlen >= filter->bit0min && bitlen <= filter->bit0max)
		return BS_0;
	else if (bitlen >= filter->bit1min && bitlen <= filter->bit1max)
		return BS_1;
	return BS_NONE;
}

void DCCAnalyzer::WorkerThread()
{
	mSampleRateHz = GetSampleRate();

	BitTimingFilterType filter_strict;
	BitTimingFilterType filter_relaxed;
	BitTimingFilterType filter_outofspec;

	filter_strict.bit0min = (U64)mSampleRateHz * 95 / 1000000;
	filter_strict.bit0max = (U64)mSampleRateHz * 9900 / 1000000;
	filter_strict.bit0summax = (U64)mSampleRateHz * 12000 / 1000000;
	filter_strict.bit1min = (U64)mSampleRateHz * 55 / 1000000;
	filter_strict.bit1max = (U64)mSampleRateHz * 61 / 1000000;
	filter_strict.bit1diffmax = (U64)mSampleRateHz * 3 / 1000000;
	filter_strict.preamble_count_min = 14;

	filter_relaxed.bit0min = (U64)mSampleRateHz * 90 / 1000000;
	filter_relaxed.bit0max = (U64)mSampleRateHz * 10000 / 1000000;
	filter_relaxed.bit0summax = (U64)mSampleRateHz * 12000 / 1000000;	// Note: Value not specified in NMRA standards
	filter_relaxed.bit1min = (U64)mSampleRateHz * 52 / 1000000;
	filter_relaxed.bit1max = (U64)mSampleRateHz * 64 / 1000000;
	filter_relaxed.bit1diffmax = (U64)mSampleRateHz * 6 / 1000000;
	filter_relaxed.preamble_count_min = 10;

	filter_outofspec.bit0min = (U64)mSampleRateHz * 80 / 1000000;		// Out-of-spec but decodeable is not part of the NMRA standards
	filter_outofspec.bit0max = (U64)mSampleRateHz * 15000 / 1000000;
	filter_outofspec.bit0summax = (U64)mSampleRateHz * 20000 / 1000000;
	filter_outofspec.bit1min = (U64)mSampleRateHz * 46 / 1000000;
	filter_outofspec.bit1max = (U64)mSampleRateHz * 70 / 1000000;
	filter_outofspec.bit1diffmax = (U64)mSampleRateHz * 10 / 1000000;
	filter_outofspec.preamble_count_min = 6;

	mSerial = GetAnalyzerChannelData( mSettings->mInputChannel );
	
	mState = DS_IDLE;
	DCCBitState prev_bit;
	U64 prev_bittime;
	U64 frame_start;
	U64 frame_end;
	U32 bit_count;
	U32 data;
	U32 checksum;
	BitTimingFilterType* pFilter;

	if (mSettings->mStrictTiming)
		pFilter = &filter_strict;
	else
		pFilter = &filter_relaxed;

	for( ; ; )
	{

		mPrevEdge = mSerial->GetSampleNumber();
		mSerial->AdvanceToNextEdge();

		U64 bittime = mSerial->GetSampleNumber() - mPrevEdge;
		DCCBitState bittype = DetermineHalfBitType(bittime, pFilter);
		bool timing_error = false;
		if (bittype == BS_NONE)
		{
			timing_error = true;
			bittype = DetermineHalfBitType(bittime, &filter_outofspec);
		}
		// bittype now contain a DCC half-bit
		
		if (bittype == BS_NONE)
		{
			if (mState != DS_IDLE && mState != DS_PREAMBLE)
			{
				mResults->AddMarker( mPrevEdge, AnalyzerResults::Stop, mSettings->mInputChannel );
			}
			mState = DS_IDLE;	// Reset DCC packet decoder state machine
			continue;
		}
		
		// Half-bit is more or less valid. Continue attempt to decode

		// Display timing errors
		if (timing_error)
			mResults->AddMarker( mPrevEdge + (bittime >> 1), AnalyzerResults::ErrorSquare, mSettings->mInputChannel );

		// Pass half-bits as full bits in IDLE and PREAMBLE. Do proper full-bit decoding otherwise
		if (mState == DS_IDLE || mState == DS_PREAMBLE)
		{
			prev_bit = BS_NONE;
		}
		else
		{
			if (prev_bit == BS_NONE)	// No previous half-bit. Just note this one and continue
			{
				prev_bit = bittype;
				prev_bittime = bittime;
				continue;
			}
			if (prev_bit != bittype)	// Previous half-bit does not match this one. Sync error
			{
				prev_bit = BS_NONE;
				mState = DS_IDLE;
				// Place error marker here
				mResults->AddMarker( mPrevEdge, AnalyzerResults::Stop, mSettings->mInputChannel );
				continue;
			}
			// Both half-bits matches. We now have a full 0 or 1 bit
			prev_bit = BS_NONE;
			if (bittype == BS_0)
			{
				// More error-checking. Sum of both half-bits must not exceed limit
				U64 sum = prev_bittime + bittime;
				if (sum > pFilter->bit0summax)
				{
					// Place error marker here
					mResults->AddMarker( mPrevEdge, AnalyzerResults::ErrorX, mSettings->mInputChannel );
					if (sum > filter_outofspec.bit0summax)
					{
						// Completely out-of-spec. Reset frame decoder
						mState = DS_IDLE;
						continue;
					}
				}
			}
			else	// BS_1
			{
				// More error-checking. Difference of both half-bits must not exceed limit
				U64 diff;
				if (prev_bittime > bittime)
					diff = prev_bittime - bittime;
				else
					diff = bittime - prev_bittime;
				if (diff > pFilter->bit1diffmax)
				{
					// Place error marker here
					mResults->AddMarker( mPrevEdge, AnalyzerResults::ErrorX, mSettings->mInputChannel );
					if (diff > filter_outofspec.bit1diffmax)
					{
						// Completely out-of-spec. Reset frame decoder
						mState = DS_IDLE;
						continue;
					}
				}
			}
		}

		// bittype is now a fully valid 0 or 1 bit

		switch (mState)
		{
		case DS_IDLE:
			{
				if (bittype == BS_1)
				{
					frame_start = mPrevEdge + 1;
					bit_count = 1;
					mState = DS_PREAMBLE;
				}
				break;
			}

		case DS_PREAMBLE:
			{
				if (bittype == BS_1)
				{
					bit_count++;
				}
				else if (bittype == BS_0)
				{
					bit_count >>= 1;

					if (bit_count < filter_outofspec.preamble_count_min)	// Don't report preamble with less than 6 full 1 bits
					{
						mState = DS_IDLE;
					}
					else
					{
						//we have data to report.
						Frame frame;
						frame.mData1 = bit_count & 0xFFFF;
						frame.mData2 = DCCAnalyzerResults::FT_Preamble;
						frame.mFlags = 0;
						frame.mStartingSampleInclusive = frame_start;
						frame.mEndingSampleInclusive = mPrevEdge;

						if (bit_count < pFilter->preamble_count_min)
						{
							frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
							mResults->AddMarker( (frame_start + mPrevEdge) >> 1, AnalyzerResults::Stop, mSettings->mInputChannel );
						}
						mResults->AddFrame( frame );
						mResults->CommitResults();
						ReportProgress( frame.mEndingSampleInclusive );

						prev_bit = BS_0;			// Seed half-bit decoder with this half-bit 0
						prev_bittime = bittime;
						mState = DS_HALFSTART;
					}
				}
				break;
			}

		case DS_HALFSTART:
			{
				// bittype can only be BS_0 if we ended up here
				
				// Set data byte start marker
				mResults->AddMarker( mPrevEdge, AnalyzerResults::Zero, mSettings->mInputChannel );

				frame_start = mSerial->GetSampleNumber() + 1;
				data = 0;
				checksum = 0;
				bit_count = 1 << 7;
				mState = DS_DATABYTE;
				break;
			}

		case DS_DATABYTE:
			{
				if (bittype == BS_1)
					data |= bit_count;
				bit_count >>= 1;
				if (!bit_count)
				{
					// Full byte received now
					frame_end = mSerial->GetSampleNumber();
					checksum ^= data;
					mState = DS_STARTEND;
				}
				break;
			}
		
		case DS_STARTEND:
			{
				// Report previous byte frame now that we know what type it was
				//we have data to report.
				Frame frame;
				frame.mData1 = data & 0xFF;
				frame.mFlags = 0;
				frame.mStartingSampleInclusive = frame_start;
				frame.mEndingSampleInclusive = frame_end;

				if (bittype == BS_0)	// Start on another databyte
				{
					frame.mData2 = DCCAnalyzerResults::FT_Data;
				}
				else
				{
					frame.mData2 = DCCAnalyzerResults::FT_Checksum;
					if ((checksum & 0xFF) != 0)
					{
						frame.mFlags |= DISPLAY_AS_ERROR_FLAG;
						frame.mData1 |= ((data ^ checksum) & 0xFF) << 8;
						mResults->AddMarker( (frame_start + frame_end) >> 1, AnalyzerResults::Stop, mSettings->mInputChannel );
					}
				}

				mResults->AddFrame( frame );
				mResults->CommitResults();
				ReportProgress( frame.mEndingSampleInclusive );

				if (bittype == BS_0)	// Start on another databyte
				{
					// Set data byte start marker
					mResults->AddMarker( mPrevEdge, AnalyzerResults::Zero, mSettings->mInputChannel );

					frame_start = mSerial->GetSampleNumber() + 1;
					data = 0;
					bit_count = 1 << 7;
					mState = DS_DATABYTE;
				}
				else
				{
					// Set data byte end marker
					mResults->AddMarker( mPrevEdge, AnalyzerResults::One, mSettings->mInputChannel );

					mState = DS_IDLE;
				}
				break;
			}

		default:
			// Should not happen
			mState = DS_IDLE;
			break;
		}



	}
}

bool DCCAnalyzer::NeedsRerun()
{
	return false;
}

U32 DCCAnalyzer::GenerateSimulationData( U64 minimum_sample_index, U32 device_sample_rate, SimulationChannelDescriptor** simulation_channels )
{
	if( mSimulationInitialized == false )
	{
		mSimulationDataGenerator.Initialize( GetSimulationSampleRate(), mSettings.get() );
		mSimulationInitialized = true;
	}

	return mSimulationDataGenerator.GenerateSimulationData( minimum_sample_index, device_sample_rate, simulation_channels );
}

U32 DCCAnalyzer::GetMinimumSampleRateHz()
{
	return 1000000;		// 1 MHz
}

const char* DCCAnalyzer::GetAnalyzerName() const
{
	return "DCC";
}

const char* GetAnalyzerName()
{
	return "DCC";
}

Analyzer* CreateAnalyzer()
{
	return new DCCAnalyzer();
}

void DestroyAnalyzer( Analyzer* analyzer )
{
	delete analyzer;
}