/**
\mainpage

\section intro_dg_tutorial Introduction

This tutorial implements a simple application of the dynamic-graph package with two entities represented by the following classes:
\li dynamicgraph::tutorial::InvertedPendulum: that implements the dynamics of a pendulum on a cart and
\li dynamicgraph::tutorial::FeedbackControl: that implements a closed-loop control loop that stabilizes the pendulum.

\section dg_tutorial_inverted_pendulum_ Prerequisite

This tutorial requires prior installation of packages:
\li <c> dynamic-graph </c>
\li <c> dynamic-graph-python </c>

\section dg_tutorial_inverted_pendulum_overview Overview

This tutorial show:
\li how to \ref dg_tutorial_inverted_pendulum_cxx "create a new entity" with signals and commands,
\li how to build a python module to create and control this entity in a python
interpreter.

\page dg_tutorial_inverted_pendulum_cxx C++ implementation

\section dg_tutorial_inverted_pendulum_cxx_intro Introduction

New entity types are defined by
\li deriving dynamicgraph::Entity class,
\li adding signals, and
\li adding commands.
As an example, we will review class dynamicgraph::tutorial::InvertedPendulum.

\section dg_tutorial_inverted_pendulum_cxx_interface Interface

The interface is defined in file <c>include/dynamic-graph/tutorial/inverted-pendulum.hh</c>.

First, we include
\li the header defining Entity class and
\li the header defining SignalPtr template class

\code
    #include <dynamic-graph/entity.h>
    #include <dynamic-graph/signal-ptr.h>
\endcode

Then in namespace <c>dynamicgraph::tutorial</c> we define class InvertedPendulum
\code
    namespace dynamicgraph {
      namespace tutorial {
        class DG_TUTORIAL_EXPORT InvertedPendulum : public Entity
        {
\endcode
with a constructor taking a name as an input
\code
          InvertedPendulum(const std::string& inName);
\endcode

For the internal machinery, each entity can provide the name of the class it
belongs to:
\code
          virtual const std::string& getClassName (void) const {
            return CLASS_NAME;
          }
\endcode

Class InvertedPendulum represents a dynamical system. The following method
integrates the equation of motion over a time step
\code
          void incr(double inTimeStep);
\endcode

Setters and getters will enable us later to control parameters through commands.
\code
          void setCartMass (const double& inMass) {
            cartMass_ = inMass;
          }

          double getCartMass () const {
            return cartMass_;
          }

          void setPendulumMass (const double& inMass) {
            pendulumMass_ = inMass;
          }

          double getPendulumMass () const {
           return pendulumMass_;
          }

          void setPendulumLength (const double& inLength) {
            pendulumLength_ = inLength;
          }

          double getPendulumLength () const {
            return pendulumLength_;
	  }
\endcode

The name of the class is stored as a static member
\code
          static const std::string CLASS_NAME;
\endcode
In the private part of the class, we store signals
\code
          SignalPtr< Vector, int > forceSIN;
          Signal< Vector, int> stateSOUT;
\endcode
and parameters
\code
          double cartMass_;
          double pendulumMass_;
          double pendulumLength_;
          double viscosity_;
\endcode

\section dg_tutorial_inverted_pendulum_cxx_implementation Implementation

The implementation is written in file <c>src/inverted-pendulum.cc</c>.

First, we include headers defining
\li class FactoryStorage,
\li general setter and getter commands
\li the previously defined header, and
\li local Increment command class:

\subsection dg_tutorial_inverted_pendulum_cxx_implementation_headers Headers

\code
    #include <dynamic-graph/factory.h>
    #include <dynamic-graph/command-setter.h>
    #include <dynamic-graph/command-getter.h>
    #include "dynamic-graph/tutorial/inverted-pendulum.hh"
    #include "command-increment.h"
\endcode

\subsection dg_tutorial_inverted_pendulum_cxx_implementation_entity_registration Entity registration

The second step consists in
\li registering our new class into the entity factory and
\li instantiating the static variable CLASS_NAME

using a macro defined in <c>dynamic-graph/factory.h</c>:
\code
    DYNAMICGRAPH_FACTORY_ENTITY_PLUGIN(InvertedPendulum, "InvertedPendulum");
\endcode

\subsection dg_tutorial_inverted_pendulum_cxx_implementation_constructor Constructor

Then we define the class constructor
\li passing the instance name to Entity class constructor,
\li initializing signals with a string following the specified format and
\li initializing parameters with default values:

\code
    InvertedPendulum::InvertedPendulum(const std::string& inName) :
      Entity(inName),
      forceSIN(NULL, "InvertedPendulum("+inName+")::input(vector)::forcein"),
      stateSOUT("InvertedPendulum("+inName+")::output(vector)::state"),
      cartMass_(1.0), pendulumMass_(1.0), pendulumLength_(1.0), viscosity_(0.1)
\endcode

We register signals into an associative array stored into Entity class
\code
      signalRegistration (forceSIN);
      signalRegistration (stateSOUT);
\endcode

We set input and output signal as constant with a given value
\code
      Vector state = ZeroVector(4);
      Vector input = ZeroVector(1);
      stateSOUT.setConstant(state);
      forceSIN.setConstant(input);
\endcode

The following lines of code define and register commands into the entity.
A Command is created by calling a constructor with 
\li a string: the name of the command and
\li a pointer to a newly created command:

\code
      addCommand(std::string("incr"),
                 new command::Increment(*this));
\endcode

In this example, command::Increment is a command specific to our class
InvertedPendulum and defined in file <c>src/command-increment.h</c>

Setter and getter commands are available through classes templated by the type of entity using the command and the type of the parameter. Be aware that only a prespecified set of types are supported for commands, see class dynamicgraph::command::Value.
\code
      addCommand(std::string("setCartMass"),
                 new ::dynamicgraph::command::Setter<InvertedPendulum, double>
                 (*this, &InvertedPendulum::setCartMass));

      addCommand(std::string("getCartMass"),
                 new ::dynamicgraph::command::Getter<InvertedPendulum, double>
                 (*this, &InvertedPendulum::getCartMass));
\endcode

\note
It is important to notice that
\li commands passed to method Entity::addCommand will be destroyed automatically by Entity class destructor. The user should therefore not destroy them,
\li commands should be defined and registered in the class constructor. Commands defined later on will not be reachable by python bindings.

\subsection dg_tutorial_inverted_pendulum_cxx_implementation_newtypes Registering new types: advanced feature

Signals are templated by the type of data they convey. In this example, we hae defined our own class of vectors InvertedPendulum::Vector. In order to be able to create signals with this type, we need to register the new type:
\code
dynamicgraph::DefaultCastRegisterer<InvertedPendulum::Vector> IPVectorCast;
\endcode

\note
The new type should implement operator<< and operator>> in order to store variables in streams.


\page dg_tutorial_inverted_pendulum_python Python module

\section dg_tutorial_inverted_pendulum_python_intro Introduction



*/
