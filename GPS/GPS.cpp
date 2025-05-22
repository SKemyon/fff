#include "GPS.h"

std::vector<TrackPoint> GPXParser::parse(const std::string& filename) {
        std::vector<TrackPoint> points;
        std::ifstream file(filename);
        std::string line;
        TrackPoint point;
        bool inTrkpt = false;
        while (std::getline(file, line)) {
            if (line.find("<trkpt") != std::string::npos) {
                inTrkpt = true;
                point.lat = takeDouble(line, "lat");
                point.lon = takeDouble(line, "lon");
            }
            if (inTrkpt && line.find("<ele>") != std::string::npos)
                point.ele = takeTagDouble(line, "ele");
            if (inTrkpt && line.find("<time>") != std::string::npos)
                point.time = parseTime(takeTagString(line, "time"));
            if (line.find("</trkpt>") != std::string::npos) {
                points.push_back(point);
                inTrkpt = false;
            }
        }
        return points;
    }
 double GPXParser::takeDouble(const std::string& line, const std::string& attr) {
        auto pos = line.find(attr + "=\"");
        if (pos == std::string::npos) return 0.0;
        pos += attr.length() + 2;
        auto end = line.find("\"", pos);
        return std::stod(line.substr(pos, end - pos));
    }
double GPXParser::takeTagDouble(const std::string& line, const std::string& tag) {
        auto val = takeTagString(line, tag);
        return std::stod(val);
    }
std::string GPXParser::takeTagString(const std::string& line, const std::string& tag) {
        auto start = line.find("<" + tag + ">");
        if (start == std::string::npos) return "";
        start += tag.length() + 2;
        auto end = line.find("</" + tag + ">", start);
        return line.substr(start, end - start);
    }
std::time_t GPXParser::parseTime(const std::string& timestr) {
        std::tm t = {};
        std::istringstream ss(timestr);
        ss >> std::get_time(&t, "%Y-%m-%dT%H:%M:%SZ");
        return mktime(&t);
    }





FinalAnalyzis::FinalAnalyzis(std::optional<double> totalDist, std::optional<double> totalTm, 
    std::optional<double> movingTm, std::optional<double> maxSpd, std::optional<double> avgSpd,
    std::optional<double> avgMovingSpd, const std::optional<std::unordered_map<int, double >>& speedDistr, 
    std::optional<double> mele, std::optional<double> maxele, std::optional<double> elegain, std::optional<double> eleloss) {
        totalDistance = totalDist;
        totalTime = totalTm;
        movingTime = movingTm;
        maxSpeed = maxSpd;
        avgSpeed = avgSpd;
        avgMovingSpeed = avgMovingSpd;
        speedDistribution = speedDistr;
        minEle = mele;
        maxEle = maxele;
        elevationGain = elegain;
        elevationLoss = eleloss;
    }



TrackAnalyzer::TrackAnalyzer(const std::vector<TrackPoint>& pts): points(pts) {}


double TrackAnalyzer::deg2rad(double deg) { return deg * M_PI / 180.0; }

double TrackAnalyzer::haversine(const TrackPoint& a, const TrackPoint& b) {
        static const double R = 6371.0;
        double dLat = deg2rad(b.lat - a.lat);
        double dLon = deg2rad(b.lon - a.lon);
        double lat1 = deg2rad(a.lat);
        double lat2 = deg2rad(b.lat);
        double h = pow(sin(dLat / 2), 2) + cos(lat1) * cos(lat2) * pow(sin(dLon / 2), 2);
        return 2 * R * asin(sqrt(h));
    }


EleAnalyzer::EleAnalyzer(const std::vector<TrackPoint>& pts): TrackAnalyzer(pts){}

