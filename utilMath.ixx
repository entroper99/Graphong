#define _SILENCE_ALL_CXX23_DEPRECATION_WARNINGS
#include <SDL.h>
#include <Eigen/Dense>
#include <Eigen/Core>
#include <unsupported/Eigen/CXX11/Tensor>
#include <fftw3.h>

export module utilMath;

import std;
import constVar;

export void printRotationMatrix(Eigen::Matrix3d inputMat)
{
    Eigen::Matrix3d checkMat = inputMat;
    double sy = std::sqrt(checkMat(0, 0) * checkMat(0, 0) + checkMat(1, 0) * checkMat(1, 0));
    bool isSingular = sy < 1e-6;
    double x, y, z;
    if (!isSingular)
    {
        x = std::atan2(checkMat(2, 1), checkMat(2, 2));
        y = std::atan2(-checkMat(2, 0), sy);
        z = std::atan2(checkMat(1, 0), checkMat(0, 0));
    }
    else
    {
        x = std::atan2(-checkMat(1, 2), checkMat(1, 1));
        y = std::atan2(-checkMat(2, 0), sy);
        z = 0;
    }

    x *= 180.0 / M_PI;
    y *= 180.0 / M_PI;
    z *= 180.0 / M_PI;

    std::cout << "x�� ȸ��: " << x << "��" << std::endl;
    std::cout << "y�� ȸ��: " << y << "��" << std::endl;
    std::cout << "z�� ȸ��: " << z << "��" << std::endl;
}

export double calcGaussian(double dx, double dy, double dz, double sigma, double amplitude)
{
    return amplitude * exp(-(dx * dx + dy * dy + dz * dz) / (2 * sigma * sigma));
}


export Eigen::Matrix3d angleToMatrix(double xAngle, double yAngle, double zAngle)
{
    double xRad = xAngle * DEGREE_TO_RADIAN; //x�� ȸ��
    double yRad = yAngle * DEGREE_TO_RADIAN; //y�� ȸ��
    double zRad = zAngle * DEGREE_TO_RADIAN; //z�� ȸ��

    Eigen::Matrix3d rotX, rotY, rotZ;
    rotX << 1, 0, 0,
        0, cos(xRad), -sin(xRad),
        0, sin(xRad), cos(xRad);

    rotY << cos(yRad), 0, sin(yRad),
        0, 1, 0,
        -sin(yRad), 0, cos(yRad);

    rotZ << cos(zRad), -sin(zRad), 0,
        sin(zRad), cos(zRad), 0,
        0, 0, 1;

    return rotZ * rotY * rotX;
}
