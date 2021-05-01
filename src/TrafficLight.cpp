#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{ 
    // perform vector modification under the lock
    std::unique_lock<std::mutex> uLock(_mutex);
    _cond.wait(uLock, [this] { return !_messages.empty(); }); 
    TrafficLightPhase phase = std::move(_messages.back());
    _messages.pop_back();
    return phase; 
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // perform vector modification under the lock
    std::lock_guard<std::mutex> uLock(_mutex);
    _messages.push_back(std::move(msg));
    _cond.notify_one();
}


/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

void TrafficLight::waitForGreen()
{
    while(true)
    {
        TrafficLightPhase phase = _messageQueue.receive();
        if(phase == TrafficLightPhase::green)
            break;
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    int cycle_duration = 4 + rand() % (6 -4);

    std::chrono::time_point<std::chrono::steady_clock> start = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> end;
    int diff = 0;
    
    while(true)
    {
        end = std::chrono::steady_clock::now();
        diff = std::chrono::duration_cast<std::chrono::seconds>(end - start).count();
        if(diff>=cycle_duration)
        {
            if(_currentPhase == TrafficLightPhase::red)
            {
                _currentPhase = TrafficLightPhase::green;
                _messageQueue.send(TrafficLightPhase::green);
            }
            else
            {
                _currentPhase = TrafficLightPhase::red;
                _messageQueue.send(TrafficLightPhase::red);
            }
            start = std::chrono::steady_clock::now();
            cycle_duration = 4 + rand() % (6 -4);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}

