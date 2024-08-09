#include "TaskStats.hpp"
#include "FreeRTOS.h"
#include "pico/time.h"
#include "task.h"
#include <string>

TaskStats::TaskStats(TaskHandle_t taskHandle, const char *taskName)
{
	this->taskHandle = taskHandle;
	this->taskName = taskName;
}

void TaskStats::MarkSwitchedIn()
{
	mySwitchedInTime[myIndex] = time_us_64();

	if (myIndex >= mySwitchedInTime.size())
	{
		myIndex = 0;
	}
}

void TaskStats::MarkSwitchedOut()
{
	mySwitchedOutTime[myIndex] = time_us_64();

	if (myIndex >= mySwitchedOutTime.size())
	{
		myIndex = 0;
	}

	myIndex++;

	if (myIndex >= mySwitchedInTime.size())
	{
		myIndex = 0;

		uint64_t totalRunTime = 0;
		for (int i = 0; i < mySwitchedInTime.size(); i++)
		{
			totalRunTime += (mySwitchedOutTime[i] - mySwitchedInTime[i]);
		}
		myAverageRunTime = totalRunTime / mySwitchedInTime.size();
	}
}

TaskStatsManager *TaskStatsManager::myInstance = nullptr;

TaskStatsManager::TaskStatsManager()
{
	myInstance = this;
}

TaskStatsManager *TaskStatsManager::GetInstance()
{
	if (myInstance == nullptr)
	{
		myInstance = new TaskStatsManager();
	}
	return myInstance;
}

void TaskStatsManager::MarkTaskSwitchedIn(TaskHandle_t taskHandle)
{
	auto taskStatIterator = myTaskStatsMap.find(taskHandle);
	if (taskStatIterator == myTaskStatsMap.end())
	{
		myTaskStatsMap.emplace(taskHandle, TaskStats(taskHandle, pcTaskGetName(taskHandle)));
	}
	else
	{
		taskStatIterator->second.MarkSwitchedIn();
	}
}

void TaskStatsManager::MarkTaskSwitchedOut(TaskHandle_t taskHandle)
{
	auto taskStatIterator = myTaskStatsMap.find(taskHandle);
	if (taskStatIterator != myTaskStatsMap.end())
	{
		taskStatIterator->second.MarkSwitchedOut();
	}
	else
	{
		panic("Task not found in TaskStatsManager, switched out might have been called before switched in!!");
	}
}

std::string TaskStatsManager::GetTaskAvgRuntime()
{
	std::string result = "";
	for (auto &taskStats : myTaskStatsMap)
	{
		result += taskStats.second.GetName();
		result += ": ";
		result += std::to_string(taskStats.second.GetAverageRunTime());
		result += "us";
		result += "\n";
	}
	return result;
}

std::string TaskStatsManager::GetHeapHighWaterMark()
{
	size_t heapHighWaterMark = xPortGetMinimumEverFreeHeapSize();
	return "Minimum ever free heap size: " + std::to_string(heapHighWaterMark) + " bytes\n";
}

std::string TaskStatsManager::GetTasksStackHighWaterMark()
{
	std::string result = "";
	for (auto &taskStats : myTaskStatsMap)
	{
		UBaseType_t stackHighWaterMark = uxTaskGetStackHighWaterMark(taskStats.second.GetHandle());
		result += taskStats.second.GetName();
		result += ": ";
		result += std::to_string(stackHighWaterMark);
		result += " words\n";
	}
	return result;
}

/******* the functions defined in FreeRTOS_config.h *******/

void vTaskSwitchedIn(void)
{
	TaskHandle_t xTask;
	char *pcTaskName;

	xTask = xTaskGetCurrentTaskHandle();
	pcTaskName = pcTaskGetName(xTask);

	if (pcTaskName != nullptr && strcmp(pcTaskName, "Tmr Svc") != 0)
	{
		TaskStatsManager::GetInstance()->MarkTaskSwitchedIn(xTask);
	}
}

void vTaskSwitchedOut(void)
{
	TaskHandle_t xTask;
	char *pcTaskName;

	xTask = xTaskGetCurrentTaskHandle();
	pcTaskName = pcTaskGetName(xTask);

	if (pcTaskName != nullptr && strcmp(pcTaskName, "Tmr Svc") != 0)
	{
		TaskStatsManager::GetInstance()->MarkTaskSwitchedIn(xTask);
	}
}