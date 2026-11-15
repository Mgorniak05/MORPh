#ifndef FILTERS_H
#define FILTERS_H

class DistanceFilter{

    private:
        static const int MED_SIZE = 5;
        static const int AVG_SIZE = 6;
        int rawDistance[MED_SIZE];
        int avgDistance[AVG_SIZE];
        int index0;
        int index1;
        int counter0;
        int counter1;
        int calculateMedian();
        int calculateAverage();
    
    public:
        DistanceFilter();
        void reset();
        int processDistance(int distance);
};
#endif