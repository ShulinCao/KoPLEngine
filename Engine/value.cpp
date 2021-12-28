#include "value.h"


//'string' 'quantity' 'date' 'year'
unsigned short BaseValue::convertStringTypeToShort(const std::string &type_str, const json &val) {
    if (type_str == "string") {
        return string_type;
    }
    else if (type_str == "quantity") {
        if (val.is_number_float()) {
//            std::cout << "float!" << std::endl;
            return float_type;
        }
        else if (val.is_number_integer()) {
//            std::cout << "int!" << std::endl;
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

void BaseValue::parseValue(BaseValue *&value_ptr, const json &type_value_unit) {
    std::string value_type_in_string(type_value_unit.at("type"));
    auto value_val = type_value_unit.at("value");
    auto value_type = convertStringTypeToShort(value_type_in_string, value_val);

    std::string unit;
    if (value_type == int_type || value_type == float_type) {
        unit = type_value_unit.at("unit");
    }

    if (value_type == string_type) {
        auto val = value_val.get<std::string>();
        value_ptr = new StringValue(val, value_type);
    }
    else if (value_type == int_type) {
        auto val = value_val.get<int>();
        value_ptr = new QuantityValue(val, unit, value_type);
    }
    else if (value_type == float_type) {
        auto val = value_val.get<double>();
        value_ptr = new QuantityValue(val, unit, value_type);
    }
    else if (value_type == date_type) {
        auto val = value_val.get<std::string>();
        value_ptr = new DateValue(val, value_type);
    }
    else if (value_type == year_type) {
        auto val = value_val.get<short>();
        value_ptr = new YearValue(val, value_type);
    }
    else {
        std::cout << "Type Error!\n";
        exit(112);
    }
}

bool BaseValue::canCompare(const BaseValue* a, const BaseValue* b) {
    if (a -> type == string_type) {
        return b -> type == string_type;
    }
    else if (a -> type == int_type || a -> type == float_type) {
        return (b -> type == int_type || b -> type == float_type) && a -> getUnit() == b -> getUnit();
    }
    else {
        return (b -> type == year_type || b -> type == date_type);
    }
}

const std::string &BaseValue::getUnit() const {
    std::cout << "Calling from Non-Quantitive value\n";
    exit(1232);
}

bool BaseValue::valueContains(const BaseValue *another) const {
    std::cout << "valueContains is not implemented!\n";
    exit(1232);
}

bool StringValue::operator==(const StringValue &compare_value) const {
    return this -> value == compare_value.value;
}

bool StringValue::operator< (const StringValue &compare_value) const {
    return this -> value <  compare_value.value;
}

bool StringValue::operator> (const StringValue &compare_value) const {
    return this -> value >  compare_value.value;
}

bool StringValue::operator!=(const StringValue &compare_value) const {
    return this -> value != compare_value.value;
}

std::string StringValue::toPrintStr() const {
    return std::string{value};
}

bool StringValue::valueCompare(const StringValue &compare_value, const std::string & op) const {
    if (op == "==") {
        return *this == compare_value;
    }
    else if (op == "<") {
        return *this < compare_value;
    }
    else if (op == ">") {
        return *this > compare_value;
    }
    else if(op == "!=") {
        return *this != compare_value;
    }
    else {
        std::cout << "Undefined operator " << op << std::endl;
        exit(124);
    }
}


bool QuantityValue::operator==(const QuantityValue &compare_value) const {
    return this -> value == compare_value.value;
}

bool QuantityValue::operator< (const QuantityValue &compare_value) const {
    return this -> value <  compare_value.value;
}

bool QuantityValue::operator> (const QuantityValue &compare_value) const {
    return this -> value >  compare_value.value;
}

bool QuantityValue::operator!=(const QuantityValue &compare_value) const {
    return this -> value != compare_value.value;
}

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

bool QuantityValue::valueCompare(const QuantityValue & compare_value, const std::string & op) const {
    if (op == "==") {
        return *this == compare_value;
    }
    else if (op == "<") {
        return *this < compare_value;
    }
    else if (op == ">") {
        return *this > compare_value;
    }
    else if(op == "!=") {
        return *this != compare_value;
    }
    else {
        std::cout << "Undefined operator " << op << std::endl;
        exit(124);
    }
}

const std::string & QuantityValue::getUnit() const {
    return unit;
}


bool DateValue::operator==(const DateValue &compare_value) const {
    return (this -> year == compare_value.year && this -> month == compare_value.month && this -> day == compare_value.day);
}

bool DateValue::operator< (const DateValue &compare_value) const {
    return (
         this -> year <  compare_value.year ||
        (this -> year == compare_value.year && this -> month <  compare_value.month) ||
        (this -> year == compare_value.year && this -> month == compare_value.month && this -> day < compare_value.day)
    );
}

bool DateValue::operator> (const DateValue &compare_value) const {
    return (
         this -> year >  compare_value.year ||
        (this -> year == compare_value.year && this -> month >  compare_value.month) ||
        (this -> year == compare_value.year && this -> month == compare_value.month && this -> day > compare_value.day)
    );
}

bool DateValue::operator!=(const DateValue &compare_value) const {
    return !(*this == compare_value);
}

std::string DateValue::toPrintStr() const {
    char date_str[200];
    sprintf(date_str, "%d-%d-%d", year, month, day);
    return std::string{date_str};
}

bool DateValue::valueCompare(const DateValue &compare_value, const std::string &op) const {
    if (op == "==") {
        return *this == compare_value;
    }
    else if (op == "<") {
        return *this < compare_value;
    }
    else if (op == ">") {
        return *this > compare_value;
    }
    else if(op == "!=") {
        return *this != compare_value;
    }
    else {
        std::cout << "Undefined operator " << op << std::endl;
        exit(124);
    }
}


bool YearValue::operator==(const YearValue &compare_value) const {
    return this -> value == compare_value.value;
}

bool YearValue::operator< (const YearValue &compare_value) const {
    return this -> value <  compare_value.value;
}

bool YearValue::operator> (const YearValue &compare_value) const {
    return this -> value >  compare_value.value;
}

bool YearValue::operator!=(const YearValue &compare_value) const {
    return this -> value != compare_value.value;
}

std::string YearValue::toPrintStr() const {
    char year_str[10];
    sprintf(year_str, "%d", value);
    return std::string{year_str};
}

bool YearValue::valueCompare(const YearValue &compare_value, const std::string &op) const {
    if (op == "==") {
        return *this == compare_value;
    }
    else if (op == "<") {
        return *this < compare_value;
    }
    else if (op == ">") {
        return *this > compare_value;
    }
    else if(op == "!=") {
        return *this != compare_value;
    }
    else {
        std::cout << "Undefined operator " << op << std::endl;
        exit(124);
    }
}

bool YearValue::valueContains(const BaseValue *another) const {
    if (another -> type == year_type) {
        return *this == *((YearValue*)another);
    }
    else if (another -> type == date_type) {
        auto another_year_value = ((DateValue*)another) -> year;
        auto another_year = YearValue(another_year_value, year_type);
        return *this == another_year;
    }
    return false;
}

