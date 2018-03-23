#include "MyFloat.h"
#include <cmath>

MyFloat::MyFloat(){
  sign = 0;
  exponent = 0;
  mantissa = 0;
}

MyFloat::MyFloat(float f){
  unpackFloat(f);
}

MyFloat::MyFloat(const MyFloat & rhs){
	sign = rhs.sign;
	exponent = rhs.exponent;
	mantissa = rhs.mantissa;
}

void MyFloat::checkSign(MyFloat& lhs, MyFloat& rhs){
	if (lhs.sign == 1)
	{
		lhs.mantissa = lhs.mantissa * -1;
	}

	if (rhs.sign == 1)
	{
		rhs.mantissa = rhs.mantissa * -1;
	}
}


ostream& operator<<(std::ostream &strm, const MyFloat &f){
	strm << f.packFloat();
	return strm;

}

bool MyFloat::operator==(const float rhs) const{
	if (this->packFloat() == rhs)
	{
		return true;
	} 

	else
	{
		return false;
	}
}


MyFloat MyFloat::operator+(const MyFloat& rhs) const{
	this->mantissa += pow(2, 24);
	rhs.mantissa += pow(2, 24);

	int diff = 0;
	if (this->exponent > rhs.exponent)
	{
		diff = this->exponent - rhs.exponent;
		rhs.mantissa = rhs.mantissa >> diff;
		rhs.exponent = this->exponent;
		checkSign(*this, rhs);
	}

	else
	{
		diff = rhs.exponent - this->exponent;
		this->mantissa = this->mantissa >> diff;
		this->exponent = rhs.exponent;
		checkSign(*this, rhs);
	}

	this->mantissa += rhs.mantissa;

	if (this->mantissa < 0)
	{
		this->sign = 1;
	}

	else
	{
		this->sign = 0;
	}

	if ((this->mantissa & 33554432) == 1)
	{
		int mask = ((1 << 23) - 1) << 3;
		this->mantissa = (this->mantissa & mask) >> 3;
	}

	else
	{
		int mask = ((1 << 23) - 1) << 2;
		this->mantissa = (this->mantissa & mask) >> 2;
	}

	return *this;
}//operator+


MyFloat::MyFloat operator-(const MyFloat& rhs) const{
	rhs.mantissa = rhs.mantissa * -1;
	this->operator+(rhs);
}
    

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
	"movl %%eax, %%edi" //moves float to register we use to store exponent
	"andl %%ecx, %%edi;" //does float & mask
	"shrl $23, %%edi;" //float should now contain just the bits we want, shift left by 23
	"subl $127, %%edi;" //subtracts offset 127

	"movl $1, %%ecx;"
	"shll $23, %%ecx;" //does 1<<23
	"subl $1, %%ecx;" //(( 1 << 23) - 1)
	"movl %%eax, %%edx;"
	"andl %%ecx, %%edx;"




//output
	"=S" (sign), "=D" (exponent), "=d" (mantissa):
//input
	"a" (f) : //stores f in register eax
//clobber list
	"cc"
); //__asm__






void MyFloat::unpackFloat(float f) {
__asm__(
//assembly code
	"movl $0, %%ecx;" //int i = 0 
	"movl $0, %%ebx;" //int bit = 0
	"movl %%eax, %%edx;" // edx = temp = eax = f
	"for_loop:;" 
	"cmpl $31, %%ecx;"
	"jge end_for_loop;" 
	"shrl %%ecx, %%edx;"
	"and $1, %%edx;"
	"movl %%edx, %%ebx;" //bit = edx & 1 
	"cmpl $31, %%ecx;"
	"jz if;"
	"cmpl $23, %%ecx;"
	"jl end_for_loop;"
	"cmpl $30, %%ecx;"
	"jg end_for_loop;"
	"movl $-23, %%edx;" //j = -23
	"addl %%ecx, %%edx;" //j = j + i
	"shll %%edx, %%ebx;" //bit = bit << j = bit * pow(2, j)
	"addl %%ebx, %%edi;" //exp =+ bit
	"incl %%ecx;" //i++
	"movl %%eax, %%edx;"
	"jmp for_loop;" 

	"if:;"
	"movl %%ebx, %%esi;" //sign = bit

	"end_for_loop:;"
	"subl $127, %%edi;" //exp = exp - 127

	"movl $0, %%ecx;" //int i = 0 
	"movl %%eax, %%ebx;" //int bit = 0
	"for_mantissa:;" 
	"cmpl $23, %%ecx;"
	"jge end_for_mantissa;" 
	"shr %%ecx, %%ebx;"
	"and $1, %%ecx;"
	"shl %%edx, %%ecx;" //bit = bit << i = bit * pow(2, i)
	"addl %%ecx, %%edx;" //mantissa =+ bit
	"incl %%ecx;" //i++
	"movl %%eax, %%ebx;" //restore float f into ebx
	"jmp for_mantissa;"

	"end_for_mantissa:;"
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
  float f = 0;

/*
int dotplace;
int mantissa, exponent, sign;
int bit;
dotplace = 23 - exponent
float mfloat;
for (i = 0; i < 24; i++)
{
	bit = ((mantissa >> i)&1);
	mfloat = mfloat + bit * pow(2, i-dotplace);
}
if (this->sign == 1){
	mfloat *= -1;
}
*/

__asm__(
//assembly code
	"movl $0, %%ecx;" // ecx is our counter i
	"for:;"
	"movl $23, %%ebx;" // ebx is dotplace
	"subl %%edi, %%ebx;" // dotplace = dotplace - exponent
	"pushl %%esi;" //saves sign
	"movl %%edi, %%esi;" // bit = mantissa, esi = bit
	"cmpl $24, %%ecx;"
	"jge end_for;"
	"shrl %%ecx, %%esi;"
	"andl $1, %%esi;"
	"pushl %%ecx;" //saves counter
	"subl %%ebx, %%ecx;" //i - dotplace
	"shll %%ecx, %%esi;" //bit * pow(2, i-dotplace);
	"pop %%ecx;" //retores counter
	"addl %%esi, %%eax;"
	"pop %%esi;"
	"incl %%ecx;"
	"jmp for;"

	"end_for:;"

	"if:;"
	"cmpl $1, %%esi;"
	"jnz end_if;"
	"mull $-1;"

	"end_if:;"

//output
	"=a" (f):
//input
	"=S" (sign), "=D" (exponent), "=d" (mantissa): 
//clobber list
	"cc"
); //__asm__

  return f;
}//packFloat
//