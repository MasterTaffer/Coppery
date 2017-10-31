
namespace DataTableIO
{	

[Test]
void TestReadTOMLSimple()
{
    DataTable@ dt = DataTable::Load("assets/simple.toml");
    Assert(dt !is null);

    string outString;
    Assert(dt.getString("test_value", outString));
    Assert(outString == "success!!1");
}

//Refer to the test.toml at assets folder
[Test]
void TestReadTOMLComplex()
{
    DataTable@ dt = DataTable::Load("assets/test.toml");
    Assert(dt !is null);

    int64 outInt;
    double outDouble;
    string outString;

    Assert(dt.getString("v_string", outString));
    Assert(outString == "Toml test");

    Assert(dt.getInt("v_int", outInt));
    Assert(outInt == 3215665);

    Assert(dt.getInt("v_bool", outInt));
    Assert(outInt == 1);

    Assert(dt.getDouble("v_double", outDouble));
    Assert(EqualsDelta(outDouble, 64.01234, 0.001));

    DataTable@ st = dt.getTable("v_array1");

    Assert(st !is null);

    Assert(st.getInt("0", outInt));
    Assert(outInt == 2);
    Assert(st.getInt("1", outInt));
    Assert(outInt == 4);
    Assert(st.getInt("2", outInt));
    Assert(outInt == 8);
    Assert(st.getInt("3", outInt));
    Assert(outInt == 16);


    Assert(dt.getString("v_array2/0", outString));
    Assert(outString == "a");
    Assert(dt.getString("v_array2/1", outString));
    Assert(outString == "b");
    Assert(dt.getString("v_array2/2", outString));
    Assert(outString == "c");
    Assert(dt.getString("v_array2/3", outString));
    Assert(outString == "d");


    Assert(dt.getString("subtable/v_string", outString));
    Assert(outString == "test");
    Assert(dt.getString("subtable/subtable/v_string", outString));
    Assert(outString == "test2");

    @st = dt.getTable("arrayoftables");
    Assert(st !is null);

    Assert(dt.getString("arrayoftables/0/v_string", outString));
    Assert(outString == "4");
    Assert(dt.getString("arrayoftables/1/v_string", outString));
    Assert(outString == "5");
    Assert(dt.getString("arrayoftables/2/v_string", outString));
    Assert(outString == "6");



    Assert(dt.getString("v_str (hash-key)", outString));
    Assert(outString == "hash this pls");

    Assert(dt.getInt("v_str", outInt));
    Assert(hash_t(outInt) == Hash(outString));
}

[Test]
void TestReadTOMLIllegal()
{
    DataTable@ dt = DataTable::Load("assets/illegal.toml");
    Assert(dt is null);
}



//Refer to the test.xml at assets folder
[Test]
void TestReadXMLComplex()
{
    DataTable@ dt = DataTable::Load("assets/test.xml");
    Assert(dt !is null);

    int64 outInt;
    double outDouble;
    string outString;

    Assert(dt.getString("v_string", outString));
    Assert(outString == "Xml test");

    Assert(dt.getInt("v_int", outInt));
    Assert(outInt == 3215665);

    Assert(dt.getDouble("v_double", outDouble));
    Assert(EqualsDelta(outDouble, 64.01234, 0.001));

    DataTable@ st = dt.getTable("v_array1");

    Assert(st !is null);

    Assert(st.getInt("0", outInt));
    Assert(outInt == 2);
    Assert(st.getString("1", outString));
    Assert(outString == "4");
    Assert(st.getInt("2", outInt));
    Assert(outInt == 8);
    Assert(st.getString("3", outString));
    Assert(outString == "16");


    @st = dt.getTable("arrayoftables");
    Assert(st !is null);

    Assert(dt.getString("arrayoftables/0/v_string", outString));
    Assert(outString == "4");
    Assert(dt.getString("arrayoftables/1/v_string", outString));
    Assert(outString == "5");
    Assert(dt.getString("arrayoftables/2/v_string", outString));
    Assert(outString == "6");


    Assert(dt.getString("v_hash (hash-key)", outString));
    Assert(outString == "hash this pls");

    Assert(dt.getInt("v_hash", outInt));
    Assert(hash_t(outInt) == Hash("hash this pls"));
}


[Test]
void TestReadXMLIllegal()
{
    DataTable@ dt = DataTable::Load("assets/illegal.xml");
    Assert(dt is null);
}


}	

