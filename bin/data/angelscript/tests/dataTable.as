
namespace DataTable
{	

[Test]
void DatatableCreation()
{
    DataTable@ dt = DataTable::Create();
    Assert(dt !is null);

    dt.newTable("table2");
    DataTable@ dt2 = dt.getTable("table2");
    Assert(dt2 !is null);

    dt2.newTable("table3");
    DataTable@ dt3 = dt2.getTable("table3");
    Assert(dt3 !is null);
}

[Test]
void DatatableElements()
{
    DataTable@ dt = DataTable::Create();
    
    dt.setInt("integer", 32);
    dt.setDouble("floating", 64.64);
    dt.setString("string", "a string indeed");

    int64 outInt;
    double outDouble;
    string outString;

    Assert(dt.getInt("integer", outInt));
    Assert(outInt == 32);

    Assert(dt.getDouble("floating", outDouble));
    Assert(EqualsDelta(outDouble, 64.64, 0.1));

    Assert(dt.getString("string", outString));
    Assert(outString == "a string indeed");

    Assert(dt.getInt("floating", outInt));
    Assert(outInt == 64); //trunaction

    Assert(dt.getInt("string", outInt) == false);

    //there is no guarantee that Assert(outInt == 64),
    //e.g the old value is preserved
    //it most likely will be preserved though


    Assert(dt.getString("floating", outString) == false);
    Assert(dt.getString("integer", outString) == false);

    //there is no guarantee that Assert(outString == "a string indeed")
    //and it will not be preserved in the case of string


    Assert(dt.getDouble("integer", outDouble));
    Assert(EqualsDelta(outDouble, 32, 0.01)); //cast to double

    Assert(dt.getDouble("string", outDouble) == false);


}


[Test]
void DatatableTestFailedAccess()
{

    DataTable@ dt = DataTable::Create();
    
    dt.setInt("integer", 32);
    dt.setDouble("floating", 64.64);
    dt.setString("string", "a string indeed");

    int64 outInt;
    double outDouble;
    string outString;

    Assert(dt.getInt("someother", outInt) == false);
    Assert(dt.getString("someother", outString) == false);
    Assert(dt.getDouble("someother", outDouble) == false);

    Assert(dt.getTable("string") is null);
    Assert(dt.getTable("integer") is null);
    Assert(dt.getTable("someother") is null);
}

[Test]
void DatatablePath()
{
    DataTable@ dt = DataTable::Create();
    
    dt.newTable("table2");
    DataTable@ dt2 = dt.getTable("table2");
    
    dt2.setInt("int1", 1);
    dt2.newTable("table3");
    DataTable@ dt3 = dt2.getTable("table3");

    dt3.setInt("int2", 2);
    dt3.setString("str1", "1");
    dt3.setDouble("dbl1", 1.0);


    dt.setInt("table2/table3/int3", 3);

    int64 outInt;
    double outDouble;
    string outString;

    Assert(dt.getInt("table2/int1", outInt));
    Assert(outInt == 1);

    Assert(dt.getInt("table2/table3/int2", outInt));
    Assert(outInt == 2);

    Assert(dt.getInt("table2/table3/int3", outInt));
    Assert(outInt == 3);

    Assert(dt.getInt("table2/table_doesnt_exist/int3", outInt) == false);
    Assert(dt.getInt("table2/table/int_doesnt_exist", outInt) == false);

    Assert(dt2.getString("table3/str1", outString));
    Assert(outString == "1");

    Assert(dt2.getDouble("table3/dbl1", outDouble));
    Assert(EqualsDelta(outDouble, 1.0, 0.001)); 

    //Table getting is not supported via paths
    Assert(dt.getTable("table2/table3") is null);
    
    //Remove table2 from root
    dt.remove("table2");
    Assert(dt.getInt("table2/table3/int3", outInt) == false);
    
    Assert(dt2.getInt("table3/int3", outInt));
    Assert(outInt == 3);
}


[Test]
void DatatablePathFail()
{
    DataTable@ dt = DataTable::Create();
    
    dt.newTable("table2");
    DataTable@ dt2 = dt.getTable("table2");
    
    dt2.setInt("int1", 1);
    dt2.newTable("table3");
    DataTable@ dt3 = dt2.getTable("table3");

    dt3.setInt("int2", 2);
    dt3.setString("str1", "1");
    dt3.setDouble("dbl1", 1.0);


    AssertThrowsAfterThis();
    dt.setInt("table2/table2/int3", 3);
}

[Test]
void DatatableRemoveTwiceFail()
{
    DataTable@ dt = DataTable::Create();
    int64 outInt;
    
    dt.setInt("int1", 1);
    Assert(dt.getInt("int1", outInt));
    
    dt.remove("int1");
    Assert(dt.getInt("int1", outInt) == false);
    
    AssertThrowsAfterThis();
    dt.remove("int1");
}

class NestedObject
{
    int integer = 0;
    string str = "";
}

class SimpleSerializedObject
{
    int integer;
    string str;
    double floating;	
    NestedObject nestedDirect;
    NestedObject@ nestedHandle;
    string notset;

}


[Test]
void DatatableDeSerialize()
{
    DataTable@ dt = DataTable::Create();
    
    dt.newTable("nestedDirect");
    dt.newTable("nestedHandle");
    
    dt.setInt("nestedHandle/integer", 10);
    dt.setInt("nestedDirect/integer", 11);

    dt.setString("nestedHandle/str", "a string");
    dt.setString("nestedDirect/str", "another string");

    dt.setString("str", "just a string");
    dt.setInt("integer", 1);
    dt.setDouble("floating", 1.0);

    SimpleSerializedObject sso;

    Assert(DataTable::DeSerialize(dt, sso));

    Assert(sso !is null);

    Assert(sso.integer == 1);
    Assert(EqualsDelta(sso.floating, 1.0, 0.00001));
    Assert(sso.str == "just a string");

    Assert(sso.nestedDirect.integer == 11);
    Assert(sso.nestedDirect.str == "another string");

    //No deserialization for handles
    Assert(sso.nestedHandle is null);
}




[Test]
void DatatableDeSerializeHandle()
{
    DataTable@ dt = DataTable::Create();
    
    dt.newTable("nestedDirect");
    dt.newTable("nestedHandle");
    
    dt.setInt("nestedHandle/integer", 10);
    dt.setInt("nestedDirect/integer", 11);

    dt.setString("nestedHandle/str", "a string");
    dt.setString("nestedDirect/str", "another string");

    dt.setString("str", "just a string");
    dt.setInt("integer", 1);
    dt.setDouble("floating", 1.0);

    SimpleSerializedObject@ sso;

    Assert(DataTable::DeSerialize(dt, @sso));

    Assert(sso !is null);

    Assert(sso.integer == 1);
    Assert(EqualsDelta(sso.floating, 1.0, 0.00001));
    Assert(sso.str == "just a string");

    Assert(sso.nestedDirect.integer == 11);
    Assert(sso.nestedDirect.str == "another string");

    //No deserialization for handles
    Assert(sso.nestedHandle is null);
}


class ObjectWithArray
{
    SimpleSerializedObject[] array1;
    SimpleSerializedObject@[] array2;
    SimpleSerializedObject@[]@ array3;
    SimpleSerializedObject[]@ array4;
}


void FillWithSerializedObjects(DataTable@ dt)
{
    dt.newTable("0");
    dt.setInt("0/integer", 10);
    dt.newTable("1");
    dt.setInt("1/integer", 11);
    dt.newTable("2");
    dt.setInt("2/integer", 12);
}

[Test]
void DatatableDeSerializeArray()
{
    DataTable@ dt = DataTable::Create();
    
    dt.newTable("array1");
    dt.newTable("array2");
    dt.newTable("array3");
    dt.newTable("array4");

    FillWithSerializedObjects(dt.getTable("array1"));
    FillWithSerializedObjects(dt.getTable("array2"));
    FillWithSerializedObjects(dt.getTable("array3"));
    FillWithSerializedObjects(dt.getTable("array4"));
    
    
    ObjectWithArray owa;

    Assert(DataTable::DeSerialize(dt, owa));

    Assert(owa.array1.length() == 3);
    Assert(owa.array1[0].integer == 10);
    Assert(owa.array1[1].integer == 11);
    Assert(owa.array1[2].integer == 12);

    //No deserialization for handles
    Assert(owa.array2.length() == 0);
    Assert(owa.array3 is null);
    Assert(owa.array4 is null);

}

[Test]
void DatatableOverwrite()
{
    DataTable@ dt = DataTable::Create();
    

    int64 outInt;
    double outDouble;
    string outString;

    dt.setInt("int", 1);

    Assert(dt.getInt("int", outInt));
    Assert(outInt == 1);

    dt.setInt("int", 2);

    Assert(dt.getInt("int", outInt));
    Assert(outInt == 2);

    dt.setString("int", "not int");

    Assert(dt.getString("int", outString));
    Assert(outString == "not int");
    Assert(dt.getInt("int", outInt) == false);


    dt.newTable("table2");
    DataTable@ dt2 = dt.getTable("table2");

    dt.setInt("table2/int", 3);
    
    dt.setString("table2", "not table");

    Assert(dt.getString("table2", outString));
    Assert(outString == "not table");
    Assert(dt.getTable("table2") is null);

    Assert(dt2.getInt("int", outInt));
    Assert(outInt == 3);
}


[Test]
void DatatableClone()
{
    DataTable@ dt = DataTable::Create();
    
    dt.setInt("integer", 32);
    dt.newTable("subtable");
    dt.setInt("subtable/integer", 64);

    DataTable@ dt2 = dt.clone();

    int64 outInt;

    Assert(dt2.getInt("integer", outInt));
    Assert(outInt == 32);
    Assert(dt2.getInt("subtable/integer", outInt));
    Assert(outInt == 64);

    dt2.setInt("integer", 125);
    dt2.setInt("subtable/integer", 126);


    Assert(dt.getInt("integer", outInt));
    Assert(outInt == 32);
    Assert(dt.getInt("subtable/integer", outInt));
    Assert(outInt == 64);

    Assert(dt2.getInt("integer", outInt));
    Assert(outInt == 125);
    Assert(dt2.getInt("subtable/integer", outInt));
    Assert(outInt == 126);

}






}	

