#pragma once
#include <string>
#include <stdexcept>
#include <vector>
#include <cstdint>
#include <unordered_map>


//! Simple data serialization functions and structures
namespace Dataser
{
    //! Integer type used in Dataser
    typedef int64_t Integer_t;
    //! Floating point type used in Dataser
    typedef double Float_t;
    //! String type used in Dataser
    typedef std::string String_t;

    //C++ 17 has std::optional
    
    //! \brief Structure representing value types with possibility for null
    template <typename T>
    struct Optional
    {
        bool isNull;
        T value;

        //! Constructor from value 
        Optional(const T& val)
        {
            isNull = false;
            value = val;
        }

        //! Constructor with null initializer
        Optional()
        {
            isNull = true;
        }
        
        /*! Access the optional value
         * 
         * The Optional accessed must be non-null, otherwise an
         * std::logic_error exception is thrown.
         */
        T access()
        {
            if (isNull)
                throw std::logic_error("Accessing null Optional");
            return value;
        }
    };

    //! Different possible value types for Value
    enum ValueType
    {
        Null,
        Integer,
        Double,
        String,
        Array,
        TableRef,
        Undef
    };

    /*! \brief Structure represtenting a single piece of serialized data
     * 
     * The different possible types are found in ValueType enumeration.
     * Typically a failure or absence of data is represented by type
     * #ValueType Null.
     *
     * If the value is Null, it shall not be assigned to.
     */
    struct Value
    {
        //! The type of the value stored
        ValueType type;

        //! Integer value stored
        int64_t integerValue;
        
        //! Floating point value stored
        double doubleValue;

        //! String value stored
        std::string stringValue;

        //! Default contructor with type Null
        Value()
        {
            type = Null;
        }

        //! Deafult constructor with explicit type
        explicit Value(ValueType t)
        {
            type = t;
        }
        
        //! Returns if the Value is of type Null
        bool isNull()
        {
			return (type == Null);
        }

        //! Try retrieving integer value stored
        Optional<int64_t> getInteger() const
        {
            if (type == Integer)
                return Optional<int64_t>(integerValue);
            return Optional<int64_t>();
        }

        //! Try retrieving floating point value stored
        Optional<double> getDouble() const
        {
            if (type == Double)
                return Optional<double>(doubleValue);
            if (type == Integer)
                return Optional<double>(integerValue);
            return Optional<double>();
        }

        //! Try retrieving string value stored
        Optional<std::string> getString() const
        {
            if (type == String)
                return Optional<std::string>(stringValue);
            return Optional<std::string>();
        }

        //! Set the value
        void set(double d)
        {
            if (type == Null)
                throw std::logic_error("Attempt to assign into a null value");
            type = Double;
            doubleValue = d;
        }

        //! Set the value
        void set(int64_t v)
        {
            if (type == Null)
                throw std::logic_error("Attempt to assign into a null value");
            type = Integer;
            integerValue = v;
        }

        //! Set the value
        void set(const std::string& v)
        {
            if (type == Null)
                throw std::logic_error("Attempt to assign into a null value");
            type = String;
            stringValue = v;
        }

        /*! Sets the value to null
         * 
         * After calling this, the value is effectively removed. A null value
         * may never be written or read from.
         */
		void remove()
		{
			type = Null;
		}


        int tableId;
    };
    
    /*! \brief An associative array of serialized values
     * 
     * The BasicTable associative array may contain multiple key value pairs.
     * The keys are always unique while the values are not, and any attempt
     * to insert a new duplicate key will cause an exception.
     * 
     * \tparam Key the type used as the associative array key
     */
    template <typename Key>
    class BasicTable
    {
        Value nullValue;
    public:
        
        //! List of sub-tables
        std::vector<BasicTable*> subTables;
        
        //! Associative array of all key-value pairs
        std::unordered_map<Key, Value> values;

        //! Constructor
        BasicTable()
        {
            nullValue.type = Null;
        }

