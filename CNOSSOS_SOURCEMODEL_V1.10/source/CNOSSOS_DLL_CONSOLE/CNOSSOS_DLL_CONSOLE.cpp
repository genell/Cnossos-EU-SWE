#pragma once
// CNOSSOS_DLL_CONSOLE.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#ifdef WIN32
#include <tchar.h>
#endif
#include <string>
#include <sstream>

#include "../CNOSSOS_ROADNOISE_DLL/CNOSSOS_ROADNOISE_DLL.h"
#include "../CNOSSOS_INDUSTRIAL_NOISE_DLL/CNOSSOS_INDUSTRIAL_NOISE_DLL.h"
#include "../CNOSSOS_RAILNOISE_DLL/CNOSSOS_RAILNOISE_DLL.h"

using namespace std;

void show_usage(string program_name)
{
	cout << "Usage:" << endl;
	cout << program_name << " <-road | -rail | -industry> infile outfile" << endl;
}

int main(int argc, char** argv)
{
	if (argc == 4)
	{
		string t = argv[1];		
		string infile = argv[2];
		string outfile = argv[3];
		// ----------------------------------------------------------------------------------------
		if (t.compare("-road") == 0)
		{
			cout << "Starting CNOSSOS road noise calculation" << endl;
			if (CNOSSOS_ROADNOISE::InitDLL() >= 0)
			{
				int result = CNOSSOS_ROADNOISE::CalcFromFile(infile,outfile);
				CNOSSOS_ROADNOISE::ReleaseDLL();
				if (result == 0)
				{
					cout << "Calculation of file " << infile << " is done" << endl;
					cout << "Result was saved in file " << outfile << "." << endl;
				}
				return result;
			}
			else
			{
				cerr << "Failed to initialize DLL" << endl;
				return 1;
			}
		}
		// ----------------------------------------------------------------------------------------
		else if (t.compare("-industry") == 0)
		{
			
			cout << "Starting CNOSSOS industrial noise calculation" << endl;
			if (CNOSSOS_INDUSTRIAL_NOISE::InitDLL() >= 0)
			{
				int result = CNOSSOS_INDUSTRIAL_NOISE::CalcFromFile(infile,outfile);
				CNOSSOS_INDUSTRIAL_NOISE::ReleaseDLL();
				if (result == 0)
				{
					cout << "Calculation of file " << infile << " is done" << endl;
					cout << "Result was saved in file " << outfile << "." << endl;
				}
				return result;
			}
			else
			{
				cerr << "Failed to initialize DLL" << endl;
				return 1;
			}

			
		}
		// ----------------------------------------------------------------------------------------
		else if (t.compare("-rail") == 0)
		{
			
			cout << "Starting CNOSSOS rail noise calculation" << endl;
			if (CNOSSOS_RAILNOISE::InitDLL() >= 0)
			{

				int result = CNOSSOS_RAILNOISE::CalcFromFile(infile,outfile);
				CNOSSOS_RAILNOISE::ReleaseDLL();
				if (result == 0)
				{
					cout << "Calculation of file " << infile << " done" << endl;
					cout << "Result was saved in file " << outfile << "." << endl;
				}
				return result;
			}
			else
			{
				cerr << "Failed to initialize DLL" << endl;
				return 1;
			}
			
		}
		else
		{
			show_usage(argv[0]);
		}
	}
	else
	{
		show_usage(argv[0]);
	}

}


