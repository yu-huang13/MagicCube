#include <cmath>
#include <assert.h>

#include "Vector3.h"

const double EPS = 1e-6;

Vector3::Vector3(double xx, double yy, double zz) : x(xx), y(yy), z(zz) {}
Vector3::Vector3(const Vector3 &v){
	x = v.x; y = v.y; z = v.z;
}

Vector3 operator - (const Vector3 &A){
	return Vector3(-A.x, -A.y, -A.z);
}

Vector3 operator + (const Vector3 &A, const Vector3 &B){
	return Vector3(A.x + B.x, A.y + B.y, A.z + B.z);
}
Vector3 operator - (const Vector3 &A, const Vector3 &B){
	return Vector3(A.x - B.x, A.y - B.y, A.z - B.z);
}
Vector3 operator * (const Vector3 &A, const double &num){
	return Vector3(A.x * num, A.y * num, A.z * num);
}
Vector3 operator / (const Vector3 &A, const double &num){
	return Vector3(A.x / num, A.y / num, A.z / num);
}
Vector3 operator * (const Vector3 &A, const Vector3 &B){
	return Vector3(A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
}

Vector3& operator += (Vector3 &A, const Vector3 &B){
	A = A + B;	
	return A;
}
Vector3& operator -= (Vector3 &A, const Vector3 &B){
	A = A - B;
	return A;
}
Vector3& operator *= (Vector3 &A, const double &num){
	A = A * num;
	return A;
}
Vector3& operator /= (Vector3 &A, const double &num){
	A = A / num;
	return A;
}
Vector3& operator *= (Vector3 &A, const Vector3 &B){
	A = A * B;
	return A;
}
double& Vector3::operator [] (int rank){
    switch(rank){
        case 0: return x; break;
        case 1: return y; break;
        case 2: return z; break;
    }
    assert(false);
    return x;
}

double Vector3::dot(const Vector3 &B){
	return x * B.x + y * B.y + z * B.z;
}
double Vector3::module(){
	return sqrt(x * x + y * y + z * z);
}
double Vector3::module2(){
	return x * x + y * y + z * z;
}
Vector3 Vector3::unitVector(){
	return (*this) / module();
}

bool Vector3::isZero(){
	return fabs(x) < EPS && fabs(y) < EPS && fabs(z) < EPS;
}
Vector3 Vector3::reflect(const Vector3 &N){
	return N * dot(N) * 2 - (*this);
}
bool Vector3::refract(const Vector3 &N, const double n, Vector3& ref){//N为法向量
	Vector3 V = unitVector();
	double cosH1 = V.dot(N);
	double temp = 1 - (1 - cosH1 * cosH1) / (n * n);
	if (temp > EPS){
		double cosH2 = sqrt(temp);
		ref = V * ((-1) / (n)) - N * (cosH2 - cosH1 / n);
		return true;
	}
	else
		return false;
}

bool operator == (const Vector3 &A, const Vector3 &B){
	return fabs(A.x - B.x) < EPS && fabs(A.y - B.y) < EPS && fabs(A.z - B.z) < EPS;
}

ostream& operator << (ostream& output, Vector3 &A){
	output << "(" << A.x << ", " << A.y << ", " << A.z << ")" << endl;
	return output;
}
