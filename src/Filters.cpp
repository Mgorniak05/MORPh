#include "Filters.h"
#include <algorithm>

DistanceFilter::DistanceFilter() {
    reset();
}

void DistanceFilter::reset(){
    index0 = 0;
    index1 = 0;
    counter0 = 0;
    counter1 = 0;
}

int DistanceFilter:: calculateMedian() {
    int temp[MED_SIZE];
    for(int i = 0 ; i < counter0; i++) {
        temp[i] = rawDistance[i];
    }
    std::sort(temp, temp + counter0);
    return temp[counter0 / 2];
}

int DistanceFilter::calculateAverage() {
    long sum = 0;
    for(int i = 0 ; i < counter1; i++) {
        sum += avgDistance[i];
    }
    return sum / counter1;
}

int DistanceFilter::processDistance(int distance) {
    rawDistance[index0] = distance;
    index0 = (index0 + 1) % MED_SIZE;
    if (counter0 < MED_SIZE) {
        counter0++;
    }
    int med = calculateMedian();

    avgDistance[index1] = med;
    index1 = (index1 + 1) % AVG_SIZE;
    if (counter1 < AVG_SIZE) {
        counter1++;
    }
    int avg = calculateAverage();
    return ((avg + 1) / 2) * 2;
}