        /*! \brief Creates and inserts a new value with key
         * 
         * Creates a new value with key \p k. The new value is initialized
         * to type #ValueType Undef. A reference to the value is returned
         * which can be used to write information to it.
         * 
         * If the key is a duplicate key, an exception is thrown.
         */
        Value& insert(const Key& k)
        {
            auto it = values.find(k);
            if (it != values.end() && (!it->second.isNull()))
            {
                throw std::logic_error("Attempt to insert with a duplicate key");
            }
            return values[k] = Value(Undef);
        }
        
        /*! \brief Creates and inserts a new table with key
         * 
         * Creates a new sub-table and inserts it. The pointer to the new
         * table is returned, while the ownership stays in the table.
         * 
         * If the key is a duplicate key, an exception is thrown.
         */
        BasicTable* newTable(const Key& k)
        {
            auto it = values.find(k);
            if (it != values.end() && (!it->second.isNull()))
            {
                throw std::logic_error("Attempt to insert with a duplicate key");
            }

            Value tm;
            tm.type = TableRef;
            tm.tableId = subTables.size();

            values[k] = tm;

            subTables.push_back(new BasicTable);
            return subTables[tm.tableId];
        }
        
        /*! \brief Insert a sub-table into the table
         * 
         * The ownership of the sub-table is transferred to the table. No
         * clone is performed. Typically one would call newTable method
         * instead for new tables, instead of creating a new table manually.
         * 
         * If the key \p k is already in the table, an exception will be
         * thrown.
         */
		void insertTable(const Key& k, BasicTable* b)
        {
            auto it = values.find(k);
            if (it != values.end() && (!it->second.isNull()))
            {
                throw std::logic_error("Attempt to insert with a duplicate key");
            }

            Value tm;
            tm.type = TableRef;
            tm.tableId = subTables.size();

            values[k] = tm;

            subTables.push_back(b);
        }

        /*! \brief Get a sub-table by key
         * 
         * Returns a sub-table found by key \p k. If the value doesn't exist
         * or the value is not a sub-table, method will return a nullptr.
         * 
         */
        BasicTable* getTable(const Key& k)
        {
            Value& v = get(k);
            if (v.type != TableRef)
                return nullptr;

            return subTables.at(v.tableId);
        }

        /*! \brief Fetches or creates a missing value based on key
         * 
         * If the table has a value with key \p k, this method retuns a
         * reference to that key. Otherwise the value is inserted into the
         * table. 
         */
		Value& getOrInsert(const Key& k)
		{
			auto it = values.find(k);
			if (it == values.end())
			{
				return insert(k);
			}
			
            if (it->second.type == Null)
                it->second.type = Undef;
			return it->second;
		}
		
		/*! \brief Fetches a value matching for the key
         * 
         * The method will perform a table lookup for the key
         * \p k and return the found value. If no value is found, a null
         * value is returned.
         */

        Value& get(const Key& k)
        {
            auto it = values.find(k);
            if (it == values.end())
            {
                return nullValue;
            }
            return it->second;
        }

        /*! \brief Fetches a value using an argument list resource path
         * 
         * Similar to get(const std::vector<Key>& keys) except the resource
         * path is provided in method call via separate arguments.
         */
        template <typename ... keys>
        Value& get(const Key& first, keys ... rest)
        {
            auto t = getTable(first);
            if (t)
                return t->get(rest...);
            return nullValue;
        }

        /*! \brief Fetches a value using a resource path and an index
         * 
         * Works exactly like get(const std::vector<Key>& keys) ignores the
         * first \p it elements of the \p keys array.
         */
		Value& get(const std::vector<Key>& keys, size_t it)
		{
			if (it == keys.size() - 1)
				return get(keys[it]);

			auto t = getTable(keys[it]);
			if (t)
				return t->get(keys, it + 1);
			return nullValue;
		}
        
        /*! \brief Fetches a value using a resource path
         * 
         * The resource path is simply an array of keys. Except for the
         * last key, the \p keys must point to other tables. Starting from the
         * first key a new table is retrieved, and that new table is used
         * to query the next key.
         * 
         * If \p keys has only one element, the call is equivalent to calling
         * get(const Key& first) with said element.
         * 
         * \return The value pointed by the path, or a null value on error
         */
		Value& get(const std::vector<Key>& keys)
		{
			if (keys.size() == 0)
				return nullValue;
			return get(keys, 0);
		}

