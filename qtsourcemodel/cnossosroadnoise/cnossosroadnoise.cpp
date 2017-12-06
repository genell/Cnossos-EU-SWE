#include "cnossosroadnoise.h"
#include "../tinyxml/tinyxml.h"
//#include <boost/python.hpp>
#include <QDebug>


CnossosRoadNoise::CnossosRoadNoise()
{

}

// --------------------------------------------------------------------------------------------------------
/// <summary>
/// Reads a xml input file, calculates the RoadNoiseSegment source power and outputs the results the an xml output file
/// </summary>
/// <param name='infile'>filename of the xml input file</param>
/// <param name='outfile'>filename of the xml output file</param>
/// <returns>0 = OK, 1 = error while loading, 2 = error while calculating, 3 = error while saving, 4 = internal error</returns>
// --------------------------------------------------------------------------------------------------------
int CnossosRoadNoise::CalcFromFile(const string infile, const string outfile)
{
  int result = 0; // 0 = OK, 1 = error while loading, 2 = error while calculating, 3 = error while saving, 4 = internal error
  string tmp = getDllPath();
  cout << tmp << endl;
  if (currentSegment == NULL)
  {
    return 4;
  }
  else if (currentSegment->loadFromXMLFile(tmp + '/' + infile))
  {
    result = currentSegment->CalcSegment();
    if (!currentSegment->saveResultsToXMLFile(outfile))
    {
      cerr << "Error while trying to save " << outfile << endl;
      if (result == 0)
        result = 2;
    }

    if (currentSegment->doDebug)
    {
      string debugfile = outfile;
      debugfile = debugfile.substr(0, debugfile.find_last_of(".")) + ".csv";
      if (currentSegment->writeDebugData(debugfile))
      {
        cout << "Test file saved to " << debugfile << "." << endl;
      }

    }

    return result;
  }
  return 1;
}

// --------------------------------------------------------------------------------------------------------
    /// <summary>
    /// Get the current version of the Cnossos Road Source module shared library.
    /// </summary>
    /// <returns>String encoded version of the shared library</returns>
//    CNOSSOS_DLL_API char* GetVersionDLL (void)
//    {
//        return "1.00";
//    };

    // --------------------------------------------------------------------------------------------------------
    /// <summary>
    /// Release the dll
    ///
    /// Frees the created segment.
    /// </summary>
    /// <returns>0 if all went well, -1 if something went wrong</returns>
    int CnossosRoadNoise::ReleaseDLL( void )
    {
        try
        {
            if (currentSegment != NULL)
                delete currentSegment;

            return 0;
        }
        catch (...)
        {
            return -1;
        }

    }

    // --------------------------------------------------------------------------------------------------------
    /// <summary>
    /// Calculates the current roadsegment
    /// </summary>
    /// <returns>0 if all is OK, an error code if something went wrong.</returns>
    // --------------------------------------------------------------------------------------------------------

    int CnossosRoadNoise::CalcSegment(void)
    {
        if (currentSegment != NULL)
            return currentSegment->CalcSegment();
        else
            return -1;
    }


    // --------------------------------------------------------------------------------------------------------
    /// <summary>
    /// Set the properties for the temperature correection
    /// </summary>
    /// <param name='temperature'>The average yearly air temperature, in °C</param>
    // --------------------------------------------------------------------------------------------------------
    void CnossosRoadNoise::SetTemperatureProperties(const double temperature)
    {
        if (currentSegment != NULL)
        {
            currentSegment->TempProp = new TempProperties(temperature);
        }
    }


    // --------------------------------------------------------------------------------------------------------
    // Set the properties for the gradient correection
    //
    // Parameter :
    //				gradient	: The gradient of the current roadsegment in %
    //
    // --------------------------------------------------------------------------------------------------------
    void CnossosRoadNoise::SetGradientProperties(const double gradient)
    {
        if (currentSegment != NULL)
        {
            currentSegment->GradProp = new GradientProperties(gradient);
        }
    }

    // --------------------------------------------------------------------------------------------------------
    // Set the properties for the acceleration correction
    //
    // Parameter :
    //				distance	: distance in m to the junction
    //				junctionType : 1 = crossing with traffic ligths, 2 = roundabout
    //
    // --------------------------------------------------------------------------------------------------------
    void CnossosRoadNoise::SetAccelerationProperties(const double distance, const int junctionType)
    {
        if (currentSegment != NULL)
        {
            currentSegment->AccProp = new AccelerationProperties(distance, junctionType);

        }
    }

    // --------------------------------------------------------------------------------------------------------
    // Set the properties for the studded tyre correction
    //
    // Parameter :
    //				months		: amount of months a year studded tyres are used
    //
    // --------------------------------------------------------------------------------------------------------
    void CnossosRoadNoise::SetStuddedMonths(const int months)
    {
        if (currentSegment != NULL)
        {
            currentSegment->studdedMonths = months;
        }
    }

    // --------------------------------------------------------------------------------------------------------
    // Set the the speed for a specific category of vehicles
    //
    // Parameter :
    //				cat			: the category of vehicles ( 0 .. 5)
    //				speed		: the speed of the category of vehicles
    //
    // --------------------------------------------------------------------------------------------------------
    void CnossosRoadNoise::SetSpeed(const int cat, const double speed)
    {
        if (currentSegment != NULL)
        {
            currentSegment->v[cat] = speed;
        }
    }


    // --------------------------------------------------------------------------------------------------------
    // Set the the amount for a specific category of vehicles
    //
    // Parameter :
    //				cat			: the category of vehicles ( 0 .. 5)
    //				amount		: the amount of vehicles of category "cat" on the roadsegment
    //
    // --------------------------------------------------------------------------------------------------------
    void CnossosRoadNoise::SetTraffic(const int cat, const double amount)
    {
        if (currentSegment != NULL)
        {
            currentSegment->Q[cat] = amount;
        }
    }




    // --------------------------------------------------------------------------------------------------------
    string CnossosRoadNoise::getDllPath()
    {
//      char path[MAX_PATH];
//      HMODULE hm = NULL;
//      if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
//            GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
//            (LPCSTR) &getDllPath, &hm))
//      {
//        int ret = GetLastError();
//        fprintf(stderr, "GetModuleHandle returned %d\n", ret);
//      }
//      GetModuleFileNameA(hm, path, sizeof(path));
//      return path;
      return QCoreApplication::applicationDirPath().toStdString();
    }

    // --------------------------------------------------------------------------------------------------------
    /// <summary>
    /// Creates a new road segment and read the required settings and data files
    /// </summary>
    /// <returns>0 if finished correctly, -1 otherwise</returns>
    // --------------------------------------------------------------------------------------------------------
    int CnossosRoadNoise::InitDLL()
    {
        try
        {
            catalog = new RoadNoiseCatalog();

            string dllPath = getDllPath();
            dllPath = dllPath.substr(0, dllPath.find_last_of("\\/") + 1);
            catalog->LoadRoadParamsFromFile(dllPath + "cnossos_road_params.xml");
            catalog->LoadRoadSurfacesFromFile(dllPath + "cnossos_road_surfaces.xml");

            currentSegment = new RoadNoiseSegment(catalog);
        }
        catch (...)
        {
            return -1;
        }
        return 0;
    }


// --------------------------------------------------------------------------------------------------------
void CnossosRoadNoise::WriteToXMLFile(const string fn)
{
    if (currentSegment != NULL)
        currentSegment->saveResultsToXMLFile(fn);
}
