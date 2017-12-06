#include <QCoreApplication>
#include <QDebug>
#include <iostream>
#include "commandlineparser.h"
#include "../cnossosroadnoise/cnossosroadnoise.h"

void show_usage(QString program_name)
{
  std::cout << "Usage:" << std::endl;
  std::cout << program_name.toStdString() << " <-road | -rail | -industry> infile outfile" << std::endl;
}

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);

  CommandLineParser parser;
  parser.addOption(CommandLineOption("-road", "description", "value_name", "default_value"));
  //  parser.addOption(CommandLineOption("-rail", "description", "value_name", "default_value"));
  //  parser.addOption(CommandLineOption("-industry", "description", "value_name", "default_value"));

  if (!parser.parse(argc, argv)) {
    std::cout << "Error while parsing command line: " << parser.getParseError().toStdString() << std::endl;
    //std::cout << parser.getHelpText().toStdString() << std::endl;
    show_usage(a.applicationName());
    exit(EXIT_FAILURE);
  }

  {
    if (parser.isSet("-road") && parser.getValuesAsVariants("-road").count() == 2)
    {
      CnossosRoadNoise roadnoise;

      if(roadnoise.InitDLL() >= 0) {

        std::cout << "Starting CNOSSOS road noise calculation" << std::endl;

        QString infile = parser.getValuesAsVariants("-road").at(0).toString();
        QString outfile = parser.getValuesAsVariants("-road").at(1).toString();

        int result = roadnoise.CalcFromFile(infile.toStdString(),outfile.toStdString());
        roadnoise.ReleaseDLL();
        if (result == 0)
        {
          std::cout << "Calculation of file " << infile.toStdString() << " is done" << std::endl;
          std::cout << "Result was saved in file " << outfile.toStdString() << "." << std::endl;
        }
        return result;
      }
      else
      {
        cerr << "Failed to initialize DLL" << endl;
        return 1;
      }
    }
    // TODO: Rail and industry
    else
    {
      show_usage(a.applicationName());
    }
  }
  return a.exec();
}