        //! Destructor. Frees all subtables as well.
        ~BasicTable()
        {
            for (auto* t : subTables)
                delete t;
        }

        /*! \brief Construct and return a deep copy of the table
         * 
         * The method constructs a new BasicTable and gives away the
         * ownership to the caller: the caller is responsible for freeing
         * the BasicTable.
         */
        BasicTable* clone()
        {
            BasicTable* n = new BasicTable;

            //Copies the whole datastructure
            n->values = values;

            //Copy this as well
            n->subTables = subTables;


            for (BasicTable*& t : n->subTables)
            {
                //replace all the entries in subTables with new clones
                t = t->clone();
            }

            return n;
        }
    };



    typedef BasicTable<std::string> Table;

    //! Generic interface for all types of serialized data
    template <typename Key>
    class BasicReadInterface
    {
    public:

        typedef Key Key_t;

        //! Virtual destructor
        virtual ~BasicReadInterface()
        {
        }

        //! Get operation
        virtual bool getInteger(const Key_t& k, Integer_t& v)
        {
            return false;
        }

        //! Get operation
        virtual bool getFloat(const Key_t& k, Float_t& v)
        {
            return false;
        }

        //! Get operation
        virtual bool getString(const Key_t& k, String_t& v)
        {
            return false;
        }

        //! Type get operation
        virtual ValueType getType(const Key_t&k)
        {
            return Null;
        }

        //! Key iteration
        virtual bool getKey(size_t index, Key_t& k)
        {
            return false;
        }


        /*! \brief Get operation.

            Caller should delete the interface returned.
        */
        virtual BasicReadInterface* getTable(const Key_t& k)
        {
            return nullptr;
        }
    };

    typedef BasicReadInterface<std::string> ReadInterface;

    //!ReadInterface for Dataser::Table
    class TableReadInterface : public ReadInterface
    {
    protected:
        Table* table;
    public:

        /*! \brief Constructor from Dataser table
        
            table is passed by reference: caller should guarantee that the
            table will not be destructed before this object, or any of the
            subtables built from this interface.
        */
        TableReadInterface(Table* table)
        : table(table)
        {
        }


        //! Virtual destructor
        virtual ~TableReadInterface()
        {
        }

        virtual bool getInteger(const Key_t& k, Integer_t& v)
        {
            auto opt = table->get(k).getInteger();
            if (opt.isNull)
                return false;
            v = opt.value;
            return true;
        }

        virtual bool getFloat(const Key_t& k, Float_t& v)
        {
            auto opt = table->get(k).getDouble();
            if (opt.isNull)
                return false;
            v = opt.value;
            return true;
        }

        virtual bool getString(const Key_t& k, String_t& v)
        {

            auto val = table->get(k);
            if (val.type == String)
            {
                v = val.stringValue;
                return true;
            }
            
            return false;
        }

        virtual bool getKey(size_t index, Key_t& k)
        {
            if (index >= table->values.size())
                return false;


            auto it = table->values.begin();
            for (size_t i = 0; i < index; i++)
                it++;

            k = it->first;
            return true;
        }

        virtual ValueType getType(const Key_t&k)
        {
            auto val = table->get(k);
            return val.type;
        }

        virtual BasicReadInterface* getTable(const Key_t& k)
        {
            auto* t = table->getTable(k);
            if (t)
                return new TableReadInterface(t);
            return nullptr;
        }
    };



    //! TableReadInterface that maintains a copy of the Dataser::Table
    class MaintainedTableReadInterface : public TableReadInterface
    {
    protected:
        struct TableState
        {
            int refCount;
            Table* baseTable;
        };
        TableState* state;

        MaintainedTableReadInterface(Table* table, TableState* state)
        : TableReadInterface(table), state(state)
        {
            state->refCount += 1;
        }

    public:

        /*! \brief Constructor from Dataser table

            Ownership of \p table is transferred. The table should also point
            to the root of the hierarchy. If one wishes to pass a table
            that isn't at the root of the hierarchy, one should call
            table->clone() and pass that instead.
        */
        MaintainedTableReadInterface(Table* table)
        : TableReadInterface(table)
        {
            state = new TableState();
            state->refCount = 1;
            state->baseTable = table;
        }


