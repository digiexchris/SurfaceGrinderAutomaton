#pragma once

#include <FreeRTOS.h>
#include <array>
#include <cstring>
#include <string>
#include <task.h>
#include <unordered_map>

class TaskStats
{
public:
	TaskStats(TaskHandle_t taskHandle, const char *taskName);
	std::string GetName() { return taskName; }
	TaskHandle_t GetHandle() { return taskHandle; }
	void MarkSwitchedIn();

	void MarkSwitchedOut();

	uint64_t GetAverageRunTime() { return myAverageRunTime; }

private:
	TaskHandle_t taskHandle;
	const char *taskName;
	uint8_t myIndex = 0;
	std::array<uint32_t, 10> mySwitchedInTime = {0};
	std::array<uint32_t, 10> mySwitchedOutTime = {0};
	uint64_t myAverageRunTime = 0;
};

using TaskStatsMap = std::unordered_map<TaskHandle_t, TaskStats>;

class TaskStatsManager
{
public:
	TaskStatsManager();
	static TaskStatsManager *GetInstance();
	void MarkTaskSwitchedIn(TaskHandle_t taskHandle);
	void MarkTaskSwitchedOut(TaskHandle_t taskHandle);
	std::string GetTaskAvgRuntime();
	std::string GetHeapHighWaterMark();
	std::string GetTasksStackHighWaterMark();

private:
	TaskStatsMap myTaskStatsMap;
	static TaskStatsManager *myInstance;
};

extern "C" void vTaskSwitchedIn(void);
extern "C" void vTaskSwitchedOut(void);
