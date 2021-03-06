#pragma once

#include "streamc/runtime/HashHelpers.h"
#include "streamc/runtime/OperatorContextImpl.h"

#include <chrono>
#include <condition_variable>
#include <unordered_set>
#include <unordered_map>
#include <iostream>

namespace streamc
{
class ThreadInfo
{
public:
  enum ThreadState { Running,  // running an operator
                     Ready,    // can run an operator (given scheduler assigns one)
                     Waiting,  // waiting for an operator to be available
                     Completed // no more work to do
  };
  ThreadInfo() {}
  ThreadInfo(WorkerThread * thread) : thread_(thread), state_(Ready), oper_(nullptr) {}
  void init() { state_ = Ready; oper_ = nullptr; }
  WorkerThread & getThread() { return *thread_; }
  ThreadState getState() { return state_; }
  void setState(ThreadState state) { state_ = state; }
  OperatorContextImpl & getOperator() { return *oper_; }
  void setOperator(OperatorContextImpl & oper) { oper_ = &oper; }
  std::condition_variable & getCV() { return cv_; }
private:
  WorkerThread * thread_; 
  ThreadState state_;
  OperatorContextImpl * oper_; // last operator executed by this thread
  std::condition_variable cv_;
};
  
class OperatorInfo
{
private:
  static constexpr double const decayFactor_ = 0.85; 
public:
  struct ThresholdAndCount
  {
    size_t threshold;
    size_t currentCount;
    ThresholdAndCount() {}
    ThresholdAndCount(size_t thresholdIn, size_t currentCountIn)
      : threshold(thresholdIn), currentCount(currentCountIn) {}
  };
  struct ProfileAndCounter
  {
    bool firstTime;
    double profile;
    size_t counter;
    ProfileAndCounter() { }
    ProfileAndCounter(double profileIn, size_t counterIn)
      : profile(profileIn), counter(counterIn) {}

    double getProfile() { return profile; }
    void updateProfile(double term) {
      if(firstTime) {
        profile = term;
        firstTime = false;
      }
      else {
        profile = decayFactor_ * profile + (1-decayFactor_) * term;
      }
    }
    size_t getCounter() { return counter; }
    void resetCounter() { counter = 0; }
    void incrementCounter() { counter++; }
  };

  class ReadWaitCondition
  {
  public:
    enum Kind { Conjunctive, Disjunctive }; 
    ReadWaitCondition() : kind_(Conjunctive) {}
    ReadWaitCondition(OperatorContextImpl & oper);
    void init();
    void setWaitThreshold(InputPortImpl & iport, size_t thresh);
    size_t getWaitThreshold(InputPortImpl & iport);
    void makeConjunctive() { kind_ = Conjunctive; }
    void makeDisjunctive() { kind_ = Disjunctive; }
    bool computeReadiness();
    bool isReady(InputPortImpl & iport);
    void reset(); 
    typedef std::unordered_map<InputPortImpl *, ThresholdAndCount> PortWaitList;
    PortWaitList const & getWaitList() { return portWaits_; }
  private:
    Kind kind_;
    OperatorContextImpl * oper_;
    PortWaitList portWaits_;
  };
  class WriteWaitCondition
  {
  public:
    WriteWaitCondition() {}
    WriteWaitCondition(OperatorContextImpl & oper);
    void init();
    void setWaitThreshold(InputPortImpl & iport, size_t thresh);
    size_t getWaitThreshold(InputPortImpl & iport);
    bool computeReadiness();
    bool isReady(InputPortImpl & iport);
    typedef std::unordered_map<InputPortImpl *, ThresholdAndCount> PortWaitList;
    PortWaitList const & getWaitList() { return portWaits_; }
  private:
    OperatorContextImpl * oper_;
    PortWaitList portWaits_;
  };
public:
  enum OperatorState { Running,      // running on a thread
                       Ready,        // can run on a thread (given scheduler assigns one)
                       ReadBlocked,  // waiting for data to be available at its input ports
                       WriteBlocked, // waiting for data to be available at its output ports
                       Completed     // no work to do
  };
  OperatorInfo() {}
  OperatorInfo(OperatorContextImpl & oper) : oper_(&oper), state_(Ready), thread_(nullptr) , 
                                             readCond_(oper), writeCond_(oper) {}
  void init() { state_=Ready; thread_ = nullptr; readCond_.init(); writeCond_.init();
              initProfileLists(); conjCount=0; disjCount=0; setEndTime(std::chrono::high_resolution_clock::now()); }
  OperatorContextImpl & getOperatorContext() { return *oper_; }
  OperatorState getState() { return state_; }
  void setState(OperatorState state) { state_ = state; }
  WorkerThread & getThread() { return *thread_; }
  void setThread(WorkerThread & thread) { thread_ = &thread; }
  ReadWaitCondition & getReadWaitCondition() { return readCond_; }
  WriteWaitCondition & getWriteWaitCondition() { return writeCond_; }
  void setBeginTime(std::chrono::high_resolution_clock::time_point beginTime) { beginTime_ = beginTime; }
  std::chrono::high_resolution_clock::time_point getBeginTime() { return beginTime_; }
  void setEndTime(std::chrono::high_resolution_clock::time_point endTime) { endTime_ = endTime; }
  std::chrono::high_resolution_clock::time_point getEndTime() { return endTime_; }