FinalAnalyzis EleAnalyzer::Analyze(double stopSpeed, int speedBin){
        double minEle = 0;
        double maxEle = 0;
        double elevationGain = 0;
        double elevationLoss = 0;
        if (points.size() < 2) {
            throw std::runtime_error("error: Wrong type");
        }
        double prevEle = points[0].ele;
        double prevLat = points[0].lat, prevLon = points[0].lon;
        for (size_t i = 1; i < points.size(); ++i) {
            if (points[i].ele > maxEle) maxEle = points[i].ele;
            if (points[i].ele < minEle) minEle = points[i].ele;
            double deltaEle = points[i].ele - prevEle;
            if (deltaEle > 0) elevationGain += deltaEle;
            else elevationLoss -= deltaEle;
            prevEle = points[i].ele;
        }
        
        return (FinalAnalyzis(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, minEle, maxEle, elevationGain, elevationLoss));
    }




 TimeDistAnalyzer::TimeDistAnalyzer(const std::vector<TrackPoint>& pts): TrackAnalyzer(pts){
    }

 FinalAnalyzis TimeDistAnalyzer::Analyze(double stopSpeed, int speedBin){
        double totalDistance = 0;
        double totalTime = 0;
        double movingTime = 0;
        double maxSpeed = 0; 
        double avgSpeed = 0; 
        double avgMovingSpeed = 0;
        std::unordered_map<int, double> speedDistribution;
        if (points.size() < 2) {
            throw std::runtime_error("error: Wrong type");
        }
        std::time_t prevTime = points[0].time;
        double prevLat = points[0].lat, prevLon = points[0].lon;
        bool moving = false;
        double segmentDist = 0;
        std::time_t segmentTime = 0;
        for (size_t i = 1; i < points.size(); ++i) {
            double dist = haversine(points[i - 1], points[i]);
            double dt = difftime(points[i].time, points[i - 1].time);
            if (dt <= 0) continue;
            totalDistance += dist;
            totalTime += dt;
            double speed = dist / (dt / 3600.0); // km/h
            speedDistribution[int(speed) / speedBin * speedBin] += dt;
            if (speed > stopSpeed) {
                movingTime += dt;
                if (speed > maxSpeed) maxSpeed = speed;
            }
        }
        avgSpeed = totalDistance / (totalTime / 3600.0);
        avgMovingSpeed = totalDistance / (movingTime / 3600.0);

        return(FinalAnalyzis(totalDistance, totalTime, movingTime, maxSpeed, avgSpeed, avgMovingSpeed, speedDistribution, std::nullopt, std::nullopt, std::nullopt, std::nullopt));
    }







 void AnalysisSaver::saveAnalysis(const std::string& outFile) {
     std::ofstream out(outFile);

     for (TrackAnalyzer* a : Analyzers) {
         FinalAnalyzis analyzer = a->Analyze();
         if (analyzer.minEle) {
             out << "Минимальная высота: " << *analyzer.minEle << "\n";
         }

         if (analyzer.maxEle) {
             out << "Максимальная высота: " << *analyzer.maxEle << "\n";
         }

         if (analyzer.elevationGain) {
             out << "Общий набор высоты: " << *analyzer.elevationGain << "\n";
         }
         
         if (analyzer.elevationLoss) {
             out << "Общий спуск: " << *analyzer.elevationLoss << "\n";
         }

         if (analyzer.totalTime) {
             out << "Общая продолжительность: " << *analyzer.totalTime << "\n";
         }

         if (analyzer.totalDistance) {
             out << "Общее расстояние: " << *analyzer.totalDistance << "\n";
         }

         if (analyzer.avgSpeed) {
             out << "Средняя скорость: " << *analyzer.avgSpeed << "\n";
         }

         if (analyzer.movingTime) {
             out << "Время движения: " << *analyzer.movingTime << "\n";
         }

         if (analyzer.movingTime) {
             out << "Время стоянок: " << (*analyzer.totalTime - *analyzer.movingTime) << "\n";
         }

         if (analyzer.avgMovingSpeed) {
             out << "Средняя скорость движения: " << *analyzer.avgMovingSpeed << "\n";
         }

         if (analyzer.maxSpeed) {
             out << "Максимальная скорость: " << *analyzer.maxSpeed << "\n";
         }

         if (analyzer.speedDistribution) {
             out << "Распределение скоростей:\n";
             for (const auto& kv : *analyzer.speedDistribution) {
                 out << kv.first << "-" << kv.first + 5 - 1 << ", "
                     << kv.second << "\n";
             }
         }          
          
     }
          
        out.close();
    }


