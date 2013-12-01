#pragma once

/** @file Lambdas.h
 * The source file for the tuple lamda macros.
 */

/**
 * @defgroup Lambdas Tuple lambda macros
 * Tuple lambda macros.
 */

/** 
 * @addtogroup Lambdas 
 * @{
 * @def MEXP1(exp)
 * Create a single paramter tuple lambda expression.
 * 
 * This macro can be used to create a lambda expression that references a single
 * input tuple represented by <code>t_</code>, which is of type <code>Tuple
 * &</code>. Example:
 * @code
   MEXP1(t_.get<Type::Integer>("age") > 20)
   @endcode
 * is equivalent to:
 * @code
   [] (Tuple & t_) { return t_.get<Type::Integer>("age") > 20; }
   @endcode
 * @param exp an expression
 * @}
 */
#define MEXP1(exp) [] (Tuple & t_) { return exp; }

/**
 * @addtogroup Lambdas 
 * @{
 * @def MEXP2(exp)
 * Create a two paramter tuple lambda expression.
 * 
 * This macro can be used to create a lambda expression that references twp
 * input tuples represented by <code>t0_</code> and <code>t1_</code>, which are
 * of type <code>Tuple *</code>. Example:
 * @code
   MEXP2(t0_.get<Type::Double>("salary") + t1_.get<Type::Double>("bonus") )
   @endcode
 * is equivalent to:
 * @code
   [] (Tuple & t0_, Tuple & t1_) { return t0_.get<Type::Double>("salary") + t1_.get<Type::Double>("bonus") ; }
   @endcode
 * @param exp an expression
 * @}
 */
#define MEXP2(exp) [] (Tuple & t0_, Tuple & t1_) { return exp; }

/**
 * @addtogroup Lambdas 
 * @{
 * @def MEXPn(exp)
 * Create a multi-paramter tuple lambda expression.
 * 
 * This macro can be used to create a lambda expression that references multiple
 * input tuples represented by <code>ts_</code>, which is of type
 * <code>vector<Tuple *></code>. Example:
 * @code
   MEXPn(t_.front()->get<Type::Integer>("temp") - t_.back()->get<Type::Integer>("temp") )
   @endcode
 * is equivalent to:
 * @code
   [] (vector<Tuple &> const & ts_) { return t_.front()->get<Type::Integer>("temp") - t_.back()->get<Type::Integer>("temp"); }
   @endcode
 * @param exp an expression
 * @}
 */
#define MEXPn(exp) [] (std::vector<Tuple *> const & ts_) { return exp; }

