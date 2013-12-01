#pragma once

namespace streamc { namespace connectors 
{

/**
 * Out end of a connection (input port index, downstream operator) pair.
 */
class ToConnection
{
public:
  /**
   * Construct the connection.
   *
   * @param downstreamOp downstream operator 
   * @param inPort input port index
   */
  ToConnection(Operator & downstreamOp, size_t inPort) 
    : downstreamOp_(downstreamOp), inPort_(inPort) 
  {}

  /**
   * Get the input port index.
   *
   * @return the input port index
   */
  size_t getInputPort() const { return inPort_; }

  /**
   * Get the downstream operator.   
   *
   * @return the downstream operator
   */
  Operator & getOperator() const { return downstreamOp_; }
private:
  Operator & downstreamOp_;
  size_t inPort_;
};

/**
 * @defgroup FlowOps Flow construction operators
 * %Flow construction operators.
 */

/** @addtogroup FlowOps 
 * @{ */
/**
 * Create a to connection from an (input port index, downstream operator) pair.
 *
 * @param inPort the input port index
 * @param downstreamOp the downstream operator
 * @return the connection
 */
inline ToConnection operator,(size_t inPort, Operator & downstreamOp)
{
  return ToConnection(downstreamOp, inPort);
}
/** @} */

/**
 * In end of a connection (upstream operator, output port index) pair.
 */
class FromConnection
{
public:
  /**
   * Construct the connection.
   *
   * @param upstreamOp upstream operator 
   * @param outPort output port index
   */
  FromConnection(Operator & upstreamOp, size_t outPort) 
    : upstreamOp_(upstreamOp), outPort_(outPort) 
  {}

  /**
   * Get the output port index.
   *
   * @return the output port index
   */
  size_t getOutputPort() const { return outPort_; }

  /**
   * Get the upstream operator.   
   *
   * @return the downstream operator
   */
  Operator & getOperator() const { return upstreamOp_; }
private:
  Operator & upstreamOp_;
  size_t outPort_;
};

/** @addtogroup FlowOps 
 * @{ */
/**
 * Create a from connection from an (upstream operator, output port index) pair.
 *
 * @param upstreamOp the upstream operator
 * @param outPort the output port index
 * @return the connection
 */
inline FromConnection operator,(Operator & upstreamOp, size_t outPort)
{
  return FromConnection(upstreamOp, outPort);
}
/** @} */

/**
 * A connection, which is a combination of a from connection and to connection.
 */
class Connection
{
public:
  /**
   * Construct the connection.
   *
   * @param fromConn the from connection
   * @param toConn the to connection
   */  
  Connection (FromConnection const & fromConn, ToConnection const & toConn)
    : fromConn_(fromConn), toConn_(toConn)
  {}

  /**
   * Construct the connection.
   *
   * @param upstreamOp the upstream operator
   * @param outPort the output port index
   * @param downstreamOp the downstream operator
   * @param inPort the input port index
   */  
  Connection (Operator & upstreamOp, size_t outPort, Operator & downstreamOp, size_t inPort)
    : Connection(FromConnection(upstreamOp, outPort), ToConnection(downstreamOp, inPort))
  {}
  
  /**
   * Get the in end of the connection.
   *
   * @return the in end of the connection 
   */
  FromConnection const & getInConnection() const { return fromConn_; }

  /**
   * Get the out end of the connection.
   *
   * @return the out end of the connection 
   */
  ToConnection const & getOutConnection() const { return toConn_; }
private:
  FromConnection fromConn_;
  ToConnection toConn_;
};

/** @addtogroup FlowOps 
 * @{ */
/**
 * Create a connection from a (from connection, to connection) pair.
 *
 * @param fromConn from connection
 * @param toConn to connection
 * @return the connection
 */
inline Connection operator>>(FromConnection const & fromConn, ToConnection const & toConn)
{
  return Connection(fromConn, toConn);
}
/** @} */

/**
 * A connection knot, which is an operator with input and output port indices.
 */
class ConnectionKnot
{
public:
  /**
   * Construct the knot.
   *
   * @param inPort the input port index
   * @param op the operator
   * @param outPort the output port index
   */
  ConnectionKnot(size_t inPort, Operator & op, size_t outPort)
    : inPort_(inPort), op_(op), outPort_(outPort)
  {}

  /**
   * Get the input port index.
   *
   * @return the input port index
   */
  size_t getInputPort() const { return inPort_; }

  /**
   * Get the operator.
   *
   * @return the operator
   */  
  Operator & getOperator() const { return op_; }

