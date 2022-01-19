#ifndef KOPL_VALUE_H
#define KOPL_VALUE_H

#include <string>
#include <iostream>
#include <cstdlib>
#include <nlohmann/json.hpp>
#include <utility>
#include <regex>
#include <cmath>

using json = nlohmann::json;


class CompareResult {
public:
    // not_sure for non-comparable
    // yew for comparable, and satisfy the comparison conditions
    // yes for comparable, and not satisfy the comparison conditions
    enum CompRes { no, yes, not_sure };
    CompRes compare_result;

    explicit CompareResult() : compare_result(CompRes::not_sure) {};
    explicit CompareResult(bool bool_result) {
        if (bool_result)        compare_result = CompRes::yes;
        else                    compare_result = CompRes::no;
    }
    bool toBool() const {
        if (compare_result == yes)      return true;            // true  for 'yes'
        else                            return false;           // false for 'no', and 'not_sure'
    }
};


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

    static unsigned short convertJsonStringTypeToShort(const std::string &type_str, const json &val);
    static void parseValue(std::shared_ptr<BaseValue> & value_ptr, const json & type_value_unit);
//    static std::shared_ptr<BaseValue> convertStringToValue(const std::string & value_in_string);
    static std::shared_ptr<BaseValue> convertStringToValue(const std::string & value_in_string, unsigned short type);

    template<typename SelfValueType, typename TargetValueType>
    static bool compareWithOperator(const BaseValue * self_value, const BaseValue * compare_value, const std::string & op);

    virtual std::string toPrintStr() const = 0;
//    virtual std::string toStandardStr() const = 0;

    virtual bool valueCompare(const BaseValue* compare_value, const std::string & op) const = 0;

    virtual ~BaseValue() = default;
};

class StringValue;
class QuantityValue;
class DateValue;
class YearValue;

class StringValue: public BaseValue {
public:
    std::string value;

    explicit StringValue(std::string val, unsigned short type = string_type): BaseValue(type), value(std::move(val)) {
        assert (type == string_type);
    };

    bool operator == (const StringValue & compare_value) const;
    bool operator <  (const StringValue & compare_value) const;
    bool operator >  (const StringValue & compare_value) const;
    bool operator != (const StringValue & compare_value) const;
    bool valueCompare(const BaseValue * compare_value, const std::string & op) const override;

    std::string toPrintStr() const override;
//    std::string toStandardStr() const override;
};


class QuantityValue: public BaseValue {
public:
    double value;
    std::string unit;

    explicit QuantityValue(double val, std::string u, unsigned short type) : BaseValue(type), value(val), unit(std::move(u)) {
        assert (type == float_type);
    }
    explicit QuantityValue(long long val, std::string u, unsigned short type) : BaseValue(type), value(val), unit(std::move(u)) {
        assert (type == int_type);
    }
    explicit QuantityValue(std::string val) : BaseValue(float_type) {
        auto end_of_number = val.size();
        for (std::size_t i = 0; i < val.size(); i++) {
            if (val[i] == ' ') {
                end_of_number = i;
                break;
            }
        }
        auto quantity_value_string = val.substr(0, end_of_number);
        value = std::stod(quantity_value_string);
        if (end_of_number < val.size()) {
            unit = val.substr(end_of_number + 1, val.size() - end_of_number - 1);
        }
        else {
            unit = std::string("1");
        }

    }

    bool operator == (const QuantityValue & compare_value) const;
    bool operator <  (const QuantityValue & compare_value) const;
    bool operator >  (const QuantityValue & compare_value) const;
    bool operator != (const QuantityValue & compare_value) const;

//    bool operator == (const YearValue & compare_value) const;
//    bool operator <  (const YearValue & compare_value) const;
//    bool operator >  (const YearValue & compare_value) const;
//    bool operator != (const YearValue & compare_value) const;


    bool valueCompare(const BaseValue * compare_value, const std::string & op) const override;

    int _floatPoint(double num) const;
    std::string toPrintStr() const override;
//    std::string toStandardStr() const override;

};


class DateValue: public BaseValue {
public:
    short year = -1;
    short month = -1;
    short day = -1;

    explicit DateValue(const std::string & val, unsigned short type = date_type) : BaseValue(type) {
        assert (type == date_type);

        size_t begin = 0;
        for (size_t i = 0; i < val.size(); i++) {
            if (val[i] == '/' || val[i] == '-' || i == val.size() - 1) {
                short cur_val;
                if (i != val.size() - 1) {
//                    cur_val = atoi(val.substr(begin, i - begin).c_str());
                    cur_val = (short)std::stoi(val.substr(begin, i - begin));
                }
                else {
//                    cur_val = atoi(val.substr(begin, val.size() - begin).c_str());
                    cur_val = (short)std::stoi(val.substr(begin, val.size() - begin));
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
    }

    bool operator == (const DateValue & compare_value) const;
    bool operator <  (const DateValue & compare_value) const;
    bool operator >  (const DateValue & compare_value) const;
    bool operator != (const DateValue & compare_value) const;

    bool operator == (const YearValue & compare_value) const;
    bool operator <  (const YearValue & compare_value) const;
    bool operator >  (const YearValue & compare_value) const;
    bool operator != (const YearValue & compare_value) const;

    bool valueCompare(const BaseValue * compare_value, const std::string & op) const override;

    std::string toPrintStr() const override;
//    std::string toStandardStr() const override;
};


class YearValue: public BaseValue {
public:
    long long value = -1;

    explicit YearValue(long long val, unsigned short type = year_type) : BaseValue(type), value(val) {
        assert (type == year_type);
    }
    explicit YearValue(const std::string & val) : BaseValue(year_type), value((long long)std::stoll(val)) {};

    bool operator == (const YearValue & compare_value) const;
    bool operator <  (const YearValue & compare_value) const;
    bool operator >  (const YearValue & compare_value) const;
    bool operator != (const YearValue & compare_value) const;

    bool operator == (const DateValue & compare_value) const;
    bool operator <  (const DateValue & compare_value) const;
    bool operator >  (const DateValue & compare_value) const;
    bool operator != (const DateValue & compare_value) const;

//    bool operator == (const QuantityValue & compare_value) const;
//    bool operator <  (const QuantityValue & compare_value) const;
//    bool operator >  (const QuantityValue & compare_value) const;
//    bool operator != (const QuantityValue & compare_value) const;

    bool valueCompare(const BaseValue * compare_value, const std::string & op) const override;

    std::string toPrintStr() const override;
//    std::string toStandardStr() const override;
};

#endif //KOPL_VALUE_H