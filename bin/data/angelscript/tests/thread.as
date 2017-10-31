namespace Thread
{
/*
    These tests are not deterministic, due to the very nature of threading -
    one should merely observe these tests as an example usecase of threads.
    
    Preferably, one should never use the threads.
*/

void receiveAndSend()
{
    for (uint i = 0; i < 1000; i++)
    {
        any@ object = Threads::ReceiveForever();
        int num = 0;
        object.retrieve(num);
        Threads::Send(any(num + 1));
    }
}

[Test]
void TestThreadMessaging()
{
    Thread@ thread = Threads::CreateThread(@receiveAndSend);
    Assert(thread.isFinished() == true); //isFinished returns true if the thread is not running
    
    Assert(thread.run());
    Assert(thread.isFinished() == false);
    for (uint i = 0; i < 1000; i++)
    {
        thread.send(any(560));
        any@ object = thread.receiveForever();

        int num = 0;
        Assert(object.retrieve(num));
        Assert(num == 561);
    }

    //10 second better be enough.
    Assert (thread.wait(10000) != 0);

    Assert(thread.isFinished() == true);
}


void circleSend()
{

    while (true)
    {
        any@ object = Threads::Receive(1);
        if (object is null)
            continue;

        Thread@ next;
        object.retrieve(@next);

        int num = 0;
        @object = Threads::ReceiveForever();
        object.retrieve(num);

        next.send(any(num + 1));
        break;
    }
}

void circleLast()
{
    any@ object = Threads::ReceiveForever();
    int num = 0;
    object.retrieve(num);
    Threads::Send(any(num));
}

[Test]
void TestInterThreadMessaging()
{
    Thread@[] threads;

    for (uint i = 0; i < 5; i++)
    {
        Thread@ t = Threads::CreateThread(@circleSend);
        threads.insertLast(@t);

        t.run();
    }

    Thread@ last = Threads::CreateThread(@circleLast);
    threads.insertLast(@last);

    last.run();

    for (uint i = 0; i < 5; i++)
    {
        Thread@ t = threads[i];
        Thread@ next = threads[i + 1];
        t.send(any(@next));
    }
    threads[0].send(any(654));

    //Wait for 10 secs max
    Assert(last.wait(10000) != 0);


    for (uint i = 0; i < threads.length(); i++)
    {
        Thread@ t = threads[i];
        Assert(t.wait(10000) != 0);
    }
    any@ object = last.receiveForever();
    int val = 0;
    object.retrieve(val);
    Assert(val == 654 + 5);
}


}
