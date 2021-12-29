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

void BaseValue::parseValue(BaseValue* & value_ptr, const json & type_value_unit) {
    std::string value_type_in_string(type_value_unit.at("type"));
    auto value_val = type_value_unit.at("value");
    auto value_type = convertJsonStringTypeToShort(value_type_in_string, value_val);

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

BaseValue* BaseValue::convertStringToValue(const std::string & value_in_string, unsigned short value_type) {
    BaseValue* value_ptr = nullptr;
    if (value_type == string_type) {
        value_ptr = new StringValue(value_in_string, value_type);
    }
    else if (value_type == int_type || value_type == float_type) {
        auto end_of_number = value_in_string.size();
        for (std::size_t i = 0; i < value_in_string.size(); i++) {
            if (value_in_string[i] == ' ') {
                end_of_number = i;
                break;
            }
        }
        auto quantity_value_string = value_in_string.substr(0, end_of_number);
        auto quantity_unit_string = value_in_string.substr(end_of_number + 1, value_in_string.size() - end_of_number - 1);

        auto quantity_value = atof(value_in_string.c_str());
        value_ptr = new QuantityValue(quantity_value, quantity_unit_string, float_type);
    }
    else if (value_type == year_type) {
        auto year_value = (short)atoi(value_in_string.c_str());
        value_ptr = new YearValue(year_value, year_type);
    }
    else if (value_type == date_type) {
        value_ptr = new DateValue(value_in_string, date_type);
    }
    else {
        std::cout << "Type Error!\n";
        exit(125);
    }
    return value_ptr;
}

bool BaseValue::canCompare(const BaseValue* a, const BaseValue* b) {
    if (a -> type == string_type) {
        return b -> type == string_type;
    }
    else if (a -> type == int_type || a -> type == float_type) {
        return (b -> type == int_type || b -> type == float_type) && (a->_getUnit() == b->_getUnit());
    }
    else {
        return (b -> type == year_type || b -> type == date_type);
    }
}

const std::string & BaseValue::_getUnit() const {
    std::cout << "Calling from Non-Quantitive value\n";
    exit(1232);
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

bool StringValue::valueCompare(const BaseValue *compare_value, const std::string &op) const {

}

bool StringValue::valueCompare(const StringValue & compare_value, const std::string & op) const {
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


bool QuantityValue::operator==(const QuantityValue & compare_value) const {
    return this -> value == compare_value.value;
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

const std::string & QuantityValue::_getUnit() const {
    return unit;
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

bool DateValue::valueCompare(const DateValue & compare_value, const std::string &op) const {
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

std::string YearValue::toPrintStr() const {
    char year_str[10];
    sprintf(year_str, "%d", value);
    return std::string{year_str};
}

bool YearValue::valueCompare(const YearValue & compare_value, const std::string & op) const {
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
