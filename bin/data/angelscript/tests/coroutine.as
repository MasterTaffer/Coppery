namespace Coroutine
{

void emptyCoroutineFunction()
{

}

[Test]
void TestCoroutineCreation()
{
    Assert(Coroutines::CreateCoroutine(@emptyCoroutineFunction) !is null);

    CoroutineFunction@ cf = emptyCoroutineFunction;

    Coroutine@ crt = Coroutines::CreateCoroutine(@cf);
    Assert(crt !is null);

    Assert(crt.isFinished() == false);

    crt.run();

    Assert(crt.isFinished());	

}

class SimpleClass
{
    int value = 0;
    void setValueToOne()
    {
        value = 1;
    }
}

[Test]
void TestCoroutineObject()
{
    SimpleClass sc;

    CoroutineFunction@ cf = CoroutineFunction(sc.setValueToOne);

    Coroutine@ crt = Coroutines::CreateCoroutine(@cf);
    Assert(crt !is null);

    Assert(crt.isFinished() == false);

    Assert(sc.value == 0);
    crt.run();

    Assert(crt.isFinished());
    Assert(sc.value == 1);
}


void yieldOnceCoroutineFunction()
{
    Coroutines::Yield();
}

[Test]
void TestCoroutineYield()
{
    Coroutine@ crt = Coroutines::CreateCoroutine(@yieldOnceCoroutineFunction);

    Assert(crt.isFinished() == false);
    crt.run();

    Assert(crt.isFinished() == false);
    crt.run();

    Assert(crt.isFinished());
}


void shouldReceiveCoroutineFunction()
{
    Assert(Coroutines::GetMailboxSize() == 1);
    any@ object = Coroutines::Receive();

    SimpleClass sc;
    
    Assert(object.retrieve(sc));
    Assert(sc.value == 1);
}



[Test]
void TestCoroutineMessage()
{
    Coroutine@ crt = Coroutines::CreateCoroutine(@shouldReceiveCoroutineFunction);

    SimpleClass sc;
    sc.value = 1;

    Assert(crt.isFinished() == false);
    crt.send(any(sc));
    crt.run();

    Assert(crt.isFinished());
}


void nestedCoroutineRunner()
{
    auto crt = Coroutines::CreateCoroutine(@yieldOnceCoroutineFunction);
    Assert(crt.isFinished() == false);
    crt.run();
    Assert(crt.isFinished() == false);

    yieldOnceCoroutineFunction();

    crt.run();
    Assert(crt.isFinished() == true);

    @crt = Coroutines::CreateCoroutine(@yieldOnceCoroutineFunction);
    Assert(crt.isFinished() == false);

    yieldOnceCoroutineFunction();

    crt.run();
    Assert(crt.isFinished() == false);
    crt.run();
    Assert(crt.isFinished() == true);
}

[Test]
void TestNestedCoroutines()
{
    Coroutine@ crt = Coroutines::CreateCoroutine(@nestedCoroutineRunner);
    Assert(crt.isFinished() == false);
    crt.run();
    Assert(crt.isFinished() == false);
    crt.run();
    Assert(crt.isFinished() == false);
    crt.run();
    Assert(crt.isFinished() == true);
}


/*
    The following tests mess with sending coroutines to each other,
    possibly recursively calling each other

    One must remember while investigating the control flow below, that the
    way a coroutine is suspended via Yield is exactly the same as calling
    .run for another coroutine.

    The active coroutines are also in a "stack": the coroutine stack. When
    someone calls coroutine.run(), the coroutine is pushed to the stack,
    regardless if it already exists somewhere in the stack. The running
    coroutine is always the topmost on the stack. When the running coroutine
    yields or returns, it is popped from the stack and the next routine on 
    the stack is continued.
*/


void passingCoroutinesFunc()
{

    Coroutine@ myroutine;
    
    any@ object = Coroutines::Receive();
    Assert(object.retrieve(@myroutine));

    myroutine.run();
    Coroutines::Yield();
}

[Test]
void TestPassingCoroutinesSimple()
{
    Coroutine@ crt = Coroutines::CreateCoroutine(@passingCoroutinesFunc);

    Assert(crt.isFinished() == false);
    crt.send(any(@crt));
    crt.run();

    Assert(crt.isFinished());
}




/*
    Rather complex test with a quite convoluted control flow

    CoroutineMonitor is used to observe the control flow in the coroutines
*/



class CoroutineMonitor
{
    int[] values;
}

void passingCoroutinesFuncComplex()
{
    CoroutineMonitor@ monitor;
    Coroutine@ myroutine;
    
    //Receive the monitor object
    any@ object = Coroutines::Receive();
    Assert(object.retrieve(@monitor));

    

    //And the special value for this coroutine

    object = Coroutines::Receive();
    int val = 0;
    Assert(object.retrieve(val));

    //And finally receive another couroutine
    @object = Coroutines::Receive();
    Assert(object.retrieve(@myroutine));

    monitor.values.insertLast(val);

    myroutine.run();

    monitor.values.insertLast(val+1);

    Coroutines::Yield();

    monitor.values.insertLast(val+2);
}


[Test]
void TestPassingCoroutinesRecursive()
{
    CoroutineMonitor monitor;

    Coroutine@ crt1 = Coroutines::CreateCoroutine(@passingCoroutinesFuncComplex);
    Coroutine@ crt2 = Coroutines::CreateCoroutine(@passingCoroutinesFuncComplex);


    Assert(crt1.isFinished() == false);
    Assert(crt2.isFinished() == false);

    crt1.send(any(@monitor));
    crt2.send(any(@monitor));

    crt1.send(any(100));
    crt2.send(any(200));

    crt1.send(any(@crt2));
    crt2.send(any(@crt1));

    crt1.run();

    Assert(crt1.isFinished());
    Assert(crt2.isFinished() == false);

    crt2.run();
    Assert(crt2.isFinished());

    //Assert the correct control flow
    Assert(monitor.values[0] == 100); //crt1 starts -> calls crt2.run
    Assert(monitor.values[1] == 200); //crt2 starts -> calls crt1.run
    Assert(monitor.values[2] == 101); //crt1 continues from run function -> yields
    Assert(monitor.values[3] == 201); //crt2 continues from run function -> yields
    Assert(monitor.values[4] == 102); //crt1 returns from the function
    Assert(monitor.values[5] == 202); //crt2.run() is called afterwards by the main thread
}

/*
    Execute the above test in a coroutine
*/
[Test]
void TestPassingCoroutinesRecursiveInCoroutine()
{

    Coroutine@ crt = Coroutines::CreateCoroutine(@TestPassingCoroutinesRecursive);
    crt.run();
    Assert(crt.isFinished() == true);
}

/*
    The following test is mainly a memory leak test
*/
[Test]
void TestPassingCoroutinesRecursiveMemoryLeak()
{
    CoroutineMonitor monitor;

    Coroutine@ crt1 = Coroutines::CreateCoroutine(@passingCoroutinesFuncComplex);
    Coroutine@ crt2 = Coroutines::CreateCoroutine(@passingCoroutinesFuncComplex);
    Coroutine@ crt3 = Coroutines::CreateCoroutine(@passingCoroutinesFuncComplex);


    Assert(crt1.isFinished() == false);
    Assert(crt2.isFinished() == false);
    Assert(crt3.isFinished() == false);

    crt1.send(any(@monitor));
    crt2.send(any(@monitor));
    crt3.send(any(@monitor));


    crt1.send(any(100));
    crt2.send(any(200));
    crt3.send(any(300));

    crt1.send(any(@crt2));
    crt2.send(any(@crt3));
    crt3.send(any(@crt1));

    //Send a bunch of data
    crt1.send(any(@crt2));
    crt3.send(any(@crt2));
    crt2.send(any(@crt3));
    crt1.send(any(@crt3));
    crt3.send(any(@crt1));
    crt2.send(any(@crt1));

    crt1.run();

    //Never finish the two other coroutines

    Assert(crt1.isFinished());
    Assert(crt2.isFinished() == false);
    Assert(crt3.isFinished() == false);
}


[Test]
void TestYieldFromMainThreadFails()
{
    AssertThrowsAfterThis();
    Coroutines::Yield();
}



[Test]
void TestReceiveFromMainThreadFails()
{
    AssertThrowsAfterThis();
    Coroutines::Receive();
}


void receiveTwiceFunction()
{
    any@ object = Coroutines::Receive();
    any@ object2 = Coroutines::Receive();
}

[Test]
void TestReceiveEmptyMailboxFails()
{
    Coroutine@ crt = Coroutines::CreateCoroutine(@receiveTwiceFunction);
    crt.send(any(1));
    
    AssertThrowsAfterThis();
    crt.run();
}


[Test]
void TestNullCoroutineFunctionFails()
{
    AssertThrowsAfterThis();
    Coroutine@ crt = Coroutines::CreateCoroutine(null);
}


}
