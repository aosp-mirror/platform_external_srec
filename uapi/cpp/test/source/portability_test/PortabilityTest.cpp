/*---------------------------------------------------------------------------*
 *  PortabilityTest.cpp                                                      *
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

#include "PortabilityTest.h"
#include <stdio.h>


#ifndef SKIP_TEST1
ThrowIntWithDeclaration::ThrowIntWithDeclaration() throw(int)
{
  throw 5;
}
#endif

#ifndef SKIP_TEST2
ThrowIntWithoutDeclaration::ThrowIntWithoutDeclaration()
{
  throw 5;
}
#endif

#ifndef SKIP_TEST3
CovariantParent& CovariantParent::getInstance()
{
  return *this;
}

CovariantChild& CovariantChild::getInstance()
{
  return *this;
}
#endif

#ifndef SKIP_TEST4
namespace MyNamespace
{
  NamespacedClass::NamespacedClass()
  {}
}
#endif

#ifndef SKIP_TEST5
const char* PolymorphismParent::getResult()
{
  return "- Test5 failed";
}

const char* PolymorphismChild::getResult()
{
  return "+ Test5 passed";
}
#endif

#ifndef SKIP_TEST6
PureChild::PureChild():
    value(0)
{}

void PureChild::increment()
{
  ++value;
}

bool PureChild::testPassed()
{
  return value == 1;
}
#endif

#ifndef SKIP_TEST7
ImpureParent1::ImpureParent1():
    value(0)
{}

void ImpureParent1::increment()
{
  ++value;
}

bool ImpureParent2::testPassed()
{
  return false;
}

bool ImpureChild::testPassed()
{
  return value == 1;
}
#endif


int main(int argc, char* argv[])
{
#ifndef SKIP_TEST1
  try
  {
    ThrowIntWithDeclaration temp;
    printf("- Test1 failed\n");
  }
  catch (int)
  {
    printf("+ Test1 passed\n");
  }
  catch (...)
  {
    printf("- Test1 failed\n");
  }
#endif
  
#ifndef SKIP_TEST2
  try
  {
    ThrowIntWithoutDeclaration temp;
    printf("- Test2 failed\n");
  }
  catch (int)
  {
    printf("+ Test2 passed\n");
  }
  catch (...)
  {
    printf("- Test2 failed\n");
  }
#endif
  
#ifndef SKIP_TEST3
  {
    CovariantChild child;
    child = child.getInstance();
    printf("+ Test3 passed\n");
  }
#endif
  
#ifndef SKIP_TEST4
  {
    MyNamespace::NamespacedClass temp;
    printf("+ Test4 passed\n");
  }
#endif
  
#ifndef SKIP_TEST5
  {
    PolymorphismChild child;
    PolymorphismParent* parent = &child;
    printf("%s\n", parent->getResult());
  }
#endif
  
#ifndef SKIP_TEST6
  {
    PureChild pureChild;
    pureChild.increment();
    if (pureChild.testPassed())
      printf("+ Test6 passed\n");
    else
      printf("- Test6 failed\n");
  }
#endif
  
#ifndef SKIP_TEST7
  {
    ImpureChild impureChild;
    impureChild.increment();
    if (impureChild.testPassed())
      printf("+ Test7 passed\n");
    else
      printf("- Test7 failed\n");
  }
#endif
}
