#include "GPS.h"

int main() {
    std::string gpxFile = "input.gpx";
    std::string outFile = "analysis.txt";
    auto points = GPXParser::parse(gpxFile);
    EleAnalyzer* analyzer = new EleAnalyzer(points);
    TimeDistAnalyzer* analyzer2 = new TimeDistAnalyzer(points);
    AnalysisSaver f;
    f.adddAnalyzer(analyzer);
    f.adddAnalyzer(analyzer2);
    f.saveAnalysis(outFile);
    return 0;
}