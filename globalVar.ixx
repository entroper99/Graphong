export module globalVar;

import std;
import Shapes;

// 카메라 변수
export float camX = 0.0f;
export float camY = 0.0f;
export float camZ = 5.0f;
export float camYaw = 0.0f;
export float camPitch = 0.0f;
export float mouseSensitivity = 0.1f;
export float camSpd = 0.01f;

export bool camFixX = false;
export bool camFixMinusX = false;
export bool camFixY = false;
export bool camFixMinusY = false;
export bool camFixZ = false;
export bool camFixMinusZ = false;

export bool dim2 = true;
export bool dim3 = true;

export float xScale = 1;
export float yScale = 1;
export float zScale = 1.0;

export bool visDataPoint = true;
export bool visInterPoint = true;
export bool visUI = true;
export bool darkMode = true;

export bool interLine = false;
export bool crosshair = false;

export float pointSize = 3.0;

export std::string xAxisName, yAxisName, zAxisName;
export std::wstring xScaleUnit, yScaleUnit, zScaleUnit;

export std::vector<void*> funcSet;

export std::wstring graphName = L"NO TITLE";

export bool hasFourierRef = false;

export std::vector<std::pair<Point, std::vector<std::array<std::complex<double>, 4>>>> fourierAngleSave;
export std::vector<std::pair<Point, std::vector<std::array<std::complex<double>, 4>>>> fourierTransSave;

export std::vector<std::pair<Point, std::vector<std::array<std::complex<double>, 4>>>> fourierTransSaveX;
export std::vector<std::pair<Point, std::vector<std::array<std::complex<double>, 4>>>> fourierAngleSaveX;
