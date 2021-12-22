#ifndef KOPL_VALUE_H
#define KOPL_VALUE_H

#include <string>
#include <iostream>
#include <cstdlib>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

class BaseValue {
public:
    // 0 -> base class, 1 -> string, 2 -> int, 3 -> float, 4 -> date, 5 -> year
    const static unsigned short base_type   = 0;
    const static unsigned short string_type = 1;
    const static unsigned short int_type    = 2;
    const static unsigned short float_type  = 3;
    const static unsigned short date_type   = 4;
    const static unsigned short year_type   = 5;

    // 0 -> base class, 1 -> string, 2 -> int, 3 -> float, 4 -> date, 5 -> year
    unsigned short type;
    explicit BaseValue(unsigned short type = base_type) : type(type) {};
    static unsigned short convertStringTypeToShort(const std::string &type_str, const json &val);
    static void parseValue(BaseValue* & value_ptr, const json & type_value_unit);
};


class StringValue: public BaseValue {
public:
    std::string value;

    explicit StringValue(std::string & val, unsigned short type = string_type): BaseValue(type), value(val) {
        assert (type == string_type);
    };

    bool operator == (const StringValue & compare_value) const;
    bool operator <  (const StringValue & compare_value) const;
    bool operator >  (const StringValue & compare_value) const;
    bool operator != (const StringValue & compare_value) const;
};


class QuantityValue: public BaseValue {
public:
    double value;
    std::string unit;

    explicit QuantityValue(double val, std::string & u, unsigned short type) : BaseValue(type), value(val), unit(u) {
        assert (type == float_type);
    }
    explicit QuantityValue(int val, std::string & u, unsigned short type) : BaseValue(type), value(val), unit(u) {
        assert (type == int_type);
    }

    bool operator == (const QuantityValue & compare_value) const;
    bool operator <  (const QuantityValue & compare_value) const;
    bool operator >  (const QuantityValue & compare_value) const;
    bool operator != (const QuantityValue & compare_value) const;
};


class DateValue: public BaseValue {
public:
    short year = -1;
    short month = -1;
    short day = -1;

    explicit DateValue(std::string & val, unsigned short type = date_type) : BaseValue(type) {
        assert (type == date_type && val.length() == 10);

        size_t begin = 0;
        for (size_t i = 0; i < val.size(); i++) {
            if (val[i] == '/' || i == val.size() - 1) {
                short cur_val;
                if (i != val.size() - 1) {
                    cur_val = atoi(val.substr(begin, i - begin).c_str());
                }
                else {
                    cur_val = atoi(val.substr(begin, val.size() - begin).c_str());
                }

                if (year == -1) {
                    year = cur_val;
                }
                else if (month == -1) {
                    month = cur_val;
                }
                else if (day == -1) {
                    day = cur_val;
                }
                begin = i + 1;
            }
        }

//        std::cout << val << " " << year << " " << month << " " << day << std::endl;
//        year  = atoi(val.substr(0, 4).c_str());
//        month = atoi(val.substr(5, 2).c_str());
//        day   = atoi(val.substr(8, 2).c_str());
    }

    bool operator == (const DateValue & compare_value) const;
    bool operator <  (const DateValue & compare_value) const;
    bool operator >  (const DateValue & compare_value) const;
    bool operator != (const DateValue & compare_value) const;
};


class YearValue: public BaseValue {
public:
    short value = -1;

    explicit YearValue(short val, unsigned short type = year_type) : BaseValue(type), value(val) {
        assert (type == year_type);
    }

    bool operator == (const YearValue & compare_value) const;
    bool operator <  (const YearValue & compare_value) const;
    bool operator >  (const YearValue & compare_value) const;
    bool operator != (const YearValue & compare_value) const;
};

#endif //KOPL_VALUE_H
