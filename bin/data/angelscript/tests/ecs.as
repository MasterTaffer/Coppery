namespace ECS
{
    void clear()
    {
        ESM::UpdateEntityLists();
        ESM::KillAllEntities();
        ESM::UpdateEntityLists();
        ESM::SendEvents();
    }

    class TestEvent
    {
        int value = 0;
    }

    [Component]
    class TestComponent
    {
        Entity@ entity;
        bool initCalled = false;
        bool deInitCalled = false;
        int value = 0;

        [InitHandler]
        void init()
        {
            initCalled = true;
        }

        [DeinitHandler]
        void deinit()
        {
            deInitCalled = true;
        }


        [EventHandler]
        void update(const TestEvent&in ev)
        {
            value = ev.value;
        }

    }

    EntityMold@ EM_Test = {
        ComponentInfo<TestComponent>().getId()
    };

    [Test]
    void ComponentTest()
    {
        clear();
        TestComponent@ tc;
        Entity@ e = ESM::ConstructEntity(EM_Test);

        Assert(e !is null);

        e.getComponent(@tc);

        Assert(tc !is null);

        Assert(tc.entity is e);
        Assert(tc.entity == e);
        Assert(tc.initCalled == false);

        Assert(e.dead == false);
        Assert(e.id > 0);

        ESM::UpdateEntityLists();

        Assert(tc.initCalled == true);

        ESM::KillEntity(e);
        Assert(e.dead == false);
        Assert(e.id > 0);
        Assert(tc.deInitCalled == false);

        ESM::UpdateEntityLists();
        Assert(e.dead == true);
        Assert(e.id == 0);
        Assert(tc.deInitCalled == true);

        Assert(tc.entity.dead == true);

        TestComponent@ tc2;
        e.getComponent(@tc2);

        Assert(tc2 is null);

        Assert(e.dead == true);
        ESM::KillEntity(e);
        Assert(e.dead == true);
        ESM::UpdateEntityLists();
        Assert(e.dead == true);
        clear();

    }


    [Component]
    class TestComponentRef
    {
        Entity@ entity;
        [ComponentRef]
        TestComponentRef@ tcr;
        [ComponentRef]
        TestComponent@ tc;
    }

    EntityMold@ EM_TestRef = {
        ComponentInfo<TestComponent>().getId(),
        ComponentInfo<TestComponentRef>().getId()
    };

    [Test]
    void ComponentRefTest()
    {
        clear();

        TestComponent@ tc;
        TestComponentRef@ tcr;
        Entity@ e = ESM::ConstructEntity(EM_TestRef);

        Assert(e !is null);

        e.getComponent(@tcr);
        e.getComponent(@tc);

        Assert(tcr !is null);

        Assert(tc !is null);


        Assert(tcr.tcr is null); //Component reference to self is always null

        Assert(tcr.tc is tc);
        Assert(tcr.entity is tc.entity);

        ESM::UpdateEntityLists();
        ESM::KillEntity(e);
        ESM::UpdateEntityLists();

        Assert(e.dead == true);
        Assert(e.id == 0);

        Assert(tcr.tcr is null);
        Assert(tcr.tc is tc);

        clear();
    }

}
