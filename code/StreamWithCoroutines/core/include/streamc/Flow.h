#pragma once

#include "streamc/Operator.h"
#include "streamc/FlowConnectors.h"

#include <iostream>
#include <memory>
#include <unordered_map>
#include <vector>
#include <functional>

namespace streamc
{

class OperatorConnections;

/**
 * %Flow that represents a graph of stream processing operators.
 */
class Flow
{
public:
  /**
   * Construct a flow.
   *
   * @param name name of the flow
   */
  Flow(std::string const & name);

  ~Flow();

  /**
   * Get the name of the flow.
   *
   * @return the name of the flow
   */
  std::string const & getName() const { return name_; }

  /**
   * Create an operator and add it to the flow.
   *

   * This function instantiates an operator and adds it it the flow. It is the
   * preferred way to add operator's to a flow. Example:
   * @code{.cpp}
     Operator & op = flow.createOperator<FileSource>("op-name")
        .set_fileName("in.data")
        .set_fileFormat({{"name", Type::String}, {"age", Type::Integer}});
     @endcode
   * or alternatively:
   * @code{.cpp}
     Operator & op = flow.createOperator<FileSource>("op-name", 
         "in.data", {{"name", Type::String}, {"age", Type::Integer}});
     @endcode   
   * @tparam T operator class
   * @param name name of the operator
   * @param args the arguments of the operator's constructor
   * @return the operator that is created
   */
  template <typename T, typename... Args>
  T & createOperator(std::string const & name, Args&&... args)
  {
    T * op = new T(name, std::forward<Args>(args)...);
    op->setCloneFunction(
      [&](std::string const & newName) -> Operator *
      {
        return new T(newName, std::forward<Args>(args)...);
      }
    );
    addOperator(*op);
    return *op;
  }

  /**
   * Add a connection to the flow.
   *
   * @param fromOp from opeator of the connection
   * @param fromOutPort from output port of the connection
   * @param toOp to opeator of the connection
   * @param toInPort to input port of the connection
   */
  void addConnection(Operator & fromOp, uint32_t fromOutPort, Operator & toOp, uint32_t toInPort);

  /**
   * Add a connection to the flow.
   *
   * @param fromConn the from connection
   * @param toConn the to connection
   */
  void addConnection(connectors::FromConnection const & fromConn, connectors::ToConnection const & toConn);

  /**
   * Add a connection to the flow.
   *
   * @param conn the connect to add
   */
  void addConnection(connectors::Connection const & conn);

  /**
   * Add a series of connections to the flow.
   *
   * @param conns the connection chain to be added to the flow
   */
  void addConnections(connectors::ConnectionChain const & conns);

  /**
   * Add a connection to the flow.
   *
   * @param fromOp from opeator of the connection
   * @param fromOutPort from output port of the connection
   * @param toOp to opeator of the connection
   * @param toInPort to input port of the connection
   */
  void removeConnection(Operator & fromOp, uint32_t fromOutPort, Operator & toOp, uint32_t toInPort);

  /**
   * Add a connection to the flow.
   *
   * @param fromConn the from connection
   * @param toConn the to connection
   */
  void removeConnection(connectors::FromConnection const & fromConn, connectors::ToConnection const & toConn);

  /**
   * Add a connection to the flow.
   *
   * @param conn the connect to add
   */
  void removeConnection(connectors::Connection const & conn);

  /**
   * Add a series of connections to the flow.
   *
   * @param conns the connection chain to be added to the flow
   */
  void removeConnection(connectors::ConnectionChain const & conns);

  /**
   * Get an operator by name.
   *
   * @param opName name of the operator
   * @return the operator in the flow with the given name
   */
  Operator & getOperatorByName(std::string const & opName) const;

  /**
   * Get a list of all the operators.
   *
   * @return the list of all operators
   */
  std::vector<Operator *> const & getOperators() const { return opList_; }

  /**
   * Get all out connections of an operator on a given output port.
   *
   * @param op the operator
   * @param outPort the output port index
   * @return the list of all to connections of the operator on the given output port 
   */
  std::vector<connectors::ToConnection> const & getOutConnections(Operator & op, size_t outPort) const;

  /**
   * Get all in connections of an operator on a given input port.
   *
   * @param op the operator
   * @param inPort the input port index
   * @return the list of all from connections of the operator on the given input port 
   */
  std::vector<connectors::FromConnection> const & getInConnections(Operator & op, size_t inPort) const;

  /**
   * Print the topology.
   *
   * @param ostream the output stream to print to
   */
  void printTopology(std::ostream & ostream) const; 

private:
  void addOperator(Operator & op);

private:
  //name of the flow
  std::string name_;

  //operators in the flow
  std::vector<Operator *> opList_;

  //maps name to operator
  std::unordered_map<std::string, std::unique_ptr<Operator>> ops_;

  // maps operator to operator connections
  std::unordered_map<Operator *, std::unique_ptr<OperatorConnections>> opConnections_;
};

} // namespace streamc
