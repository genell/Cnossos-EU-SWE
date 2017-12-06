#include "stdafx.h"
#include "CNOSSOS_RAILNOISE_DLL_CONST.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;

namespace CNOSSOS_RAILNOISE
{	

	void PrintDoubleDataTable(ofstream& fs, const double Table[SRC_HEIGHT][MAX_FREQ_BAND], string name)
	{
		fs << endl << name.c_str() << endl;
		fs << "\t\t";
		for (int j=0; j < MAX_FREQ_BAND; j++)
			fs <<  setw(8) << FreqBands[j] << "\t";	
		fs << endl << endl;


		for (int i=0; i < 5; i++)
		{
			fs << "Cat" << i << "\t";
			for (int j=0; j < MAX_FREQ_BAND; j++)
			{
				fs <<  setw(8) << Table[i][j] << "\t";
			}
			fs << endl;
		}
	}

}