#include "value.h"


//'string' 'quantity' 'date' 'year'
unsigned short BaseValue::convertJsonStringTypeToShort(const std::string & type_str, const json & val) {
    if (type_str == "string") {
        return string_type;
    }
    else if (type_str == "quantity") {
        if (val.is_number_float()) {
            return float_type;
        }
        else if (val.is_number_integer()) {
            return int_type;
        }
        else {
            std::cout << "Error!" << std::endl;
            exit(112);
        }
    }
    else if (type_str == "date") {
        return date_type;
    }
    else if (type_str == "year") {
        return year_type;
    }
    else {
        std::cout << "Error!" << std::endl;
        exit(112);
    }
}

void BaseValue::parseValue(std::shared_ptr<BaseValue> & value_ptr, const json & type_value_unit) {
    std::string value_type_in_string(type_value_unit.at("type"));
    auto value_val = type_value_unit.at("value");
    auto value_type = convertJsonStringTypeToShort(value_type_in_string, value_val);

    std::string unit;
    if (value_type == int_type || value_type == float_type) {
        unit = type_value_unit.at("unit");
    }

    if (value_type == string_type) {
        auto val = value_val.get<std::string>();
        value_ptr = std::make_shared<StringValue>(val, value_type);
    }
    else if (value_type == int_type) {
        auto val = value_val.get<int>();
        value_ptr = std::make_shared<QuantityValue>(val, unit, value_type);
    }
    else if (value_type == float_type) {
        auto val = value_val.get<double>();
        value_ptr = std::make_shared<QuantityValue>(val, unit, value_type);
    }
    else if (value_type == date_type) {
        auto val = value_val.get<std::string>();
        value_ptr = std::make_shared<DateValue>(val, value_type);
    }
    else if (value_type == year_type) {
        auto val = value_val.get<short>();
        value_ptr = std::make_shared<YearValue>(val, value_type);
    }
    else {
        std::cout << "Type Error!\n";
        exit(112);
    }
}

//std::shared_ptr<BaseValue> BaseValue::convertStringToValue(const std::string & value_in_string) {
//    std::shared_ptr<BaseValue> construct_value_ptr;
//    std::regex date_regex("[0-9]{3,4}-[0-9]{1,2}-[0-9]{1,2}");
//    if (std::regex_match(value_in_string, date_regex)) {
//        return std::make_shared<DateValue>(value_in_string, date_type);
//    }
//    else {
//        std::regex year_regex("[0-9]{3,4}");
//        if (std::regex_match(value_in_string, year_regex)) {
//            return std::make_shared<YearValue>(value_in_string);
//        }
//        else {
//            return std::make_shared<StringValue>(value_in_string);
//        }
//    }
//}


std::shared_ptr<BaseValue> BaseValue::convertStringToValue(const std::string & value_in_string, unsigned short type) {
    if (type == string_type) {
        return std::make_shared<StringValue>(value_in_string);
    }
    else if (type == int_type || type == float_type) {
        auto return_ptr = std::make_shared<QuantityValue>(value_in_string);
        return_ptr -> type = type;
        return return_ptr;
    }
    else if (type == date_type) {
        return std::make_shared<YearValue>(value_in_string);
    }
    else if (type == year_type) {
        return std::make_shared<StringValue>(value_in_string);
    }
}

template<typename SelfValueType, typename TargetValueType>
bool BaseValue::compareWithOperator (const BaseValue * self_value, const BaseValue * compare_value, const std::string & op) {
    if (op[0] == '=') {
        return *((SelfValueType*)self_value) == *((TargetValueType*)compare_value);
    }
    else if (op == "<") {
        return *((SelfValueType*)self_value) <  *((TargetValueType*)compare_value);
    }
    else if (op == ">") {
        return *((SelfValueType*)self_value) >  *((TargetValueType*)compare_value);
    }
    else if (op[0] == '!') {
        return *((SelfValueType*)self_value) != *((TargetValueType*)compare_value);
    }
    else {
        std::cout << "Undefined operator " << op << std::endl;
        exit(125);
    }
}

bool StringValue::operator==(const StringValue & compare_value) const {
    return this -> value == compare_value.value;
}

bool StringValue::operator< (const StringValue & compare_value) const {
    return this -> value <  compare_value.value;
}

bool StringValue::operator> (const StringValue & compare_value) const {
    return this -> value >  compare_value.value;
}

bool StringValue::operator!=(const StringValue & compare_value) const {
    return this -> value != compare_value.value;
}

std::string StringValue::toPrintStr() const {
    return std::string{value};
}

bool StringValue::valueCompare(const BaseValue * compare_value, const std::string & op) const {
    if (compare_value -> type == string_type) {
        return compareWithOperator<StringValue, StringValue>(this, compare_value, op);
    }
    else if (op[0] == '!') {
        return true;
    }
    return false;
}

bool QuantityValue::operator==(const QuantityValue & compare_value) const {
    return abs(this -> value - compare_value.value) < 1e-8;
}

bool QuantityValue::operator< (const QuantityValue & compare_value) const {
    return this -> value <  compare_value.value;
}

bool QuantityValue::operator> (const QuantityValue & compare_value) const {
    return this -> value >  compare_value.value;
}

