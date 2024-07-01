#include <iostream>
#include <random>
#include "TrafficLight.h"

/* Implementation of class "MessageQueue" */


template <typename T>
T MessageQueue<T>::receive()
{
    // FP.5a : The method receive should use std::unique_lock<std::mutex> and _condition.wait() 
    // to wait for and receive new messages and pull them from the queue using move semantics. 
    // The received object should then be returned by the receive function.

    std::unique_lock<std::mutex> uLock(_mutex);
    _condition.wait(uLock, [this]
    {
        return !_queue.empty();
    });

    T msg = std::move(_queue.back());
    return msg;
}

template <typename T>
void MessageQueue<T>::send(T &&msg)
{
    // FP.4a : The method send should use the mechanisms std::lock_guard<std::mutex> 
    // as well as _condition.notify_one() to add a new message to the queue and afterwards send a notification.

    //Perform message queueing under the lock
    std::lock_guard<std::mutex> uLock(_mutex);

    // add message to the queue
    _queue.push_back(std::move(msg));

    //notify client after notifying about the TrafficLightPhase
    _condition.notify_one();
}

/* Implementation of class "TrafficLight" */


TrafficLight::TrafficLight()
{
    _currentPhase = TrafficLightPhase::red;
}

TrafficLight::~TrafficLight()
{
}

void TrafficLight::waitForGreen()
{
    // FP.5b : add the implementation of the method waitForGreen, in which an infinite while-loop 
    // runs and repeatedly calls the receive function on the message queue. 
    // Once it receives TrafficLightPhase::green, the method returns.
    while(true)
    {
        if (lightPhaseQueue.receive() == TrafficLightPhase::green)
        {
            return;
        }
    }
}

TrafficLightPhase TrafficLight::getCurrentPhase()
{
    return _currentPhase;
}

void TrafficLight::simulate()
{
    // FP.2b : Finally, the private method „cycleThroughPhases“ should be started in a thread when the public method „simulate“ is called.
    // To do this, use the thread queue in the base class.
    std::thread simulationThread(&TrafficLight::cycleThroughPhases, this);
    threads.push_back(std::move(simulationThread));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases()
{
    // FP.2a : Implement the function with an infinite loop that measures the time between two loop cycles 
    // and toggles the current phase of the traffic light between red and green and sends an update method 
    // to the message queue using move semantics. The cycle duration should be a random value between 4 and 6 seconds. 
    // Also, the while-loop should use std::this_thread::sleep_for to wait 1ms between two cycles.
    float min = 4;
    float max = 6;
    float cycleDuration = ((max - min) * ((float)rand() / RAND_MAX)) + min;
    while(true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        cycleDuration -= 0.001;
        if(cycleDuration <= 0)
        {
            switch (_currentPhase)
            {
                case TrafficLightPhase::red:
                    _currentPhase = TrafficLightPhase::green;
                    goto end;
                    break;
                case TrafficLightPhase::green:
                    _currentPhase = TrafficLightPhase::red;
                    goto end;
                    break;
                end:
                    lightPhaseQueue.send(std::move(_currentPhase));
                    cycleDuration = ((max - min) * ((float)rand() / RAND_MAX)) + min;
                    break;
                default:
                    break;
            }
        }
    }

}

