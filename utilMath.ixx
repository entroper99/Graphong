#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>

export module utilMath;

import std;

export void printRotationMatrix(Eigen::Matrix3d inputMat)
{
    {
        Eigen::Matrix3d checkMat = inputMat;
        double trace = checkMat.trace();
        double theta = std::acos((trace - 1) / 2);
        Eigen::Vector3d axis;
        axis << checkMat(2, 1) - checkMat(1, 2),
            checkMat(0, 2) - checkMat(2, 0),
            checkMat(1, 0) - checkMat(0, 1);
        if (axis.norm() != 0) axis.normalize();
        else axis << 1, 0, 0;
        std::cout << "회전각 : " << theta * 180.0 / M_PI << std::endl;
        std::cout << "회전축: (" << axis.x() << ", " << axis.y() << ", " << axis.z() << ")" << std::endl;
    }
}

export double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    double exponent = -(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma);
    return amplitude * exp(exponent);
}


