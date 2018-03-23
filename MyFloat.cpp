#include "MyFloat.h"
#include <cmath>

MyFloat::MyFloat(){
  sign = 0;
  exponent = 0;
  mantissa = 0;
} //default constructor

MyFloat::MyFloat(float f){
  unpackFloat(f);
} //construtor

MyFloat::MyFloat(const MyFloat & rhs){
	sign = rhs.sign;
	exponent = rhs.exponent;
	mantissa = rhs.mantissa;
} //copy constructor

ostream& operator<<(std::ostream &strm, const MyFloat &f){
	strm << f.packFloat();
	return strm;
}//operator<<

bool MyFloat::operator==(const float rhs) const{
//if packed float is equal to the regular float
	if (this->packFloat() == rhs)
	{
		return true; //returns true
	} 

	else
	{
		return false; //otherwise returns false
	}
}//operator==


MyFloat MyFloat::operator+(const MyFloat& rhs) const{
	//makes a copy of the MyFloat values
	MyFloat new_float_left(*this);
	MyFloat new_float_right(rhs);
	MyFloat result_float(rhs); //creates a new MyFloat to store the result in

	int diff = 0; //creates a diffference variable to take in the difference between exponents

	// checks if the signs are different
	if (new_float_left.sign != new_float_right.sign)
	{
		new_float_right.sign = new_float_left.sign; //makes the right sign match the left
		result_float = new_float_left.operator-(new_float_right); //calls the minus operator
	}

	//otherwise signs are the same
	else 
	{
		//restores leading one of the mantissas
		new_float_left.mantissa += pow(2, 23);
		new_float_right.mantissa += pow(2, 23);

		result_float.sign = new_float_left.sign;

		//checks which exponent is greater
		//left is greater
		if (new_float_left.exponent > new_float_right.exponent)
		{
			diff = new_float_left.exponent - new_float_right.exponent; //finds difference between exponents
			new_float_right.mantissa = new_float_right.mantissa >> diff; //shifts the mantissa by the difference
			new_float_right.exponent = new_float_left.exponent; //sets the right exponent to be equal to the left exponent
			result_float.exponent = new_float_left.exponent; //sets the result exponent
		}

		//otherwise right is greater or equal
		else
		{
			diff = rhs.exponent - new_float_left.exponent; //finds difference between exponents
			new_float_left.mantissa = new_float_left.mantissa >> diff; //shifts the mantissa by the difference
			new_float_left.exponent = new_float_right.exponent; //sets the left exponent to be equal to the right exponent
			result_float.exponent = new_float_left.exponent; //sets the result exponent
		}

		//adds the left and right mantissa and stores it in result_float.mantissa
		result_float.mantissa = new_float_left.mantissa + new_float_right.mantissa;

		//checks if the 24th bit of the mantissa is 1 (most sig bit)
		if (((result_float.mantissa & 16777216) >> 24) == 1)
		{
			//normalizes the mantissa
			result_float.mantissa = result_float.mantissa >> 1;
			result_float.exponent += 1;
			result_float.mantissa -= pow(2, 23); //takes out the leading 1
		}

		//otherwise the 23rd bit is the most sig bit
		else
		{
			int mask = ((1 << 24) - 1); //creates a mask to check the 23rd bit
			result_float.mantissa = (result_float.mantissa & mask); //extracts 23 bits of the mantissa
			result_float.mantissa -= pow(2, 23); //takes out the leading 1
		}
	}
	return result_float;
}//operator+


