namespace Format
{
    [Test]
    void EmptyFormatTest()
    {
        FormatString fs;
        Assert(fs.compile(""));
        
        FormatString::Formatter fsf;
        string output;
        int res = fsf.format(fs, output);
        Assert(res == 0);
        Assert(output == "");
    }

    [Test]
    void SimpleFormatNoArgumentsTest()
    {
        FormatString fs;
        Assert(fs.compile("%a %b %c"));
        
        FormatString::Formatter fsf;
        string output;
        int res;
        
        res = fsf.format(fs, output);
        Assert(res == -1);
        Assert(output == "");
        
        fs.compile("%a %b");
        res = fsf.format(fs, output);
        Assert(res == -1);
        Assert(output == "");
        
        fs.compile("%%%");
        res = fsf.format(fs, output);
        Assert(res == -1);
        Assert(output == "");
        
        fs.compile("%%");
        res = fsf.format(fs, output);
        Assert(res == 0);
        Assert(output == "%");
    }
    
    [Test]
    void SimpleFormattingTest()
    {
        FormatString fs;
        Assert(fs.compile("%s % %c"));
        
        FormatString::Formatter fsf;
        string output;
        int res;
        
        fsf.add("one");
        fsf.add(32);
        fsf.add(64.0);
        
        res = fsf.format(fs, output);
        Assert(res == 0);
        Assert(output == "ones 32 64c");
    }
    
    [Test]
    void MixedIndexTest()
    {
        FormatString fs;
        Assert(fs.compile("%$%2 %$%1%!"));
        
        FormatString::Formatter fsf;
        fsf.add(1);
        fsf.add(22);
        fsf.add(333);
        fsf.add(4444);
        
        
        string output;
        int ret;
        
        ret = fsf.format(fs, output);
        Assert(ret == 0);
        Assert(output == "1333 444422333!");
    }
    
    
    [Test]
    void FormatEscapesTest()
    {
        FormatString fs;
        auto res = fs.compile("%$%%2%%%$$");
        Assert(res);
        
        FormatString::Formatter fsf;
        fsf.add("yaourt");
        fsf.add("seven");
        
        string output;
        int ret;
        ret = fsf.format(fs, output);
        Assert(ret == 0);
        Assert(output == "yaourt%2%seven$");
        
        fsf.add("two");
        ret = fsf.format(fs, output);
        Assert(ret == -1);
    }
    
    
    [Test]
    void MultipleFormattingTest()
    {
        FormatString fs1, fs2;
        auto res = fs1.compile("Hello %0, it's %1.");
        Assert(res);
        
        res = fs2.compile("It's %1, %0.");
        Assert(res);
        
        FormatString::Formatter fsf1;
        fsf1.add("name");
        fsf1.add("time");
        
        FormatString::Formatter fsf2;
        fsf2.add("J.J%");
        fsf2.add(1200);
        
        string output;
        int ret;
        
        ret = fsf1.format(fs1, output);
        Assert(ret == 0);
        Assert(output == "Hello name, it's time.");
        
        
        ret = fsf2.format(fs1, output);
        Assert(ret == 0);
        Assert(output == "Hello J.J%, it's 1200.");
        
        ret = fsf1.format(fs2, output);
        Assert(ret == 0);
        Assert(output == "It's time, name.");
        
        ret = fsf2.format(fs2, output);
        Assert(ret == 0);
        Assert(output == "It's 1200, J.J%.");
    }


    
    [Test]
    void FormatterIndexedTest()
    {
        FormatString fs;
        auto res = fs.compile("%0$-%1$");
        Assert(res);
        
        FormatString::Formatter fsf;
        fsf.set("B", 1);
        fsf.set("A", 0);
        
        string output;
        int ret;
        
        ret = fsf.format(fs, output);
        Assert(ret == 0);
        Assert(output == "A-B");
    }
    
    
    [Test]
    void FormatterMixedTest()
    {
        FormatString fs;
        auto res = fs.compile("%0$-%1$-%2$");
        Assert(res);

        FormatString::Formatter fsf;
        fsf.set(2, 1);
        fsf.set(1, 0);
        fsf.add(3);
        

        string output;
        int ret;

        ret = fsf.format(fs, output);
        Assert(ret == 0);
        Assert(output == "1-2-3");
    }
    
    [Test]
    void ParentedFormattingTest()
    {
        FormatString fs;
        auto res = fs.compile("%0$-%1$-%2$");
        Assert(res);
        
        FormatString::Formatter parent;
        parent.set(3, 2);
        
        FormatString::Formatter fsf;
        fsf.set(2, 1);
        fsf.set(1, 0);
        

        string output;
        int ret;

        ret = fsf.formatWithParent(fs, parent, output);
        Assert(ret == 0);
        Assert(output == "1-2-3");
    }
    
    
    [Test]
    void ClearTest()
    {
        FormatString fs;
        auto res = fs.compile("%0$-%1$-%2$");
        Assert(res);
        
        FormatString::Formatter fsf;
        fsf.clear(2);
        fsf.set(2, 1);
        fsf.set(1, 0);
        fsf.clear(1);
        

        string output;
        int ret;

        ret = fsf.format(fs, output);
        Assert(ret == 0);
        Assert(output == "1--");
    }
    
    
    
    [Test]
    void ParentedFormattingIncompleteArgumentListTest()
    {
        FormatString fs;
        auto res = fs.compile("%$1%$2%$3%$4%$5%$6%$7%$8%$9%$0"); //10 arguments required
        Assert(res);
        
        //The parented formatter has only 9 arguments
        FormatString::Formatter parent;
        parent.clear(7);
        parent.add("Ninth argument");
        
        
        //Base formatter has full 10
        FormatString::Formatter fsf;     
        
        fsf.add("First");
        fsf.add("Second");
        fsf.add("Third");
        fsf.add("Fourth");
        fsf.add("Fifth");
        fsf.add("Sixth");
        fsf.add("Seventh");
        fsf.add("Eighth");
        fsf.add("Ninth");
        fsf.add("Tenth");

        string output;
        int ret;
        
        //Error returned: parent requires full 10
        ret = fsf.formatWithParent(fs, parent, output);
        Assert(ret == -2);
    }
    
    
    [Test]
    void ParentedFormattingEmptyTest()
    {
        FormatString fs;
        auto res = fs.compile("%$1%$2%$3%$4%$5%$6%$7%$8%$9%$0");
        Assert(res);
        
        FormatString::Formatter parent;
        parent.clear(9);
        
        FormatString::Formatter fsf;        

        string output;
        int ret;

        ret = fsf.formatWithParent(fs, parent, output);
        Assert(ret == 0);
        Assert(output == "1234567890");
    }
    
    
    
    
    [Test]
    void ParentedFormattingOverrideTest()
    {
        FormatString fs;
        auto res = fs.compile("%$1%$2%$3%$4%$5%$6%$7%$8%$9%$0");
        Assert(res);
        
        FormatString::Formatter parent;
        FormatString::Formatter fsf;
        
        //Indexes less than 5 will be set on the base formatter
        for (uint i = 0; i < 10; i++)
        {
            if (i < 5)
                fsf.set(i + 1, i);
            parent.set("Parent", i);
        }
        
        
        

        string output;
        int ret;

        ret = fsf.formatWithParent(fs, parent, output);
        Assert(ret == 0);
        Assert(output == "1122334455Parent6Parent7Parent8Parent9Parent0");
    }
    
}