        //! Virtual destructor
        virtual ~MaintainedTableReadInterface()
        {
            state->refCount -= 1;

            //Is this the final table referencing the table hierarchy?
            if (state->refCount == 0)
            {
                delete state->baseTable;
                delete state;
            }
        }

        virtual BasicReadInterface* getTable(const Key_t& k)
        {
            auto* t = table->getTable(k);
            if (t)
            {
                return new MaintainedTableReadInterface(t, state);
            }
            return nullptr;
        }
    };


    //! Structure containing either table or error
    struct TableOrError
    {
        //! Pointer to the table or nullptr on error
        Table* table = nullptr;
        //! Error message if \a table is nullptr
        std::string errorMessage;
    };

    //! Load a table from XML
    TableOrError ReadXMLFromMemory(char* mem);

    //! Write a table with a output stream in XML format
    void WriteXMLToStream(Table* t, std::ostream& os);

    //! Load a table from TOML
    TableOrError ReadTOMLFromStream(std::istream& stream);

}


#ifdef DATASER_SERIALIZATION_IMPLEMENTATION

#include "hash.hpp"
#include <sstream>
#include <rapidxml.hpp>
#include <rapidxml_print.hpp>
#include <cpptoml.h>

namespace Dataser
{
    constexpr static const char* hashPostfix = " (hash-key)";
    constexpr static const int hashPostfixLen = 11;

    static Table* ParseTableFromXML(rapidxml::xml_node<>* node)
    {
        Table* table = new Table();
        auto * fnode = node->first_node();
        unsigned int unnamed = 0;
        while (fnode)
        {
            auto next = [&](){
                fnode = fnode->next_sibling();
            };
            std::string name = fnode->name();
            auto * akey = fnode->first_attribute("key");
            std::string key;
            if (!akey)
            {
                key = std::to_string(unnamed);
                
                unnamed++;
            }
            else
                key = akey->value();
            if (name == "table")
            {
                Table* subtable = ParseTableFromXML(fnode);
                if (subtable)
                {
                    try
                    {
                        table->insertTable(key, subtable);
                    }
                    catch (std::exception&)
                    {
                        delete subtable;
                    }
                }
                next();
                continue;
            }

            auto * avalue = fnode->first_attribute("value");
            if (!avalue)
            {
                next();
                continue;
            }
            std::string value = avalue->value();
            std::stringstream conv;
            conv << value;

            try
            {
                if (name == "int")
                {
                    int64_t v;
                    conv >> v;
                    table->insert(key).set(v);
                }
                else
                if (name == "double")
                {
                    double v;
                    conv >> v;
                    table->insert(key).set(v);
                }
                else
                if (name == "string")
                {
                    table->insert(key).set(value);
                }
                else
                if (name == "hstring")
                {
                    table->insert(key).set((int64_t)Hash::dyn_hash_string(value));
                    table->insert(key + hashPostfix).set(value);
                }
            }
            catch (std::exception& e)
            {
            }
            next();
        }
        return table;
    }

    static void AppendToXML(Table* t, rapidxml::xml_document<>* doc, rapidxml::xml_node<>* tnode)
    {
        for (auto& p: t->values)
        {
            const char* name = "";
            std::stringstream val;

            auto& v = p.second;
            switch (v.type)
            {
            case Integer:
                name = "int";
                val << v.getInteger().access();
                break;
            case Double:
                name = "double";
                val << v.getDouble().access();
                break;
            case String:
                name = "string";
                val << v.getString().access();
                break;
            case TableRef:
                name = "table";
                break;
            default:
                continue;
            }
            std::string vastr = "";
            if (v.type != TableRef)
            {
                vastr = val.str();
            }
            auto pair = doc->allocate_node(rapidxml::node_element, name);
            tnode->append_node(pair);
            rapidxml::xml_attribute<> *attr = doc->allocate_attribute("key", doc->allocate_string(p.first.c_str()));
            pair->append_attribute(attr);

            if (v.type == TableRef)
            {
                Table* b = t->subTables.at(v.tableId);
                AppendToXML(b, doc, pair);
            }
            else
            {
                rapidxml::xml_attribute<> *attr3 = doc->allocate_attribute("value", doc->allocate_string(vastr.c_str()));
                pair->append_attribute(attr3);

            }
        }
    }
    
