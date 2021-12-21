#include "value.h"


//'string' 'quantity' 'date' 'year'
unsigned short BaseValue::convertStringTypeToShort(std::string & type_str, json & val) {
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

