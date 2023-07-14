#include "DCCAnalyzerResults.h"
#include <AnalyzerHelpers.h>
#include "DCCAnalyzer.h"
#include "DCCAnalyzerSettings.h"
#include <iostream>
#include <fstream>

DCCAnalyzerResults::DCCAnalyzerResults( DCCAnalyzer* analyzer, DCCAnalyzerSettings* settings )
:	AnalyzerResults(),
	mSettings( settings ),
	mAnalyzer( analyzer )
{
}

DCCAnalyzerResults::~DCCAnalyzerResults()
{
}

void DCCAnalyzerResults::GenerateBubbleText( U64 frame_index, Channel& channel, DisplayBase display_base )
{
	ClearResultStrings();
	Frame frame = GetFrame( frame_index );

	switch (frame.mData2)
	{
	case FT_Preamble:
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString( frame.mData1 & 0xFFFF, Decimal, 16, number_str, 128 );
			AddResultString( "P" );
			AddResultString( "Pre" );
			AddResultString( "Preamble" );
			AddResultString( "Preamble (", number_str, ")" );
			if (frame.mFlags)
				AddResultString( "Preamble (", number_str, ") too few" );
			break;
		}

	case FT_Data:
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString( frame.mData1 & 0xFF, display_base, 8, number_str, 128 );
			//AddResultString( "D" );
			AddResultString( number_str );
			break;
		}

	case FT_Checksum:
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString( frame.mData1 & 0xFF, display_base, 8, number_str, 128 );
			AddResultString( "C" );
			AddResultString( "Chk" );
			AddResultString( "Chk ", number_str );
			AddResultString( "Checksum ", number_str );
			if ((frame.mFlags & DISPLAY_AS_ERROR_FLAG) != 0)
			{
				char expected_str[128];
				AnalyzerHelpers::GetNumberString( (frame.mData1 >> 8) & 0xFF, display_base, 8, expected_str, 128 );
				AddResultString( "Checksum ", number_str, " expected ", expected_str );
			}
			break;
		}

	default:
		{
			AddResultString( "?" );
			AddResultString( "Analyzer error" );
			break;
		}
	}
}

void DCCAnalyzerResults::GenerateExportFile( const char* file, DisplayBase display_base, U32 export_type_user_id )
{
	std::ofstream file_stream( file, std::ios::out );

	U64 trigger_sample = mAnalyzer->GetTriggerSample();
	U32 sample_rate = mAnalyzer->GetSampleRate();

	file_stream << "Time [s],Value,Type" << std::endl;

	U64 num_frames = GetNumFrames();
	for( U32 i=0; i < num_frames; i++ )
	{
		Frame frame = GetFrame( i );
		
		char time_str[128];
		AnalyzerHelpers::GetTimeString( frame.mStartingSampleInclusive, trigger_sample, sample_rate, time_str, 128 );

		char number_str[128];
		AnalyzerHelpers::GetNumberString( frame.mData1 & 0xFF, display_base, 8, number_str, 128 );

		file_stream << time_str << "," << number_str << ",";

		switch (frame.mData2)
		{
		case FT_Preamble:
			file_stream << "Preamble" << std::endl;
			break;

		case FT_Data:
			file_stream << "Data" << std::endl;
			break;

		case FT_Checksum:
			file_stream << "Checksum" << std::endl;
			break;

		default:
			file_stream << "Analyzer error" << std::endl;
			break;
		}


		if( UpdateExportProgressAndCheckForCancel( i, num_frames ) == true )
		{
			file_stream.close();
			return;
		}
	}

	file_stream.close();
}

void DCCAnalyzerResults::GenerateFrameTabularText( U64 frame_index, DisplayBase display_base )
{
/*
	Frame frame = GetFrame( frame_index );
	ClearTabularText();

	char number_str[128];
	AnalyzerHelpers::GetNumberString( frame.mData1, display_base, 8, number_str, 128 );
	AddTabularText( number_str );
*/

	ClearTabularText();
	Frame frame = GetFrame(frame_index);

	switch (frame.mData2)
	{
	case FT_Preamble:
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xFFFF, Decimal, 16, number_str, 128);
			if (frame.mFlags)
				AddTabularText("Preamble (", number_str, ") too few");
			else
				AddTabularText("Preamble (", number_str, ")");
			break;
		}

	case FT_Data:
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xFF, display_base, 8, number_str, 128);
			AddTabularText("Data ", number_str);
			break;
		}

	case FT_Checksum:
		{
			char number_str[128];
			AnalyzerHelpers::GetNumberString(frame.mData1 & 0xFF, display_base, 8, number_str, 128);
			if ((frame.mFlags & DISPLAY_AS_ERROR_FLAG) != 0)
			{
				char expected_str[128];
				AnalyzerHelpers::GetNumberString((frame.mData1 >> 8) & 0xFF, display_base, 8, expected_str, 128);
				AddTabularText("Checksum error ", number_str, " expected ", expected_str);
			}
			else
			{
				AddTabularText("Checksum ", number_str);
			}
			break;
		}

	default:
		{
			AddTabularText("Analyzer error");
			break;
		}
	}
}

void DCCAnalyzerResults::GeneratePacketTabularText( U64 packet_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}

void DCCAnalyzerResults::GenerateTransactionTabularText( U64 transaction_id, DisplayBase display_base )
{
	ClearResultStrings();
	AddResultString( "not supported" );
}