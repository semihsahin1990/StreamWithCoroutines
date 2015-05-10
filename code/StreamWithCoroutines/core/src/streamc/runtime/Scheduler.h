#pragma once

#include "streamc/runtime/SchedulerPluginService.h"

#include <mutex>
#include <vector>
#include <unordered_map>
#include <unordered_set>

namespace streamc
{

class WorkerThread;
class OperatorContextImpl;
class FlowContext;
class InputPortImpl;
class SchedulerPlugin;
class FissionController;
class UtilityController;

class Scheduler : public SchedulerPluginService
{
public:
  Scheduler(FlowContext & flowContext, SchedulerPlugin * plugin);
  ~Scheduler();
  void addThread(WorkerThread & thread);
  //void removeThreads();
  void addOperatorContext(OperatorContextImpl & oper);
  void removeOperatorContext(OperatorContextImpl & oper);
  //void start();
  void stop();
  // return the operator to execute, nullptr if the thread should exit
  OperatorContextImpl * getThreadWork(WorkerThread & thread);
  void markOperatorAsCompleted(OperatorContextImpl & oper);
  void markOperatorAsReadBlocked(OperatorContextImpl & oper, 
  std::unordered_map<InputPortImpl *, size_t> const & waitSpec, bool conjunctive);  
  void markOperatorAsWriteBlocked(OperatorContextImpl & oper, 
  std::unordered_map<InputPortImpl *, size_t> const & waitSpec);  
  void markInputPortAsWritten(InputPortImpl & iport);
  void markInputPortAsRead(InputPortImpl & iport);
  void checkOperatorForPreemption(OperatorContextImpl & oper);
  OperatorInfo & getOperatorInfo(OperatorContextImpl *oper);
  void setFissionController(FissionController * fissionController) { fissionController_ = fissionController; };
  bool requestPartialBlock(OperatorContextImpl & oper);
  bool requestCompleteBlock(OperatorContextImpl & oper);
  void checkOperatorForBlocking(OperatorContextImpl & oper);
  void unblockOperators();
  void setUtilityController(UtilityController * utilityController) { utilityController_ = utilityController; };
  void requestThreadBlock();
  void markThreadCompleted(WorkerThread * thread);
  /**/
  bool checkOperator(OperatorContextImpl * oper) {
    return operContexts_.find(oper) != operContexts_.end();
  }
  /**/
private:
  void updateThreadState(WorkerThread & thread, ThreadInfo::ThreadState state);
  void updateOperatorState(OperatorContextImpl & oper, OperatorInfo::OperatorState state);
public: // interfaces for SchdulerPluginService
  std::unordered_map<WorkerThread *, ThreadInfo *> const & getThreads() const { return threads_; }
  std::unordered_map<OperatorContextImpl *, std::unique_ptr<OperatorInfo>> const & getOperators() const { return operContexts_; }
  std::unordered_set<WorkerThread *> const & getReadyThreads() const { return readyThreads_; }
  std::unordered_set<OperatorContextImpl *> const & getReadyOperators() const { return readyOperators_; }
private:
  bool stopped_;
  FlowContext & flowContext_;
  FissionController * fissionController_;
  UtilityController * utilityController_;
  std::vector<std::unique_ptr<ThreadInfo>> threadInfos_;
  //std::vector<std::unique_ptr<OperatorInfo>> operatorInfos_;
  std::unique_ptr<SchedulerPlugin> plugin_;
  // scheduler state, including indices
  std::unordered_map<WorkerThread *, ThreadInfo *> threads_;
  std::unordered_set<WorkerThread *> waitingThreads_;
  std::unordered_set<WorkerThread *> readyThreads_;
  std::unordered_map<OperatorContextImpl *, std::unique_ptr<OperatorInfo>> operContexts_;
  std::unordered_map<InputPortImpl *, OperatorContextImpl *> readBlockedOperators_;
  std::unordered_map<InputPortImpl *, std::unordered_set<OperatorContextImpl *>> writeBlockedOperators_;
  std::unordered_set<OperatorContextImpl *> readyOperators_;
  std::unordered_set<OperatorContextImpl *> partialBlockRequestedOperators_;
  std::unordered_set<OperatorContextImpl *> completeBlockRequestedOperators_;
  std::unordered_set<OperatorContextImpl *> outOfServiceOperators_;
  std::mutex mutex_;
  bool threadBlockRequested_;
};

} /* streamc */
