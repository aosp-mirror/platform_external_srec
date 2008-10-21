/*---------------------------------------------------------------------------*
 *  PortabilityTest.h                                                        *
 *                                                                           *
 *  Copyright 2007, 2008 Nuance Communciations, Inc.                               *
 *                                                                           *
 *  Licensed under the Apache License, Version 2.0 (the 'License');          *
 *  you may not use this file except in compliance with the License.         *
 *                                                                           *
 *  You may obtain a copy of the License at                                  *
 *      http://www.apache.org/licenses/LICENSE-2.0                           *
 *                                                                           *
 *  Unless required by applicable law or agreed to in writing, software      *
 *  distributed under the License is distributed on an 'AS IS' BASIS,        *
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
 *  See the License for the specific language governing permissions and      *
 *  limitations under the License.                                           *
 *                                                                           *
 *---------------------------------------------------------------------------*/

#ifndef __UAPI__PORTABILITYTEST
#define __UAPI__PORTABILITYTEST

#ifdef _WIN32
#  pragma warning (disable: 4290)
#endif

#ifndef SKIP_TEST1
// Throw an exception with a declaration
class ThrowIntWithDeclaration
{
  public:
    ThrowIntWithDeclaration() throw(int);
};
#endif

#ifndef SKIP_TEST2
// Throw an exception without a declaration
class ThrowIntWithoutDeclaration
{
  public:
    ThrowIntWithoutDeclaration();
};
#endif

#ifndef SKIP_TEST3
// Covariant return types
class CovariantParent
{
  public:
    virtual CovariantParent& getInstance();
    virtual ~CovariantParent() {};
};

class CovariantChild: public CovariantParent
{
  public:
    virtual CovariantChild& getInstance();
    virtual ~CovariantChild() {};
};
#endif

#ifndef SKIP_TEST4
// Namespaces
namespace MyNamespace
{
  class NamespacedClass
  {
    public:
      NamespacedClass();
  };
}
#endif

#ifndef SKIP_TEST5
// Polymorphism
class PolymorphismParent
{
  public:
    virtual const char* getResult();
    virtual ~PolymorphismParent() {};
};

class PolymorphismChild: public PolymorphismParent
{
  public:
    virtual const char* getResult();
    virtual ~PolymorphismChild() {};
};
#endif

#ifndef SKIP_TEST6
// Multiple inheritance of two pure-virtual classes
class PureParent1
{
  public:
    virtual ~PureParent1() {}
    virtual void increment() = 0;
};

class PureParent2
{
  public:
    virtual ~PureParent2() {}
    virtual bool testPassed() = 0;
};

class PureChild: public PureParent1, public PureParent2
{
  public:
    PureChild();
    virtual ~PureChild(){}
    virtual void increment();
    virtual bool testPassed();
  protected:
    int value;
};
#endif

#ifndef SKIP_TEST7
// Multiple inheritance of concrete classes
class ImpureParent1
{
  public:
    virtual ~ImpureParent1(){}
    ImpureParent1();
    virtual void increment();
  protected:
    int value;
};

class ImpureParent2
{
  public:
    virtual ~ImpureParent2(){}
    virtual bool testPassed();
};

class ImpureChild: public ImpureParent1, public ImpureParent2
{
  public:
    virtual ~ImpureChild(){}
    virtual bool testPassed();
};
#endif

#endif
