#pragma once

#include <opencv2/opencv.hpp>
#include <GL/glew.h>
#include <stdexcept>
#include <memory>
#include <cmath>
#include "shader.hpp"

class Camera
{
public:
    Camera()
    {
        // open default camera
        if(!_cam.open(0, cv::CAP_ANY) || !_cam.isOpened())
            throw std::runtime_error("Failed to open camera!");
        // set properties
        _cam.set(cv::CAP_PROP_BUFFERSIZE, 1);
        _cam.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        _width = static_cast<int>(_cam.get(cv::CAP_PROP_FRAME_WIDTH));
        _height = static_cast<int>(_cam.get(cv::CAP_PROP_FRAME_HEIGHT));
        _ratio = static_cast<float>(_width) / _height;
        _groupX = static_cast<int>(std::ceil(_width / 32.0f));
        _groupY = static_cast<int>(std::ceil(_height / 32.0f));
        // init opengl texture
        glGenTextures(2, _tex);
        for(int i = 0; i < 2; i++)
        {
            glBindTexture(GL_TEXTURE_2D, _tex[i]);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, _width, _height);
            glBindTexture(GL_TEXTURE_2D, 0);
        }
        // try to get first frame
        if(!update())
            throw std::runtime_error("Failed to get frame from camera!");
        // init VAO
        const float vertices[] = {
            -1.0f, -1.0f,
             1.0f, -1.0f,
             1.0f,  1.0f,
            -1.0f,  1.0f,
        };
        GLuint vbo;
        glGenVertexArrays(1, &_vao);
        glGenBuffers(1, &vbo);
        glBindVertexArray(_vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)(0));
        glEnableVertexAttribArray(0);
        glBindVertexArray(0);
        // prepare denoise shader
        _denoise_shader = std::make_shared<Shader>();
        _denoise_shader->add("shaders/denoise.comp.glsl", GL_COMPUTE_SHADER);
        _denoise_shader->compile();
    }

    ~Camera()
    {
        glDeleteTextures(2, _tex);
        glDeleteVertexArrays(1, &_vao);
    }

    bool update()
    {
        bool updated = _cam.read(_frame);
        // if updated, update texture pixels
        if(updated)
        {
            cv::flip(_frame, _frame, 0);
            // cv::fastNlMeansDenoisingColored(_frame, _frame);
            glBindTexture(GL_TEXTURE_2D, fetchTex());
            glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, _width, _height, GL_BGR, GL_UNSIGNED_BYTE, _frame.data);
            glBindTexture(GL_TEXTURE_2D, 0);
            if(_denoise) denoise();
        }
        return updated;
    }

    void draw()
    {
        glBindVertexArray(_vao);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        glBindVertexArray(0);
    }

    void denoise()
    {
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glUseProgram(_denoise_shader->program());
        glBindImageTexture(0, lastTex(),  0, GL_FALSE, 0, GL_READ_ONLY,  GL_RGBA32F);
        glBindImageTexture(1, fetchTex(), 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        _denoise_shader->uniformFloat("coeff_sigma", _sigma);
        _denoise_shader->uniformFloat("coeff_kSigma", _kSigma);
        _denoise_shader->uniformFloat("coeff_threshold", _threshold);
        glDispatchCompute(
            static_cast<GLuint>(_groupX),
            static_cast<GLuint>(_groupY), 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
        glUseProgram(0);
    }

    // fetch texture and update index
    GLuint fetchTex()
    {
        GLuint tex = _tex[_currentTex];
        _currentTex = !_currentTex;
        return tex;
    }
    // only get current texture
    GLuint lastTex() {return _tex[!_currentTex];}
    GLuint vao() const {return _vao;}
    int width() const {return _width;}
    int height() const {return _height;}
    float ratio() const {return _ratio;}
    int groupX() const {return _groupX;}
    int groupY() const {return _groupY;}

    void UI();

private:
    int _width, _height, _groupX, _groupY;
    float _ratio;
    cv::VideoCapture _cam;
    cv::Mat _frame;
    GLuint _tex[2], _vao;
    int _currentTex = 0;
    std::shared_ptr<Shader> _denoise_shader;
    bool _denoise = false;
    float _sigma = 3.0f, _kSigma = 6.0f, _threshold = 0.1f;
};