#pragma once

#include <chrono>
#include <ctime>

namespace Utils 
{

	struct Timer
	{
		Timer() : m_startTime(std::chrono::high_resolution_clock::now()) {};

		void startTimer()
		{
			m_startTime = std::chrono::high_resolution_clock::now();
		}

		double stoptimer()
		{
			auto endTime = std::chrono::high_resolution_clock::now();
			auto ms = std::chrono::duration_cast<std::chrono::microseconds>(endTime - m_startTime);
			return ms.count() / 1000.0;
		}

		std::chrono::time_point<std::chrono::high_resolution_clock> m_startTime;
	};

}