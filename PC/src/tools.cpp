#include "camera.hpp"
#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

void Camera::loadCameraData()
{
    const std::string filename = "camera.txt";
    std::ifstream inFile(filename.c_str());
    if(inFile.good())
    {
        float cameraCalibWidth = 1.0f, cameraCalibHeight = 1.0f;
        std::string line;
        std::stringstream sstr;
        float x = 0.0f;
        int y = 0;
        char dot = ',';

        // first line get measurement picture size
        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> y)) goto invaliddata;
        cameraCalibHeight = cameraCalibHeight / y;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> y)) goto invaliddata;
        cameraCalibWidth = cameraCalibWidth / y;

        // second line get camera matrix
        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto invaliddata;
        _camK[0][0] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[1][0] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[2][0] = x;

        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto invaliddata;
        _camK[0][1] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[1][1] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[2][1] = x;

        if(!std::getline(inFile, line)) goto invaliddata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto invaliddata;
        _camK[0][2] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[1][2] = x;
        if(!(sstr >> dot)) goto invaliddata;
        if(!(sstr >> x)) goto invaliddata;
        _camK[2][2] = x;

        // last line get distortion coefficients
        if(!std::getline(inFile, line)) goto validdata;
        sstr.clear();
        sstr.str(line);
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffK[0] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffK[1] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffP[0] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffP[1] = x;
        if(!(sstr >> dot)) goto validdata;
        if(!(sstr >> x)) goto validdata;
        _camDistCoeffK[2] = x;

        validdata:
        _camK[0][0] *= _width * cameraCalibWidth;
        _camK[2][0] *= _width * cameraCalibWidth;
        _camK[1][1] *= _height * cameraCalibHeight;
        _camK[2][1] *= _height * cameraCalibHeight;
        _camInvK = glm::inverse(_camK);
        inFile.close();
        return;
    }

    invaliddata:
    inFile.close();
    std::cout << "Warn: invalid camera.txt! Will use default parameters!" << std::endl;
    // camera intrinsic and distortion data measured on my webcam
    _camK = glm::mat3(
        glm::vec3(776.39107688f, 0.0f,          0.0f),
        glm::vec3(0.0f,          770.19292802f, 0.0f),
        glm::vec3(346.33206594f, 315.46822203f, 1.0f)
    );
    const float cameraCalibWidth = 1.0f / 800.0f, cameraCalibHeight = 1.0f / 600.0f;
    _camK[0][0] *= _width * cameraCalibWidth;
    _camK[2][0] *= _width * cameraCalibWidth;
    _camK[1][1] *= _height * cameraCalibHeight;
    _camK[2][1] *= _height * cameraCalibHeight;
    _camInvK = glm::inverse(_camK);
    _camDistCoeffK = glm::vec3(-3.58177671e-1,  5.58704262e-1, -8.93211088e-1);
    _camDistCoeffP = glm::vec2(8.46171348e-4, -4.12405134e-3);
}