bool QuantityValue::operator!=(const QuantityValue & compare_value) const {
    return this -> value != compare_value.value;
}

//bool QuantityValue::operator==(const YearValue & compare_value) const {
//    return abs(this -> value - compare_value.value) < 1e-8;
//}
//
//bool QuantityValue::operator< (const YearValue & compare_value) const {
//    return this -> value <  compare_value.value;
//}
//
//bool QuantityValue::operator> (const YearValue & compare_value) const {
//    return this -> value >  compare_value.value;
//}
//
//bool QuantityValue::operator!=(const YearValue & compare_value) const {
//    return this -> value != compare_value.value;
//}

std::string QuantityValue::toPrintStr() const {
    char quant_str[200];
    if (type == float_type) {
        sprintf(quant_str, "%.3f %s", value, unit.c_str());
    }
    else if (type == int_type) {
        sprintf(quant_str, "%d %s", int(value), unit.c_str());
    }

    return std::string{quant_str};
}

bool QuantityValue::valueCompare(const BaseValue * compare_value, const std::string & op) const {
    if ((compare_value -> type == int_type || compare_value -> type == float_type)
                                       && this -> unit == ((QuantityValue*)compare_value) -> unit) {
        return compareWithOperator<QuantityValue, QuantityValue>(this, compare_value, op);
    }
    else if (op[0] == '!') {
        return true;
    }
    return false;
}


bool DateValue::operator==(const DateValue & compare_value) const {
    return (this -> year == compare_value.year && this -> month == compare_value.month && this -> day == compare_value.day);
}

bool DateValue::operator< (const DateValue & compare_value) const {
    return (
         this -> year <  compare_value.year ||
        (this -> year == compare_value.year && this -> month <  compare_value.month) ||
        (this -> year == compare_value.year && this -> month == compare_value.month && this -> day < compare_value.day)
    );
}

bool DateValue::operator> (const DateValue & compare_value) const {
    return (
         this -> year >  compare_value.year ||
        (this -> year == compare_value.year && this -> month >  compare_value.month) ||
        (this -> year == compare_value.year && this -> month == compare_value.month && this -> day > compare_value.day)
    );
}

bool DateValue::operator!=(const DateValue & compare_value) const {
    return !(*this == compare_value);
}



std::string DateValue::toPrintStr() const {
    char date_str[200];
    sprintf(date_str, "%d-%d-%d", year, month, day);
    return std::string{date_str};
}

bool DateValue::operator==(const YearValue & compare_value) const {
    return false;
}

bool DateValue::operator< (const YearValue & compare_value) const {
    return year < compare_value.value;
}

bool DateValue::operator> (const YearValue & compare_value) const {
    return year > compare_value.value;
}

bool DateValue::operator!=(const YearValue & compare_value) const {
    return true;
}

bool DateValue::valueCompare(const BaseValue * compare_value, const std::string & op) const {
    if (compare_value -> type == date_type) {
        return compareWithOperator<DateValue, DateValue>(this, compare_value, op);
    }
    else if (compare_value -> type == year_type) {
        return compareWithOperator<DateValue, YearValue>(this, compare_value, op);
    }
    else if (op[0] == '!') {
        return true;
    }
    return false;
}



bool YearValue::operator==(const YearValue & compare_value) const {
    return this -> value == compare_value.value;
}

bool YearValue::operator< (const YearValue & compare_value) const {
    return this -> value <  compare_value.value;
}

bool YearValue::operator> (const YearValue & compare_value) const {
    return this -> value >  compare_value.value;
}

bool YearValue::operator!=(const YearValue & compare_value) const {
    return this -> value != compare_value.value;
}

//bool YearValue::operator==(const QuantityValue & compare_value) const {
//    return abs(this -> value - compare_value.value) < 1e-8;
//}
//
//bool YearValue::operator< (const QuantityValue & compare_value) const {
//    return this -> value <  compare_value.value;
//}
//
//bool YearValue::operator> (const QuantityValue & compare_value) const {
//    return this -> value >  compare_value.value;
//}
//
//bool YearValue::operator!=(const QuantityValue & compare_value) const {
//    return this -> value != compare_value.value;
//}

std::string YearValue::toPrintStr() const {
    char year_str[10];
    sprintf(year_str, "%d", value);
    return std::string{year_str};
}

bool YearValue::operator==(const DateValue & compare_value) const {
    return value == compare_value.year;
}

bool YearValue::operator<(const DateValue & compare_value) const {
    return value < compare_value.year;
}

bool YearValue::operator>(const DateValue & compare_value) const {
    return value > compare_value.year;
}

bool YearValue::operator!=(const DateValue & compare_value) const {
    return value != compare_value.year;
}

bool YearValue::valueCompare(const BaseValue * compare_value, const std::string & op) const {
    if (compare_value -> type == year_type) {
        return compareWithOperator<YearValue, YearValue>(this, compare_value, op);
    }
    else if (compare_value -> type == date_type) {
        return compareWithOperator<YearValue, DateValue>(this, compare_value, op);
    }
    else if (op[0] == '!') {
        return true;
    }
    return false;
}