    void WriteXMLToStream(Table* t, std::ostream& os)
    {
        rapidxml::xml_document<char> doc;

        auto tnode = doc.allocate_node(rapidxml::node_element, "table");
        doc.append_node(tnode);
        AppendToXML(t, &doc, tnode);
        os << doc;
    }


    TableOrError ReadXMLFromMemory(char* mem, const std::string& name)
    {
        TableOrError ret;

        rapidxml::xml_document<char> doc;
        try
        {
            doc.parse<0>(mem);
        }
        catch (rapidxml::parse_error ex)
        {
            const char* cm = ex.where<char>();
            
            ret.errorMessage = std::string(cm) +": " +ex.what() ;
            return ret;
        }

        auto rootnode = doc.first_node("table");
        if (rootnode)
        {
            ret.table = ParseTableFromXML(rootnode);
        }
        else
        {
            ret.errorMessage = "No root node";
        }
        return ret;
    }

    static Table* ParseTableFromTOML(std::shared_ptr<cpptoml::table> node);

    static void AddValueFromTOML(Table* table, std::string& name, std::shared_ptr<cpptoml::base> val)
    {
        if (!val)
            return;

        if (auto v = val->as<int64_t>())
        {
            table->insert(name).set(v->get());
            return;
        }

        if (auto v = val->as<bool>())
        {
            table->insert(name).set((int64_t)(v->get()));
            return;
        }
        if (auto v = val->as<double>())
        {
            table->insert(name).set(v->get());
            return;
        }

        if (auto v = val->as<std::string>())
        {
            auto& strval = v->get();
            table->insert(name).set(strval);


            if (name.size() > hashPostfixLen
                && name.find(hashPostfix, name.size() - hashPostfixLen) == name.size() - hashPostfixLen)
            {
                table->insert(name.substr(0, name.size() - hashPostfixLen))
                    .set(static_cast<Integer_t>(Hash::dyn_hash_string(strval)));
            }
            return;
        }
        if (val->is_table())
        {
            Table* subtable = ParseTableFromTOML(val->as_table());
            if (subtable)
            {
                try
                {
                    table->insertTable(name, subtable);
                }
                catch (std::exception&)
                {
                    delete subtable;
                }
            }
            return;
        }

        if (val->is_array())
        {
            auto v = val->as_array();
            if (v)
            {
                Table* subtable = new Table();
                try
                {
                    int array_int_key = 0;
                    std::string array_string_key;
                    for (auto& aval : *v)
                    {
                        array_string_key = std::to_string(array_int_key);
                        AddValueFromTOML(subtable, array_string_key, aval);
                        array_int_key += 1;
                    }
                    table->insertTable(name, subtable);
                }
                catch (std::exception&)
                {
                    delete subtable;
                }
                return;
            }
        }

        if (val->is_table_array())
        {
            auto v = val->as_table_array();
            if (v)
            {
                Table* subtable = new Table();
                try
                {
                    int array_int_key = 0;
                    for (auto& aval : *v)
                    {
                        std::string array_string_key = std::to_string(array_int_key);
                        Table* subsubtable = ParseTableFromTOML(aval);
                        if (subsubtable)
                        {
                            subtable->insertTable(array_string_key, subsubtable);
                        }
                        array_int_key += 1;
                    }
                    table->insertTable(name, subtable);
                }
                catch (std::exception&)
                {
                    delete subtable;
                }
                return;
            }
        }
    }

    static Table* ParseTableFromTOML(std::shared_ptr<cpptoml::table> node)
    {
        Table* table = new Table();

        for (auto& base : *node)
        {
            std::string name = base.first;

            AddValueFromTOML(table, name, base.second);
        }
        return table;
    }

    TableOrError ReadTOMLFromStream(std::istream& stream)
    {
        TableOrError ret;

        cpptoml::parser parser(stream);


        try
        {
            auto t = parser.parse();
            ret.table = ParseTableFromTOML(t);
        }
        catch (std::exception& ex)
        {
            ret.errorMessage = +ex.what();
            return ret;
        }
 
        return ret;
    }


}

#endif



