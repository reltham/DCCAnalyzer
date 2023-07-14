#include "DCCSimulationDataGenerator.h"
#include "DCCAnalyzerSettings.h"

#include <AnalyzerHelpers.h>

DCCSimulationDataGenerator::DCCSimulationDataGenerator()
{
}

DCCSimulationDataGenerator::~DCCSimulationDataGenerator()
{
}

void DCCSimulationDataGenerator::Initialize( U32 simulation_sample_rate, DCCAnalyzerSettings* settings )
{
	mSimulationSampleRateHz = simulation_sample_rate;
	mSettings = settings;

	mSerialSimulationData.SetChannel( mSettings->mInputChannel );
	mSerialSimulationData.SetSampleRate( simulation_sample_rate );
	mSerialSimulationData.SetInitialBitState( BIT_HIGH );

	mBitLen0 = (U64)mSimulationSampleRateHz * 100 / 1000000;
	mBitLen1 = (U64)mSimulationSampleRateHz * 58 / 1000000;
}

U32 DCCSimulationDataGenerator::GenerateSimulationData( U64 largest_sample_requested, U32 sample_rate, SimulationChannelDescriptor** simulation_channel )
{
	U64 adjusted_largest_sample_requested = AnalyzerHelpers::AdjustSimulationTargetSample( largest_sample_requested, sample_rate, mSimulationSampleRateHz );

	while( mSerialSimulationData.GetCurrentSampleNumber() < adjusted_largest_sample_requested )
	{
		// Create normal packet
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x37);
		Create0bit();
		CreateByte(0x74);
		Create0bit();
		CreateByte(0x43);
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance(8 * mBitLen0);

		// Test problematic packet from sporskiftet.dk
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x03);
		Create0bit();
		CreateByte(0x3F);
		Create0bit();
		CreateByte(0x80);
		Create1bit();
		// Test problematic packet from sporskiftet.dk
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x03);
		Create0bit();
		CreateByte(0x3F);
		Create0bit();
		CreateByte(0x80);
		Create1bit();
		// Test problematic packet from sporskiftet.dk
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x03);
		Create0bit();
		CreateByte(0x3F);
		Create0bit();
		CreateByte(0x80);
		Create1bit();
		// Test problematic packet from sporskiftet.dk
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x03);
		Create0bit();
		CreateByte(0x3F);
		Create0bit();
		CreateByte(0x80);
		Create1bit();
		// Test problematic packet from sporskiftet.dk
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x03);
		Create0bit();
		CreateByte(0x3F);
		Create0bit();
		CreateByte(0x80);
		Create1bit();


		// Create long packet
		CreatePreamble(14);
		Create0bit();
		CreateByte(0xB5);
		Create0bit();
		CreateByte(0xC9);
		Create0bit();
		CreateByte(0xE6);
		Create0bit();
		CreateByte(0x4A);
		Create0bit();
		CreateByte(0x21);
		Create0bit();
		CreateByte(0xF1);
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance(8 * mBitLen0);

		// Create packet with short preamble (only detectable in strict timing mode)
		CreatePreamble(12);
		Create0bit();
		CreateByte(0x37);
		Create0bit();
		CreateByte(0x74);
		Create0bit();
		CreateByte(0x43);
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance(8 * mBitLen0);

		// Create very short preamble
		CreatePreamble(2);
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance(1 * mBitLen0);

		// Create packet with checksum error
		CreatePreamble(14);
		Create0bit();
		CreateByte(0x37);
		Create0bit();
		CreateByte(0x74);
		Create0bit();
		CreateByte(0x53);
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance(8 * mBitLen0);

		// Create packet with differential timing error in first byte (only detectable in strict timing mode)
		// and out-of-spec errors in second byte
		CreatePreamble(14);
		Create0bit();
		// Manually write 0x37 with timing error in a bit
		Create0bit();
		Create0bit();
		Create1bit();
		Create1bit();
		Create0bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (58 - 2) / 1000000);	// -2 us off
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (58 + 2) / 1000000);	// +2 us off
		Create1bit();
		Create1bit();
		//
		Create0bit();
		// Manually write 0x74 with out-of-spec timings in both a 0 and a 1 bit
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (100) / 1000000);	// ok 0 half-bit
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (100 - 14) / 1000000);	// -14 us ( = 4 us less than relaxed timing permits)
		Create1bit();
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (58 + 8) / 1000000);	// +8 us ( = 2 us more than relaxed timing permits)
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (58 + 3) / 1000000);	// +3 us (ok)
		Create0bit();
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (100) / 1000000);	// ok 0 half-bit
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance((U64)mSimulationSampleRateHz * (230) / 1000000);	// ok 0 half-bit (stretched)
		Create0bit();
		//
		Create0bit();
		CreateByte(0x43);
		Create1bit();
		mSerialSimulationData.Transition();
		mSerialSimulationData.Advance(8 * mBitLen0);

	}

	*simulation_channel = &mSerialSimulationData;
	return 1;
}

void DCCSimulationDataGenerator::Create0bit()
{
	mSerialSimulationData.Transition();
	mSerialSimulationData.Advance(mBitLen0);
	mSerialSimulationData.Transition();
	mSerialSimulationData.Advance(mBitLen0);
}

void DCCSimulationDataGenerator::Create1bit()
{
	mSerialSimulationData.Transition();
	mSerialSimulationData.Advance(mBitLen1);
	mSerialSimulationData.Transition();
	mSerialSimulationData.Advance(mBitLen1);
}

void DCCSimulationDataGenerator::CreatePreamble(U32 len)
{
	for (U32 i=0; i<len; i++)
		Create1bit();
}

void DCCSimulationDataGenerator::CreateByte(U8 data)
{
	for(U32 i=0; i<8; i++)
	{
		if((data & 0x80 ) != 0)
			Create1bit();
		else
			Create0bit();
		data <<= 1;
	}
}
