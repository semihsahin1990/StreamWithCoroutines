#pragma once

#include <inttypes.h>

namespace streamc
{

class Tuple;

/**
 * Input port that provides tuple receiving services to operators.
 *
 * These services include accessing input tuples, removing them from
 * the input and waiting for availability of tuples.
 */
class InputPort
{
public:	
  virtual ~InputPort() {}

  /**
   * Check whether the input port is closed.
   *
   * An input port is closed if it cannot receive any future tuples. It may still
   * have tuples in its queue, that may be consumed.
   * @return <code>true</code> if the port is complete, <code>false</code> otherwise.
   */
  virtual bool isClosed() = 0;

  /**
   * Check whether the input port has a tuple.
   *
   * @return <code>true</code> if the port has a tuple, <code>false</code> otherwise.
   */
  virtual bool hasTuple() = 0;

  /**
   * Get the current number of tuples queued in the input port.
   *
   * @return the current number of tuples queued in the input port
   */
  virtual size_t getTupleCount() = 0;

  /**
   * Wait until a tuple is available.
   *
   * This function will block until a tuple is available in the input port
   * queue. It will return true, if the port is closed and no more
   * tuples can be received from it. A typical use is as follows:
   * @code{.cpp}
     bool closed = iport.waitTuple();
     if (closed) 
         break;
     Tuple & tuple = iport.getFrontTuple();    
     // do something with the tuple
     @endcode
   * @return true if a tuple will never be available, false otherwise
   */
  virtual bool waitTuple() = 0;

  /**
   * Wait until a given number of tuples are available.
   *
   * This function will block until a given number of tuples are available in
   * the input port queue. It will return true, if the port is closed, no more
   * tuples can be received from it, and the current number of available tuples
   * are still less than the desired. Even if the required tuples are avaiabe,
   * this function can block if the system decides to preempt the operator.
   *
   * A typical use is as follows:
   * 
   * @code{.cpp}
     bool closed = iport.waitTuple(n);
     if (closed) 
         break;
     for (size_t i=0; i<n; ++i) {
       Tuple & tuple = iport.getTupleAt(i);    
       // do something with the tuple
     }
     @endcode
   * @param n number of tuples to wait for
   * @return true if <code>n</code> tuples will never be available, false otherwise
   */
  virtual bool waitTuple(size_t n) = 0;  
  
  /**
   * Get the tuple at the front of the input port queue.
   *
   * @return the tuple at the front of the input port queue
   */
  virtual Tuple & getFrontTuple() = 0;

  /**
   * Get the tuple at a given index within the input port queue.
   *
   * The first tuple is at index 0.
   * @return the tuple at a given index within the input port queue
   */
  virtual Tuple & getTupleAt(size_t index) = 0;

  /**
   * Remove the first tuple from the input port queue.
   */
  virtual void popTuple() = 0; 
};

} // namespace streamc