  void updateIPortProfile(InputPortImpl &iport) {
    size_t portCounter = iportProfileList_[&iport].getCounter();
    double timeElapsed = (double)(std::chrono::duration_cast<std::chrono::microseconds>(endTime_ - beginTime_).count());
    double term = portCounter / timeElapsed;
    iportProfileList_[&iport].updateProfile(term);
  }
  double getIPortProfile(InputPortImpl &iport) { return iportProfileList_[&iport].getProfile(); }
  void updateIPortCounter(InputPortImpl &iport) { iportProfileList_[&iport].incrementCounter(); }
  void resetIPortCounter(InputPortImpl &iport) { iportProfileList_[&iport].resetCounter(); }

  void updateOPortProfile(OutputPortImpl &oport) {
    size_t portCounter = oportProfileList_[&oport].getCounter();
    double timeElapsed = (double)(std::chrono::duration_cast<std::chrono::microseconds>(endTime_ - beginTime_).count());
    double term = portCounter / timeElapsed;
    oportProfileList_[&oport].updateProfile(term);
  }
  double getOPortProfile(OutputPortImpl &oport) { return oportProfileList_[&oport].getProfile(); }
  void updateOPortCounter(OutputPortImpl &oport) { oportProfileList_[&oport].incrementCounter(); }
  void resetOPortCounter(OutputPortImpl &oport) { oportProfileList_[&oport].resetCounter(); }

  void initProfileLists() {
    size_t nIPorts = oper_->getNumberOfInputPorts();
    for(int i=0; i<nIPorts; i++) {
      InputPortImpl &iport = oper_->getInputPortImpl(i);
      iportProfileList_[&iport] = ProfileAndCounter(0, 0);;
    }

    size_t nOports = oper_->getNumberOfOutputPorts();
    for(int i=0; i<nOports; i++) {
      OutputPortImpl &oport = oper_->getOutputPortImpl(i);
      oportProfileList_[&oport] = ProfileAndCounter(0, 0);
    }
  }

  void updateConjCount() { conjCount++; }
  void updateDisjCount() { disjCount++; }
  bool isConjuntive() { return conjCount > disjCount; }

private:
  size_t conjCount, disjCount;
  OperatorContextImpl * oper_;
  OperatorState state_;
  WorkerThread * thread_; // last thread that executed this operator
  std::chrono::high_resolution_clock::time_point beginTime_;
  std::chrono::high_resolution_clock::time_point endTime_;
  ReadWaitCondition readCond_;
  WriteWaitCondition writeCond_;
  std::unordered_map<InputPortImpl *, ProfileAndCounter> iportProfileList_;
  std::unordered_map<OutputPortImpl *, ProfileAndCounter> oportProfileList_;
};

} /* namespace streamc */
