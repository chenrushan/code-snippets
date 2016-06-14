// from http://www.learncpp.com/cpp-tutorial/97-overloading-the-increment-and-decrement-operators/
#include <iostream>

class Digit
{
private:
    int digit_;
public:
    Digit(int digit=0)
        : digit_(digit)
    {
    }
 
    Digit& operator++(); // prefix
    Digit& operator--(); // prefix
 
    Digit operator++(int); // postfix
    Digit operator--(int); // postfix
 
    friend std::ostream& operator<< (std::ostream &out, const Digit &d);
};
 
Digit& Digit::operator++()
{
    // If our number is already at 9, wrap around to 0
    if (digit_ == 9)
        digit_ = 0;
    // otherwise just increment to next number
    else
        ++digit_;
 
    return *this;
}
 
Digit& Digit::operator--()
{
    // If our number is already at 0, wrap around to 9
    if (digit_ == 0)
        digit_ = 9;
    // otherwise just decrement to next number
    else
        --digit_;
 
    return *this;
}
 
Digit Digit::operator++(int)
{
    // Create a temporary variable with our current digit
    Digit temp(digit_);
 
    // Use prefix operator to increment this digit
    ++(*this); // apply operator
 
    // return temporary result
    return temp; // return saved state
}
 
Digit Digit::operator--(int)
{
    // Create a temporary variable with our current digit
    Digit temp(digit_);
 
    // Use prefix operator to decrement this digit
    --(*this); // apply operator
 
    // return temporary result
    return temp; // return saved state
}
 
std::ostream& operator<< (std::ostream &out, const Digit &d)
{
    out << d.digit_;
    return out;
}
 
int main()
{
    Digit digit(5);
 
    std::cout << digit;
    std::cout << ++digit; // calls Digit::operator++();
    std::cout << digit++; // calls Digit::operator++(int);
    std::cout << digit;
    std::cout << --digit; // calls Digit::operator--();
    std::cout << digit--; // calls Digit::operator--(int);
    std::cout << digit;
 
    return 0;
}