  /**
   * Get the output port index.
   *
   * @return the output port index
   */
  size_t getOutputPort() const { return outPort_; }

private:
  size_t inPort_;
  Operator & op_;
  size_t outPort_;
};

/** @addtogroup FlowOps 
 * @{ */
/**
 * Create a connection knot from a (to connection, output port index) pair.
 *
 * @param toConn to connection
 * @param outPort output port index
 * @return the connection knot
 */
inline ConnectionKnot operator,(ToConnection const & toConn, size_t outPort)
{
  return ConnectionKnot(toConn.getInputPort(), toConn.getOperator(), outPort);
}
/** @} */

class OpenConnectionChain;

/**
 * A connection chain, which represents a complete series of connections.
 */
class ConnectionChain
{
public:
  /**
   * Construct an empty connection chain (aka the default constructor).
   */
  ConnectionChain() {}

  /**
   * Construct a connection chain from an open connection chain temporary and a connection.
   *
   * @param other the open connection chain temporary
   * @param conn the connection
   */
  ConnectionChain(OpenConnectionChain && other, Connection const & conn);

  /**
   * Get all the connections.
   *
   * @return all the connections
   */
  std::vector<Connection> const & getConnections() const { return connections_; }
protected:
  /// @cond hidden
  std::vector<Connection> connections_;
  /// @endcond
};

/**
 * A connection chain with a dangling output port at the end.
 */
class OpenConnectionChain : public ConnectionChain
{
public:
  /**
   * Construct an empty open connection chain (aka the default constructor).
   */
  OpenConnectionChain() {}

  /**
   * Construct an open connection chain from a connection, and an output port index
   *
   * @param conn the connection
   * @param outPort the output port index
   */
  OpenConnectionChain(Connection const & conn, size_t outPort)
    : outputPort_(outPort)
  {
    connections_.push_back(conn);
  }

  /**
   * Construct an open connection chain from an open connection chain temporary,
   * a connection, and an output port index
   *
   * @param other the open connection chain temporary
   * @param conn the connection
   * @param outPort the output port index
   */
  OpenConnectionChain(OpenConnectionChain && other, Connection const & conn, size_t outPort)
    : ConnectionChain(std::move(other), conn), outputPort_(outPort)
  {}

  /**
   * Get the open output port index.
   *
   * @return the open output port index
   */
  size_t getOpenOutputPort() const { return outputPort_; }
private:
  size_t outputPort_;
};

inline ConnectionChain::ConnectionChain(OpenConnectionChain && other, Connection const & conn) 
  : connections_(std::move(other.connections_))
{
  connections_.push_back(conn);
}

/** @addtogroup FlowOps 
 * @{ */
/**
 * Create an open connection chain from a (from connection, connection knot) pair.
 *
 * @param fromConn from connection
 * @param knot connection knot
 * @return the open connection chain
 */
inline OpenConnectionChain operator>>(FromConnection const & fromConn, ConnectionKnot const & knot)
{
  Connection conn(fromConn.getOperator(), fromConn.getOutputPort(), knot.getOperator(), knot.getInputPort());
  OpenConnectionChain chain(conn, knot.getOutputPort());
  return chain;
}
/** @} */

/** @addtogroup FlowOps 
 * @{ */
/**
 * Extend an open connection chain using a connection knot.
 *
 * @param chain open connection chain to be extended
 * @param knot connection knot to be added to the open connection chain
 * @return the resulting open connection chain
 */
inline OpenConnectionChain operator>>(OpenConnectionChain && chain, ConnectionKnot const & knot)
{
  Connection conn(chain.getConnections().rbegin()->getOutConnection().getOperator(), 
                  chain.getOpenOutputPort(), knot.getOperator(), knot.getInputPort());
  OpenConnectionChain newChain(std::move(chain), conn, knot.getOutputPort());
  return newChain;
}
/** @} */

/** @addtogroup FlowOps 
 * @{ */
/**
 * Convert an open connection chain into a connection chain using a to connection.
 *
 * @param chain open connection chain to be converted into an open connection
 * @param toConn to connection to be added into the open connection chain
 * @return the resultingconnection chain
 */
inline ConnectionChain operator>>(OpenConnectionChain && chain, ToConnection const & toConn)
{
  Connection conn(chain.getConnections().rbegin()->getOutConnection().getOperator(), 
                  chain.getOpenOutputPort(), toConn.getOperator(), toConn.getInputPort());
  ConnectionChain newChain(std::move(chain), conn);
  return newChain;
}
/** @} */

} } // namespace streamc::connectors 