MyFloat MyFloat::operator-(const MyFloat& rhs) const{
	//creates copies of the MyFloat values
	MyFloat new_float_left(*this);
	MyFloat new_float_right(rhs);
	MyFloat result_float(rhs); //creates a new MyFloat to store the result in

	int diff = 0; //creates a diffference variable to take in the difference between exponents

	bool borrow = false; //creates a bool to keep check of whether there is a borrowed bit when subtracting

	int firstbit = 23; //the starting position is at 23

	//swaps the sign of the second value (right)
	if (new_float_right.sign == 1)
	{
		new_float_right.sign = 0;
	}
	else if (new_float_right.sign == 0)
	{
		new_float_right.sign = 1;
	}

	//checks if the signs after the swap are equal
	//if equal then calls operator+
	if (new_float_right.sign == new_float_left.sign)
	{
		result_float = new_float_left.operator+(new_float_right);
	}

	//otherwise continues with subtraction
	else
	{
		//restores leading 1 of the mantissas
		new_float_left.mantissa += pow(2, 23);
		new_float_right.mantissa += pow(2, 23);

		//checks if the two MyFloats are the same number
		//if they are, then the result is just 0
		if ((new_float_right.exponent == new_float_left.exponent) && (new_float_right.mantissa == new_float_left.mantissa))
		{
			result_float = 0;
			return result_float;
		}

		//otherwise checks which exponent is greater
		//if the left is greater
		else if (new_float_left.exponent > new_float_right.exponent)
		{
			diff = new_float_left.exponent - new_float_right.exponent; //gets the difference between exponents
			//checks if the last bit shifted out is a 1
			if (((new_float_right.mantissa >> (diff-1) & 1) == 1))
				{ borrow = true;} //if it is, borrow is true

			new_float_right.mantissa = new_float_right.mantissa >> diff; //shifts the mantissa by the difference
			new_float_right.exponent = new_float_left.exponent; //sets the right exponent to the left
			result_float.exponent = new_float_left.exponent; //sets the result exponent
		}

		//same as above, except the right is greater or equal
		else
		{
			diff = rhs.exponent - new_float_left.exponent;
			if (((new_float_left.mantissa >> (diff-1) & 1) == 1))
				{ borrow = true;}
			new_float_left.mantissa = new_float_left.mantissa >> diff;
			new_float_left.exponent = new_float_right.exponent;
			result_float.exponent = new_float_right.exponent;
		}

		//checks which mantissa is greater
		//if the left is greater
		if (new_float_left.mantissa > new_float_right.mantissa)
		{
			result_float.mantissa = new_float_left.mantissa - new_float_right.mantissa; //substracts the right mantissa from the left and stores it in result
			result_float.sign = new_float_left.sign; //sets the result's sign equal to the left

			//checks the value of borrow
			if (borrow == true)
			{
				result_float.mantissa -= 1; //if borrow is true, substract 1 bit form the resulting mantissa
			}
		}

		//otherwise if the mantissa are equal
		else if (new_float_left.mantissa == new_float_right.mantissa)
		{
			result_float.mantissa = 0; //sets the result mantissa to 0
		}

		//otherwise the right is greater
		else
		{
			result_float.mantissa = new_float_right.mantissa - new_float_left.mantissa; //substracts the left mantissa from the right and stores it in result
			result_float.sign = new_float_right.sign; //sets the result's sign equal to the right
			
			//checks the value of borrow
			if (borrow == true)
			{
				result_float.mantissa -= 1; //if borrow is true, substract 1 bit form the resulting mantissa
			}
		}

		//for loop to normalize the mantissa
		for (int i = 23; i >= 0; i--)
		{
			if (((result_float.mantissa >> i) & 1) == 1) //checks location of the leading bit
			{
				firstbit = firstbit - i; //stores location in firstbit
				break; //if found, break;
			}
		}
		result_float.mantissa = result_float.mantissa << firstbit; //nomalizing mantissa by shifting left to get 1.mantissa
		result_float.exponent -= firstbit; //changes exponent accordingly
		result_float.mantissa -= pow(2, 23); //takes out the leading one
	}
	return result_float; 
}//operator-
    

void MyFloat::unpackFloat(float f) {
//C code simplified to extract bits
/*
int sign;
int exponent;
int mantissa;
int mask;
int float;

sign = number >> 31;
mask = (( 1 << 8) - 1) << 23;
exponent = (float & mask) >> 23;
exponent = exponent - 127;

mask = ((1 << 23) - 1) << 0;
mantissa = (float & mask) >> 0;
*/

__asm__(
//assembly code
	"movl %%eax, %%esi;"//moves float to variable we're gonna store sign in
	"shrl $31, %%esi;"//shift temp by 31, esi should now be either 1 or 0

	"movl $1, %%ecx;" //moves 1 into a register we can use
	"shll $8, %%ecx;" //does 1<<8
	"subl $1, %%ecx;" //(( 1 << 8) - 1)
	"shll $23, %%ecx;" //(( 1 << 8) - 1) << 23;
	"movl %%eax, %%edi;" //moves float to register we use to store exponent
	"andl %%ecx, %%edi;" //does float & mask
	"shrl $23, %%edi;" //float should now contain just the bits we want, shift left by 23
	"subl $127, %%edi;" //subtracts offset 127

	"movl %%eax, %%edx;"
	"shll $9, %%edx;" //does shifts number by 9
	"shrl $9, %%edx;" //shifts number back by 9
	//"movl %%eax, %%edx;"
	//"andl %%ecx, %%edx"
	:

//output
	"=S" (sign), "=D" (exponent), "=d" (mantissa):
//input
	"a" (f) : //stores f in register eax
//clobber list
	"cc"
); //__asm__


}//unpackFloat

float MyFloat::packFloat() const{
  //returns the floating point number represented by this
/* 
result_float.sign = result_float.sign << 31;
result_float.exponent = result_float.exponent + 127;
result_float.exponent << 23
result_float = result_float.sign + result_float.exponent + result_float.mantissa;
*/
  float f = 0;
	MyFloat result_float(*this);

	__asm__(
	//assembly code translating the above C code
		"movl $0, %%eax;" //moves 0 into float
		"shll $31, %%esi;" //shifts the sign by 31
		"addl $127, %%edi;" //subtracts the offset 127 from exponent
		"shll $23, %%edi;" //shifts it by 23
		"addl %%esi, %%eax;" //adds sign, exponent, and mantissa into float
		"addl %%edi, %%eax;"
		"addl %%edx, %%eax"
	:

	//output
		"=a" (f):
	//input
		"S" (result_float.sign), "D" (result_float.exponent), "d" (result_float.mantissa):
	//clobber list
		"cc"
	);

  return f;
}//packFloat
//
