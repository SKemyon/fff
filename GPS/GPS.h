#define _USE_MATH_DEFINES
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <cmath>
#include <ctime>
#include <iomanip>
#include <map>
#include <optional>
#include <unordered_map>


struct TrackPoint {
    double lat, lon, ele;
    std::time_t time;
};

class GPXParser {
public:
    GPXParser() = delete;
    static std::vector<TrackPoint> parse(const std::string& filename);
private:
    static double takeDouble(const std::string& line, const std::string& attr);
    static double takeTagDouble(const std::string& line, const std::string& tag);
    static std::string takeTagString(const std::string& line, const std::string& tag);
    static std::time_t parseTime(const std::string& timestr);
};

struct FinalAnalyzis {
    FinalAnalyzis(std::optional<double> totalDist, std::optional<double> totalTm,
        std::optional<double> movingTm, std::optional<double> maxSpd, std::optional<double> avgSpd,
        std::optional<double> avgMovingSpd, const std::optional<std::unordered_map<int, double >>& speedDistr,
        std::optional<double> mele, std::optional<double> maxele, std::optional<double> elegain, std::optional<double> eleloss);//const, unordered_map


    std::optional<double> minEle, maxEle, elevationGain, elevationLoss;
    std::optional<double> totalDistance, totalTime, movingTime, maxSpeed, avgSpeed, avgMovingSpeed;
    std::optional<std::unordered_map<int, double>> speedDistribution;
};




class TrackAnalyzer {
public:
    TrackAnalyzer(const std::vector<TrackPoint>& pts);
    virtual FinalAnalyzis Analyze(double stopSpeed = 1.0, int speedBin = 5) = 0;
    virtual ~TrackAnalyzer() {};

protected:
    const std::vector<TrackPoint>& points;

    static double deg2rad(double deg);

    static double haversine(const TrackPoint& a, const TrackPoint& b);
};

class EleAnalyzer final:public TrackAnalyzer {
public:
    EleAnalyzer(const std::vector<TrackPoint>& pts);

    FinalAnalyzis Analyze(double stopSpeed = 1.0, int speedBin = 5) override;
};



class TimeDistAnalyzer final:public TrackAnalyzer{
public:
    TimeDistAnalyzer(const std::vector<TrackPoint>& pts);

    FinalAnalyzis Analyze(double stopSpeed = 1.0, int speedBin = 5) override;

};



class AnalysisSaver {
    std::vector<TrackAnalyzer*> Analyzers;
public:
    void adddAnalyzer(TrackAnalyzer* A) {
        Analyzers.push_back(A);
    }
    void saveAnalysis(const std::string& outFile);
};