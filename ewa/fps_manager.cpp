#include "fps_manager.hpp"

#include "gl/gl_common.hpp"
#include "log.hpp"
#include <chrono>
#include <thread>

std::chrono::milliseconds oneMilliSecond(1);


FPSManager::FPSManager(const int maxFPS): m_maxFPS(maxFPS), m_targetFrameDuration(1.0f/m_maxFPS),m_frameStartTime(0),m_frameEndTime(0) {
}

float FPSManager::ManageFPS() {

    m_frameEndTime = (float)glfwGetTime();

    float frameDuration = m_frameEndTime - m_frameStartTime;

    if ((m_frameEndTime - m_lastReportTime) > 1.0f)
    {
	m_lastReportTime = m_frameEndTime;
	float currentFps =  (float)m_frameCount / 1.0f;
	m_frameCount = 1;

	fpsString = "fps: " + std::to_string(currentFps);
    } else {
	++m_frameCount;
    }

    const float sleepDuration = m_targetFrameDuration - frameDuration;

    if (sleepDuration > 0.0) {
	std::this_thread::sleep_for(std::chrono::milliseconds((int)sleepDuration));
    }

    m_frameStartTime = (float)glfwGetTime();

    return frameDuration + (m_frameStartTime - m_frameEndTime);
}

void FPSManager::Start() {
    m_frameStartTime = (float)glfwGetTime();
    m_lastReportTime = m_frameStartTime;
    m_frameCount = 0;
}

std::string FPSManager::GetFpsString() {
    return fpsString;
}
