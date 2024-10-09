#pragma once
#include <vector>
#include <cmath>
#include <array>

std::vector<std::array<double, 3>> createHistogramDel(const std::vector<double>& inputDataset, double del)
{
    //xy데이터로 히스토그램을 만듭니다
    std::vector<std::array<double, 3>> newPoints;

    double minVal = *std::min_element(inputDataset.begin(), inputDataset.end());
    double maxVal = *std::max_element(inputDataset.begin(), inputDataset.end());
    int numBins = std::ceil((maxVal - minVal) / del); //구간의 숫자

    //std::printf("numBins : %d\n", numBins);

    newPoints.resize(numBins);
    for (int i = 0; i < numBins; i++)
    {
        newPoints[i] = { (double)i,0.0,0.0 };
    }

    for (size_t i = 0; i < inputDataset.size(); i++)
    {
        int binIndex = (int)((inputDataset[i] - minVal) / del);
        if (binIndex == numBins) binIndex--;

        if (binIndex < numBins && binIndex >= 0)
        {
            newPoints[binIndex][1] += 1;
        }
    }

    //std::printf("Histogram data (index, count, placeholder):\n");
    //for (const auto& point : newPoints) std::printf("Bin: %.2f, Count: %.2f, Placeholder: %.2f\n", point[0], point[1], point[2]);

    return newPoints;
}
std::vector<std::array<double, 3>> createHistogramSize(const std::vector<double>& inputDataset, int inputSize)
{
    double minVal = *std::min_element(inputDataset.begin(), inputDataset.end());
    double maxVal = *std::max_element(inputDataset.begin(), inputDataset.end());
    double del = (maxVal - minVal) / (double)inputSize;
    return createHistogramDel(inputDataset, del);
}